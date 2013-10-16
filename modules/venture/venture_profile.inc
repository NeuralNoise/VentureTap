<?php

/**
* Alter the account profile form
*/
function venture_profile_account_alter(&$form) {
  // Content type editing page passes the same form_id as the content
  // editing form. The cck_dummy_node_form allows to distinguish the two.
  if (!$form['#node']->cck_dummy_node_form) {
    $form['#validate']['venture_profile_account_validate'] = array();
    $form['#pre_render'][] = 'venture_profile_account_pre_render';
    $form['#submit'] = array('venture_profile_account_submit' => array());
    $form['submit']['#value'] = 'Save Changes';
    unset($form['title']);
    
    // Hide qualification info
    unset($form['field_qualification']);
    
    // Remove Delete button for non-admins
    if (!venture_profile_is_admin()) {
      unset($form['delete']);
    }
    
    // Remove formatting instructions
    unset($form['field_about_me'][0]['format']);
    unset($form['field_industry_experience'][0]['format']);
    unset($form['field_investing_experience'][0]['format']);
    unset($form['field_guidelines'][0]['format']);
       
    // Add user info to the profile
    $user_info = user_edit();
    $form['mail'] = $user_info['account']['mail'];
    $form['timezone'] = $user_info['timezone']['timezone'];
    $form['pass'] = $user_info['account']['pass'];
    $form['_category'] = $user_info['_category'];
    $form['_account'] = $user_info['_account'];
    $form['mail']['#title'] = 'Email';
    $form['timezone']['#title'] = 'Time Zone';
    unset($form['mail']['#description']);
    unset($form['pass']['#description']);
    unset($form['timezone']['#description']);
    
    // Add Current Password field
    $form['current_pass'] = array(
      '#type' => 'password', 
      '#title' => 'Current Password',
      '#size' => 25
    );
    
    // Make sure fields are ordered properly within the form
    $fields = array(
      'field_organization', 'field_title', 'field_country', 'field_state', 'field_city', 'field_phone',
      'field_website', 'field_investor_type', 'field_funding_stages', 'field_industries',
      'field_investment_preferences', 'field_other_preferences', 'field_countries', 'field_states',
      'field_minimum_investment', 'field_maximum_investment', 'field_guidelines', 'field_about_me',
      'field_industry_experience', 'field_investing_experience', 'field_photo', 'field_first_name',
      'field_last_name', 'mail', 'field_email_alerts', 'timezone', 'pass', 'current_pass'
    );
    venture_order_fields($form, $fields);
    
    
    venture_profile_photo_alter($form);
    venture_profile_shared_alter($form);
  }
}

/**
* Account form pre_render event handler
*/
function venture_profile_account_pre_render($form_id, &$form) {
  $form['pass']['pass1']['#title'] = 'New Password';
  $form['pass']['pass2']['#title'] = 'Confirm Password';
  
  // Hide geocoded location in pre-render in order to be able to clear it in after_build
  unset($form['field_location']);
}

/**
* Override the default submit method (node_form_submit)
*/
function venture_profile_account_submit($form_id, $form_values) {
  // Save the profile
  $node = node_submit($form_values);
  node_save($node);
  
  // Save the user
  $form_values = venture_profile_get_form_values($form_values);
  user_edit_submit($form_id, $form_values);
}

/**
* Validate the user info
*/
function venture_profile_account_validate($form_id, $form_values, $form) {
  global $user;

  // Validate the correct fields
  $valid_form_values = venture_profile_get_form_values($form_values);
  user_edit_validate($form_values['form_id'], $valid_form_values);
  
  if (!venture_profile_is_admin($user)) {
    // Require current password for user fields
    $match_fields = array(
      array('old' => $form['#node']->field_first_name[0]['value'], 'new' => $form_values['field_first_name'][0]['value']),
      array('old' => $form['#node']->field_last_name[0]['value'], 'new' => $form_values['field_last_name'][0]['value']),
      array('old' => $user->mail, 'new' => $form_values['mail']),
      array('old' => $form['#node']->field_email_alerts['default keys'], 
            'new' => array_keys(array_filter($form_values['field_email_alerts']['keys']))),
      array('old' => $user->timezone, 'new' => $form_values['timezone']),
      array('old' => '', 'new' => $form_values['pass'])
    );
    foreach($match_fields as $match) {
      if ($match['old'] != $match['new']) {
        if (!$form_values['current_pass']) {
          form_set_error('current_pass', 'Current Password field is required when making changes to the Advanced section.');
        }
        else if (md5($form_values['current_pass']) != $user->pass) {
          form_set_error('current_pass', 'Invalid current password provided.');
        }
        else {
          // Log name and email changes
          $name_old = $match_fields[0]['old'] . ' ' . $match_fields[1]['old'];
          $name_new = $match_fields[0]['new'] . ' ' . $match_fields[1]['new'];
          if ($name_old != $name_new) {
            watchdog('venture', "User changed name from $name_old to $name_new", WATCHDOG_NOTICE);
          }
          if ($user->mail != $form_values['mail']) {
            watchdog('venture', "User changed email from {$user->mail} to {$form_values['mail']}", WATCHDOG_NOTICE);
          }
        }
        break;
      }
    }
  }
}

