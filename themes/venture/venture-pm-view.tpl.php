<div id="pm-view">
  <div class="page-title"><h1><?php
    $is_sent = ($current_folder == PRIVATEMSG_FOLDER_SENT);
    $is_deleted = ($current_folder == PRIVATEMSG_FOLDER_RECYCLE_BIN);
    print $is_sent ? 'View sent message' : ($is_deleted ? 'View deleted message' : 'View message');
  ?></h1></div>
  <div class="desc-container">
    <img src="<?php print $image_path ?>/pm-view.png" alt="" title="" />
    <p class="page-desc"><?php
      $page_desc = 'You may view your private message here. If this message is deleted, it will be ';
      $page_desc .= $is_deleted ? 'removed permanently.' : 'moved to the Recycle Bin.';
      print $page_desc;
    ?></p>
  </div>
  <div id="pm-view-msg">  
    <div id="msg-container">
      <div id="msg-header">
        <table>
          <tr> <?php
            global $user;
            if ($message->author == $user->uid) {
              $title = 'To';
              $uid = $message->recipient;
            }
            else {
              $title = 'From';
              $uid = $message->author;
            }
            $name = theme('username', user_load(array('uid' => $uid)));
            print "<td class='title-cell'>$title:</td><td>$name</td>";
          ?></tr>
          <tr>
            <td class="title-cell">Subject:</td>
            <td><?php print check_plain($message->subject) ?></td>
          </tr>
          <tr>
            <td class="title-cell">Date:</td>
            <td><?php print format_date($message->timestamp, 'custom', 'l, m/d/Y - g:i A') ?></td>
          </tr>
        </table>
      </div>
      <div id="message">
        <?php print check_markup($message->message, $message->format, FALSE) ?>
      </div>
    </div>
    <div id="actions"><?php
      print drupal_get_form('privatemsg_view_form', $message);
      print l('Return', "privatemsg/list/$current_folder");
    ?></div>
  </div>
</div>
