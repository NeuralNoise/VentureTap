<?php

// Define connection constants
venture_define(
  'VENTURE_CONN_NONE',
  'VENTURE_CONN_EXISTS',
  'VENTURE_CONN_INITIATED',
  'VENTURE_CONN_REQUESTED'
);

/**
* Accept connection request
*/
function venture_network_accept($user, $buddy, $info) {
  $error = buddy_api_accept_request($user, $buddy, $info);
  $name = venture_profile_get_name($buddy->uid, TRUE);
  
  if ($error === 'other_user_cancelled') {
    venture_profile_remove_period($name);
    drupal_set_message("There are no outstanding requests to connect from $name.", 'error');
  }
  else {
    venture_network_insert_activity_add($buddy, $user, 'conn_hdl');
    drupal_set_message("$name has been added to your network.");
  }
}

/**
* Add user to the network. If a request was already made by the other user,
* accept that request instead of initiating a new one.
*/
function venture_network_add($user, $buddy, $info) {
  if (in_array($user->uid, array_keys(buddy_api_get_requestees($buddy->uid)))) {
    venture_network_accept($user, $buddy, $info);
  }
  else {
    $error = buddy_api_add($user, $buddy, $info);
    $name = venture_profile_get_name($buddy->uid);
          
    if ($error === 'error_already_requested') {
      venture_profile_remove_period($name);
      drupal_set_message("You have already requested to be connected with $name.", 'error');
    }
    else if ($error === 'error_already_on_buddylist') {
      drupal_set_message("$name is already in your network.", 'error');
    }
    else {
      drupal_set_message("Your request to add $name to your network has been submitted. User will be notified.");
      
      // Send email alert
      $name = venture_profile_get_name($user->uid, FALSE);
      $link = url('profile/' . $user->uid, NULL, NULL, TRUE);
      $subject = "$name sent you a connection request on VentureTap";
      $body = "You received a request to connect with $name on VentureTap. To accept or deny " . 
        "the connection request, follow the link below:\n$link";
      venture_mail_alert('conn_req', $buddy, $subject, $body);
    }
  }
}

/**
* Cancel connection request
*/
function venture_network_cancel($user, $buddy, $info) {
  if (in_array($buddy->uid, array_keys(buddy_api_get_requestees($user->uid)))) {
    buddy_api_cancel_request($user, $buddy, $info);
    $name = venture_profile_get_name($buddy->uid);
    drupal_set_message("Your request to add $name to your network has been cancelled.");
  }
  else {
    $name = venture_profile_get_name($buddy->uid);
    venture_profile_remove_period($name);
    drupal_set_message("There are no outstanding requests to connect with $name.", 'error');
  }
}

/**
* Connect with the inviter
*/
function venture_network_connect_with_inviter($args) {
  if (venture_profile_is_valid($args['inviter'])) {
    $buddyinfo = array('sendnotification' => FALSE);
    buddy_api_add_oneway($args['inviter'], $args['invitee'], $buddyinfo);
    buddy_api_add_oneway($args['invitee'], $args['inviter'], $buddyinfo);
    venture_network_insert_activity_add($args['inviter'], $args['invitee'], 'invitee');
  }
}

/**
* Remove database records referencing the deleted user
*/
function venture_network_delete_user($uid) {
  db_query("DELETE FROM {buddylist} WHERE uid = %d OR buddy = %d", $uid, $uid);
  db_query("DELETE FROM {buddylist_pending_requests} WHERE requester_uid = %d OR requestee_uid = %d", $uid, $uid);
}

/**
* Deny connection request
*/
function venture_network_deny($user, $buddy, $info) {
  if (in_array($user->uid, array_keys(buddy_api_get_requestees($buddy->uid)))) {
    buddy_api_deny_request($user, $buddy, $info);
    $name = venture_profile_get_name($buddy->uid);
    drupal_set_message("The connection request from $name has been denied.");
    
    // Send email alert
    $name = venture_profile_get_name($user->uid, FALSE);
    $link = url('pm/new/' . $user->uid, NULL, NULL, TRUE);
    $subject = "$name denied your connection request on VentureTap";
    $body = "Your request to connect with $name was denied. To send the user a private message, follow the link below:\n$link";
    venture_mail_alert('conn_hdl', $buddy, $subject, $body);
  }
  else {
    $name = venture_profile_get_name($buddy->uid);
    venture_profile_remove_period($name);
    drupal_set_message("There are no outstanding requests to connect from $name.", 'error');
  }
}

/**
* Get all users that requested to be connected
*/
function venture_network_get_requesting_users($uid) {
  $query = "SELECT u.uid FROM {buddylist_pending_requests} bpr INNER JOIN {users} u
    ON bpr.requester_uid = u.uid WHERE bpr.requestee_uid = %d AND u.status > 0";
  $result = db_query($query, $uid);
  
  $uids = array();
  while ($user = db_fetch_object($result)) {
    $uids[] = $user->uid;
  }
  
  return venture_profile_get_sorted_users($uids);
}

/**
* Get the connection status to provided account uid
*/
function venture_network_get_status($uid) {
  global $user;
  static $requestees;
  
  if ($user->uid != $uid) {
    if (venture_network_is_connected($uid)) {
      return VENTURE_CONN_EXISTS;
    }
    
    if (!isset($requestees[$user->uid])) {
      $requestees[$user->uid] = array_keys(buddy_api_get_requestees($user->uid));
    }
    if (in_array($uid, $requestees[$user->uid])) {
      return $status = VENTURE_CONN_INITIATED;
    }
    
    if (!isset($requestees[$uid])) {
      $requestees[$uid] = array_keys(buddy_api_get_requestees($uid));
    }
    if (in_array($user->uid, $requestees[$uid])) {
      return VENTURE_CONN_REQUESTED;
    }
  }
  
  return VENTURE_CONN_NONE;
}

