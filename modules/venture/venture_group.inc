<?php

// Define group membership constants
venture_define(
  'VENTURE_GROUP_NONE',
  'VENTURE_GROUP_MEMBER',
  'VENTURE_GROUP_REQUESTED'
);

/**
* Add user to group
*/
function venture_group_add($group, $title, $account, $name, $subscribed, $active) {
  global $user;
  if ($account->uid == $user->uid) {
    if ($active) {
      drupal_set_message("You are already a member of $title.", 'error');
    }
    else if ($subscribed) {
      drupal_set_message("You have already requested to join $title.", 'error');
    }
    else if ($group->og_selective == OG_MODERATED) {
      og_save_subscription($group->nid, $account->uid, array('is_active' => 0));
      drupal_set_message("Your request to join $title has been submitted. Group administrator will be notified.");
      
      // Send email alert
      $owner = user_load(array('uid' => $group->uid));
      $name = venture_profile_get_name($account->uid, FALSE);
      $link = url('group/' . $group->nid, NULL, NULL, TRUE);
      $subject = "$name would like to join {$group->title} on VentureTap";
      $body = "You received a group membership request from $name for {$group->title}. " .
        "To accept or deny the request, follow the link below:\n$link";
      venture_mail_alert('group_req', $owner, $subject, $body);
    }
    else {
      venture_group_join($account->uid, $group);
      drupal_set_message("You are now a member of $title.");
    }
  }
  else if ($subscribed && !$active && $group->og_selective == OG_MODERATED) {
    venture_group_join($account->uid, $group);
    drupal_set_message("The membership request from $name has been accepted.");
    
    // Send email alert
    $link = url('group/' . $group->nid, NULL, NULL, TRUE);
    $subject = "Your {$group->title} group membership request has been accepted.";
    $body = "You are now a member of {$group->title}. To view the group home page, follow the link below:\n$link";
    venture_mail_alert('group_hdl', $account, $subject, $body);
  }
  else {
    drupal_set_message('You may not perform this action.', 'error');
  }
}

/**
* Group edit form after_build event handler
*/
function venture_group_after_build(&$form, $form_values) {
  // Blank out the state field if a country other than US is chosen
  if ($form_values['field_group_country']['key'] != 'United States') {
    form_set_value($form['field_group_state'], NULL);
  }
  
  // Blank out the deal submission guidelines field for open group
  if ($form_values['og_selective'] != 1) {
    form_set_value($form['field_group_guidelines'][0], NULL);
  }
  
  return $form;
}

/**
* Alter the group node form
*/
function venture_group_alter(&$form) {
  // Content type editing page passes the same form_id as the content
  // editing form. The cck_dummy_node_form allows to distinguish the two.
  if (!$form['#node']->cck_dummy_node_form) {
    $form['#after_build'][] = 'venture_group_after_build'; 
    $form['#submit'] = array('venture_group_submit' => array());
    $form['submit']['#value'] = 'Save Changes';
    
    // Remove Delete button for non-admins
    if (!venture_profile_is_admin()) {
      unset($form['delete']);
    }
    
    $form['body_filter']['body']['#description'] = 'Display a message to group members.';
    $form['body_filter']['body']['#rows'] = 5;
    unset($form['body_filter']['format']);
    
    $form['og_description']['#type'] = 'textarea';
    $form['og_description']['#maxlength'] = 255;
    unset($form['og_description']['#description']);
    
    $form['og_selective']['#title'] = 'Membership';
    $form['og_selective']['#options']['0'] = 'Open - membership requests are accepted immediately';
    $form['og_selective']['#options']['1'] = 'Moderated - membership requests must be approved';
    $form['og_selective']['#description'] = 'Group deals may be submitted to moderated groups only.';
    unset($form['og_selective']['#options'][2]);
    unset($form['og_selective']['#options'][3]);
    
    // Remove formatting instructions
    unset($form['field_group_guidelines'][0]['format']);
    
    venture_group_image_alter($form);
  }
}

