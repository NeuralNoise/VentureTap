<?php

/******** THEME OVERRIDE FUNCTIONS ********/

/**
* Theme the uploaded files table
*/
function venture_filefield_form_current($element) {
  global $image_path;
  $dl_image = "<img src='$image_path/download.png' alt='Download' title='Download' />";
  
  foreach (element_children($element) as $key) {
    // Don't display (hidden) replaced items.
    if ($elementorm[$key]['delete']['#type'] == 'hidden') {
      continue;
    }
    
    // Extract the download URL
    if (preg_match('!href="(.*)"!', $element[$key]['url']['#value'], $matches)) {
      $dl_url = url($matches[1]);
      $element[$key]['url']['#value'] = "<a href='$dl_url'>$dl_image</a>";
    }

    $row = array(); 
    $row[] = drupal_render($element[$key]['delete']);
    $row[] = drupal_render($element[$key]['icon']);
    $row[] = drupal_render($element[$key]['description']);
    $row[] = drupal_render($element[$key]['size']);
    $row[] = drupal_render($element[$key]['url']);
    $rows[] = $row;
  }
  
  if ($rows) {
    $header = array('Delete', '', 'Description', 'Size', '');
    $output = theme('table', $header, $rows, array('class' => 'attachment-list'));
  }
  
  $output .= drupal_render($element);
  return $output;
}

/**
* Theme the filefield icon
*/
function venture_filefield_icon($file) {
  $mime = check_plain($file['filemime']);
  if ($icon_url = filefield_icon_url($file)) {
    $icon = "<img class='filefield-icon' src='$icon_url' alt='$mime' title='' />";
  }
  return $icon;
}

/**
* Theme an uploaded image
*/
function venture_image_upload($element) {
  $element['new']['#prefix'] = '<div id="image-container">' . $element['new']['#prefix'];
  $element['new']['#suffix'] .= '</div>';
  
  // Display the preview as a thumbnail
  foreach(array(0, 1) as $idx) {
    foreach(array('filepath', 'preview') as $path) {
      if ($element[$idx][$path]['#value']) {
        $element[$idx]['admin_preview']['#value'] = theme('imagecache', 'thumb', $element[$idx][$path]['#value']);
      }
    }
  }
  
  return drupal_render($element);
}

/**
* Theme the search excerpt
*/
function venture_search_excerpt($content) {
  $node = node_load($content['body']['#value']);
  $fields = array();
  
  if ($node->type == 'profile') {
    $fields += venture_element_get_contact_info($node);
    $fields += venture_element_get_investor_summary($node);
    $fields += venture_element_get_experience($node);
  }
  else if ($node->type == 'group') {
    $fields['Description'] = $node->og_description;
    $fields += venture_element_get_group_details($node);
    unset($fields['Statistics']);
    unset($fields['Updated']);
    unset($fields['Owner']);
  }
  else if ($node->type == 'deal') {
    $fields['Amount Seeking'] = venture_render_field($node, 'field_deal_amount_seeking');
    $fields['Funding Type'] = venture_render_field($node, 'field_deal_funding_type');
    $fields += venture_element_get_deal_summary($node);
    $fields['Headline'] = venture_render_field($node, 'field_deal_headline');
    $fields += venture_element_get_deal_details($node);
  }
  
  foreach ($fields as $title => $value) {
    // Field title will be removed during indexing.
    // Actual tags are stripped for excerpt display.
    $output .= "<remove>$title:</remove> $value ~ ";
  }
  $output = substr($output, 0, -3);
  
  return $output;
}

