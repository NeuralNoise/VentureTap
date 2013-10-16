<div class="page-title"><h1>Open a new account</h1></div>
<div class="desc-container">
  <img src="<?php print $image_path ?>/register.png" alt="" title="" />
  <div class="page-desc">
    <strong>Welcome to VentureTap, a private social network for angel investors and venture capitalists!</strong>
    <p>
      Please provide the requested information about yourself in order to create a new account.
      Your phone number and email address will be visible to your connections only.
    </p>
    <?php print l('Learn about VentureTap', 'about') ?>
  </div>
</div>
<div id="personal" class="section">
  <div class="section-header">
    <div class="header-info">
      Enter your basic contact information. The email address will be used to sign in. 
    </div>
    <div class="header-title">Personal Profile</div>
  </div>
  <div class="section-content">
    <?php
      print drupal_render($form['field_first_name']);
      print drupal_render($form['field_last_name']);
      print drupal_render($form['field_organization']);
      print drupal_render($form['field_title']);
      print drupal_render($form['field_country']);
      print drupal_render($form['field_state']);
      print drupal_render($form['field_city']);
      print drupal_render($form['field_phone']);
      print drupal_render($form['mail']);
      print drupal_render($form['field_website']);
      print drupal_render($form['pass']);
    ?>
  </div>
</div>
<div id="investor" class="section">
  <div class="section-header">
    <div class="header-info">
      Provide your investment criteria. Make sure you select all options that apply. 
    </div>
    <div class="header-title">Investor Profile</div>
  </div>
  <div class="section-content">
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
<div id="qualify" class="section">
  <div class="section-header">
    <div class="header-info">
      You must be a qualified investor in order to open a new account. 
    </div>
    <div class="header-title">Investor Qualification</div>
  </div>
  <div class="section-content">
    VentureTap membership is open to accredited investors as defined by SEC under
    <a href="http://www.sec.gov/answers/accred.htm" rel="external">Regulation D, Rule 501</a>
    as well as non-accredited international investors that invest outside of the United States.
    You qualify as an accredited investor if
    <ul>
      <li>Your net worth, or joint net worth with your spouse, exceeds $1 million; or</li>
      <li>
        Your income exceeded $200,000 in each of the two most recent years, or joint income
        with your spouse exceeded $300,000 in each of those years, and you have a reasonable
        expectation of reaching the same income level in the current year; or
      </li>
      <li>You are a principal or fiduciary of an institution listed in Rule 501(a)</li>
    </ul>
    <?php print drupal_render($form['field_qualification']); ?>
  </div>
</div>
<?php print drupal_render($form); ?>