/**
* Alter the group comment form
*/
function venture_group_comment_alter(&$form) {
  $form['#submit'] = array('venture_group_comment_submit' => array());
  
  // For existing comments, re-use the old timestamp
  $timestamp = $form['#parameters'][1]['timestamp'];
  if ($timestamp) {
    $form['date'] = array(
      '#type' => 'value',
      '#value' => date('c', $timestamp)
    );
  }
  
  $form['comment_filter']['comment']['#title'] = 'Message';
  $form['submit']['#value'] = 'Submit';
  unset($form['_author']);
  unset($form['comment_filter']['format']);
  
  // Always post to itself
  unset($form['#action']);
}

/**
* Override the default submit method (comment_form_submit)
*/
function venture_group_comment_submit($form_id, $form_values) {
  $form_values = _comment_form_submit($form_values);
  if ($cid = comment_save($form_values)) {
    $action = $form_values['cid'] ? 'updated' : 'created';
    drupal_set_message("The reply was successfully $action.");
    
    // Redirect to the proper page
    $redirect = 'post/'. $form_values['nid'];
    if ($action == 'created') {
      $total = comment_num_all($form_values['nid']);
      $page = floor($total / variable_get('comment_default_per_page', '10'));
      if ($page > 0) {
        $redirect .= "?page=$page";
      }
    }
    else if ($_GET['page']) {
      $redirect .= '?page=' . $_GET['page'];
    }
    $redirect .= "#post-$cid";
    
    // Cannot pass query parameters and fragments as a return value - use a workaround
    $_REQUEST['destination'] = $redirect;
  }
}

/**
* Get various counts for the group
*/
function venture_group_get_counts($gid) {
  static $counts = array();
  if (!$counts[$gid]) {
    // Get fast counts (unlike original query, do NOT rewrite the SQL
    // in order to see the counts for moderated groups)
    $query = 'SELECT COUNT(DISTINCT n.nid) AS fast_topic_count, COUNT(c.cid) AS fast_comment_count, 
      MAX(ncs.last_comment_timestamp) AS fast_last_updated FROM {node} n
      INNER JOIN {og_ancestry} oga ON n.nid = oga.nid LEFT JOIN {comments} c ON n.nid = c.nid
      LEFT JOIN {node_comment_statistics} ncs ON n.nid = ncs.nid WHERE n.status = 1 AND oga.group_nid = %d';
    $counts[$gid] = db_fetch_array(db_query($query, $gid));
    
    // Get the member count
    $query = preg_replace(array('/SELECT.*?FROM /As', '/ORDER BY .*/'), array('SELECT COUNT(*) FROM ', ''),
      og_list_users_sql());
    $counts[$gid]['fast_member_count'] = db_result(db_query($query, $gid));
  }
  return $counts[$gid];
}

/**
* Get post count for a user (or everyone)
*/
function venture_group_get_post_count($uid = NULL) {
  $base = 'SELECT COUNT(*) FROM {node} n INNER JOIN {og_ancestry} oga ON n.nid = oga.nid
    INNER JOIN {node} g ON oga.group_nid = g.nid';

  $topics = "$base WHERE n.status = 1";
  $comments = "$base INNER JOIN {comments} c ON n.nid = c.nid WHERE n.status = 1";
  
  if ($uid) {
    $topics = "$topics AND n.uid = %d";
    $comments = "$comments AND c.uid = %d";
  }
  else {
    $topics = db_rewrite_sql($topics);
    $comments = db_rewrite_sql($comments);
  }
  
  $query = "SELECT ($topics) + ($comments)";
  $result = db_result(db_query($query, $uid, $uid));
  
  return $result;
}

/**
* Get all users that requested membership to a group
*/
function venture_group_get_requesting_users($gid) {
  $query = "SELECT u.uid FROM {og_uid} ou INNER JOIN {users} u ON ou.uid = u.uid
    WHERE ou.nid = %d AND u.status > 0 AND ou.is_active = 0";
  $result = db_query($query, $gid);
  
  $uids = array();
  while ($user = db_fetch_object($result)) {
    $uids[] = $user->uid;
  }
  
  return venture_profile_get_sorted_users($uids);
}

