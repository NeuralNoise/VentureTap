<?php

/**
* Theme the deals view
*/
function venture_views_view_deals($view, $type, $nodes, $level = NULL, $args = NULL) {
  global $image_path;
  $view->url = $_GET['q'];
  
  foreach($nodes as $idx => $node) {
    $i = 0;
    
    $title = venture_render_view_field($view, $node, $i++);
    $created = venture_render_view_field($view, $node, $i++);
    $amount = venture_render_view_field($view, $node, $i++);
    $headline = venture_render_view_field($view, $node, $i++);
    $country = venture_render_view_field($view, $node, $i++);
    $state = venture_render_view_field($view, $node, $i++);
    $city = venture_render_view_field($view, $node, $i++);
    $industry = venture_render_view_field($view, $node, $i++);
    $funding_stage = venture_render_view_field($view, $node, $i++);
    $funding_type = venture_render_view_field($view, $node, $i++);
    $investors = venture_render_view_field($view, $node, $i++);
    $attachments = venture_render_view_field($view, $node, $i++);
    $users = venture_render_view_field($view, $node, $i++);
    $groups = venture_render_view_field($view, $node, $i++);
    
    $company = $title = venture_deal_get_title($node->users_uid, $title, $country, $state);
    $details = l('view details', "deal/{$node->nid}", array('class' => 'deal-details'));
    $location = venture_geo_get_location($city, $state, $country);
    $investors = join(', ', $investors);
    $zebra = $idx % 2 ? 'even' : 'odd';
    
    if (venture_deal_is_accessible($node->users_uid, $country)) {
      $author = $node->users_uid;
      $company = l($title, "deal/{$node->nid}", array(), NULL, NULL, FALSE, TRUE);
      $accredited = '';
    }
    else {
      $accredited = ' (accredited investors only)';
      $author = '';
      $headline = '';
      $details = '';
    }
    
    $summary = array(
      array('Location' => $location, 'Industry' => $industry),
      array('Funding Stage' => $funding_stage, 'Funding Type' => $funding_type),
      array('Prior Investors' => $investors, 'Attachments' => count($attachments))
    );
    
    // Render summary sections
    $tables = NULL;
    foreach ($summary as $idx => $section) {
      $rows = NULL;
      foreach ($section as $field => $value) {
        $rows .= "<tr><td class='summary-title'>$field:</td><td><div class='summary-value'>$value</div></td></tr>";
      }
      $tables .= "<table class='section-$idx'>$rows</table>";
    }
    
    $posted = venture_element_render_deal_posted($node->nid, $author, $created, $title, $users, $groups);

    $top = "$company seeking <strong>$$amount</strong><span>$accredited</span>";
    $top = "<div class='deal-top'><div>$top</div>$details</div>";
    $sections = "<div class='deal-summary'>$tables</div>";    
    $output .= "<div class='deal $zebra'>$top $headline $sections $posted</div>";
  }

  if ($output) {
    $output = "<div id='deals'>$output</div>";
  
    if ($view->use_pager) {
      $tags = array('single' => 'Deal', 'plural' => 'Deals');
      $pager = theme('pager', $tags, $view->pager_limit);
      $output = $pager . $output . $pager;
    }
  }
  
  if (!$output) {
    $output = $view->used_filters ? 'No results.' : 'No deals.';
    $output = "<div id='no-results'>$output</div>";
  }
  
  $output = views_theme('views_display_filters', $view) . $output;
  
  return $output;
}

/**
* Theme the deals_all view
*/
function venture_views_view_deals_all($view, $type, $nodes, $level = NULL, $args = NULL) {
  foreach($nodes as $node) {
    $i = 0;
    
    $title = venture_render_view_field($view, $node, $i++);
    $amount = venture_render_view_field($view, $node, $i++);
    $funding_type = venture_render_view_field($view, $node, $i++);
    $country = venture_render_view_field($view, $node, $i++);
    $state = venture_render_view_field($view, $node, $i++);
    $city = venture_render_view_field($view, $node, $i++);
    $latest_date = venture_render_view_field($view, $node, $i++);
    
    // Format fields
    $funding_type = strtolower($funding_type);
    $location = venture_geo_get_location($city, $state, $country);
    $title = venture_deal_get_title($node->users_uid, $title, $country, $state);
    $seeking = "$title seeking $$amount";
    
    if (venture_deal_is_accessible($node->users_uid, $country)) {
      $title = l($seeking, "deal/{$node->nid}", array(), NULL, NULL, FALSE, TRUE);
      $accredited = '';
    }
    else {
      $title = $seeking;
      $accredited = ' for accredited investors';
    }
    
    if (!$view->latest) {
      $view->latest = $seeking;
      $view->latest_date = $latest_date;
    }
    
    $output .= "<li>$title - $funding_type deal in $location$accredited</li>";
  }
  if ($output) {
    $output = "<ul id='latest-deals'>$output</ul>";
  }
  else {
    $post = l('Post a new deal', 'deal/new');
    $output = "<div id='latest-deals'>No deals found. $post.</div>";
  }
  
  return $output;
}

