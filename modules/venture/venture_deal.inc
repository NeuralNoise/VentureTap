<?php

/**
* Deal edit form after_build event handler
*/
function venture_deal_after_build(&$form, $form_values) {
  // Blank out the state field if a country other than US is chosen
  if ($form_values['field_deal_country']['key'] != 'United States') {
    form_set_value($form['field_deal_state'], NULL);
  }
  
  // Blank out capital raised and current valuation fields if no prior investors
  $investors = $form_values['field_deal_prior_investors']['keys'];
  if (count($investors) == 1 && in_array('None', $investors)) {
    form_set_value($form['field_deal_capital_raised'][0], NULL);
    form_set_value($form['field_deal_current_valuation'][0], NULL);
  }
  
  return $form;
}

/**
* Alter the deal node form
*/
function venture_deal_alter(&$form) {
  // Content type editing page passes the same form_id as the content
  // editing form. The cck_dummy_node_form allows to distinguish the two.
  if (!$form['#node']->cck_dummy_node_form) {
    // Check if the user is trying to delete the deal
    if ($_POST['op'] == 'Delete Deal') {
      venture_deal_delete($form['#node']->nid);
    }
    
    $form['#after_build'][] = 'venture_deal_after_build'; 
    $form['#pre_render'][] = 'venture_deal_pre_render';
    $form['#submit'] = array('venture_deal_submit' => array());
    $form['submit']['#value'] = 'Save Changes';
    
    if ($form['delete']) {
      $form['delete']['#value'] = 'Delete Deal';
    }
    
    $form['field_deal_state']['#validate'] = array('venture_deal_validate_state' => array());
    $form['field_deal_contact_email']['#validate'] = array('venture_deal_validate_email' => array());
    $form['field_deal_industry']['key']['#required'] = TRUE; // Required field with a blank default
    
    // Remove formatting instructions
    unset($form['field_deal_company_overview'][0]['format']);
    unset($form['field_deal_product_service'][0]['format']);
    unset($form['field_deal_management_team'][0]['format']);
    unset($form['field_deal_details'][0]['format']);
    
    // Set the recipient user/group
    if (arg(0) == 'profile') {
      $form['field_deal_users'][0]['value']['#value'] = arg(1);
      $form['field_deal_country']['#validate'] = array('venture_deal_validate_country' => array());
    }
    else if (arg(0) == 'group') {
      $form['field_deal_groups'][0]['value']['#value'] = arg(1);
    }
    
    // Clean up the attachments section
    venture_deal_attachments_alter($form);
    
    // Make sure fields are ordered properly within the form
    $fields = array(
      'title', 'field_deal_country', 'field_deal_state', 'field_deal_city', 'field_deal_year_founded',
      'field_deal_employees', 'field_deal_legal_entity', 'field_deal_website', 'field_deal_contact_name',
      'field_deal_contact_email', 'field_deal_contact_phone', 'field_deal_industry', 'field_deal_funding_stage',
      'field_deal_prior_investors', 'field_deal_capital_raised', 'field_deal_current_valuation',
      'field_deal_annual_revenue', 'field_deal_amount_seeking', 'field_deal_funding_type',
      'field_deal_headline', 'field_deal_company_overview', 'field_deal_product_service',
      'field_deal_management_team', 'field_deal_details', 'field_deal_attachments'
    );
    venture_order_fields($form, $fields);
  }
}

/**
* Alter the uploaded files form
*/
function venture_deal_attachments_alter(&$form) {
  venture_attachments_alter($form, 'field_deal_attachments', 'deal');
}

/**
* Delete the deal
*/
function venture_deal_delete($nid) {
  $deal = node_load($nid);
  $title = check_plain($deal->title);
  node_delete($nid);
  
  // Fix up the error message wording
  if ($_SESSION['messages']['status']) {
    $messages = &$_SESSION['messages']['status'];
    $messages = str_replace("<em>$title</em>", "The $title deal", $messages);
  }

  drupal_goto('deals');
}

/**
* Get number of deals for current user
*/
function venture_deal_get_count() {
  global $user;
  $query = "SELECT COUNT(*) FROM {node} WHERE type = 'deal' AND uid = %d";
  $result = db_result(db_query($query, $user->uid));
  return $result;
}

/**
* Get number of deals for each group
*/
function venture_deal_get_group_counts($gids) {
  $placeholders = implode(',', array_fill(0, count($gids), "'%s'"));
  $query = "SELECT field_deal_groups_value AS gid, COUNT(nid) AS deal_count
    FROM {content_field_deal_groups} WHERE field_deal_groups_value IN ($placeholders) GROUP BY field_deal_groups_value";
  $result = db_query($query, $gids);
  
  while ($group = db_fetch_object($result)) {
    $counts[$group->gid] = $group->deal_count;
  }
  
  return $counts;
}