/**
* Comparison function for profile sorting
*/
function venture_profile_compare($uid1, $uid2) {
  $profile1 = venture_profile_retrieve($uid1);
  $profile2 = venture_profile_retrieve($uid2);
  
  $first1 = strtolower($profile1->field_first_name[0]['value']);
  $first2 = strtolower($profile2->field_first_name[0]['value']);
  $last1 = strtolower($profile1->field_last_name[0]['value']);
  $last2 = strtolower($profile2->field_last_name[0]['value']);
  
  if ($last1 > $last2) {
    return 1;
  }
  else if ($last1 < $last2) {
    return -1;
  }
  else if ($first1 > $first2) {
    return 1;
  }
  else if ($first1 < $first2) {
    return -1;
  }
  else {
    return 0;
  }
}

/**
* Create a dummy profile object
*/
function venture_profile_create($uid, $first_name, $last_name) {
  $profile->uid = $uid;
  $profile->field_first_name[0]['value'] = $first_name;
  $profile->field_last_name[0]['value'] = $last_name;
  return $profile;
}

/**
* Return form values relevant to the user object only
*/
function venture_profile_get_form_values($form_values) {
  return array(
    '_category' => $form_values['_category'],
    '_account' => $form_values['_account'],
    'mail' => $form_values['mail'],
    'timezone' => $form_values['timezone'],
    'pass' => $form_values['pass'],
    'name' => $form_values['_account']->name // Necessary for admin users
  );
}

/**
* Accepts either the uid or the profile node and returns the user's name
*/
function venture_profile_get_name($object, $format = TRUE) {
  global $user;
  
  $profile = (is_numeric($object) ? venture_profile_retrieve($object) : $object);
  $first_name = $profile->field_first_name[0]['value'];
  $last_name = $profile->field_last_name[0]['value'];
  
  $name = "$first_name $last_name";
  return $format ? check_plain($name) : $name;
}

/**
* Return sorted array of users
*/
function venture_profile_get_sorted_users($uids) {
  $users = array();
  
  usort($uids, 'venture_profile_compare');
  foreach($uids as $uid) {
    $users[] = user_load(array('uid' => $uid));
  }
  
  return $users;
}

