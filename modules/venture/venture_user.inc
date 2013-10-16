<?php

/**
* Implementation of hook_user().
*/
function venture_user_hook($op, &$edit, &$account, $category = NULL) {
  switch ($op) {
    case 'delete':
      // Clean up the database
      venture_network_delete_user($account->uid);
      break;
      
    case 'insert':
      // Clean up the form values
      $edit = venture_profile_insert_user($edit);
      break;
      
    case 'login':
      // Check for new private messages
      venture_pm_check_messages();
      break;
      
    case 'logout':
      // Redirect to login page
      $_REQUEST['destination'] = 'login?out=';
      break;
      
    case 'update':
      // Handle blocked/unblocked users
      if ($edit['status'] === "0" && $account->status) {
        $edit['init'] = $account->mail;
        $edit['mail'] = 'blocked-' . $account->uid . '-' . time() . '@venturetap.com';
        
        // Unpublish the profile and remove from search index
        $profile = venture_profile_retrieve($account->uid);
        $profile->status = 0;
        node_save($profile);
      }
      else if ($edit['status'] && !$account->status) {
        $edit['mail'] = $account->init;
        
        // Republish the profile and add to search index
        $profile = venture_profile_retrieve($account->uid);
        $profile->status = 1;
        node_save($profile);
      }
      
      if ($edit['mail']) {
        $edit['name'] = $edit['mail'];
      }
      break;
  }
}

/**
* Retrieve all activities for this user as well as this user's buddies
*/
function venture_user_get_activities($uid, $buddies) {
  // Get the activities for the last week
  $uids = array_merge(array($uid), array_keys($buddies));
  $filter = array('created' => array('gt' => strtotime('-1 week')));
  $activities = activity_get_activity($uids, $filter, 20);
  
  $results = $conn = array();
  foreach($activities as $idx => $activity) {
    if ($activity['type'] == 'buddylist' && $activity['operation'] == 'add') {
      // Remove invalid records
      $data = $activity['data'];
      $dupe = is_array($conn[$data['author-uid']]) && in_array($data['buddy-uid'], $conn[$data['author-uid']]);
      $buddy = array_key_exists($data['buddy-uid'], $buddies);
      
      if ($data['buddy-uid'] == $uid || $dupe || !$buddy) {
        continue;
      }
      
      $conn[$data['author-uid']][] = $data['buddy-uid'];
      $conn[$data['buddy-uid']][] = $data['author-uid'];
    }
    $results[] = $activity;
  }
  
  return $results;
}

/**
* Only show those buddy activities that happened after the connection has been made
*/
function venture_user_get_activities_rewrite_sql() {
  global $user;
  
  $return['join'] = "LEFT OUTER JOIN {buddylist} buddylist ON buddylist.buddy = activity_targets.target_uid 
    AND buddylist.uid = {$user->uid}";
  $return['where'] = "buddylist.uid IS NULL OR (buddylist.uid = {$user->uid} AND activity.created > buddylist.timestamp)";
  
  return $return;
}

/**
* Alter the login form
*/
function venture_user_login_alter(&$form) {
  $form['#validate'] = array('venture_user_login_validate' => array());
  $form['#submit']['venture_user_login_submit'] = array();
  $form['#pre_render'][] = 'venture_user_login_pre_render';
  $form['submit']['#value'] = 'Sign In';
  $form['name']['#title'] = 'Email';
  $form['name']['#description'] = NULL;
  $form['pass']['#description'] = NULL;
  
  // Show the sign out message only once
  if (isset($_GET['out']) && !$_SESSION['out']) {
    $_SESSION['out'] = TRUE;
    drupal_set_message('You are now signed out of VentureTap.');
  }
}

/**
* Login form pre-render event handler
*/
function venture_user_login_pre_render($form_id, &$form) {
  // Fix up the message wording
  if ($_SESSION['messages']['status']) {
    $messages = &$_SESSION['messages']['status'];
    $messages = str_replace('e-mail', 'email', $messages);
  }
}

/**
* On log-in, redirect to the originally-requested page
*/
function venture_user_login_submit($form_id, $form_values) {
  return $_REQUEST['q'];
}

/**
* Custom user validation - access rules are ignored for now
*/
function venture_user_login_validate($form_id, $form_values) {
  if ($form_values['name']) {
    if (user_is_blocked($form_values['name'])) {
      // blocked in user administration
      form_set_error('name', "The email {$form_values['name']} has not been activated or is blocked.");
    }
    else if ($form_values['pass']) {
      $user = user_authenticate($form_values['name'], trim($form_values['pass']));

      if (!$user->uid) {
        form_set_error('name', 'Sorry, unrecognized email or password. ' . l('Have you forgotten your password?', 'user/password'));
        watchdog('user', "Login attempt failed for {$form_values['name']}.");
      }
    }
  }
}

/**
* Alter the password recovery form
*/
function venture_user_pass_alter(&$form) {
  $form['#pre_render'][] = 'venture_user_pass_pre_render';
  $form['name']['#title'] = 'Email';
  $form['name']['#size'] = 40;
  $form['submit']['#value'] = 'Reset Password';
}

/**
* Password recovery form pre-render event handler
*/
function venture_user_pass_pre_render($form_id, &$form) {
  // Fix up the error message wording
  if ($_SESSION['messages']['error']) {
    $errors = &$_SESSION['messages']['error'];
    $errors = str_replace('user name or an', 'valid', $errors);
  }
}

/**
* Alter the password reset form
*/
function venture_user_pass_reset_alter(&$form) {
  $form['#submit'] = array('venture_user_pass_reset_submit' => array());
  $form['#pre_render'][] = 'venture_user_pass_reset_pre_render';
  $form['submit']['#value'] = 'Change Password';
  $form['pass'] = array(
    '#type' => 'password_confirm',
    '#required' => TRUE,
    '#size' => 25,
  );
  unset($form['message']);
  unset($form['help']);
  
  // Post back to this form
  $form['#action'] = str_replace('/login', '', $form['#action']);
}

/**
* Password reset form pre_render event handler
*/
function venture_user_pass_reset_pre_render($form_id, &$form) {
  $form['pass']['pass1']['#title'] = 'New Password';
  $form['pass']['pass2']['#title'] = 'Confirm Password';
}

/**
* Submit handler for the password reset form
*/
function venture_user_pass_reset_submit($form_id, $form_values) {
  global $user;
  
  // Update the password (this also sets the login timestamp,
  // rendering the reset link expired)
  $account = user_load(array('uid' => arg(2)));
  user_save($account, array('pass' => $form_values['pass']));
  
  // Log the user in
  $user = $account;
  user_module_invoke('login', $edit, $user);
  drupal_set_message('Your password was successfully updated.');
  
  // Redirect to home page
  return '';
}

?>