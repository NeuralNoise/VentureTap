<?php

global $venture_msg_count;

/**
* Check if the user received any new messages
*/
function venture_pm_check_messages() {
  global $user, $venture_msg_count;
  $venture_msg_count = _privatemsg_get_new_messages();
  if ($venture_msg_count) {
    $query = 'SELECT COUNT(*) FROM {privatemsg} WHERE recipient = %d AND newmsg = 1 AND recipient_del = 0 AND timestamp > %d';
    $result = db_result(db_query($query, $user->uid, $user->access));
    if ($result) {
      $msg = ($result == 1) ? 'You have a new ' . l('private message', 'pm') : "You have $result new " . l('private messages', 'pm');
      drupal_set_message($msg);
    }
  }
}

/**
* Alter the current folder form
*/
function venture_pm_cur_folder_alter(&$form) {
  $form['header']['folder_select']['#options'] = array(
    PRIVATEMSG_FOLDER_INBOX => 'Inbox',
    PRIVATEMSG_FOLDER_SENT => 'Sent Messages',
    PRIVATEMSG_FOLDER_RECYCLE_BIN => 'Recycle Bin'
  );
  $form['header']['#type'] = 'item';
  unset($form['header']['type']);
  unset($form['header']['filter_type']);
}

/**
* Get an array of possible actions for the list page
*/
function venture_pm_get_actions() {
  return array(
    'delete' => array('old' => 'Delete', 'new' => 'Delete Selected', 'weight' => 1),
    'mark_read' => array('old' => 'Mark as read', 'new' => 'Mark as Read', 'weight' => 2),
    'mark_unread' => array('old' => 'Mark as unread', 'new' => 'Mark as Unread', 'weight' => 3),
    'permanent' => array('old' => 'Delete permanently', 'new' => 'Delete Selected', 'weight' => 1),
    'undelete' => array('old' => 'Restore', 'new' => 'Restore Selected', 'weight' => 2),
    'empty' => array('old' => 'Empty recycle bin', 'new' => 'Empty Folder', 'weight' => 3)
  );
}

/**
* Get the current folder that the message resides in
*/
function venture_pm_get_current_folder($message) {
  global $user;
  static $messages = array();
  
  if (!$messages[$message->id]) {
    $query = 'SELECT author_del, recipient_del FROM {privatemsg} WHERE id = %d';
    $result = db_query($query, $message->id);
    $info = db_fetch_object($result);
    
    if ($message->author != $user->uid && !$info->recipient_del) {
      $folder = PRIVATEMSG_FOLDER_INBOX;
    }
    else if ($message->author == $user->uid && !$info->author_del) {
      $folder = PRIVATEMSG_FOLDER_SENT;
    }
    else {
      $folder = PRIVATEMSG_FOLDER_RECYCLE_BIN;
    }
    
    $messages[$message->id] = $folder;
  }
  
  return $messages[$message->id];
}

/**
* Return the correct new message count
*/
function venture_pm_get_msg_count() {
  global $venture_msg_count;
  return $venture_msg_count;
}

/**
* Return the new messages
*/
function venture_pm_get_new_messages($uid) {
  $query = 'SELECT id, author, subject, timestamp FROM {privatemsg}
    WHERE recipient = %d AND newmsg = 1 AND recipient_del = 0 ORDER BY timestamp DESC LIMIT 20';
  $result = db_query($query, $uid);
  
  $messages = array();
  while ($message = db_fetch_object($result)) {
    $messages[] = $message;
  }
  
  return $messages;
}

/**
* Get the value for a view field
*/
function venture_pm_get_view_field($view, $item, $index) {
  $field = $view->field[$index]['queryname'];
  return $item->{$field};
}

/**
* Handle privatemsg hook
*/
function venture_pm_hook($message, $op) {
  if ($op == 'sent') {
    // Send email alert
    $name = venture_profile_get_name($message->uid, FALSE);
    $link = url('pm/view/' . $message->id, NULL, NULL, TRUE);
    $recipient = user_load(array('uid' => $message->recipient));
    $subject = "$name sent you a message on VentureTap";
    $body = "You received a new private message from $name with the following subject:\n\n\"" . $message->subject .
      "\"\n\nTo view the entire message, follow the link below:\n$link";
    venture_mail_alert('pm', $recipient, $subject, $body);
  }
}

/**
* List message form after_build event handler
*/
function venture_pm_list_after_build($form, &$form_values) {
  // Restore old action values so that form submit will work
  if ($form_values['op']) {
    foreach(venture_pm_get_actions() as $name => $action) {
      if ($form_values['op'] == $form_values[$name]) {
        $form_values['op'] = $action['old'];
        break;
      }
    }
  }
  return $form;
}

/**
* Alter the message list form
*/
function venture_pm_list_alter(&$form) {
  $form['#after_build'][] = 'venture_pm_list_after_build'; 
  $form['#pre_render'][] = 'venture_pm_list_pre_render';
  
  // Set up the actions
  if ($form['selected']) {
    $form['selected']['#type'] = 'item';    
    $actions = venture_pm_get_actions();
    foreach ($form['selected']['actions'] as $name => &$action) {
      $info = $actions[$name];
      $action['#value'] = $info['new'];
      $action['#weight'] = $info['weight'];
    }
  }
  
  // Bypass delete confirmation
  $form['js_bypass']['#value'] = 1;
  
  // Remove list SQL from session to prevent unnecessary
  // DB hits on message viewing
  unset($_SESSION['privatemsg_list_sql']);
  
  // Add author information to each message
  foreach ($form['messages'] as $id => $msg) {
    if (is_numeric($id)) {
      $messages[] = $id;
    }
  }
  if ($messages) {
    $placeholders = implode(',', array_fill(0, count($messages), '%d'));
    $query = "SELECT id, author FROM {privatemsg} WHERE id IN ($placeholders)";
    $result = db_query($query, $messages);
    
    while ($message = db_fetch_object($result)) {
      $form['messages'][$message->id]['message']['#value']->author = $message->author;
    }
  }
}