/**
* Get deal name, respecting non-accredited users
*/
function venture_deal_get_node_title($deal) {
  $title = check_plain($deal->title);
  $country = $deal->field_deal_country[0]['value'];
  $state = $deal->field_deal_state[0]['value'];
  return venture_deal_get_title($deal->uid, $title, $country, $state);
}

/**
* Get deal name, respecting non-accredited users
*/
function venture_deal_get_title($uid, $title, $country, $state) {
  $accessible = venture_deal_is_accessible($uid, $country);
  if (!$accessible) {
    $title = "$state firm";
  }
  return $title;
}

/**
* Determine if the deal is accessible by user
*/
function venture_deal_is_accessible($deal_uid, $deal_country) {
  global $user;
  return $deal_uid == $user->uid || venture_profile_is_accredited() || $deal_country != 'United States';
}

/**
* Determine if the deal is accessible by user
*/
function venture_deal_is_node_accessible($deal) {
  return venture_deal_is_accessible($deal->uid, $deal->field_deal_country[0]['value']);
}

/**
* Add deal-related menu items
*/
function venture_deal_menu($may_cache) {
  global $user;
  $items = array();
  $access = user_access('access content');
  
  if ($may_cache) {
    $items[] = array(
      'path' => 'deals',
      'callback' => 'theme',
      'callback arguments' => array('deals'),
      'access' => $access,
      'type' => MENU_CALLBACK
    );
    $items[] = array(
      'path' => "deal/new",
      'callback' => 'node_add',
      'callback arguments' => array('deal'),
      'access' => $access,
      'type' => MENU_CALLBACK
    );
  }
  else {    
    if (arg(0) == 'deal' && is_numeric(arg(1)) && $deal = node_load(arg(1))) {
      if ($deal->type == 'deal') {
        $items[] = array(
          'path' => "deal/{$deal->nid}",
          'callback' => 'theme',
          'callback arguments' => array('deal_view'),
          'access' => $access,
          'type' => MENU_CALLBACK
        );
        $items[] = array(
          'path'  => "deal/{$deal->nid}/edit",
          'callback' => 'node_page_edit',
          'callback arguments' => array($deal),
          'access' => node_access('update', $deal),
          'type' => MENU_CALLBACK
        );
        $items[] = array(
          'path' => "deal/{$deal->nid}/remove",
          'callback' => 'venture_deal_remove_handler',
          'access' => $access,
          'type' => MENU_CALLBACK
        );
      }
    }
  }
  
  return $items;
}

/**
* Get list of grants for the group node
*/
function venture_deal_node_access_records($node) {
  $grants = array();
  $private = FALSE;

  if ($node->field_deal_users) {
    foreach($node->field_deal_users as $user) {
      $private = $private || $user['value'];
      if ($user['value'] > 0) {
        $grants[] = array(
          'realm' => 'venture_deal_user',
          'gid' => $user['value'],
          'grant_view' => TRUE,
          'grant_update' => FALSE,
          'grant_delete' => FALSE,
          'priority' => 0
        );
      }
    }
  }
  
  if ($node->field_deal_groups) {
    foreach($node->field_deal_groups as $group) {
      $private = $private || $group['value'];
      if ($group['value'] > 0) {
        $grants[] = array(
          'realm' => 'venture_deal_group',
          'gid' => $group['value'],
          'grant_view' => TRUE,
          'grant_update' => FALSE,
          'grant_delete' => FALSE,
          'priority' => 0
        );
      }
    }
  }
  
  if ($private) {
    // Give full rights to deal creator
    $grants[] = array(
        'realm' => 'venture_deal_user',
        'gid' => $node->uid,
        'grant_view' => TRUE,
        'grant_update' => TRUE,
        'grant_delete' => TRUE,
        'priority' => 0
      );
  }
  
  return $grants;
}

/**
* Implementation of hook_node_access_records(). 
*/
function venture_deal_node_grants($account, $op) {
  $grants = array();
  $grants['venture_deal_user'] = array($account->uid);
  $grants['venture_deal_group'] = array_keys($account->og_groups);
  
  return $grants;
}

/**
* Deal edit form pre_render event handler
*/
function venture_deal_pre_render($form_id, &$form) {
  $form['field_deal_state']['key']['#required'] = TRUE;
  venture_format_funding_stages($form['field_deal_funding_stage']['key']);
  
  // Hide the recipient user/group fields
  unset($form['field_deal_users']);
  unset($form['field_deal_groups']);
}