/**
* Theme the groups view
*/
function venture_views_view_groups($view, $type, $nodes, $level = NULL, $args = NULL) {
  $view->url = $_GET['q'];
  
  foreach($nodes as $node) {
    $i = 0;
    
    $group_name = venture_render_view_field($view, $node, $i++);
    $description = venture_render_view_field($view, $node, $i++);
    $image = venture_render_view_field($view, $node, $i++);
    $members = venture_render_view_field($view, $node, $i++);
    $topics = venture_render_view_field($view, $node, $i++);
    $comments = venture_render_view_field($view, $node, $i++);
    $updated = venture_render_view_field($view, $node, $i++);
    $investor_type = venture_render_view_field($view, $node, $i++);
    $country = venture_render_view_field($view, $node, $i++);
    $state = venture_render_view_field($view, $node, $i++);
    $selective = venture_render_view_field($view, $node, $i++);
    $gid = $node->nid;
    $owner = $node->users_uid;
    
    // Format fields
    $member_status = venture_group_get_status($gid);
    $status_label = venture_element_render_membership_status($member_status);
    $type = ($selective == OG_MODERATED ? 'Moderated' : 'Open');
    $image = venture_element_render_image($image, "group/$gid");
    $details = venture_element_render_group_details($gid, $investor_type, $country, $state, 
      $members, $topics, $comments, $updated);
    $actions = venture_element_render_group_actions($gid, $member_status, $selective, $group_name, $owner);

    $summary = "<div class='record-summary-container'>$description $details</div>";
    $group_name = "<div class='record-name'>$group_name</div>";
    $group_type = "<div class='record-misc'>$type</div>";
    $header = "<div class='record-header'>$group_name $status_label $group_type</div>";
    $main = "<div class='record-main'>$image $summary $actions</div>";
    $output .= "<div class='record'>$header $main</div>";
  }
  
  if ($view->use_pager) {
    $tags = array('single' => 'Group', 'plural' => 'Groups');
    $pager = theme('pager', $tags, $view->pager_limit);
    $output = $pager . $output . $pager;
  }
  
  if (!$output) {
    $output = $view->used_filters ? 'No results.' : 'No groups.';
    $output = "<div id='no-results'>$output</div>";
  }
  
  $output = views_theme('views_display_filters', $view) . $output;
  
  return $output;
}

/**
* Theme the groups_all view
*/
function venture_views_view_groups_all($view, $type, $nodes, $level = NULL, $args = NULL) {
  foreach($nodes as $node) {
    $i = 0;
    
    $group_name = venture_render_view_field($view, $node, $i++, FALSE);
    $investor_type = venture_render_view_field($view, $node, $i++);
    $country = venture_render_view_field($view, $node, $i++);
    $state = venture_render_view_field($view, $node, $i++);
    $selective = venture_render_view_field($view, $node, $i++);
    $latest_date = venture_render_view_field($view, $node, $i++);
    $gid = $node->nid;
    
    // Format fields
    $title = l($group_name, "group/$gid");
    $desc = ($selective == OG_MODERATED ? 'a moderated' : 'an open') . ' group';
    if ($investor_type || $country || $state) {
      $audience = $investor_type ? strtolower($investor_type) . 's' : 'anyone';
      if ($country || $state) {
        $audience .= ' in ' . ($state ? $state : $country);
      }
      $desc .= " for $audience";
    }
    
    if (!$view->latest) {
      $view->latest = $group_name;
      $view->latest_date = $latest_date;
    }
    
    $output .= "<li>$title - $desc</li>";
  }
  if ($output) {
    $output = "<ul id='latest-groups'>$output</ul>";
  }
  
  return $output;
}