/**
* Theme the search results
*/
function venture_search_page($results, $type) {
  foreach ($results as $entry) {
    $result = $footer = $type = '';
    $node = $entry['node'];
    
    if ($node->type == 'profile') {
      $result = theme('username', $entry['node']);
      $type = 'User Profile';
      $investor = strtolower(implode(', ', venture_render_field($node, 'field_investor_type', TRUE)));
      $investor = ($investor == 'venture capitalist' ? 'a ' : 'an ') . $investor;
      $info = venture_element_get_contact_info($node);
      $footer = " for $investor in " . $info['Location'];
    }
    else if ($node->type == 'group') {
      $result = l($node->title, "group/{$node->nid}");
      $info = venture_element_get_group_details($node);
      $type = $info['Group'] . ' Group';
      if ($info['Audience']) {
        $footer = ' for ' . strtolower(substr($info['Audience'], 0, 1)) . substr($info['Audience'], 1);
      }
      $footer .= ' - ' . $info['Statistics'];
    }
    else if ($node->type == 'group_post') {
      $result = l($node->title, "post/{$node->nid}");
      $type = 'Group Post';
      $gid = $node->og_groups[0];
      $group = node_load($gid);
      $group_title = $node->og_groups_both[$gid];
      $footer = ' in ' . l($group_title, "group/$gid");
    }
    else if ($node->type == 'deal') {
      $title = venture_deal_get_node_title($node);
      $amount = venture_render_field($node, 'field_deal_amount_seeking');
      $result = l("$title seeking $$amount", "deal/{$node->nid}", array(), NULL, NULL, FALSE, TRUE);
      $funding = venture_render_field($node, 'field_deal_funding_type');
      $type = "$funding Deal";
      $info = venture_element_get_deal_summary($node);
      $footer = ' in ' . $info['Location'];
      
      if (!venture_deal_is_node_accessible($node)) {
        $result = "$title seeking $$amount";
        $entry['snippet'] = 'Deal accessible by accredited investors only.';
      }
    }
    
    $result = "<span class='search-header'>$result</span>";
    $result .= "<div class='search-excerpt'>{$entry['snippet']}</div>";
    $result .= "<span class='search-footer'><strong>$type</strong>$footer</span>";
    $output .= "<div class='search-result'>$result</div>";
  }
  
  if ($output) {
    $pager = theme('pager', NULL, 10, 0);
    $output = $pager . $output . $pager;
  }
  
  $output = "<div id='search-page'>$output</div>";  
  return $output;
}

/******** UTILITY FUNCTIONS ********/

/**
* Get contact info fields
*/
function venture_element_get_contact_info($account_profile, $conn_status = NULL) {
  global $user, $account;
  $fields = array();
  
  // Render Position/Title/Organization
  $title = venture_render_field($account_profile, 'field_title');
  $organization = venture_render_field($account_profile, 'field_organization');
  if ($title && $organization) {
    $fields['Position'] = "$title at $organization";
  }
  else if ($title) {
    $fields['Title'] = $title;
  }
  else if ($organization) {
    $fields['Organization'] = $organization;
  }
  
  // Render Location
  $city = venture_render_field($account_profile, 'field_city');
  $state = venture_render_field($account_profile, 'field_state');
  $country = venture_render_field($account_profile, 'field_country');
  $location = venture_geo_get_location($city, $state, $country);
  $fields['Location'] = $location;
  
  // Render Phone and Email to connections only
  if ($account && ($user->uid == $account->uid ||
                   $conn_status == VENTURE_CONN_EXISTS ||
                   venture_profile_is_admin($user))) {
    $phone = venture_render_field($account_profile, 'field_phone');
    $email = check_plain($account->mail);
    
    if ($phone) {
      $fields['Phone'] = $phone;
    }
    $fields['Email'] = "<a href='mailto:$email'>$email</a>";
  }
  
  // Render Website
  $website = venture_render_field($account_profile, 'field_website');
  if ($website) {
    $fields['Website'] = $website;
  }
  
  return $fields;
}

/**
* Get deal detail fields
*/
function venture_element_get_deal_details($deal) {
  $fields = array();
  $details = array(
    'Company Overview' => 'field_deal_company_overview',
    'Product/Service' => 'field_deal_product_service',
    'Management Team' => 'field_deal_management_team',
    'Deal Details' => 'field_deal_details'
  );
  
  foreach ($details as $title => $field_name) {
    $field = venture_render_field($deal, $field_name);
    if ($field) {
      $fields[$title] = $field;
    }
  }
  
  return $fields;
}

