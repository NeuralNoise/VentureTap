<?php

/**
* Theme the comment edit form
*/
function venture_comment_form($form) {
  global $user, $image_path;
  
  // Determine the parent message
  if (arg(3)) {
    if (arg(2) == 'reply') {
      $pid = arg(3);
    }
    else {
      $reply = venture_get_comment(arg(3));
      $pid = $reply->pid;
    }

    if ($pid) {
      $pc = venture_get_comment($pid);
      $puid = $pc->uid;
      $parent = venture_element_render_group_post(NULL, $pc->nid, $pc->cid, $pc->uid, $pc->timestamp,
        $pc->subject, $pc->comment, $pc->format);
    }
  }
  if (!$parent) {
    $pn = node_load(arg(1));
    $puid = $pn->uid;
    $parent = venture_element_render_group_post(NULL, $pn->nid, NULL, $pn->uid, $pn->created,
      $pn->title, $pn->body, $pn->format);
  }
   
  $query = $_GET['page'] ? 'page=' . $_GET['page'] : NULL;
  $fragment = arg(3) ? 'post-' . arg(3) : NULL;
  $cancel = l('Cancel', 'post/' . arg(1), array(), $query, $fragment);
  $puser = venture_profile_get_name($puid);
  
  if (arg(2) == 'reply') {
    // Create new reply
    $title = 'Reply to message';
    $page_desc = "You may reply to $puser's message here.";
  }
  else {
    // Edit reply
    $title = 'Edit reply';
    $page_desc = "You may modify your reply to $puser's message here.";
  }
  
  drupal_set_title($title);
  
  $rendered_form = "<div id='reply-label'>Reply To:</div>$parent";
  $rendered_form .= drupal_render($form['title']) . drupal_render($form['body_filter']);
  $rendered_form .= '<div id="actions">' . drupal_render($form) . $cancel . '</div>';
  
  $desc = "<img src='$image_path/post-edit.png' alt='' title='' />";
  $desc .= "<div class='page-desc'>$page_desc</div>";
  $output = "<div class='page-title'><h1>$title</h1></div>";
  $output .= "<div class='desc-container'>$desc</div>";
  $output .= "<div id='edit-container'>$rendered_form</div>";
  $output = "<div id='post-edit'>$output</div>";
  
  return $output;
}

/**
* Theme the deal edit form
*/
function venture_deal_node_form($form) {
  if (arg(0) == 'profile') {
    $profile = venture_profile_retrieve(arg(1));
    $name = venture_profile_get_name($profile);
    $page_title = "Submit new deal to $name";
    $page_desc = "You may submit a new private deal here. It will be accessible by $name only.";
    if (!venture_profile_is_accredited($profile)) {
      $page_desc .= " Only deals outside of the United States may be submitted, since $name is not an accredited investor.";
    }
    $guidelines = venture_render_field($profile, 'field_guidelines');
    $cancel_path = "profile/{$profile->uid}";
  }
  else if (arg(0) == 'group') {
    $group = node_load(arg(1));
    $group_name = check_plain($group->title);
    $page_title = "Submit new deal to $group_name";
    $page_desc = "You may submit a new group deal here. It will be accessible by $group_name members only.";
    $guidelines = venture_render_field($group, 'field_group_guidelines');
    $cancel_path = "group/{$group->nid}";
  }
  else if (is_numeric(arg(1))) {
    $deal = node_load(arg(1));
    $company_name = check_plain($deal->title);
    $page_title = "Edit $company_name deal";
    $page_desc = "You may edit your $company_name deal here.";
    $cancel_path = "deal/{$deal->nid}";
  }
  else {
    $page_title = 'Post new public deal';
    $page_desc = 'You may post a new public deal here. It will be accessible by all VentureTap investors.';
    $cancel_path = 'deals';
  }
  
  $page_desc .= ' We encourage you to upload business plans, video pitches, and other appropriate documents.';
  if ($deal) {
    $created = format_date($deal->created, 'custom', 'm/d/Y');
    $title = venture_deal_get_node_title($deal);
    $users = venture_render_field($deal, 'field_deal_users', TRUE);
    $groups = venture_render_field($deal, 'field_deal_groups', TRUE);
    $posted = venture_element_render_deal_posted($deal->nid, $deal->uid, $created, $title, $users, $groups, TRUE);
    $page_desc .= " $posted.";
  }
  
  drupal_set_title($page_title);
  drupal_add_js('$(document).ready(function(){ init_deal_edit() })', 'inline');
  
  $variables = array(
    'form' => $form,
    'page_title' => $page_title,
    'page_desc' => $page_desc,
    'guidelines' => $guidelines,
    'cancel_path' => $cancel_path
  ); 
  return _phptemplate_callback('venture-deal-edit', $variables);
}