/**
* Theme the groups_summary view
*/
function venture_views_view_groups_summary($view, $type, $nodes, $level = NULL, $args = NULL) {
  foreach($nodes as $node) {
    $title = venture_render_view_field($view, $node, 0, FALSE);
    $gid = $node->nid;
    $output .= '<li>' . l($title, "group/$gid") . '</li>';
  }
  if ($output) {
    $output = "<ul>$output</ul>";
  }
  
  if ($view->total_rows > count($nodes)) {
    $total = count($nodes) . ' of ' . $view->total_rows;
  }
  else {
    $total = $view->total_rows . ' total';
  }
  $output .= "<div class='total'><span>$total</span></div>";
  return $output;
}

/**
* Theme the network view
*/
function venture_views_view_network($view, $type, $nodes, $level = NULL, $args = NULL) {
  $view->url = $_GET['q'];
  
  foreach($nodes as $node) {
    $i = 0;
    
    $first_name = venture_render_view_field($view, $node, $i++);
    $last_name = venture_render_view_field($view, $node, $i++);
    $photo = venture_render_view_field($view, $node, $i++);
    $city = venture_render_view_field($view, $node, $i++);
    $state = venture_render_view_field($view, $node, $i++);
    $country = venture_render_view_field($view, $node, $i++);
    $investor_type = venture_render_view_field($view, $node, $i++);
    $funding_stages = venture_render_view_field($view, $node, $i++);
    $industries = venture_render_view_field($view, $node, $i++);
    $invest_prefs = venture_render_view_field($view, $node, $i++);
    $other_prefs = venture_render_view_field($view, $node, $i++);
    $states = venture_render_view_field($view, $node, $i++);
    $countries = venture_render_view_field($view, $node, $i++);
    $min_invest = venture_render_view_field($view, $node, $i++);
    $max_invest = venture_render_view_field($view, $node, $i++);
    $uid = $node->users_uid;
    
    // Format fields
    $conn_status = venture_network_get_status($uid);
    $status_label = venture_element_render_connection_status($conn_status);
    $photo = venture_element_render_image($photo, "user/$uid");
    $location = venture_geo_get_location($city, $state, $country);
    $summary = venture_element_render_investor_summary($investor_type, $funding_stages, $industries,
      $invest_prefs, $other_prefs, $states, $countries, $min_invest, $max_invest);
    $profile = venture_profile_create($uid, $first_name, $last_name);
    $full_name = venture_profile_get_name($profile, FALSE);
    $actions = venture_element_render_investor_actions($uid, $conn_status, $full_name);
    
    $conn_name = "<div class='record-name'>$full_name</div>";
    $conn_location = "<div class='record-misc'>$location</div>";
    $header = "<div class='record-header'>$conn_name $status_label $conn_location</div>";
    $main = "<div class='record-main'>$photo $summary $actions</div>";
    $output .= "<div class='record'>$header $main</div>";
  }
  
  if ($view->use_pager) {
    $tags = array();
    if (is_numeric($args[0])) {
      $tags = array('single' => 'Connection', 'plural' => 'Connections');
    }
    else {
      $tags = array('single' => 'User', 'plural' => 'Users');
    }
    
    $pager = theme('pager', $tags, $view->pager_limit);
    $output = $pager . $output . $pager;
  }
  
  if (!$output) {
    if ($view->used_filters) {
      $output = 'No results.';
    }
    else {
      $output = is_numeric($args[0]) ? 'No connections.' : 'No users.';
    }
    $output = "<div id='no-results'>$output</div>";
  }
  
  $output = views_theme('views_display_filters', $view) . $output;
  
  return $output;
}

/**
* Theme the network_all view
*/
function venture_views_view_network_all($view, $type, $nodes, $level = NULL, $args = NULL) {
  foreach($nodes as $node) {
    $i = 0;
    
    $first_name = venture_render_view_field($view, $node, $i++, FALSE);
    $last_name = venture_render_view_field($view, $node, $i++, FALSE);
    $photo = venture_render_view_field($view, $node, $i++);
    $city = venture_render_view_field($view, $node, $i++);
    $state = venture_render_view_field($view, $node, $i++);
    $country = venture_render_view_field($view, $node, $i++);
    $type = implode(', ', venture_render_view_field($view, $node, $i++));
    $latest_date = venture_render_view_field($view, $node, $i++);
    $uid = $node->users_uid;
    
    // Format fields
    $photo = venture_element_render_image($photo, "user/$uid");
    $location = venture_geo_get_location($city, $state, $country);
    $profile = venture_profile_create($uid, $first_name, $last_name);
    $full_name = venture_profile_get_name($profile, FALSE);
    $name = l($full_name, "user/$uid");
    
    if (!$view->latest) {
      $view->latest = $full_name;
      $view->latest_date = $latest_date;
    }
    
    $output .= "<div class='latest-member'>$name $photo $type<div class='location'>$location</div></div>";
  }
  
  return $output;
}