/**
* Handler for deal remove action
*/
function venture_deal_remove_handler() {
  global $user;
  
  $deal = node_load(arg(1));
  $title = venture_deal_get_node_title($deal);
  
  if (!arg(3)) {
    foreach($deal->field_deal_users as &$to) {
      if ($to['value'] == $user->uid) {
        drupal_set_message("$title private deal has been successfully removed.");
        $to['value'] = $to['value'] * -1;
        node_save($deal);
        return;
      }
    }
  }
  else if (is_numeric(arg(3))) {
    foreach($user->og_groups as $gid => $group) {
      foreach($deal->field_deal_groups as &$to) {
        if ($to['value'] == $gid && arg(3) == $gid) {
          $group_name = check_plain($group['title']);
          drupal_set_message("$title group deal has been successfully removed from $group_name.");
          $to['value'] = $to['value'] * -1;
          node_save($deal);
          return;
        }
      }
    }
  }
  drupal_set_message("Unable to remove the $title deal.", 'error');
}

/**
* Override the default submit method (node_form_submit)
*/
function venture_deal_submit($form_id, $form_values) {
  // Save the deal
  $deal = node_submit($form_values);
  $action = $deal->nid ? 'updated' : 'created';
  node_save($deal);
  
  // Remove directory creation message(s)
  if ($messages = &$_SESSION['messages']['status']) {
    foreach ($messages as $idx => $message) {
      if (strpos($message, 'directory') !== FALSE) {
        unset($messages[$idx]);
      }
    }
    // Unset messes with array index counter - reset indices
    $messages = array_values($messages);
  }
  
  // Show a message and log
  $title = check_plain($deal->title);
  watchdog('content', "Deal: $action $title", WATCHDOG_NOTICE, l(t('view'), 'deal/'. $deal->nid));
  drupal_set_message("The $title deal has been $action.");
  
  return 'deal/'. $deal->nid;
}

/**
* Return available tokens for deal activity
*/
function venture_deal_token_values($type, $data) {
  $values = array();
  if ($type == 'nodeactivity' && !empty($data) && $data['node-type'] == 'deal') {
    $values['node-link'] = l($data['node-title'], 'deal/' . $data['node-id']);
  }
  return $values;
}

/**
* Validate the country field for user deals
*/
function venture_deal_validate_country($element) {
  global $form_values;
  if ($form_values['field_deal_country']['key'] == 'United States') {
    if ($profile = venture_profile_retrieve(arg(1))) {
      if (!venture_profile_is_accredited($profile)) {
        form_set_error('field_deal_country', 'Country field is invalid. United States deals may not be submitted to a non-accredited investor.');
      }
    }
  }
}

/**
* Validate email field
*/
function venture_deal_validate_email($element) {
  global $form_values;
  $email = $form_values['field_deal_contact_email'][0]['value'];
  if ($email && !valid_email_address($email)) {
    form_set_error('field_deal_contact_email', 'Contact Email field is not a valid email address.');
  }
}

/**
* Validate the state field
*/
function venture_deal_validate_state($element) {
  global $form_values;
  if ($form_values['field_deal_country']['key'] == 'United States') {
    if (!$form_values['field_deal_state']['key']) {
      form_set_error('field_deal_state', 'State field is required.');
    }
  }
}

/**
* Alter the deal filters
*/
function venture_deal_views_filters_alter(&$form) {
  if (venture_profile_is_accredited()) {
    unset($form['filter11']);
  }
  else {
    $form['filter11']['#type'] = 'checkbox';
  }
}

/**
* Handler to only select deals outside of the US
*/
function venture_deal_views_handler_not_united_states($op, $filter, $filterinfo, &$query) {
  if ($filter['value'] == 1) {
    $table = $filterinfo['table'];
    $field = $filterinfo['field'];
    $query->add_where("$table.$field != 'United States'");
  }
}

/**
* Add an accredited deal filter
*/
function venture_deal_views_tables_alter(&$table_data) {
  $table_data['node_data_field_deal_country']['filters']['field_deal_country_value_not_united_states'] = array(
    'name' => t('Venture: Hide non-accessible deals'),
    'field' => 'field_deal_country_value',
    'operator' => array('!=' => 'Is Not'),
    'value' => array(
      '#type' => 'select',
      '#options' => array(
        1 => t('United States'),
      ),
    ),
    'handler' => 'venture_deal_views_handler_not_united_states',
    'help' => t('Hide deals not viewable by non-accredited investors.')
  );
}

?>