/**
* Message list form pre-render event handler
*/
function venture_pm_list_pre_render($form_id, &$form) {
  // Fix up the status message wording
  if ($_SESSION['messages']['status']) {
    $messages = &$_SESSION['messages']['status'];
    $messages = str_replace('Recycle bin', 'Recycle Bin', $messages);
  }
}

/**
* New message form after_build event handler
*/
function venture_pm_new_after_build($form, &$form_values) {
  global $user;
  
  if ($form_values['op'] == $form_values['send']) {
    // Privatemsg module expects username for the recipient
    if (arg(0) == 'group') {
      // Message group members
      $view = views_get_view('network_summary');
      $result = views_build_view('items', $view,  array('all', arg(1)));
      
      $members = array();
      foreach ($result['items'] as $item) {
        if ($item->users_name != $user->name) {
          $members[] = $item->users_name;
        }
      }
      form_set_value($form['header']['recipient'], implode(',', $members));
      
      // Privatemsg module does not allow for modifying the redirect path
      // with an additional submit handler, so force a path change here
      $_REQUEST['destination'] = 'group/' . arg(1);
    }
    else if (is_numeric($form_values['recipient'])) {
      // Message recipient directly
      $account = user_load(array('uid' => $form_values['recipient']));
      form_set_value($form['header']['recipient'], $account->name);
    }
    
    // Restore old action value so that form submit will work
    $form_values['op'] = 'Send private message';
  }
  
  return $form;
}

/**
* Alter the new message form
*/
function venture_pm_new_alter(&$form) {
  global $user;
  
  $form['#after_build'][] = 'venture_pm_new_after_build';
  
  // Privatemsg module does not enforce subject length.
  // Fix for the length increase due to Re:
  $subject = &$form['header']['subject']['#default_value'];
  $subject = substr($subject, 0, $form['header']['subject']['#maxlength']);
  
  // Remove filter instructions
  unset($form['filter_form']['format']);
  unset($form['filter_form'][2]);
  
  $form['send']['#value'] = 'Send Message';
  
  if (arg(0) == 'group') {
    // Message group members
    $group = node_load(arg(1));
    if (og_is_node_admin($group)) {
      $form['header']['recipient'] = array('#title' => 'To');
      
      // Figure out member count
      $form['header']['members'] = array(
        '#type' => 'value',
        '#value' => venture_views_get_node_count('network_summary', array('all', arg(1)))
      );
      
      return;
    }
    else {
      // Must be group admin to message all users
      drupal_access_denied();
      exit();
    }
  }
  
  $to = $form['header']['recipient']['#default_value'];
  if ($to) {
    // Replace username/email with UID as a security precaution
    $account = user_load(array('name' => $to));
    if ((venture_profile_is_valid($account) || venture_profile_is_admin($account)) && $account->uid != $user->uid) {
      $form['header']['recipient'] = array(
        '#type' => 'hidden',
        '#title' => 'To',
        '#value' => $account->uid
      );
      $form['header']['account'] = array(
        '#type' => 'value',
        '#value' => $account
      );
    }
    else {
      // Cannot PM this user
      drupal_access_denied();
      exit();
    }
  }
  else { 
    // Build the To list
    $view = views_get_view('network_summary');
    $result = views_build_view('items', $view,  array($user->uid, 'all'));
    
    $options = array(NULL => NULL);
    foreach ($result['items'] as $item) {
      $first_name = venture_pm_get_view_field($view, $item, 0);
      $last_name = venture_pm_get_view_field($view, $item, 1);
      $options[$item->users_uid] = "$first_name $last_name";
    }

    $form['header']['recipient'] = array(
      '#type' => 'select',
      '#title' => 'To',
      '#options' => $options,
      '#required' => TRUE
    );
  }
}

/**
* Alter the view message form
*/
function venture_pm_view_alter(&$form) {
  global $user, $venture_msg_count;
  
  $form['actions']['#type'] = 'item';
  $form['actions']['delete']['#weight'] = 1;
  $form['actions']['reply']['#weight'] = 2;
  unset($form['actions']['text']);
  unset($form['actions']['delete']['#prefix']);
  
  // Remove Reply button form for blocked authors
  $author = $form['author']['#value'];
  if (!venture_profile_is_valid($author) && !venture_profile_is_admin($author)) {
    unset($form['actions']['reply']);
  }
  
  // Bypass delete confirmation
  unset($form['actions']['delete']['#attributes']);
  $form['js_bypass']['#value'] = 1;
  
  // Figure out the current folder
  $message = $form['#parameters'][1];
  $folder = venture_pm_get_current_folder($message);
  
  // Decrement number of new messages if needed
  if ($message->newmsg && $folder == PRIVATEMSG_FOLDER_INBOX) {
    $venture_msg_count--;
  }
  
  // Permanently delete message in the Recycle Bin
  if ($folder == PRIVATEMSG_FOLDER_RECYCLE_BIN && $form['#post']['op'] == 'Delete') {
    if (_privatemsg_delete($message->id, 2) !== FALSE) {
      drupal_set_message(t('Message permanently deleted.'));
      drupal_goto('pm/deleted');
    }
  }
}