/**
* Theme the network_summary view
*/
function venture_views_view_network_summary($view, $type, $nodes, $level = NULL, $args = NULL) {
  foreach($nodes as $node) {
    $first_name = venture_render_view_field($view, $node, 0, FALSE);
    $last_name = venture_render_view_field($view, $node, 1, FALSE);
    $type = venture_render_view_field($view, $node, 2);
    $uid = $node->users_uid;
    $profile = venture_profile_create($uid, $first_name, $last_name);
    $name = l(venture_profile_get_name($profile, FALSE), "user/$uid");
    
    if (count($type) > 1) {
      $type = 'AI, VC';
    }
    else {
      $type = ($type[0] == 'Angel Investor' ? 'Angel' : 'VC');
    }
    
    $output .= "<tr><td><div class='user-name'>$name</div></td><td>$type</td></tr>";
  }
  if ($output) {
    $output = "<table id='network-summary'>$output</table>";
  }
  
  if ($view->total_rows > count($nodes)) {
    $total = count($nodes) . ' of ' . $view->total_rows;
  }
  else {
    $total = $view->total_rows . ' total';
  }
  $output .= "<div class='total'><span>$total</span></div>";
  return $output;
}

/**
* Theme the topics view
*/
function venture_views_view_topics($view, $type, $nodes, $level = NULL, $args = NULL) {
  $view->url = $_GET['q'];
  
  foreach($nodes as $i => $node) {
    $post_links = '';
    $links = array();
    
    $title = venture_render_view_field($view, $node, 0, FALSE);
    $comments = venture_render_view_field($view, $node, 1);
    $last_time = venture_render_view_field($view, $node, 2);
    $last_author = venture_render_view_field($view, $node, 3);
    $nid = $node->nid;
    
    $replies = $comments . ' ' . format_plural($comments, 'reply', 'replies');
    $zebra = $i % 2 ? 'even' : 'odd';
    $post_title = l($title, "post/$nid");
    
    $pages = floor($comments / variable_get('comment_default_per_page', '10')) + 1;
    if ($pages > 1) {
      $post_links = ' [ Page ';
      
      $links[] = l(1, "post/$nid");
      for ($i = 1; $i < $pages; $i++) {
        $links[] = l($i + 1, "post/$nid", array(), "page=$i");
      }
      
      $post_links .= implode(', ', $links) . ' ]';
      $post_links = "<span class='post-links'>$post_links</span>";
    }
    
    $post = "<div class='post-title'>$post_title $post_links</div>";
    $post .= "$replies, last message on $last_time by $last_author";
    $output .= "<div class='post $zebra'>$post</div>";
  }
  
  if ($output) {
    $output = "<div id='posts'>$output</div>";
    
    if ($view->use_pager) {
      $link = l('Start new topic', "group/{$args[0]}/post");
      $tags = array('single' => 'Topic', 'plural' => 'Topics', 'link' => $link);
      $pager = theme('pager', $tags, $view->pager_limit);
      $output = $pager . $output . $pager;
    }
  }
  
  if (!$output) {
    $output = 'No messages. ' . l('Click here', "group/{$args[0]}/post") . ' to start a new topic.';
    $output = "<div id='no-results'>$output</div>";
  }
  
  $output = views_theme('views_display_filters', $view) . $output;
  
  return $output;
}

/**
* Theme the topics_all view
*/
function venture_views_view_topics_all($view, $type, $nodes, $level = NULL, $args = NULL) {
  foreach($nodes as $node) {
    $i = 0;
    
    $title = venture_render_view_field($view, $node, $i++, FALSE);
    $group = venture_render_view_field($view, $node, $i++);
    $comments = venture_render_view_field($view, $node, $i++);
    $latest_date = venture_render_view_field($view, $node, $i++);
    $nid = $node->nid;
    
    // Format fields
    $post = l($title, "post/$nid");
    $group = str_replace('/node/', '/group/', $group);
    $replies = $comments . ' ' . format_plural($comments, 'reply', 'replies');
    
    if (!$view->latest) {
      $view->latest = $title;
      $view->latest_date = $latest_date;
    }
    
    $output .= "<li>$post - posted in $group ($replies)</li>";
  }
  
  if ($output) {
    $output = "<ul id='latest-topics'>$output</ul>";
  }
  else {
    $groups = l('View all groups', 'groups/all');
    $output = "<div id='latest-topics'>No posts found in my groups. $groups.</div>";
  }
  
  return $output;
}