/**
* Get deal summary fields
*/
function venture_element_get_deal_summary($deal) {
  $fields = array();
  
  // Render Location
  $city = venture_render_field($deal, 'field_deal_city');
  $state = venture_render_field($deal, 'field_deal_state');
  $country = venture_render_field($deal, 'field_deal_country');
  $location = venture_geo_get_location($city, $state, $country);
  $fields['Location'] = $location;
  
  // Render Year Founded
  $founded = venture_render_field($deal, 'field_deal_year_founded');
  if ($founded) {
    $fields['Year Founded'] = $founded;
  }
  
  // Render Employees
  $employees = venture_render_field($deal, 'field_deal_employees');
  if ($employees) {
    $fields['Employees'] = $employees;
  }
  
  // Render Legal Entity
  $entity = venture_render_field($deal, 'field_deal_legal_entity');
  if ($entity) {
    $fields['Legal Entity'] = $entity;
  }
  
  // Render Website
  $website = venture_render_field($deal, 'field_deal_website');
  if ($website) {
    $fields['Website'] = $website;
  }
  
  // Render Industry, Funding Stage, Prior Investors
  $fields['Industry'] = venture_render_field($deal, 'field_deal_industry');
  $fields['Funding Stage'] = venture_render_field($deal, 'field_deal_funding_stage');
  $fields['Prior Investors'] = implode(', ', venture_render_field($deal, 'field_deal_prior_investors', TRUE));
  
  // Render Capital Raised
  $raised = venture_render_field($deal, 'field_deal_capital_raised');
  if ($raised) {
    $fields['Capital Raised'] = '$' . $raised;
  }
  
  // Render Current Valuation
  $valuation = venture_render_field($deal, 'field_deal_current_valuation');
  if ($valuation) {
    $fields['Current Valuation'] = '$' . $valuation;
  }
  
  // Render Annual Revenue
  $revenue = venture_render_field($deal, 'field_deal_annual_revenue');
  if ($revenue) {
    $fields['Annual Revenue'] = '$' . $revenue;
  }
  
  // Render Contact Name
  $contact_name = venture_render_field($deal, 'field_deal_contact_name');
  if ($contact_name) {
    $fields['Contact Name'] = $contact_name;
  }
  
  // Render Contact Email
  $contact_email = venture_render_field($deal, 'field_deal_contact_email');
  if ($contact_email) {
    $fields['Contact Email'] = "<a href='mailto:$contact_email'>$contact_email</a>";
  }
  
  // Render Contact Phone
  $contact_phone = venture_render_field($deal, 'field_deal_contact_phone');
  if ($contact_phone) {
    $fields['Contact Phone'] = $contact_phone;
  }
  
  return $fields;
}

/**
* Get experience fields
*/
function venture_element_get_experience($profile) {
  global $user, $account;
  $fields = array();
  
  $experience = array(
    'About Me' => 'field_about_me',
    'Industry Experience' => 'field_industry_experience',
    'Investing Experience' => 'field_investing_experience'
  );
  
  foreach ($experience as $title => $field_name) {
    $field = venture_render_field($profile, $field_name);
    if ($account && $user->uid == $account->uid && !$field) {
      $click = l('Click here', 'account', array(), NULL, 'experience');
      $field = "<p>No information available. $click to add this information to your profile.</p>";
    }
    if ($field) {
      $fields[$title] = $field;
    }
  }
  
  return $fields;
}