/**
* Theme the group edit form
*/
function venture_group_node_form($form) {
  if (is_numeric(arg(1))) {
    $group = node_load(arg(1));
    $group_name = check_plain($group->title);
    $link = l('Click here', 'pm/new/1');
    $page_title = "Edit $group_name";
    $page_desc = "You may edit $group_name information here. Group owners cannot delete or leave 
      their groups. $link to contact the site administrator about deleting a group or transferring
      the group ownership to another user.";
  }
  else {
    $page_title = 'Create new group';
    $page_desc = 'You may create a new group here. Please provide an informative group name as well
      as a detailed group description. As a group owner, you will have administrative privileges
      over group membership, deal flow and content.';
  }
  
  drupal_set_title($page_title);
  drupal_add_js('$(document).ready(function(){ init_group_edit() })', 'inline');
  
  $variables = array(
    'form' => $form,
    'page_title' => $page_title,
    'page_desc' => $page_desc,
    'cancel_path' => $group ? "group/{$group->nid}" : 'groups'
  ); 
  return _phptemplate_callback('venture-group-edit', $variables);
}

/**
* Theme the group post edit form
*/
function venture_group_post_node_form($form) {
  global $user, $image_path;
  
  if (arg(2) == 'post') {
    // Create new topic
    $group = node_load(arg(1));
    $group_name = check_plain($group->title);
    
    $title = 'Start new topic';
    $page_desc = "You may create a new topic in $group_name here.";
    $cancel_path = 'group/' . arg(1);
  }
  else {
    // Edit message
    $title = 'Edit group message';
    $page_desc = 'You may modify your existing group message here.';
    $cancel_path = 'post/' . arg(1);
  }
  
  drupal_set_title($title);
  
  $rendered_form = drupal_render($form['title']) . drupal_render($form['body_filter']);
  $rendered_form .= '<div id="actions">' . drupal_render($form) . l('Cancel', $cancel_path) . '</div>';
  
  $desc = "<img src='$image_path/post-edit.png' alt='' title='' />";
  $desc .= "<p class='page-desc'>$page_desc</p>";
  $output = "<div class='page-title'><h1>$title</h1></div>";
  $output .= "<div class='desc-container'>$desc</div>";
  $output .= "<div id='edit-container'>$rendered_form</div>";
  $output = "<div id='post-edit'>$output</div>";
  
  return $output;
}

/**
* Theme the invite form
*/
function venture_invite_form($form) {
  drupal_add_js('$(document).ready(function(){ init_invite() })', 'inline');
  
  $variables = array('form' => $form); 
  return _phptemplate_callback('venture-invite', $variables);
}

