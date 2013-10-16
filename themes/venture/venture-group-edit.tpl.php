<div id="group-edit">
  <div class="page-title"><h1><?php print $page_title ?></h1></div>
  <div class="desc-container">
    <img src="<?php print $image_path ?>/group.png" alt="" title="" />
    <p class="page-desc"><?php print $page_desc ?></p>
  </div>
  <div id="details" class="section">
    <div class="section-header">
      <div class="header-info">
        Enter the essential group information. 
      </div>
      <div class="header-title">Group Details</div>
    </div>
    <div class="section-content">
      <?php
        print drupal_render($form['title']);
        print drupal_render($form['og_description']);
        print drupal_render($form['body_filter']);
        print drupal_render($form['og_selective']);
      ?>
    </div>
  </div>
  <div id="audience" class="section">
    <div class="section-header">
      <div class="header-info">
        Indicate the intended audience for the group. Used for display only, not to restrict access.
      </div>
      <div class="header-title">Group Audience</div>
    </div>
    <div class="section-content">
      <?php
        print drupal_render($form['field_group_investor_type']);
        print drupal_render($form['field_group_country']);
        print drupal_render($form['field_group_state']);
      ?>
    </div>
  </div>
  <div id="image" class="section">
    <div class="section-header">
      <div class="header-info">
        Upload a group image. You may use a JPG, PNG or GIF file no larger than 2 MB in size.
      </div>
      <div class="header-title">Group Image</div>
    </div>
    <div class="section-content">
      <?php
        print drupal_render($form['field_group_image']);
      ?>
    </div>
  </div>
  <div id="submission" class="section">
    <div class="section-header">
      <div class="header-info">
        Customize the submission process for deals pitched directly to the group.
      </div>
      <div class="header-title">Deal Submission</div>
    </div>
    <div class="section-content">
      <?php
        print drupal_render($form['field_group_guidelines']);
      ?>
    </div>
  </div>
  <div id="actions">
    <?php
      print drupal_render($form);
      print l('Cancel', $cancel_path);
    ?>
  </div>
</div>