/**
* Handle imagefield events
*/
function venture_profile_imagefield($op, $file) {
  if ($op == 'delete') {
    // When a profile photo is being deleted, clear out the countent_type_profile fields.
    // This is to circumvent an existing bug in the imagefield module.
    db_query("UPDATE {content_type_profile} SET field_photo_fid = 0, field_photo_title = '', 
      field_photo_alt = '' WHERE field_photo_fid = %d", $file['fid']);
  }
}

/**
* Clean up the form values so they don't make it into the users.data column
*/
function venture_profile_insert_user($form_values) {
  foreach($form_values as $key => $value) {
    if (strncmp($key, 'field_', 6) == 0) {
      unset($form_values[$key]);
    }
  }
  return $form_values;
}

/**
* Determine if the user is an accredited investor
*/
function venture_profile_is_accredited($profile = NULL)
{
  if (!$profile) {
    global $user;
    $profile = venture_profile_retrieve($user->uid);
  }
  $accredited = $profile->field_qualification[0]['value'] == 'accredited';
  return $accredited || venture_profile_is_admin($profile);
}

/**
* Determine if the user is an admin
*/
function venture_profile_is_admin($account = NULL) {
  if (!$account) {
    global $user;
    $account = $user;
  }
  $uid = is_numeric($account) ? $account : $account->uid;
  return $uid == 1;
}

/**
* Determine if the user is valid (not admin, not blocked)
*/
function venture_profile_is_valid($account) {
  // Always allow admins
  if (venture_profile_is_admin()) return TRUE;
  
  if (is_numeric($account)) {
    if (venture_profile_is_admin($account)) {
      return FALSE;
    }
    $account = user_load(array('uid' => $account));
  } else if (!isset($account->status)) {
    // There is a chance that the object passed in is not a user object
    $account = user_load(array('uid' => $account->uid));
  }
  
  return (!venture_profile_is_admin($account) && $account->status);
}

/**
* Add profile-related menu items
*/
function venture_profile_menu($may_cache) {
  global $user;
  $items = array();
  
  if (!$may_cache) {
    if (arg(0) == 'profile' && is_numeric(arg(1)) && arg(2) == 'submit' && $user->uid != arg(1)) {
      if (($account = user_load(array('uid' => arg(1)))) && venture_profile_is_valid($account)) {
        $items[] = array(
          'path' => "profile/{$account->uid}/submit",
          'callback' => 'node_add',
          'callback arguments' => array('deal'),
          'access' => user_access('access content'),
          'type' => MENU_CALLBACK
        );
      }
    }
  } 
  return $items;
}

/**
* Alter the profile photo form
*/
function venture_profile_photo_alter(&$form) {
  global $user;
  venture_image_alter($form, 'field_photo', $user->uid);
}

/**
* Alter the register form
*/
function venture_profile_register_alter(&$form) {
  $form['#submit']['venture_profile_register_submit'] = array();
  $form['#pre_render'][] = 'venture_profile_register_pre_render';
  $form['submit']['#value'] = 'Create Account';
  
  $form['name'] = $form['account']['name'];
  $form['name']['#type'] = 'hidden';
  $form['name']['#value'] = user_password(); // Generate username
  $form['mail'] = $form['account']['mail'];
  $form['mail']['#title'] = 'Email';
  $form['mail']['#description'] = NULL;
  $form['pass'] = $form['account']['pass'];
  $form['pass']['#description'] = NULL;
  unset($form['account']);
  
  // Hide email alert checkboxes (all should be selected by default)
  $form['field_email_alerts']['keys']['#type'] = 'value';
  
  // Massage the default group info
  foreach($form['og_register']['og_register']['#options'] as $gid => $value) {
    if (preg_match('!Angel Hub!', $value)) {
      $groups['Angel Investor'] = $gid;
    }
    else if (preg_match('!VC Hub!', $value)) {
      $groups['Venture Capitalist'] = $gid;
    }
    else {
      $groups['All'] = $gid;
    }
  }
  $form['register_groups'] = array(
    '#type' => 'value',
    '#value' => $groups
  );
  unset($form['og_register']);
  
  // Automatic detection of timezone on register is implemented in D6
  $form['timezone'] = array(
    '#type' => 'hidden',
    '#default_value' => variable_get('date_default_timezone', NULL),
    '#id' => 'edit-user-register-timezone',
  );
  
  // Make sure fields are ordered properly within the form
  $fields = array(
    'field_first_name', 'field_last_name', 'field_organization', 'field_title', 'field_country',
    'field_state', 'field_city', 'field_phone', 'mail', 'field_website', 'pass', 
    'field_investor_type', 'field_funding_stages', 'field_industries', 'field_investment_preferences',
    'field_other_preferences', 'field_countries', 'field_states', 'field_minimum_investment',
    'field_maximum_investment', 'field_qualification'
  );
  venture_order_fields($form, $fields);
  
  venture_profile_shared_alter($form);
}

/**
* Registration form pre_render event handler
*/
function venture_profile_register_pre_render($form_id, &$form) {
  $form['field_qualification']['key']['#title'] = 'I hereby acknowledge and certify that I am a qualified investor';
}

/**
* Registration submit callback
*/
function venture_profile_register_submit($form_id, $form_values) {
  global $user;
  
  // Replace generated username with email
  db_query('UPDATE {users} SET name = mail WHERE uid = %d', $user->uid);
    
  // Join the default groups
  $groups[] = $form_values['register_groups']['All'];
  foreach($form_values['field_investor_type']['keys'] as $type) {
    $groups[] = $form_values['register_groups'][$type];
  }
  foreach($groups as $gid) {
    if ($gid) {
      venture_group_join($user->uid, node_load($gid));
    }
  }
}

/**
* Remove period at the end of the name
*/
function venture_profile_remove_period(&$name) {
  $name = preg_replace('!\.$!', '', $name);
}

/**
* Return the profile for a given user
*/
function venture_profile_retrieve($uid = NULL) {
  static $profiles = array();
  
  // Use current user if no uid provided
  if (!$uid) {
    global $user;
    $uid = $user->uid;
  }
  if (!$uid) {
    return NULL;
  }
  else if (venture_profile_is_admin($uid)) {
    // Hardcode name for admin user for now
    return venture_profile_create(1, 'Roman', 'Shekhtmeyster');
  }
  
  // Get the profile id
  $nid = $profiles[$uid];
  if (!$nid) $nid = bio_for_user($uid);
  if (!$nid) return NULL;
  
  // Return the profile
  $profiles[$uid] = $nid;
  return node_load($nid);
}

/**
* Profile form after_build event handler
*/
function venture_profile_shared_after_build($form, $form_values) {
  // Blank out the state fields if a country other than US is chosen
  if ($form_values['field_country']['key'] != 'United States') {
    form_set_value($form['field_state'], NULL);
  }
  
  $countries = $form_values['field_countries']['keys'];
  if (count($countries) != 1 || !in_array('United States', $countries)) {
    form_set_value($form['field_states'], NULL);
  }
  
  // If any location data changes, blank out the geocoded values
  if ($form['#node']) {
    if ($form['#node']->field_city[0]['value'] != $form_values['field_city'][0]['value'] ||
        $form['#node']->field_state[0]['value'] != $form_values['field_state']['key'] ||
        $form['#node']->field_country[0]['value'] != $form_values['field_country']['key']) {
      form_set_value($form['field_location'][0], NULL);
    }
  }
  
  return $form;
}

/**
* Alter the profile fields
*/
function venture_profile_shared_alter(&$form) {
  $form['field_state']['#validate'] = array('venture_profile_validate_state' => array());
  $form['field_investor_type']['#validate'] = 
    array('venture_profile_validate_required_checkboxes' => array('field_investor_type', 'Investor Type')); 
  $form['field_funding_stages']['#validate'] = 
    array('venture_profile_validate_required_checkboxes' => array('field_funding_stages', 'Funding Stages')); 
  
  $form['#after_build'][] = 'venture_profile_shared_after_build'; 
  $form['#pre_render'][] = 'venture_profile_shared_pre_render';
  
  $form['field_countries']['keys']['#title'] = 'Geographical Area';
  $form['field_states']['keys']['#title'] = '';
  
  // Since email is used for username, make sure maxlength matches username's  
  $mail_field['#maxlength'] = 60;
}

/**
* Profile form pre_render event handler
*/
function venture_profile_shared_pre_render($form_id, &$form) {
  $form['field_state']['key']['#required'] = TRUE;
  $form['field_investor_type']['keys']['#required'] = TRUE;

  // Format the funding stages checkboxes
  $funding_stages = &$form['field_funding_stages']['keys'];
  $funding_stages['#required'] = TRUE;
  venture_format_funding_stages($funding_stages);
  
  // Format the email alerts checkboxes
  $titles = array(
    'pm' => 'A private message is received from another user.',
    'invitee' => 'A user I invited has joined the site.',
    'conn_req' => 'A connection request is received from another user.',
    'group_req' => 'A group membership request is received for a moderated group I administer.',
    'conn_hdl' => 'My connection request is accepted or denied by another user.',
    'group_hdl' => 'My group membership request is accepted or denied by a moderated group administrator.'
  );
  
  $email_alerts = &$form['field_email_alerts']['keys'];
  foreach ($titles as $alert => $title) {
    $email_alerts[$alert]['#attributes']['title'] = $email_alerts[$alert]['#title'] . ' - ' . $title;
  }
}

/**
* Validate required checkboxes - default CCK checkboxes required
* field validation seems to be broken.
*/
function venture_profile_validate_required_checkboxes($element, $field_name, $field_title) {
  global $form_values;
  foreach($form_values[$field_name]['keys'] as $value) {
    if ($value) {
      return;
    }
  }
  form_set_error($field_name, "$field_title field is required.");
}

/**
* Validate the State field
*/
function venture_profile_validate_state($element) {
  global $form_values;
  if ($form_values['field_country']['key'] == 'United States') {
    if (!$form_values['field_state']['key']) {
      form_set_error('field_state', 'State field is required.');
    }
  }
}

/**
* Modify the welcome email
*/
function venture_profile_welcome_mail_alter($from, &$subject, &$body) {
  global $form_values;
  $name = $form_values['field_first_name'][0]['value'] . ' ' . $form_values['field_last_name'][0]['value'];
  $subject = "Welcome to VentureTap";
  $body = "$name:\n\nYour VentureTap profile has been successfully created. To start networking with other members, " .
    "sign in using the link below:\n" . url('login', NULL, NULL, TRUE) . "\n\nIf you are not the intended recipient " .
    "of this message, please let us know by replying to $from.";
  $body = wordwrap($body, 72);
}