/**
* Theme the private message list form (folder section)
*/
function venture_privatemsg_cur_folder_form($form) {
  global $image_path;
  
  // Set the title based on folder
  $folder = $form['#parameters'][2];
  $title = $form['header']['folder_select']['#options'][$folder];  
  
  drupal_set_title($title);
  
  if ($folder == PRIVATEMSG_FOLDER_INBOX) {
    // Update the title
    $new_msg = venture_pm_get_msg_count();
    if ($new_msg > 0) {
      $title .= ($new_msg == 1) ? ' (1 new message)' : " ($new_msg new messages)";
    }
    $page_desc = 'You may view incoming private messages here. Deleted messages are moved to the Recycle Bin folder.';
  }
  else if ($folder == PRIVATEMSG_FOLDER_SENT) {
    $page_desc = 'You may view your previously sent private messages here. Deleted messages are moved to the Recycle Bin folder.';
  }
  else if ($folder = PRIVATEMSG_FOLDER_RECYCLE_BIN) {
    $page_desc = 'You may view deleted private messages here. They can be restored or deleted permanently.';
  }  

  $select = drupal_render($form);
  $links = l('New message', 'pm/new') . l('Feedback', 'pm/new/1');
  
  $desc = "<img src='$image_path/pm-list.png' alt='' title='' />";
  $desc .= "<div class='page-desc'>$page_desc<div>$links</div></div>";
  $output = "<div class='page-title'><h1>$title</h1>$select</div>";
  $output .= "<div class='desc-container'>$desc</div>";
  $output = "<div id='pm-list-folder'>$output</div>";
  
  return $output;
}

/**
* Theme the private message list form (message section)
*/
function venture_privatemsg_list_form($form) {
  global $user;
  
  drupal_add_js('$(document).ready(function(){ init_pm_list() })', 'inline');
  
  // Iterate over all messages
  $msg_num = 0;
  $folder = $form['messages']['current_folder']['#value'];
  foreach($form['messages'] as $id => &$msg) {
    if (is_numeric($id)) {
      $msg_num++;
      $message = $msg['message']['#value'];
      $author->uid = $message->author;
      if ($folder == PRIVATEMSG_FOLDER_RECYCLE_BIN) {
        // In Recycle Bin always link to author
        $msg['user']['#value'] = theme('username', $author);
      }
      $msg['user']['#value'] = "<div class='name'>{$msg['user']['#value']}</div>";
      $subject = l($message->subject, 'pm/view/'. $id);
      if ($msg['#new'] && ($folder === PRIVATEMSG_FOLDER_INBOX || $author->uid != $user->uid)) {
        // Properly mark the message as new
        $subject = "<strong>$subject</strong> " . theme('mark');
      }
      $msg['subject']['#value'] = "<div class='subject'>$subject</div>";
      $msg['date']['#value'] = format_date($message->timestamp, 'custom', 'm/d/Y - g:i A');
    }
  }
  
  $tags = array('single' => 'Message', 'plural' => 'Messages');
  $form['pager']['#value'] = theme('pager', $tags, variable_get('privatemsg_per_page', 10));
  
  $output = $msg_num ? drupal_render($form) : 'No messages.';
  $output = "<div id='pm-list-msg'>$output</div>";
  
  // Remove all attributes on TD - width and nowrap are not
  // XHTML Strict compliant, active class is unnecessary
  $output = preg_replace('!<td.*?>!', '<td>', $output);
  
  return $output;
}