/**
* Get group details
*/
function venture_element_get_group_details() {
  $fields = array();
  $args = func_get_args();
  if (func_num_args() == 1) {
    $group = $args[0];
    $counts = venture_group_get_counts($group->nid);
    $owner = $group->uid;
    $fields['Group'] = ($group->og_selective == OG_MODERATED ? 'Moderated' : 'Open');
  }
  
  $gid = $group ? $group->nid : $args[0];
  $investor_type = $group ? venture_render_field($group, 'field_group_investor_type') : $args[1];
  $country = $group ? venture_render_field($group, 'field_group_country') : $args[2];
  $state = $group ? venture_render_field($group, 'field_group_state') : $args[3];
  $members = $group ? $counts['fast_member_count'] : $args[4];
  $topics = $group ? $counts['fast_topic_count'] : $args[5];
  $comments = $group ? $counts['fast_comment_count'] : $args[6];
  $updated = $group ? $counts['fast_last_updated'] : $args[7];
  
  // Format audience
  if ($investor_type || $country || $state) {
    $audience = $investor_type ? ucfirst(strtolower($investor_type)) . 's' : 'Anyone';
    if ($country || $state) {
      $audience .= ' in ' . ($state ? $state : $country);
    }
    $fields['Audience'] = $audience;
  }
  
  // Format statistics
  $members = $members . ' ' . format_plural($members, 'member', 'members');
  $members = l($members, "group/$gid/users");
  $posts = $topics + $comments;
  $topics = $topics . ' ' . format_plural($topics, 'topic', 'topics');
  $posts = $posts . ' ' . format_plural($posts, 'post', 'posts');
  $fields['Statistics'] = "$members, $topics, $posts";
  
  // Format last updated
  if ($updated) {
    $fields['Updated'] = format_date($updated, 'custom', 'l, m/d/Y - g:i A');
  }
  
  // Format owner
  if ($group) {
    $fields['Owner'] = theme('username', user_load(array('uid' => $group->uid)));
  }
  
  return $fields;
}

/**
* Get investor summary fields
*/
function venture_element_get_investor_summary() {
  $fields = array();
  $args = func_get_args();
  $profile = func_num_args() == 1 ? $args[0] : NULL;
  
  $investor_type = $profile ? venture_render_field($profile, 'field_investor_type', TRUE) : $args[0];
  $funding_stages = $profile ? venture_render_field($profile, 'field_funding_stages', TRUE) : $args[1];
  $industries = $profile ? venture_render_field($profile, 'field_industries', TRUE) : $args[2];
  $invest_prefs = $profile ? venture_render_field($profile, 'field_investment_preferences', TRUE) : $args[3];
  $other_prefs = $profile ? venture_render_field($profile, 'field_other_preferences') : $args[4];
  $states = $profile ? venture_render_field($profile, 'field_states', TRUE) : $args[5];
  $countries = $profile ? venture_render_field($profile, 'field_countries', TRUE) : $args[6];
  $min_invest = $profile ? venture_render_field($profile, 'field_minimum_investment') : $args[7];
  $max_invest = $profile ? venture_render_field($profile, 'field_maximum_investment') : $args[8];
  
  $funding_stages = str_replace(' Stage', '', $funding_stages);
    
  $fields['Investor Type'] = implode(', ', $investor_type);
  $fields['Funding Stages'] = implode(', ', $funding_stages);
  $fields['Industries'] = $industries ? implode(', ', $industries) : 'No Preference';
  
  $prefs = $invest_prefs;  
  if ($other_prefs) $prefs[] = $other_prefs;
  if ($prefs) $fields['Investment Preferences'] = implode(', ', $prefs);
  
  $fields['Geographical Area'] = ($states) ? implode(', ', $states) : implode(', ', $countries);
  $fields['Minimum Investment'] = $min_invest;
  $fields['Maximum Investment'] = $max_invest;
  
  return $fields;
}

/**
* Render activities table
*/
function venture_element_render_activities($uid, $buddies) {
  $activities = venture_user_get_activities($uid, $buddies);
  if (empty($activities)) {
    $output = '<div id="no-activity">No activity recorded for the past week.</div>';
  }
  else {
    $rows = array();
    foreach ($activities as $activity) {
      if ($activity_message = activity_token_replace($activity)) {
        $rows[] = array(
          "<div class='activity'>$activity_message</div>",
          format_date($activity['created'], 'custom', 'm/d/Y - g:i A')
        );
      }
    }
    $output = theme('table', NULL, $rows, array('class' => 'home-table'));
  }
  return $output;
}

