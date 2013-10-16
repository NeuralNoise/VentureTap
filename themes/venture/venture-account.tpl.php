<div id="user-account">
  <div class="page-title"><h1>My account</h1></div>
  <div class="desc-container">
    <img src="<?php print $image_path ?>/account.png" alt="" title="" />
    <p class="page-desc">
      You may configure your account information here. 
      Phone number and email address are visible to your connections only.
    </p>
  </div>
  <ul class="tabset">
    <li><a href="#contact"><span>Contact</span></a></li>
    <li><a href="#investments"><span>Investments</span></a></li>
    <li><a href="#experience"><span>Experience</span></a></li>
    <li><a href="#photo"><span>Photo</span></a></li>
    <li><a href="#advanced"><span>Advanced</span></a></li>
  </ul>
  <div id="contact" class="tab-container ui-tabs-hide contact">
    <div class="tab-panel">
      <div class="tab-info">
        Enter your basic contact information. Name and email address can be changed in the <strong>Advanced</strong> section.
      </div>
      <div class="tab-content">
        <?php
          print drupal_render($form['field_organization']);
          print drupal_render($form['field_title']);
          print drupal_render($form['field_country']);
          print drupal_render($form['field_state']);
          print drupal_render($form['field_city']);
          print drupal_render($form['field_phone']);
          print drupal_render($form['field_website']);
        ?>
      </div>
    </div>
  </div>
  <div id="investments" class="tab-container ui-tabs-hide investments">
    <div class="tab-panel" id="criteria">
      <div class="tab-info">Provide your investment criteria.  Make sure you select all options that apply.</div>
      <div class="tab-content">
        <?php
          print drupal_render($form['field_investor_type']);
          print drupal_render($form['field_funding_stages']);
          print drupal_render($form['field_industries']);
          print drupal_render($form['field_investment_preferences']);
          print drupal_render($form['field_other_preferences']);
        ?>
        <div id="geo-area"><?php
          print drupal_render($form['field_countries']);
          print drupal_render($form['field_states']);
        ?></div>
        <?php
          print drupal_render($form['field_minimum_investment']);
          print drupal_render($form['field_maximum_investment']);
        ?>
      </div>
    </div>
    <div class="tab-panel" id="submission">
      <div class="tab-info">Customize the submission process for deals pitched directly to you.</div>
      <div class="tab-content">
        <?php
          print drupal_render($form['field_guidelines']);
        ?>
      </div>
    </div>
  </div>
  <div id="experience" class="tab-container ui-tabs-hide experience">
    <div class="tab-panel">
      <div class="tab-info">Introduce yourself and describe your background as an industry insider and investor.</div>
      <div class="tab-content">
        <?php
          print drupal_render($form['field_about_me']);
          print drupal_render($form['field_industry_experience']);
          print drupal_render($form['field_investing_experience']);
        ?>
      </div>
    </div>
  </div>
  <div id="photo" class="tab-container ui-tabs-hide photo">
    <div class="tab-panel">
      <div class="tab-info">Upload a profile photo. You may use a JPG, PNG or GIF file no larger than 2 MB in size.</div>
      <div class="tab-content">
        <?php print drupal_render($form['field_photo']); ?>
      </div>
    </div>
  </div>
  <div id="advanced" class="tab-container ui-tabs-hide advanced">
    <div class="tab-panel">
      <div class="tab-info">Update your account settings. Current password must be provided in order to make changes.</div>
      <div class="tab-content">
        <?php
          print drupal_render($form['field_first_name']);
          print drupal_render($form['field_last_name']);
          print drupal_render($form['mail']);
          print drupal_render($form['field_email_alerts']);
          print drupal_render($form['timezone']);
          print drupal_render($form['pass']);
        ?>
      </div>
      <div id="current-pass">
        <?php print drupal_render($form['current_pass']); ?>
      </div>
    </div>
  </div>
  <div id="actions">
    <?php
      print drupal_render($form);
      print l('Cancel', 'profile');
    ?>
  </div>
</div>