/**
* Theme the new message form
*/
function venture_privatemsg_new_form($form) {
  global $image_path;
  
  if (arg(0) == 'group') {
    // Message group members
    $group = node_load(arg(1));
    $group_title = check_plain($group->title);
    $title = 'Contact group members';
    $page_desc = "You may compose a new private message to members of $group_title here. Please use this feature sparingly.";
  }
  else {
    // Message user
    $title = 'Compose new message';
    $page_desc = 'You may compose a new private message to one of your connections here. If you would like to contact
      an out-of-network user, you may do so directly from his or her profile.';
  }
  
  $recipient = &$form['header']['recipient'];
  if ($recipient['#type'] == 'select' && count($recipient['#options']) < 2) {
    $rendered_form = 'No connections.' . l('Return to Inbox', 'pm', array('id' => 'return'));
  }
  else if (arg(0) == 'group' && intval($form['header']['members']['#value']) < 2) {
    $rendered_form = 'No members to contact.' . l("Return to {$group->title}", "group/{$group->nid}", array('id' => 'return'));
  }
  else {
    if (arg(0) == 'group') {
      $members = l("Members of $group_title", "group/{$group->nid}/users", array(), NULL, NULL, FALSE, TRUE);
      $recipient['#value'] = "<div id='recipient'>$members</div>";
      $form['cancel']['#value'] = l('Cancel', "group/{$group->nid}");
    }
    else {
      if ($recipient['#type'] == 'hidden') {
        $account = $form['header']['account']['#value'];
        $recipient['#prefix'] = '<div id="recipient">' . theme('username', $account) . '</div>';
        
        if (arg(1) == 'new' && venture_profile_is_admin($recipient['#value'])) {
          // Sending message to an admin
          $title = 'Contact site administrator';
          $page_desc = 'You may compose a new private message to a VentureTap administrator here. Your feedback is very
            important to us, so please do not hesitate if you have questions or comments.';
        }
        else if (arg(1) == 'new') {
          // Sending message to out-of-network user
          $page_desc = 'You may compose a new private message here.';
        }
        else if (arg(1) == 'reply') {
          // Sending reply
          $title = 'Compose reply';
          $page_desc = 'You may reply to a private message here.';
        }
      }
    }
    
    $form['cancel']['#prefix'] = '<div id="cancel">';
    $form['cancel']['#suffix'] = '</div>';
    
    $rendered_form = drupal_render($form);
  }
  
  drupal_set_title($title);
  
  $desc = "<img src='$image_path/pm-new.png' alt='' title='' />";
  $desc .= "<p class='page-desc'>$page_desc</p>";
  $output = "<div class='page-title'><h1>$title</h1></div>";
  $output .= "<div class='desc-container'>$desc</div>";
  $output .= "<div id='pm-new-form'>$rendered_form</div>";
  $output = "<div id='pm-new'>$output</div>";
  
  return $output;
}

/**
* Theme the account form
*/
function venture_profile_node_form($form) {
  drupal_set_title('My account');
  drupal_add_js('$(document).ready(function(){ init_account_profile() })', 'inline');
  
  $variables = array('form' => $form); 
  return _phptemplate_callback('venture-account', $variables);
}

/**
* Theme the search block form
*/
function venture_search_block_form($form) {
  global $image_path;
  
  $form['submit'] = array(
    '#value' => "<input id='search-btn' type='image' src='$image_path/find.png' alt='Find' title='Find' />"
  );
  
  return drupal_render($form);
}

/**
* Theme the search form
*/
function venture_search_form($form) {
  drupal_add_js('$(document).ready(function(){ init_search() })', 'inline');
  
  $variables = array('form' => $form); 
  return _phptemplate_callback('venture-search-filters', $variables);
}

/**
* Theme the login form
*/
function venture_user_login($form) {
  drupal_set_title('Sign in');
  $variables = array('form' => $form); 
  return _phptemplate_callback('venture-login', $variables);
}

/**
* Theme the password assistance form
*/
function venture_user_pass($form) {
  $output = '<h1>Password assistance</h1>Please provide the email address you use to
    sign in to VentureTap. You will receive a link to reset your password shortly.';
  $output .= drupal_render($form['name']);
  $actions = drupal_render($form) . l('Cancel', 'login');
  $output .= "<div id='actions'>$actions</div>";
  
  drupal_set_title('Password assistance');
  return $output;
}

/**
* Theme the password reset form
*/
function venture_user_pass_reset($form) {
  $output = '<h1>Reset password</h1>Unable to sign in? Use this page to assign a new password to your account.';
  $output .= drupal_render($form['pass']);
  $actions = drupal_render($form) . l('Cancel', 'login');
  $output .= "<div id='actions'>$actions</div>";
  $output = "<div id='user-pass'>$output</div>";
  
  return $output;
}

/**
* Theme the register form
*/
function venture_user_register($form) {
  drupal_set_title('Register');
  drupal_add_js('$(document).ready(function(){ init_register_profile() })', 'inline');
  
  $variables = array('form' => $form); 
  return _phptemplate_callback('venture-register', $variables);
}