/**
* Render the connection status
*/
function venture_element_render_connection_status($status) {
  if ($status == VENTURE_CONN_INITIATED || $status == VENTURE_CONN_REQUESTED) {
    $label = 'connection pending';
  }
  else if ($status == VENTURE_CONN_EXISTS) {
    $label = 'in my network';
  }
  if ($label) {
    $label = "<div class='record-status'>[ $label ]</div>";
  }
  return $label;
}

/**
* Render the contact info list
*/
function venture_element_render_contact_info($account_profile, $conn_status) {
  $fields = venture_element_get_contact_info($account_profile, $conn_status);
  foreach ($fields as $title => $value) {
    $output .= "<tr><td>$title:</td><td><div class='contact-value'>$value</div></td></tr>";
  }
  $output = "<table id='contact-info'>$output</table>";
  
  return $output;
}

/**
* Render the deal posting details
*/
function venture_element_render_deal_posted($nid, $uid, $created, $title, $users, $groups, $for_edit = FALSE) {
  global $user;
  $recipients = array();

  foreach($users as $to) {
    if ($uid == $user->uid || venture_profile_is_admin()) {
      if ($recipient = user_load(array('uid' => abs($to)))) {
        $recipient = theme('username', $recipient);
        if ($to < 0) {
          $recipient .= ' (not interested)';
        }
        $recipients[] = $recipient;
      }
    }
    else if ($to == $user->uid) {
      $action = "action-remove-$nid";
      $remove = l('remove', "deal/$nid/remove", array('id' => $action, 'title' => 'Remove private deal'));
      $confirm = "You will no longer be able to view the $title private deal. Proceed with removing?";
      $confirm = "<div id='$action-dialog'>$confirm</div>";
      $recipients[] = "me ($remove)$confirm";
      break;
    }
  }
  
  foreach($groups as $to) {
    if($uid == $user->uid || venture_profile_is_admin() || $user->og_groups[$to]) {
      if ($group = node_load(abs($to))) {
        $group_name = check_plain($group->title);
        $recipient = l($group_name, "group/{$group->nid}", array(), NULL, NULL, FALSE, TRUE);
        if ($to < 0) {
          $recipient .= ' (not interested)';
        }
        else if ($group->uid == $user->uid) {
          $action = "action-remove-$nid-group-{$group->nid}";
          $remove = l('remove', "deal/$nid/remove/{$group->nid}", array('id' => $action, 'title' => 'Remove group deal'));
          $confirm = "$group_name members will no longer be able to view the $title group deal. Proceed with removing?";
          $confirm = "<div id='$action-dialog'>$confirm</div>";
          $recipient .= " ($remove)$confirm";
        }
        $recipients[] = $recipient;
      }
    }
  }

  $posted = count($users) + count($groups) > 0 ? 'submitted privately' : 'posted publicly';
  if ($for_edit) {
    $posted = ucfirst($posted);
  }
  else if ($uid) {
    $posted .= " by " . theme('username', user_load(array('uid' => $uid)));
  }
  $posted .= " on $created";
  
  if ($recipients) {
    $posted .= ' to ' . implode(', ', $recipients);
  }
  $posted = "<div class='deal-posted record-actions'>$posted</div>";
  
  return $posted;
}

