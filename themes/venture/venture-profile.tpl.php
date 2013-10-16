<?php global $user, $account; ?>
<div id="user-profile">
  <div id="main-left">
    <div class="page-title">
      <h1><?php
        $account_name = venture_profile_get_name($account_profile);
        print $account_name;
      ?></h1>
      <?php print venture_element_render_connection_status($conn_status); ?>
    </div>
    <div class="section">
      <div id="contact"><?php 
        print venture_element_render_contact_info($account_profile, $conn_status);
        print venture_element_render_investor_actions($account->uid, $conn_status, $account_name); 
      ?></div>
      <?php
        print venture_element_render_investor_summary($account_profile);
        
        if (!venture_profile_is_accredited($account_profile)) {
          print "<div id='non-accredited'>$account_name is not an accredited investor</div>";
        }
        
        $fields = venture_element_get_experience($account_profile);
        foreach ($fields as $title => $field) {
          print "<h2>$title</h2>$field"; 
        }
      ?>
    </div>
  </div>
  <div id="main-right">
    <div class="section"><?php
      $photo = venture_render_field($account_profile, 'field_photo');
      if ($user->uid == $account->uid) {
        $path = 'account';
        $fragment = 'photo';
      }
      print venture_element_render_image($photo, $path, $fragment);
    ?></div>
    <div class="page-title">
      <h1>Groups</h1>
      <?php print l('view all', "groups/{$account->uid}"); ?>
    </div>
    <div class="section">
       <?php print theme('view', 'groups_summary', 15, TRUE, 'embed', array($account->uid)); ?>
    </div>
    <div class="page-title">
      <h1>Connections</h1>
      <?php print l('view all', "network/{$account->uid}"); ?>
    </div>
    <div class="section">
      <?php print theme('view', 'network_summary', 15, TRUE, 'embed', array($account->uid, 'all')); ?>
    </div>
  </div>
</div>