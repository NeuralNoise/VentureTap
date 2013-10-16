<?php

/**
* Theme the about us page
*/
function venture_about() {
  drupal_set_title('About us');
  return _phptemplate_callback('venture-about');
}

/**
* Theme the blogs page
*/
function venture_blogs() {
  drupal_set_title('Blogs');
  return _phptemplate_callback('venture-blogs');
}

/**
* Theme the contact page
*/
function venture_contact() {
  $output = contact_site_page();
  $output = "<div id='site-contact'><h1>Contact us</h1>$output</div>";
  return $output;
}

/**
* Theme the deal view page
*/
function venture_deal_view() {
  global $user;
  
  $deal = node_load(arg(1));
  if (!node_access('view', $deal) || !venture_deal_is_node_accessible($deal)) {
    drupal_goto('deals');
  }
  
  $title = check_plain($deal->title);
  $amount = venture_render_field($deal, 'field_deal_amount_seeking');
  
  drupal_set_title("$title seeking $$amount");
  drupal_add_js('$(document).ready(function(){ init_deal_view() })', 'inline');
  venture_enable_media();
  
  $variables = array(
    'user' => $user,
    'deal' => $deal,
    'title' => $title,
    'amount' => $amount
  );
  return _phptemplate_callback('venture-deal-view', $variables);
}

/**
* Theme the deals page
*/
function venture_deals() {
  global $user, $image_path;

  $tabs = array(
    'All Deals' => array(
      'url' => 'deals',
      'args' => array('all', 'all', 'all')
    ),
    'Group Deals' => array(
      'url' => 'deals/group',
      'args' => array('all', 'all', array_keys($user->og_groups))
    ),
    'Received Deals' => array(
      'url' => 'deals/received',
      'args' => array('all', $user->uid, 'all')
    ),
    'Submitted Deals' => array(
      'url' => 'deals/submitted',
      'args' => array($user->uid, 'all', 'all')
    )
  );
  
  if (arg(1) == 'submitted') {
    $tabs['Submitted Deals']['class'] = 'ui-tabs-selected';
  }
  else if (arg(1) == 'received') {
    $tabs['Received Deals']['class'] = 'ui-tabs-selected';
  }
  else if (arg(1) == 'group') {
    $tabs['Group Deals']['class'] = 'ui-tabs-selected';

    $groups = array();
    foreach(array_keys($user->og_groups) as $gid) {
      $group = node_load($gid);
      if ($group->og_selective == OG_MODERATED) {
        $groups[] = $gid;
      }
    }
  }
  else {
    $tabs['All Deals']['class'] = 'ui-tabs-selected';
  }
  
  $title = 'Deal dashboard';
  $page_desc = 'You may view investment deals here.';
  
  if (views_get_filter_values()) {
    $title .= ' (search results)';
    $page_desc = 'You may view the deal search results here.';
  }
  
  $page_desc .= ' Navigate the tabs to access public deals, track the deals you submitted as well as review deals
    submitted directly to you or any moderated groups you belong to (open groups cannot accept deal flow). ';
  
  if (!venture_profile_is_accredited()) {
    $page_desc .= 'Only deals outside of the United States are accessible, because
      you registered as a non-accredited investor.';
  }
    
  drupal_set_title($title);
  drupal_add_js('$(document).ready(function(){ init_deals() })', 'inline');

  $variables = array(
    'user' => $user,
    'image_path' => $image_path,
    'title' => $title,
    'page_desc' => $page_desc,
    'tabs' => $tabs,
    'groups' => $groups
  );
  return _phptemplate_callback('venture-deals', $variables);
}

