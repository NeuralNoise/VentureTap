<div id="user-invite">
  <div id="invite-main">
    <h1>Know any venture capitalists or angel investors?</h1>
    <div id="page-description">
      <img src="<?php print $image_path ?>/alert.png" alt="" title="" />
      VentureTap membership is by invitation only, so our growth is in your hands.
      The quality of our investor community is directly proportional to the quality of candidates
      you choose to invite. Invite people you know and trust and they will do the same,
      benefiting you in the long run.
    </div>
    <em>
      Please limit invitations to <a href="http://www.sec.gov/answers/accred.htm" rel="external">accredited investors</a>
      and non-accredited international investors that invest outside of the United States.
    </em>
    <?php print drupal_render($form); ?>
  </div>
  <div id="invitations">
    <div id="invitations-title">
      <h2>My Invitations</h2>
      <?php print l('view all', 'invite/history'); ?>
    </div>
    <?php 
      print venture_element_render_invites('pending');
      print venture_element_render_invites('accepted');
      print venture_element_render_invites('expired');
    ?>
  </div>
</div>