/**
* Theme the Views filters form
*/
function venture_views_filters($form) {
  $view = $form['view']['#value'];
  
  // Add titles to the filters
  foreach ($view->exposed_filter as $count => $filter) {
    $form["filter$count"]['#title'] = $filter['label'];
  }
  
  // make the 'q' come first
  $output = drupal_render($form['q']);
  
  $handler = "venture_views_filters_{$view->name}";
  if (function_exists($handler)) {
    $output .= $handler($form);
  }
  else {
    $output .= drupal_render($form);
  }
  
  return $output;
}

/**
* Theme the filters form for the deals view
*/
function venture_views_filters_deals($form) {
  // Figure out the search caption
  $caption = 'all';
  if (arg(1) == 'group' || arg(1) == 'received' || arg(1) == 'submitted') {
    $caption = arg(1);
  }
  $form['filter0']['#title'] = "Search $caption deals";
  
  // Reassign caption to operator
  $form['op2']['#title'] = $form['filter2']['#title'];
  $form['filter2']['#field_prefix'] = '<label>&nbsp;</label>';
  unset($form['filter2']['#title']);
  
  // Format multi-select select boxes
  for ($i = 7; $i < 11; $i++) {
    $form["filter$i"]['#size'] = 5;
  }
  
  // Determine if the advanced filters should be displayed
  $filters = views_get_filter_values();
  $advanced = $filters && (count($filters) > 7 || !empty($filters[1]['filter']) ||
    !empty($filters[2]['filter']) || $filters[3]['filter'] != '**ALL**' || $filters[4]['filter'] != '**ALL**' ||
    $filters[5]['filter'] != '**ALL**' || !empty($filters[6]['filter']));

  $variables = array(
    'form' => $form,
    'advanced' => $advanced
  ); 
  return _phptemplate_callback('venture-deals-filters', $variables);
}

/**
* Theme the filters form for the groups view
*/
function venture_views_filters_groups($form) {
  global $user;
  
  // Figure out the search caption
  if (arg(1) == 'all') {
    $search = 'Search all groups';
  }
  else if (is_numeric(arg(1)) && arg(1) != $user->uid) {
    $search = 'Search user groups';
  }
  else {
    $search = 'Search my groups';
  }
  $form['filter0']['#title'] = $search;
  
  // Determine if the advanced filters should be displayed
  $filters = views_get_filter_values();
  $advanced = $filters && ($filters[1]['filter'] != '**ALL**' || $filters[2]['filter'] != '**ALL**' ||
     $filters[3]['filter'] != '**ALL**' || $filters[4]['filter'] != '**ALL**');
    
  $variables = array(
    'form' => $form,
    'advanced' => $advanced
  ); 
  return _phptemplate_callback('venture-groups-filters', $variables);
}

/**
* Theme the filters form for the network view
*/
function venture_views_filters_network($form) {
  global $user;
  
  // Figure out the search caption
  if (arg(1) == 'group' && is_numeric(arg(2))) {
    $search = 'Search group members';
  }
  else if (arg(1) == 'all') {
    $search = 'Search all users';
  }
  else if (is_numeric(arg(1)) && arg(1) != $user->uid) {
    $search = 'Search user network';
  }
  else {
    $search = 'Search my network';
  }
  $form['filter0']['#title'] = $search;
  
  // Format multi-select select boxes
  for ($i = 4; $i < 12; $i++) {
    $form["filter$i"]['#size'] = 5;
  }
  
  // Determine if the advanced filters should be displayed
  $filters = views_get_filter_values();
  $advanced = $filters && (count($filters) > 4 || $filters[1]['filter'] != '**ALL**' ||
    $filters[2]['filter'] != '**ALL**' || !empty($filters[3]['filter']));
  
  $variables = array(
    'form' => $form,
    'advanced' => $advanced
  ); 
  return _phptemplate_callback('venture-network-filters', $variables);
}

/**
* Theme the filters form for the topics view
*/
function venture_views_filters_topics($form) {
  $output = drupal_render($form);
  $output = "<div class='search-basic'>$output</div>";
  return $output;
}