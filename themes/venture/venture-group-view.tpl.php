<?php global $user ?>
<div id="group-view">
  <div id="main-left">
    <div class="page-title">
      <h1><?php print check_plain($group->title) ?></h1>
      <?php print venture_element_render_membership_status($member_status); ?>
    </div>
    <div id="group-desc">
      <?php print check_plain($group->og_description) ?>
    </div>
    <div class="section">
      <?php if ($member_status == VENTURE_GROUP_MEMBER || $is_admin): ?>
        <?php
          if ($user->uid == $group->uid) {
            $requesters = venture_group_get_requesting_users($gid);
            print venture_element_render_requests($requesters, 'group membership', "group/$gid/add", "group/$gid/remove");
          }
        ?>
        <?php if ($group->body): ?>
          <div id="note">
             <img src="<?php print $image_path ?>/note.png" alt="" title="" />
             <div><?php print check_markup($group->body, $group->format, FALSE) ?></div>
          </div>
        <?php endif ?>
        <?php print theme('view', 'topics', 10, TRUE, 'embed', array($gid)) ?>
      <?php
        else:
          $msg = 'You cannot see the contents of this group, because you are not a member.<br/>';
          if ($group->og_selective == OG_MODERATED) {
            $msg .= 'This is a moderated group, so your membership must first be approved by the group owner.<br/>';
            $msg .= 'Use the link on the right to submit a membership request.';
          }
          else {
            $msg .= 'Use the link on the right to join.';
          }
          print "<div id='join-msg'>$msg</div>";
        endif;
      ?>
    </div>
  </div>
  <div id="main-right">
    <div class="section"><?php
      $image = venture_render_field($group, 'field_group_image');
      if ($user->uid == $group->uid) {
        $path = "group/$gid/edit";
        $fragment = 'image';
      }
      print venture_element_render_image($image, $path, $fragment);
      print venture_element_render_group_actions($gid, $member_status, $group->og_selective, $group->title, $group->uid);
    ?></div>
    <div class="page-title">
      <h1>Details</h1>
    </div>
    <div class="section">
       <?php print venture_element_render_group_details($group) ?>
    </div>
    <?php if ($member_status == VENTURE_GROUP_MEMBER || $is_admin): ?>
      <div class="page-title">
        <h1>Members</h1>
        <?php print l('view all', "group/$gid/users"); ?>
      </div>
      <div class="section">
        <?php print theme('view', 'network_summary', 15, TRUE, 'embed', array('all', $group->nid)); ?>
      </div>
    <?php endif ?>
  </div>
</div>