/**
* Render the group actions list
*/
function venture_element_render_group_actions($gid, $status, $selective, $group_name, $owner) {
  global $user;
  $actions = array();
  $is_admin = venture_profile_is_admin();
  
  if ($user->uid != $owner && !$is_admin) {
    if ($status == VENTURE_GROUP_MEMBER) {
      $actions['remove'] = array(
        'title' => 'Leave group',
        'confirm' => "You have requested to leave $group_name. Proceed?"
      );
    }
    else if ($status == VENTURE_GROUP_REQUESTED) {
      $actions['remove'] = array(
        'title' => 'Cancel membership request',
        'confirm' => "You are about to cancel your membership request to $group_name. Proceed?"
      );
    }
    else if ($selective == OG_MODERATED) {
      $actions['add'] = array(
        'title' => 'Request membership',
        'confirm' => "You are about to request membership to $group_name. Proceed?"
      );
    }
    else {
      $actions['add'] = array(
        'title' => 'Join group',
        'confirm' => "You have requested to join $group_name. Proceed?"
      );
    }
  }
  
  if (arg(0) == 'groups') {
    $output = '<li>' . l('View group', "group/$gid") . '</li>';
  }
  
  if ($selective == OG_MODERATED) {
    $output .= '<li>' . l('Submit a deal', "group/$gid/submit") . '</li>';
  }
  
  foreach($actions as $action => $item) {
    $link = l($item['title'], "group/$gid/$action", array('id' => "action-$action-$gid"));
    $confirm = "<div id='action-$action-$gid-dialog'>{$item['confirm']}</div>";
    $output .= "<li>$link$confirm</li>";
  }
  
  if (arg(0) == 'group' && ($user->uid == $owner || $is_admin)) {
    $output .= '<li>' . l('Send a message', "group/$gid/pm") . '</li>';
    $output .= '<li>' . l('Edit group', "group/$gid/edit") . '</li>';
  }
  
  return "<ul class='record-actions'>$output</ul>";
}

/**
* Render group details
*/
function venture_element_render_group_details() {
  $args = func_get_args();
  $fields = call_user_func_array('venture_element_get_group_details', $args);
  
  // Format details
  foreach($fields as $title => $value) {
    $output .= "<tr><td>$title:</td><td><div class='record-summary-value'>$value</div></td></tr>";
  }
  return "<table class='record-summary'>$output</table>";
}

/**
* Render group posts/comments
*/
function venture_element_render_group_post($group, $nid, $cid, $uid, $timestamp, $title, $body, $format) {
  global $user;
  
  $deleted_by = venture_group_message_deleted_by($body);
  if ($deleted_by) {
    $name = venture_profile_get_name($deleted_by);
    $body = "<em>Message deleted by $name.</em>";
  }
  
  $profile = venture_profile_retrieve($uid);
  $profile->teaser = TRUE;
  $name = theme('username', $profile);
  
  if (venture_profile_is_admin($uid)) {
    // Admin
    $name = "<div class='admin-name'>$name</div>";
    $type = "<div class='admin-type'>Administrator</div>";
  }
  else {
    // Regular user
    $photo = venture_render_field($profile, 'field_photo');
    $photo = venture_element_render_image($photo, $profile->status ? "user/$uid" : NULL);
    $type = implode(', ', venture_render_field($profile, 'field_investor_type', TRUE));
    $city = venture_render_field($profile, 'field_city');
    $state = venture_render_field($profile, 'field_state');
    $country = venture_render_field($profile, 'field_country');
    $location = venture_geo_get_location($city, $state, $country);
  }
  
  $date = format_date($timestamp, 'custom', 'l, m/d/Y - g:i A');
  $title = check_plain($title);
  $body = check_markup($body, $format, FALSE);
  $post_id = $cid ? "post-$cid" : 'post-first';
  
  if ($group) {
    $path = $cid ? "/$cid" : NULL;
    $is_admin = venture_profile_is_admin();
    $query = $_GET['page'] ? 'page=' . $_GET['page'] : NULL;
    
    if (($user->uid == $uid && !$deleted_by) || $is_admin) {
      $links = l('Edit', "post/$nid/edit$path", array(), $query);
    }
    if (venture_group_message_delete_allowed($group, $uid) && !$deleted_by) {
      $id = "action-delete-$nid" . ($cid ? "-$cid" : '');
      $links .= "<span id='$id-dialog'>Are you sure you want to delete this message?</span>";
      $links .= l('Delete', "post/$nid/delete$path", array('id' => $id));
    }
    $links .= l('Reply', "post/$nid/reply$path", array(), $query);
  }
  
  $left = "<div class='post-left'>$name $photo $type<div class='location'>$location</div></div>";
  $top = "<div class='post-top record-actions'><div class='post-date'>$date</div>$links</div>";
  $right = "<div class='post-right'>$top<div class='post-title'>$title</div><div class='post-body'>$body</div></div>";
  $output = "<div id='$post_id' class='post'><div class='post-container'>$left $right</div></div>";
  
  return $output;
}