/**
* Get the membership status to provided group id
*/
function venture_group_get_status($gid) {
  global $user;
  $subscriptions = og_get_subscriptions($user->uid, 0);
  
  if (!in_array($gid, array_keys($subscriptions))) {
    return VENTURE_GROUP_NONE;
  }
  else if ($subscriptions[$gid]['is_active']) {
    return VENTURE_GROUP_MEMBER;
  }
  else {
    return VENTURE_GROUP_REQUESTED;
  }
}

/**
* Generic handler for all group actions
*/
function venture_group_handler() {
  global $user;
  $group = node_load(arg(1));
  
  $account = is_numeric(arg(3)) ? user_load(array('uid' => arg(3))) : $user;
  if (!$account) {
    drupal_set_message('User does not exist.', 'error');
  }
  else if (!venture_profile_is_valid($account)) {
    drupal_set_message('Invalid user specified.', 'error');
  }
  else if ($account->uid != $user->uid && !og_is_node_admin($group)) {
    drupal_set_message('You need to be a group administrator to perform this action.', 'error');
  }
  else {
    $name = venture_profile_get_name($account->uid);
    $subscriptions = og_get_subscriptions($account->uid, 0);
    $subscribed = in_array($group->nid, array_keys($subscriptions));
    $active = $subscribed && $subscriptions[$group->nid]['is_active'];
    
    $method = 'venture_group_' . arg(2);
    $method($group, check_plain($group->title), $account, $name, $subscribed, $active);
  }
}

/**
* Generic handler for Accept All and Deny All actions
*/
function venture_group_handler_all() {
  $group = node_load(arg(1));
  
  if (!og_is_node_admin($group)) {
    drupal_set_message('You need to be a group administrator to perform this action.', 'error');
  }
  else {
    $method = 'venture_group_' . arg(2);
    $users = venture_group_get_requesting_users($group->nid);
    foreach($users as $user) {
      $method($group, check_plain($group->title), $user, venture_profile_get_name($user->uid), TRUE, FALSE);
    }
  }
}

/**
* Hook into OG events.
*/
function venture_group_hook($op, $gid, $uid, $args) {
  if ($op == 'user insert' || $op == 'user update') {
    if ($uid == 0 || venture_profile_is_admin($uid)) {
      // Anonymous users and admins cannot be group members (even when designated as owners)
      og_delete_subscription($gid, $uid, $args);
    }
  }
}

/**
* Alter the group image form
*/
function venture_group_image_alter(&$form) {
  $gid = $form['nid']['#value'];
  venture_image_alter($form, 'field_group_image', $gid);
}