/**
* Generic handler for all network actions
*/
function venture_network_handler() {
  if (is_numeric(arg(2))) {
    global $user;
    if ($user->uid == arg(2)) {
      drupal_set_message('You cannot belong to your own network.', 'error');
    }
    else if (venture_profile_is_admin($user)) {
      drupal_set_message('You cannot perform this action.', 'error');
    }
    else {
      $buddy = user_load(array('uid' => arg(2)));
      if (!$buddy) {
        drupal_set_message('User does not exist.', 'error');
      }
      else if (!venture_profile_is_valid($buddy)) {
        drupal_set_message('Invalid user specified.', 'error');
      }
      else {
        $method = 'venture_network_' . arg(1);
        $method($user, $buddy, array('sendnotification' => FALSE));
      }
    }
  }
}

/**
* Generic handler for Accept All and Deny All actions
*/
function venture_network_handler_all() {
  global $user;
  
  if (venture_profile_is_admin($user)) {
    drupal_set_message('You cannot perform this action.', 'error');
  }
  else {
    $method = 'venture_network_' . arg(1);
    $requesters = venture_network_get_requesting_users($user->uid);
    foreach($requesters as $requester) {
      $method($user, $requester, array('sendnotification' => FALSE));
    }
  }
}

/**
* Create a new activity record
*/
function venture_network_insert_activity($op, $author_uid, $buddy_uid) {
  $data = array(
    'author-uid' => $author_uid,
    'buddy-uid' => $buddy_uid,
  );
  $target_users_roles = array(
    ACTIVITY_ALL => 'all',
    $author_uid => 'author',
  );
  activity_insert('buddylist2activity', 'buddylist', $op, $data, $target_users_roles);
}

/**
* Create the Add activity for the two connecting users
*/
function venture_network_insert_activity_add($requester, $requestee, $key) {
  venture_network_insert_activity('add', $requester->uid, $requestee->uid);
  venture_network_insert_activity('add', $requestee->uid, $requester->uid);
  
  // Send alert email
  if ($key == 'invitee') {
    global $form_values;
    $profile = (object)$form_values;
    $profile->uid = $requestee->uid;
    $action = 'joined VentureTap';
  }
  else {
    $profile = venture_profile_retrieve($requestee->uid);
    $action = 'accepted your connection request on VentureTap';
  }
  $name = venture_profile_get_name($profile, FALSE);
  $link = url('profile/' . $requestee->uid, NULL, NULL, TRUE);
  $body = "You are now connected to $name on VentureTap. To view your new connection's profile, follow the link below:\n$link";
  venture_mail_alert($key, $requester, "$name $action", $body);
}

/**
* Check if the user is connected to another account
*/
function venture_network_is_connected($uid) {
  global $user;
  static $buddies;
  
  if (!isset($buddies[$user->uid])) {
    $buddies[$user->uid] = array_keys(buddy_api_get_buddies($user->uid));
  }
  return in_array($uid, $buddies[$user->uid]);
}

/**
* Add network-related menu items
*/
function venture_network_menu($may_cache) {
  $items = array();
  $access = user_access('access content');
  
  if ($may_cache) {
    $items[] = array(
      'path' => 'network',
      'callback' => 'theme',
      'callback arguments' => array('network'),
      'access' => $access,
      'type' => MENU_CALLBACK
    );
    $items[] = array(
      'path' => 'network/add',
      'callback' => 'venture_network_handler',
      'access' => $access,
      'type' => MENU_CALLBACK
    );
    $items[] = array(
      'path' => 'network/remove',
      'callback' => 'venture_network_handler',
      'access' => $access,
      'type' => MENU_CALLBACK
    );
    $items[] = array(
      'path' => 'network/cancel',
      'callback' => 'venture_network_handler',
      'access' => $access,
      'type' => MENU_CALLBACK
    );
    $items[] = array(
      'path' => 'network/accept',
      'callback' => 'venture_network_handler',
      'access' => $access,
      'type' => MENU_CALLBACK
    );
    $items[] = array(
      'path' => 'network/deny',
      'callback' => 'venture_network_handler',
      'access' => $access,
      'type' => MENU_CALLBACK
    );
    $items[] = array(
      'path' => 'network/accept/all',
      'callback' => 'venture_network_handler_all',
      'access' => $access,
      'type' => MENU_CALLBACK
    );
    $items[] = array(
      'path' => 'network/deny/all',
      'callback' => 'venture_network_handler_all',
      'access' => $access,
      'type' => MENU_CALLBACK
    );
  }
  return $items;
}

/**
* Remove user from the network
*/
function venture_network_remove($user, $buddy, $info) {
  // Get full name *before* removing
  $name = venture_profile_get_name($buddy->uid);
  
  if (in_array($buddy->uid, array_keys(buddy_api_get_buddies($user->uid)))) {
    buddy_api_remove($user, $buddy, $info);
    drupal_set_message("$name has been removed from your network.");
  }
  else {
    drupal_set_message("$name is not in your network.", 'error');
  }
}

/**
* Return available tokens for buddy activity
*/
function venture_network_token_values($type, $data) {
  $values = array();
  if ($type == 'buddylist2activity' && !empty($data)) {
    global $user;
    $values['verb'] = ($data['author-uid'] == $user->uid ? 'are' : 'is');
  }
  return $values;
}