/**
* Render the investor actions list
*/
function venture_element_render_investor_actions($uid, $status, $name) {
  global $user;
  $actions = array();
  
  if ($user->uid != $uid) {
    if ($status == VENTURE_CONN_EXISTS) {
      $actions['remove'] = array(
        'title' => 'Remove from my network',
        'confirm' => "You have requested to remove $name from your network. Proceed?"
      );
    }
    else if ($status == VENTURE_CONN_INITIATED) {
      venture_profile_remove_period($name);
      $actions['cancel'] = array(
        'title' => 'Cancel connection request',
        'confirm' => "You are about to cancel your connection request to $name. Proceed?"
      );
    }
    else if ($status == VENTURE_CONN_REQUESTED) {
      venture_profile_remove_period($name);
      $actions['accept'] = array(
        'title' => 'Accept connection request',
        'confirm' => "You are about to accept the connection request from $name. Proceed?"
      );
      $actions['deny'] = array(
        'title' => 'Deny connection request',
        'confirm' => "You are about to deny the connection request from $name. Proceed?"
      );
    }
    else {
      $actions['add'] = array(
        'title' => 'Add to my network',
        'confirm' => "You have requested to add $name to your network. Proceed?"
      );
    }
  }
  
  if (arg(0) != 'user') {
    $output = '<li>' . l('View profile', "user/$uid") . '</li>';
  }
  
  if ($user->uid == $uid) {
    if (arg(0) == 'user') {
      $output .= '<li>' . l('Edit my profile', 'account') . '</li>';
    }
  }
  else {
    $output .= '<li>' . l('Send a message', "pm/new/$uid") . '</li>';
    $output .= '<li>' . l('Submit a deal', "profile/$uid/submit") . '</li>';
    
    foreach($actions as $action => $item) {
      $link = l($item['title'], "network/$action/$uid", array('id' => "action-$action-$uid"));
      $confirm = "<div id='action-$action-$uid-dialog'>{$item['confirm']}</div>";
      $output .= "<li>$link$confirm</li>";
    }
  }
  
  // For group member lists, allow admins to remove users
  if (arg(0) == 'network' && arg(1) == 'group' && $user->uid != $uid) {
    $group = node_load(arg(2));
    if ($group->uid != $uid && og_is_node_admin($group)) {
      $action_id = "action-group-remove-$uid";
      $link = l('Remove from group', "group/{$group->nid}/remove/$uid", array('id' => $action_id));
      $confirm = "<div id='$action_id-dialog'>You have requested to remove $name from {$group->title}. Proceed?</div>";
      $output .= "<li>&nbsp;</li><li>$link$confirm</li>";
    }
  }
  
  return "<ul class='record-actions'>$output</ul>";
}

/**
* Render the investor summary list
*/
function venture_element_render_investor_summary() {
  $args = func_get_args();
  $fields = call_user_func_array('venture_element_get_investor_summary', $args);
  
  foreach($fields as $title => $value) {
    $output .= "<tr><td>$title:</td><td><div class='record-summary-value'>$value</div></td></tr>";
  }
  return "<table class='record-summary'>$output</table>";
}

/**
* Render an invite history list
*/
function venture_element_render_invites($type) {
  $total_rows = venture_invite_get_by_type($type, $rows);
  foreach($rows as $row) {
    $output .= "<li>$row</li>";
  }
  if ($output) {
    $output = "<ul>$output</ul>";
  }  
  
  $header = ucfirst($type);
  if ($total_rows > count($rows)) {
    $total = count($rows) . ' of ' . $total_rows;
  }
  else {
    $total = $total_rows . ' total';
  }

  $output = "<div class='invite-type'>$header:</div>$output<div class='total'><span>$total</span></div>";
  return $output;
}