/**
* Theme the 404 error page
*/
function venture_error() {
  global $user;
  
  // Avoids infinite loop with drupal_goto during login
  unset($_REQUEST['destination']);
  
  if ($_REQUEST['q'] == 'login') {
    // When logged in, redirect login page to home
    drupal_goto();
  }
  else if (arg(1) == '403' && !$user->uid) {
    // Display login page for unathenticated users
    $output = drupal_get_form('user_login');
    drupal_set_message('Please sign in below in order to access this page.', 'error');
    return $output;
  }
  else if (arg(1) == '403') {
    $title = 'Access denied';
    $error = 'You are not authorized to access this page.';
  }
  else if (arg(1) == '404') {
    $title = 'Page not found';
    $error = 'The requested page could not be found.';
  }
  
  drupal_set_title($title);
  drupal_set_message('The website encountered an unexpected problem.', 'error');
  $contact = $user->uid ? l('contact', 'pm/new/1') : l('contact', 'contact');
  
  $output .= "<h1>What happened:</h1><p>$error</p><h1>How this will affect you:</h1><p>The requested page will not be retrieved.</p>";
  $output .= "<h1>What you can do about it:</h1><p>Close your browser, navigate back to the VentureTap website, and try repeating " .
    "your last action.<br/>Try alternative methods of performing the same action. If problems persist, $contact support.</p>";
  $output = "<div id='user-error'>$output</div>";
  
  return $output;
}

/**
* Theme the group post view page
*/
function venture_group_post_view() {
  global $image_path;
  
  $post = node_load(arg(1));
  $gid = $post->og_groups[0];
  $group = node_load($gid);
  $group_title = $post->og_groups_both[$gid];
  
  // Render first message
  $first = venture_element_render_group_post($group, $post->nid, NULL, $post->uid,
      $post->created, $post->title, $post->body, $post->format);
      
  // Render replies
  $comments_per_page = variable_get('comment_default_per_page', '10');
  $comments = venture_get_comments($post->nid, $comments_per_page);
  foreach ($comments as $comment) {
    $replies .= venture_element_render_group_post($group, $post->nid, $comment->cid, $comment->uid,
      $comment->timestamp, $comment->subject, $comment->comment, $comment->format);
  }
  
  // Theme pager
  $link = l('Reply to message', "post/{$post->nid}/reply");
  $tags = array('single' => 'Reply', 'plural' => 'Replies', 'link' => $link);
  $pager = theme('pager', $tags, $comments_per_page);
  $replies = $pager . $replies . $pager;
  if (!$replies) {
    $replies = 'No replies. ' . l('Click here', "post/{$post->nid}/reply") . ' to reply to this message.';
    $replies = "<div id='no-results'>$replies</div>";
  }
  
  drupal_set_title(check_plain($post->title));
  drupal_add_js('$(document).ready(function(){ init_post_view() })', 'inline');
  
  $title = 'View group message';
  $name = venture_profile_get_name($post->uid);
  $page_desc = "You may view $name's message along with responses here.";
  $desc_link = l("Back to $group_title", "group/$gid");

  $desc = "<img src='$image_path/post-view.png' alt='' title='' />";
  $desc .= "<div class='page-desc'><p>$page_desc</p>$desc_link</div>";
  $output = "<div class='page-title'><h1>$title</h1></div>";
  $output .= "<div class='desc-container'>$desc</div>";
  $output .= "<div id='posts'>$first $replies</div>";
  $output = "<div id='post-view'>$output</div>";
  
  return $output;
}

/**
* Theme the group view page
*/
function venture_group_view() {
  $group = node_load(arg(1));
  
  drupal_set_title(check_plain($group->title));
  drupal_add_js('$(document).ready(function(){ init_group_view() })', 'inline');
  
  $variables = array(
    'group' => $group,
    'gid' => $group->nid,
    'member_status' => venture_group_get_status($group->nid),
    'is_admin' => venture_profile_is_admin()
  );
  return _phptemplate_callback('venture-group-view', $variables);
}

