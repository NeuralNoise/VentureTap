<?php

/**
* Alter the invite form
*/
function venture_invite_alter(&$form) {
  unset($form['from']);
  unset($form['subject']);
  unset($form['body']);
  
  $form['email']['#description'] = 'Enter email addresses separated by commas or returns.';
  $form['message']['#description'] = 'Enter an optional message to the person you are inviting.';
  $form['message']['#title'] = 'Message';
  $form['submit']['#value'] = 'Invite';
  
  $form['#pre_render'][] = 'venture_invite_pre_render';
  $form['#redirect'] = 'invite';
}

/**
* Return the invite history for all invites
*/
function venture_invite_get_all() {
  global $user;
  $rows = array();
  
  $expiry_diff = variable_get('invite_expiry', 30) * 60 * 60 * 24;
  $time = time() - $expiry_diff;
  $sql = "SELECT * FROM (
    SELECT expiry - %d AS date, email, reg_code, invitee, NULL AS status FROM {invite} WHERE uid = %d AND joined = 0 AND canceled = 0
    UNION
    SELECT i.joined AS date, i.email, i.reg_code, i.invitee, u.status
    FROM {invite} i LEFT JOIN {users} u ON u.uid = i.invitee WHERE i.uid = %d AND i.joined <> 0 AND i.canceled = 0
    ) {invite_history} ORDER BY date DESC";
    
  $result = db_query($sql, $expiry_diff, $user->uid, $user->uid);
  
  while ($invite = db_fetch_object($result)) {
    $rows[] = array(
      'date' => format_date($invite->date, 'custom', 'm/d/Y'),
      'email' => check_plain($invite->email),
      'reg_code' => $invite->reg_code,
      'invitee' => $invite->invitee,
      'invitee_status' => $invite->status,
      'expired' => (!$invite->invitee && $invite->date < $time ? TRUE : FALSE)
    );
  }
  
  return $rows;
}

/**
* Return the invite hisory by type
*/
function venture_invite_get_by_type($type, &$rows) {
  global $user, $pager_total_items;
  $rows = array();
  
  switch ($type) {
    case 'pending':
      $sql = "SELECT email FROM {invite} WHERE uid = %d AND joined = 0 AND canceled = 0 AND expiry >= %d ORDER BY expiry DESC";
      break;
      
    case 'accepted':
      $sql = "SELECT email FROM {invite} WHERE uid = %d AND joined <> 0 AND canceled = 0 ORDER BY joined DESC";
      break;
   
    case 'expired':
      $sql = "SELECT email FROM {invite} WHERE uid = %d AND joined = 0 AND canceled = 0 AND expiry < %d ORDER BY expiry DESC";
      break;
  }
  
  $result = pager_query($sql, 5, 0, NULL, $user->uid, time());
  
  while ($invite = db_fetch_object($result)) {
    $rows[] = check_plain($invite->email);
  }
  
  return $pager_total_items[0];
}

/**
* Add invite-related menu items
*/
function venture_invite_menu($may_cache) {
  $items = array();
  $access = user_access('track invitations');
  
  if ($may_cache) {
    $items[] = array(
      'path' => 'invite/remove',
      'callback' => 'venture_invite_remove',
      'access' => $access,
      'type' => MENU_CALLBACK
    );
    $items[] = array(
      'path' => 'invite/history',
      'title' => 'Invitation History',
      'callback' => 'theme',
      'callback arguments' => array('invite_history'),
      'access' => $access,
      'type' => MENU_CALLBACK
    );
  }
  return $items;
}

/**
* Pre-render event handler
*/
function venture_invite_pre_render($form_id, &$form) {
  // Fix up the error message wording
  if ($_SESSION['messages']['error']) {
    $errors = &$_SESSION['messages']['error'];
    $errors = str_replace('did already invite', 'already invited', $errors);
  }
}

/**
* Remove an existing invitation
*/
function venture_invite_remove() {
  $code = arg(2);
  if ($code) {
    global $user;
    $invite = invite_load($code);
    
    // Inviter must be the current user.
    if ($invite->inviter->uid == $user->uid) {
      db_query("UPDATE {invite} SET canceled = 1 WHERE reg_code = '%s'", $invite->reg_code);
      
      if (!$invite->joined && $invite->expiry > time()) {
        drupal_set_message("Invitation to {$invite->email} has been withdrawn.");
      }
    }
    else {
      drupal_set_message('Unable to remove invitation.', 'error');
    }
  }
}

/**
* Return available tokens for invitation emails
*/
function venture_invite_token_values($type, $object) {
  $values = array();
  if ($type == 'invite' && is_object($object)) {
    global $user;
    $profile = venture_profile_retrieve($user->uid);
    
    if ($object->data['message']) {
      // Remove extraneous spaces from message as they might cause improper wrapping
      $message = preg_replace('! +(\r|\n)!', '$1', $object->data['message']);  
      $message = " {$profile->field_first_name[0]['value']} says:\n\n\"$message\"";
    }
    
    $values['venture-user-name'] = venture_profile_get_name($profile, FALSE);
    $values['venture-message'] = $message;
  }
  return $values;
}