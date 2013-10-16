<div id="deal-edit">
  <div class="page-title"><h1><?php print $page_title ?></h1></div>
  <div class="desc-container">
    <img src="<?php print $image_path ?>/deal-edit.png" alt="" title="" />
    <div class="page-desc"><?php print $page_desc ?></div>
  </div>
  <?php if ($guidelines): ?>
    <div id="submission" class="section">
      <div class="section-header">
        <div class="header-info">
          Please read the following guidelines before proceeding.
        </div>
        <div class="header-title">Submission Guidelines</div>
      </div>
      <div class="section-content">
        <?php
          print $guidelines;
        ?>
      </div>
    </div>
  <?php endif ?>
  <div id="company" class="section">
    <div class="section-header">
      <div class="header-info">
        Enter the basic company information. 
      </div>
      <div class="header-title">Company Info</div>
    </div>
    <div class="section-content">
      <?php
        print drupal_render($form['title']);
        print drupal_render($form['field_deal_country']);
        print drupal_render($form['field_deal_state']);
        print drupal_render($form['field_deal_city']);
        print drupal_render($form['field_deal_year_founded']);
        print drupal_render($form['field_deal_employees']);
        print drupal_render($form['field_deal_legal_entity']);
        print drupal_render($form['field_deal_website']);
        print drupal_render($form['field_deal_contact_name']);
        print drupal_render($form['field_deal_contact_email']);
        print drupal_render($form['field_deal_contact_phone']);
      ?>
    </div>
  </div>
  <div id="operations" class="section">
    <div class="section-header">
      <div class="header-info">
        Outline the operational specifics and financing history.
      </div>
      <div class="header-title">Operational Details</div>
    </div>
    <div class="section-content">
      <?php
        print drupal_render($form['field_deal_industry']);
        print drupal_render($form['field_deal_funding_stage']);
        print drupal_render($form['field_deal_prior_investors']);
        print drupal_render($form['field_deal_capital_raised']);
        print drupal_render($form['field_deal_current_valuation']);
        print drupal_render($form['field_deal_annual_revenue']);
      ?>
    </div>
  </div>
  <div id="financing" class="section">
    <div class="section-header">
      <div class="header-info">
        Establish the deal financing requirements.
      </div>
      <div class="header-title">Deal Financing</div>
    </div>
    <div class="section-content">
      <?php
        print drupal_render($form['field_deal_amount_seeking']);
        print drupal_render($form['field_deal_funding_type']);
      ?>
    </div>
  </div>
  <div id="summary" class="section">
    <div class="section-header">
      <div class="header-info">
        Provide a detailed description of the deal and all parties involved.
      </div>
      <div class="header-title">Executive Summary</div>
    </div>
    <div class="section-content">
      <?php
        print drupal_render($form['field_deal_headline']);
        print drupal_render($form['field_deal_company_overview']);
        print drupal_render($form['field_deal_product_service']);
        print drupal_render($form['field_deal_management_team']);
        print drupal_render($form['field_deal_details']);
      ?>
    </div>
  </div>
  <div id="attachments" class="section">
    <div class="section-header">
      <div class="header-info">
        Upload relevant documents. The Flash FLV format is recommended for videos. Maximum file size is 30 MB.
      </div>
      <div class="header-title">Attachments</div>
    </div>
    <div class="section-content">
      <?php
        print drupal_render($form['field_deal_attachments']);
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