/**
* Theme the groups page
*/
function venture_groups() {
  global $user, $image_path;
  
  $desc_link = l('Create new group', 'group/new');
  
  if (arg(1) == 'all') {
    // Show all groups
    $view_arg = 'all';
    $title = 'All groups';
    $page_desc = 'You may view all groups here.';
    $other = 'your own';
    $selected = 'selected="selected"';
  }
  else if (is_numeric(arg(1)) && arg(1) != $user->uid) {
    // Show another user's groups
    $view_arg = arg(1);
    $name = venture_profile_get_name(arg(1));
    $title = "$name's groups";
    $page_desc = "You may view $name's groups here.";
    $other = 'your own or all';
    $desc_link = l('Back to profile', "user/$view_arg");
    $user_url = url("groups/$view_arg");
    $display = "<option value='$user_url'>User Groups</option>";
    
    if (!venture_profile_is_valid(arg(1))) {
      return drupal_access_denied();
    }
  }
  else {
    // Show my groups
    $view_arg = venture_profile_is_admin() ? 'all' : $user->uid;
    $title = 'My groups';
    $page_desc = 'You may view your groups here.';
    $other = 'all';
  }
  
  if (views_get_filter_values()) {
    $title .= ' (search results)';
    $page_desc = 'You may view the group search results here.';
  }

  drupal_add_js('$(document).ready(function(){ init_groups() })', 'inline');
  drupal_set_title($title);
  
  $page_desc .= " To see $other groups, select the appropriate view in the drop-down list above.";
    
  $my_url = url('groups');
  $all_url = url('groups/all');
  $display .= "<option value='$my_url'>My Groups</option><option value='$all_url' $selected>All Groups</option>";
  $display = "<select id='current-view'>$display</select>";
  $display = "<div class='current-view'><label>Current view: </label>$display</div>";
  
  $groups = theme('view', 'groups', 10, TRUE, 'embed', array($view_arg));
  
  $desc = "<img src='$image_path/groups.png' alt='' title='' />";
  $desc .= "<div class='page-desc'><p>$page_desc</p>$desc_link</div>";
  $output = "<div class='page-title'><h1>$title</h1>$display</div>";
  $output .= "<div class='desc-container'>$desc</div>";
  $output .= "<div id='groups'>$groups</div>";
  $output = "<div id='user-groups'>$output</div>";
  
  return $output;
}

/**
* Theme the home page
*/
function venture_home() {
  global $user, $script_path;

  if ($user->uid) {
    $uid = is_numeric(arg(1)) ? arg(1) : $user->uid;
    $account = user_load(array('uid' => $uid));
    $is_admin = venture_profile_is_admin($user);
    
    if (!$account || (arg(1) && !$is_admin)) {
      return drupal_not_found();
    }
    
    drupal_add_js('$(document).ready(function(){ init_home() })', 'inline');
    
    $variables = array(
      'account' => $account,
      'is_admin' => $is_admin,
      'buddies' => buddy_api_get_buddies($account->uid),
      'network_view' => views_get_view('network_all'),
      'group_view' => views_get_view('groups_all'),
      'topic_view' => views_get_view('topics_all'),
      'deal_view' => views_get_view('deals_all')
    );
    return _phptemplate_callback('venture-home', $variables);
  }
  else {
    drupal_set_title('Angel investor and VC network');

    drupal_add_js("$(document).ready(function(){ init_front() })", 'inline');

    $variables = array(
      'map_path' => "$script_path/world.swf?data_file=" . venture_geo_get_map_path()
    );
    return _phptemplate_callback('venture-front', $variables);
  }
}

/**
* Theme the invite history page
*/
function venture_invite_history() {
  global $image_path;
  
  drupal_add_js('$(document).ready(function(){ init_invite_history() })', 'inline');

  $items = venture_invite_get_all();
  if (count($items) > 0) {
    $rows = array();
    foreach($items as $item) {
      $row = array();
      $row[] = $item['date'];
      $row[] = "<div class='email'>{$item['email']}</div>";
      
      // Set up the table rows
      if ($item['invitee']) {
        $row[] = array('data' => 'Accepted', 'class' => 'accepted');
        if ($item['invitee_status']) {
          $row[] = l('View profile', "user/{$item['invitee']}"); 
        }
        else {
          $row[] = '<em>User Removed</em>';
        }
        $row[] = l('Delete', "invite/remove/{$item['reg_code']}", array('class' => 'delete'));
      }
      else {
        if ($item['expired']) {
          $row[] = array('data' => 'Expired', 'class' => 'expired');
          $row[] = l('Resend', "invite/resend/{$item['reg_code']}");
          $row[] = l('Delete', "invite/remove/{$item['reg_code']}", array('class' => 'delete'));
        }
        else {
          $row[] = array('data' => 'Pending', 'class' => 'pending');
          $row[] = l('Withdraw', "invite/remove/{$item['reg_code']}", array('class' => 'withdraw'));
          $row[] = '';
        }
      }
      $rows[] = $row;
    }
    
    $headers = array('Date', 'Email', array('data' => 'Status', 'colspan' => 3));
    $invites .= theme('table', $headers, $rows);
  }
  else {
    $invites .= 'No invitations.';
  }
  $invites .= l('Return to main invite page', 'invite', array('id' => 'return'));
  
  $title = 'Invitation history';
  $page_desc = 'You may view your previously sent invitations here. Withdrawing an outstanding invitation will prevent the recipient from registering. Invitations expire after 30 days.';
  
  $desc = "<img src='$image_path/invite-history.png' alt='' title='' />";
  $desc .= "<p class='page-desc'>$page_desc</p>";
  $output = "<div class='page-title'><h1>$title</h1></div>";
  $output .= "<div class='desc-container'>$desc</div>";
  $output .= "<div id='invites'>$invites</div>";
  $output = "<div id='invite-history'>$output</div>";

  return $output;
}