/**
* Render new private messages
*/
function venture_element_render_new_messages($uid) {
  $messages = venture_pm_get_new_messages($uid);
  foreach($messages as $message) {
    $author = user_load(array('uid' => $message->author));
    $author = theme('username', $author);
    $subject = l($message->subject, "pm/view/{$message->id}");
    $subject = "<strong>$subject</strong> " . theme('mark');
    
    $rows[] = array(
      "<div class='name'>$author</div>",
      "<div class='subject'>$subject</div>",
      format_date($message->timestamp, 'custom', 'm/d/Y - g:i A')
    );
  }
  
  $output = theme('table', NULL, $rows, array('class' => 'home-table'));
  return $output;
}

/**
* Render the group membership status
*/
function venture_element_render_membership_status($status) {
  if ($status == VENTURE_GROUP_MEMBER) {
    $label = 'in my groups';
  }
  else if ($status == VENTURE_GROUP_REQUESTED) {
    $label = 'membership pending';
  }
  if ($label) {
    $label = "<div class='record-status'>[ $label ]</div>";
  }
  return $label;
}

/**
* Render the menu
*/
function venture_element_render_menu($items) {
  global $user;
  
  // Figure out the client-provided URL arguments
  $q = $_GET['q'];
  $alias = ($q == 'home') ? $q : drupal_get_path_alias($q);
  $args = explode('/', $alias);
  $is_me = !$args[1] || $args[1] == $user->uid;
  
  // Create each menu item (selecting it if necessary)
  foreach($items as $item) {
    $attr = (count($_GET) == 1 && $args[0] == $item && $is_me) ? array('class' => 'selected') : array();
    $menu .= l(strtoupper($item), $item, $attr);
  }
  
  return $menu;
}

/**
* Render the profile photo
*/
function venture_element_render_image($image, $path = NULL, $fragment = NULL) {
  $image = preg_replace('!(alt|title)=".*?"!', '$1=""', $image); // Empty alt/title values
  if ($path) {
    $image = l($image, $path, array(), NULL, $fragment, FALSE, TRUE);
  }
  return $image;
}

/**
* Render requests that can be accepted or denied
*/
function venture_element_render_requests($users, $type, $accept_path, $deny_path) {
  foreach($users as $user) {
    $requester = ucfirst($type) . ' request from ' . theme('username', $user);
    $name = venture_profile_get_name($user->uid);
    venture_profile_remove_period($name);

    // Create the accept action link
    $accept = "You are about to accept the $type request from $name. Proceed?";
    $accept = "<div id='action-accept-{$user->uid}-dialog'>$accept</div>";
    $accept .= l('Accept', "$accept_path/{$user->uid}", array('id' => "action-accept-{$user->uid}"));
    
    // Create the deny action link
    $deny = "You are about to deny the $type request from $name. Proceed?";
    $deny = "<div id='action-deny-{$user->uid}-dialog'>$deny</div>";
    $deny .= l('Deny', "$deny_path/{$user->uid}", array('id' => "action-deny-{$user->uid}"));
    
    $output .= "<tr><td><div class='requester'>$requester</div></td><td class='record-actions'>$accept or $deny</td></tr>";
  }
  if ($output) {
    if (count($users) > 1) {
      $all = l('Accept All', "$accept_path/all", array('id' => 'action-accept-all'));
      $all .= l('Deny All', "$deny_path/all", array('id' => 'action-deny-all'));
      $all .= "<div id='action-accept-all-dialog'>You are about to accept all $type requests. Proceed?</div>";
      $all .= "<div id='action-deny-all-dialog'>You are about to deny all $type requests. Proceed?</div>";
      $all = "<div id='requests-all' class='record-actions'>$all</div>";
    }
    $output = "$all<div id='requests'><table>$output</table></div>";
  }
  
  return $output;
}