/**
* Handle imagefield events
*/
function venture_group_imagefield($op, $file) {
  if ($op == 'save') {
    $old_name = substr($file['filename'], 0, strpos($file['filename'], '.'));
    if (!is_numeric($old_name)) {
      // Replace the temporary name with the final value
      $temp = _imagefield_file_load($file['fid']);
      
      $new_name = str_replace($old_name, $temp['nid'], $file['filename']);
      $new_path = str_replace($old_name, $temp['nid'], $file['filepath']);
      
      if (file_move($file['filepath'], $new_path, FILE_EXISTS_REPLACE)) {
        db_query("UPDATE {files} SET filename = '%s', filepath = '%s' WHERE fid = %d",
          $new_name, $new_path, $file['fid']);
      }
    }
  }
  else if ($op == 'delete') {
    // When a group image is being deleted, clear out the countent_type_group fields.
    // This is to circumvent an existing bug in the imagefield module.
    db_query("UPDATE {content_type_group} SET field_group_image_fid = 0, field_group_image_title = '', 
      field_group_image_alt = '' WHERE field_group_image_fid = %d", $file['fid']);
  }
}

/**
* Create a new activity record
*/
function venture_group_insert_activity($op, $author_uid, $group) {
  $data = array(
    'author-uid'   => $author_uid,
    'operation'   => $op,
    'node-id'     => $group->nid,
    'node-title'   => $group->title,
    'node-type'   => $group->type,
  );
  $target_users_roles = array(
    ACTIVITY_ALL => 'all',
    $author_uid => 'author',
  );
  activity_insert('ogactivity', 'ogaction', $op, $data, $target_users_roles);
}

/**
* Join the specified group
*/
function venture_group_join($uid, $group) {
  og_save_subscription($group->nid, $uid, array('is_active' => 1));
  venture_group_insert_activity('join', $uid, $group);
}

/**
* Add group-related menu items
*/
function venture_group_menu($may_cache) {
  global $user;
  $items = array();
  
  if ($may_cache) {
    $items[] = array(
      'path' => 'groups',
      'callback' => 'theme',
      'callback arguments' => array('groups'),
      'access' => user_access('access content'),
      'type' => MENU_CALLBACK
    );
    $items[] = array(
      'path' => "group/new",
      'callback' => 'node_add',
      'callback arguments' => array('group'),
      'access' => user_access('access content'),
      'type' => MENU_CALLBACK
    );
  }
  else {
    if (arg(0) == 'group' && is_numeric(arg(1)) && $group = node_load(arg(1))) {
      $access_view = node_access('view', $group);
      $access_edit = node_access('update', $group);
      
      if (og_is_group_type($group->type)) {
        $items[] = array(
          'path' => "group/{$group->nid}",
          'callback' => 'theme',
          'callback arguments' => array('group_view'),
          'access' => $access_view,
          'type' => MENU_CALLBACK
        );
        $items[] = array(
          'path' => "group/{$group->nid}/add",
          'callback' => 'venture_group_handler',
          'access' => $access_view,
          'type' => MENU_CALLBACK
        );
        $items[] = array(
          'path' => "group/{$group->nid}/remove",
          'callback' => 'venture_group_handler',
          'access' => $access_view,
          'type' => MENU_CALLBACK
        );
        $items[] = array(
          'path' => "group/{$group->nid}/add/all",
          'callback' => 'venture_group_handler_all',
          'access' => $access_edit,
          'type' => MENU_CALLBACK
        );
        $items[] = array(
          'path' => "group/{$group->nid}/remove/all",
          'callback' => 'venture_group_handler_all',
          'access' => $access_edit,
          'type' => MENU_CALLBACK
        );
        $items[] = array(
          'path'  => "group/{$group->nid}/edit",
          'callback' => 'node_page_edit',
          'callback arguments' => array($group),
          'access' => $access_edit,
          'type' => MENU_CALLBACK
        );
        $items[] = array(
          'path' => "group/{$group->nid}/pm",
          'callback' => 'drupal_get_form',
          'callback arguments' => array('privatemsg_new_form'),
          'access' => $access_edit,
          'type' => MENU_CALLBACK
        );
        $items[] = array(
          'path' => "group/{$group->nid}/post",
          'callback' => 'node_add',
          'callback arguments' => array('group_post'),
          'access' => ($access_view && isset($user->og_groups[arg(1)])) || venture_profile_is_admin(),
          'type' => MENU_CALLBACK
        );
        
        if ($group->og_selective == OG_MODERATED) {
          $items[] = array(
            'path' => "group/{$group->nid}/submit",
            'callback' => 'node_add',
            'callback arguments' => array('deal'),
            'access' => user_access('access content'),
            'type' => MENU_CALLBACK
          );
        }
        
        if (arg(2) == 'post') {
          // Provide group ID for new post creation
          $_GET['gids'][0] = arg(1);
        }
      }
    }
    else if (arg(0) == 'post' && is_numeric(arg(1)) && $post = node_load(arg(1))) {
      $access_view = node_access('view', $post);
      $access_edit = node_access('update', $post);
      
      if (og_is_group_post_type($post->type)) {
        $group = node_load($post->og_groups[0]);
        
        $items[] = array(
          'path' => "post/{$post->nid}",
          'callback' => 'theme',
          'callback arguments' => array('group_post_view'),
          'access' => $access_view,
          'type' => MENU_CALLBACK
        );
        $items[] = array(
          'path' => "post/{$post->nid}/delete",
          'callback' => 'venture_group_message_delete',
          'access' => venture_group_message_delete_allowed($group, $post->uid) &&
                      !venture_group_message_deleted_by($post->body) && $access_view,
          'type' => MENU_CALLBACK
        );
        $items[] = array(
          'path'  => "post/{$post->nid}/edit",
          'callback' => 'node_page_edit',
          'callback arguments' => array($post),
          'access' => ($access_edit && $post->uid == $user->uid &&
                      !venture_group_message_deleted_by($post->body)) || venture_profile_is_admin(),
          'type' => MENU_CALLBACK
        );
        $items[] = array(
          'path' => "post/{$post->nid}/reply",
          'callback' => 'drupal_get_form',
          'callback arguments' => array('comment_form', array('nid' => arg(1))),
          'access' => $access_view,
          'type' => MENU_CALLBACK
        );
        
        if (is_numeric(arg(3))) {
          $reply = venture_get_comment(arg(3));
          if ($reply && $reply->nid == $post->nid) {
            $items[] = array(
              'path' => "post/{$post->nid}/delete/{$reply->cid}",
              'callback' => 'venture_group_message_delete',
              'access' => venture_group_message_delete_allowed($group, $reply->uid) &&
                          !venture_group_message_deleted_by($reply->comment) && $access_view,
              'type' => MENU_CALLBACK
            );
            $items[] = array(
              'path'  => "post/{$post->nid}/edit/{$reply->cid}",
              'callback' => 'drupal_get_form',
              'callback arguments' => array('comment_form', (array)$reply),
              'access' => ($access_view && $reply->uid == $user->uid &&
                          !venture_group_message_deleted_by($reply->comment)) || venture_profile_is_admin(),
              'type' => MENU_CALLBACK
            );
            $items[] = array(
              'path' => "post/{$post->nid}/reply/{$reply->cid}",
              'callback' => 'drupal_get_form',
              'callback arguments' => array('comment_form', array('nid' => arg(1), 'pid' => arg(3))),
              'access' => $access_view,
              'type' => MENU_CALLBACK
            );
          }
        }
      }
    }
  }
  return $items;
}

/**
* Delete message/comment
*/
function venture_group_message_delete() {
  global $user;
  $deleted = "<deleted>{$user->uid}</deleted>\n";
  
  if (is_numeric(arg(3))) {
    // Delete reply
    $reply = (array)venture_get_comment(arg(3));
    $reply['comment'] = $deleted . $reply['comment'];
    comment_save($reply);
  }
  else {
    // Delete post
    $post = node_load(arg(1));
    $post->body = $deleted . $post->body;
    node_save($post);
  }

  drupal_set_message('The message was successfully deleted.');
}

/**
* Only the following can be deleted - own posts, any post by admin,
* and any non-admin post by group owner
*/
function venture_group_message_delete_allowed($group, $uid) {
  global $user;
  $own = ($user->uid == $uid);
  $is_admin = venture_profile_is_admin();
  $is_owner = og_is_node_admin($group) && !venture_profile_is_admin($uid);
  return $own || $is_admin || $is_owner;
}

/**
* Determine if the message is already deleted
*/
function venture_group_message_deleted_by($text) {
  if (preg_match('!^<deleted>(\d+)</deleted>!', $text, $matches)) {
    return $matches[1];
  }
  return FALSE;
}

/**
* Alter the group post node form
*/
function venture_group_post_alter(&$form) {
  global $user;
  
  // Content type editing page passes the same form_id as the content
  // editing form. The cck_dummy_node_form allows to distinguish the two.
  if (!$form['#node']->cck_dummy_node_form) {
    $form['#submit'] = array('venture_group_post_submit' => array());
    
    // Remove the Audience field
    unset($form['og_nodeapi']['visible']);
    if (!$form['og_nodeapi']['invisible']['og_groups']) {
      $gid = arg(0) == 'group' && arg(2) == 'post' ? arg(1) : $form['#node']->og_groups[0];
      $form['og_nodeapi']['invisible']['og_groups'] = array(
        '#type' => 'value',
        '#value' => array($gid => $gid)
      );
    }
    
    $form['body_filter']['body']['#required'] = TRUE;
    unset($form['body_filter']['format']);
    
    // Remove Delete button for non-admins
    if (!venture_profile_is_admin()) {
      unset($form['delete']);
    }
  }
}

/**
* Override the default submit method (node_form_submit)
*/
function venture_group_post_submit($form_id, $form_values) {
  // Save the group
  $post = node_submit($form_values);
  $action = $post->nid ? 'updated' : 'created';
  node_save($post);
  
  // Show a message and log
  $title = check_plain($post->title);
  watchdog('content', "Group Post: $action $title", WATCHDOG_NOTICE, l(t('view'), 'post/'. $post->nid));
  drupal_set_message("The message was successfully $action.");
  
  return 'post/'. $post->nid;
}

/**
* Remove user from group
*/
function venture_group_remove($group, $title, $account, $name, $subscribed, $active) {
  global $user;
  
  if (!$subscribed) {
    $msg = ($account->uid == $user->uid ? 'You do' : "$name does") . " not belong to $title.";
    drupal_set_message($msg, 'error');
  }
  else if ($group->uid == $account->uid) {
    drupal_set_message('Group owners may not be removed.', 'error');
  }
  else {
    og_delete_subscription($group->nid, $account->uid);    

    if ($active) {
      venture_group_insert_activity('leave', $account->uid, $group);
      $msg = ($account->uid == $user->uid ? 'You have' : "$name has") . " been removed from $title.";
    }
    else if ($account->uid == $user->uid) {
      $msg = "Your request to join $title has been cancelled.";
    }
    else {
      $msg = "The membership request from $name has been denied.";
      
      // Send email alert
      $link = url('pm/new/' . $group->uid, NULL, NULL, TRUE);
      $subject = "Your {$group->title} group membership request has been denied.";
      $body = "Your request to join {$group->title} was denied. To send the group owner a private message, " .
        "follow the link below:\n$link";
      venture_mail_alert('group_hdl', $account, $subject, $body);
    }
    
    drupal_set_message($msg);
  }
}

/**
* Override the default submit method (node_form_submit)
*/
function venture_group_submit($form_id, $form_values) {
  // Save the group
  $group = node_submit($form_values);
  $action = $group->nid ? 'updated' : 'created';
  node_save($group);
  
  // When the owner is changed, the old user is kept as an admin. Fix that.
  if ($action == 'updated') {
    $sql = 'UPDATE {og_uid} SET is_admin = 0 WHERE nid = %d AND uid != %d';
    db_query($sql, $group->nid, $group->uid);
  }
  
  // Show a message and log
  $title = check_plain($group->title);
  watchdog('content', "Group: $action $title", WATCHDOG_NOTICE, l(t('view'), 'group/'. $group->nid));
  drupal_set_message("$title has been $action.");
  
  return 'group/'. $group->nid;
}

/**
* Return available tokens for group activity
*/
function venture_group_token_values($type, $data) {
  $values = array();
  if ($type == 'ogactivity' && !empty($data)) {
    $values['node-link'] = l($data['node-title'], 'group/'. $data['node-id']);
  }
  else if ($type == 'nodeactivity' && !empty($data) && $data['node-type'] == 'group_post') {
    $post = node_load($data['node-id']);
    $gid = $post->og_groups[0];
    $values['node-link'] = l($post->title, "post/{$post->nid}");
    $values['group'] = l($post->og_groups_both[$gid], "group/$gid");
  }
  return $values;
}

/**
* Add an additional views argument
*/
function venture_group_views_arguments() {
  $args = array(
    'og_uid' => array(
      'name' => t('OG: User is subscribed to group'), 
      'handler' => 'venture_group_views_arguments_og_uid',
      'help' => t("Filter for groups that user is subscribed to.")
    ),
    'og_uid_node' => array(
      'name' => t('OG: Post in user-subscribed groups'),
      'handler' => 'venture_group_views_arguments_og_uid_node',
      'help' => t("Filter for posts in groups that user is subscribed to.")
    ),
    'uid_gid' => array(
      'name' => t('OG: Node author is in group'),
      'handler' => 'venture_group_views_arguments_uid_gid',
      'help' => t("Filter for a node author's membership in a given organic group.")
    )
  );
  return $args;
}

/**
 * Custom argument for filtering by groups that a user is subscribed to
 */
function venture_group_views_arguments_og_uid($op, &$query, $argtype, $arg = '') {
  if ($op == 'filter') {
    $query->ensure_table('og_uid');
    $query->add_where("og_uid.uid = %d", $arg);
  }
}

/**
 * Custom argument for filtering by posts in groups that a user is subscribed to
 */
function venture_group_views_arguments_og_uid_node($op, &$query, $argtype, $arg = '') {
  if ($op == 'filter') {
    $query->ensure_table('og_uid_node');
    $query->add_where("og_uid_node.uid = %d", $arg);
  }
}

/**
 * Custom argument for filtering by author's membership in an og.
 */
function venture_group_views_arguments_uid_gid($op, &$query, $argtype, $arg = '') {
  if ($op == 'filter') {
    $group_id = intval($arg);
    $query->ensure_table('og_uid_uid');
    $query->add_where("og_uid_uid.nid = %d", $group_id);
  }
}

/**
 * Views handler to display various counts for the group
 */
function venture_group_views_counts($fieldinfo, $fielddata, $value, $data) {
  $counts = venture_group_get_counts($data->nid);
  return $counts[$fielddata['field']];
}

/**
 * Views handler to display member count for the group
 */
function venture_group_views_members($fieldinfo, $fielddata, $value, $data) {
  return db_num_rows(db_query(og_list_users_sql(), $data->nid));
}

/**
* Add more OG views fields
*/
function venture_group_views_tables_alter(&$table_data) {
  $table_data['og_uid_uid'] = array(
    'name' => 'og_uid',
    'join' => array(
      'left' => array(
        'table' => 'node',
        'field' => 'uid'
      ),
      'right' => array(
        'field' => 'uid'
      ),
      'extra' => array(
        'is_active' => 1
      )
    )
  );
  $table_data['og_uid_uid_node'] = array(
    'name' => 'node',
    'join' => array(
      'left' => array(
        'table' => 'og_uid_uid',
        'field' => 'nid'
      ),
      'right' => array(
        'field' => 'nid'
      )
    )
  );
  
  $table_data['og']['fields']['fast_topic_count'] = array(
    'name' => 'OG: Group: Fast topic count',
    'handler' => 'venture_group_views_counts',
    'sortable' => FALSE,
    'notafield' => TRUE,
    'help' => t('Number of topics in a group.')
  );
  $table_data['og']['fields']['fast_comment_count'] = array(
    'name' => 'OG: Group: Fast comment count',
    'handler' => 'venture_group_views_counts',
    'sortable' => FALSE,
    'notafield' => TRUE,
    'help' => t('Number of comments in a group.')
  );
  $table_data['og']['fields']['fast_last_updated'] = array(
    'name' => 'OG: Group: Fast last updated timestamp',
    'handler' => 'venture_group_views_counts',
    'sortable' => FALSE,
    'notafield' => TRUE,
    'help' => t('Last time the group was updated (via a message or a comment).')
  );
  $table_data['og']['fields']['fast_member_count'] = array(
    'name' => 'OG: Group: Fast member count',
    'handler' => 'venture_group_views_counts',
    'sortable' => FALSE,
    'notafield' => TRUE,
    'help' => t('Number of members for a group. Excludes memberships which are pending approval.')
  );
  $table_data['og']['fields']['count']['handler'] = 'venture_group_views_members';
  
  // Remove unused group types and change the operator - default ones does not work will with Force Single
  $table_data['og']['filters']['selective']['operator'] = 'views_handler_operator_eqneq';
  unset($table_data['og']['filters']['selective']['list'][2]);
  unset($table_data['og']['filters']['selective']['list'][3]);
}

?>