/**
* Theme the legal page
*/
function venture_legal() {
  drupal_set_title('Legal');
  return _phptemplate_callback('venture-legal');
}

/**
* Theme the maintenance page
*/
function venture_maintenance_page($content) {
  drupal_set_title('Site offline');
  
  $content = "<h1>Site is down for maintenance</h1>$content";
  $output = "<div id='offline'>VentureTap is offline</div>";
  $output = "<div id='site-maint'>$output<div id='offline-msg'>$content</div></div>";
  
  return theme('page', $output);
}

/**
* Theme the network page
*/
function venture_network() {
  global $user, $image_path;
  
  $arg_user = $arg_group = 'all';
  $desc_link = l('Grow my network', 'invite');
  
  if (arg(1) == 'group' && is_numeric(arg(2))) {
    // Show group members
    $arg_group = arg(2);
    $group = node_load(arg(2));
    $group_name = check_plain($group->title);
    $title = "Members of $group_name";
    $page_desc = "You may view all members of $group_name here.";
    $other = 'your connections or all users';
    $search_desc = 'group member';
    $desc_link = l("Back to {$group->title}", "group/$arg_group");
    $group_url = url("group/$arg_group/users");
    $display = "<option value='$group_url'>Group Members</option>";
    
    if (!og_is_group_type($group->type)) {
      return drupal_not_found();
    }
  }
  else if (arg(1) == 'all') {
    // Show all users
    $title = 'All users';
    $page_desc = 'You may view all users here.';
    $other = 'your connections';
    $selected = 'selected="selected"';
    $search_desc = 'user';
  }
  else if (is_numeric(arg(1)) && arg(1) != $user->uid) {
    // Show another user's connections
    $arg_user = arg(1);
    $name = venture_profile_get_name(arg(1));
    $title = "$name's network";
    $page_desc = "You may view $name's connections here.";
    $other = 'your connections or all users';
    $search_desc = 'user network';
    $desc_link = l('Back to profile', "user/$arg_user");
    $user_url = url("network/$arg_user");
    $display = "<option value='$user_url'>User Network</option>";
    
    if (!venture_profile_is_valid(arg(1))) {
      return drupal_access_denied();
    }
  }
  else {
    // Show my connections
    if (!venture_profile_is_admin()) {
      $arg_user = $user->uid;
    }
    $title = 'My network';
    $page_desc = 'You may view your connections here.';
    $other = 'all users';
    $search_desc = 'network';
  }
  
  if (views_get_filter_values()) {
    $title .= ' (search results)';
    $page_desc = "You may view the $search_desc search results here.";
  }

  drupal_add_js('$(document).ready(function(){ init_network() })', 'inline');
  drupal_set_title($title);

  $page_desc .= " To see $other, select the appropriate view in the drop-down list above.";
    
  $my_url = url('network');
  $all_url = url('network/all');
  $display .= "<option value='$my_url'>My Network</option><option value='$all_url' $selected>All Users</option>";
  $display = "<select id='current-view'>$display</select>";
  $display = "<div class='current-view'><label>Current view: </label>$display</div>";
  
  $network = theme('view', 'network', 10, TRUE, 'embed', array($arg_user, $arg_group));
  $network .= '<div id="invite">Did not find who you were looking for? Get involved in growing our community
    by inviting your ' . l('associates and business partners', 'invite') . ' to VentureTap!</div>';
  
  $desc = "<img src='$image_path/network.png' alt='' title='' />";
  $desc .= "<div class='page-desc'><p>$page_desc</p>$desc_link</div>";
  $output = "<div class='page-title'><h1>$title</h1>$display</div>";
  $output .= "<div class='desc-container'>$desc</div>";
  $output .= "<div id='network'>$network</div>";
  $output = "<div id='user-network'>$output</div>";
  
  return $output;
}

/**
* Theme the view message page
*/
function venture_privatemsg_view($message) {
  global $image_path;
  
  drupal_set_title('View message');
  drupal_add_js('$(document).ready(function(){ init_pm_view() })', 'inline');
  
  $variables = array(
    'image_path' => $image_path,
    'message' => $message,
    'current_folder' => venture_pm_get_current_folder($message)
  ); 
  return _phptemplate_callback('venture-pm-view', $variables);
}

/**
* Theme the search page (customized search_view())
*/
function venture_search_view() {
  global $image_path;
  $type = arg(1);

  // Search form submits with POST but redirects to GET. This way we can keep
  // the search query URL clean as a whistle:
  // search/type/keyword+keyword
  if (!isset($_POST['form_id'])) {
    if ($type == '') {
      // Note: search/node can not be a default tab because it would take on the
      // path of its parent (search). It would prevent remembering keywords when
      // switching tabs. This is why we drupal_goto to it from the parent instead.
      drupal_goto('search/node');
    }

    $keys = search_get_keys();
    // Only perform search if there is non-whitespace search term:
    if (trim($keys)) {
      // Log the search keys:
      watchdog('search', t('%keys (@type).', array('%keys' => $keys, '@type' => module_invoke($type, 'search', 'name'))), WATCHDOG_NOTICE, l(t('results'), 'search/'. $type .'/'. $keys));

      // Collect the search results:
      $results = search_data($keys, $type);
      
      $title = 'Search results';
      $page_desc = 'You may view your search results here. ';
    }
    else {
      $title = 'Search VentureTap';
    }
    
    if (!$results) {
      // Add query examples
      $examples  = '<li><em>angel investor</em> will return all results with both keywords</li>';
      $examples .= '<li><em>angel OR investor</em> will return all results with either keyword</li>';
      $examples .= '<li><em>"angel investor"</em> will return all results with the exact phrase</li>';
      $examples .= '<li><em>angel -investor</em> will return all results with <em>angel</em>
        but not <em>investor</em> keywords</li>';
      
      $results = trim($keys) ? 'No results.<br/><br/>' : '';
      $results .= 'You may use the following operators to construct an advanced search query:';
      $results = "<div id='no-results'>$results<ul>$examples</ul></div>";
    }

    // Construct the search form.
    $search = drupal_get_form('search_form', NULL, $keys, $type);
    $search .= $results;
  }
  else {
    $search = drupal_get_form('search_form', NULL, $keys, $type);
  }

  $page_desc .= 'Search all of the VentureTap content by entering keywords in the search box below
    or drill down further with advanced filters.';
  
  $desc = "<img src='$image_path/search.png' alt='' title='' />";
  $desc .= "<div class='page-desc'>$page_desc</div>";
  $output = "<div class='page-title'><h1>$title</h1></div>";
  $output .= "<div class='desc-container'>$desc</div>";
  $output .= "<div id='site-search'>$search</div>";
  
  return $output;
}

/**
* Theme the profile page
*/
function venture_user_profile($account, $fields) {
  if (!venture_profile_is_valid($account)) {
    return drupal_access_denied();
  }
  
  // Set the page title
  $account_profile = venture_profile_retrieve($account->uid);
  $name = venture_profile_get_name($account_profile);
  drupal_set_title($name);
  
  drupal_add_js('$(document).ready(function(){ init_profile() })', 'inline');
  
  $GLOBALS['account'] = $account;
  $variables = array(
    'account_profile' => $account_profile,
    'conn_status' => venture_network_get_status($account->uid)
  ); 

  return _phptemplate_callback('venture-profile', $variables);
}