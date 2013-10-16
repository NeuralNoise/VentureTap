<div id="login-main">
  <div id="advantage">
    <h1>The VentureTap advantage</h1>
    <p>
      As a private equity investor, you understand the value of networking.
      Business angels and venture capitalists alike can benefit from the
      social aspects of online networks. Here are some of the ways you can
      make VentureTap work for you:
    </p>
    <ul>
      <li>Get advice from the trusted veterans in the field</li>
      <li>Locate potential partners for a promising deal</li>
      <li>Organize into groups based on common interests</li>
      <li>Discover quality investment opportunities</li>
      <li>Communicate with peers worldwide and establish new relationships</li>
    </ul>
  </div>
  <div id="login-container">
    <div id="cont-title">Sign into VentureTap</div>
    <img src="<?php print $image_path ?>/key.png" alt="" title="" />
    <div id="cont-content">
      <?php
        print drupal_render($form['name']);
        print drupal_render($form['pass']);
        print drupal_render($form['remember_me']);
      ?>
      <div id="actions">
        <?php
          print drupal_render($form);
          print l('Forgot your password?', 'password');
        ?>
      </div>  
    </div>
  </div>
</div>
<div id="not-user">
  <strong>Not a user?</strong> VentureTap membership is by invitation only - ask accredited
  investors you know to invite you.<br/>If none of your associates are members and you have
  a strong case for belonging to VentureTap, feel free to <?php print l('contact us', 'contact') ?>.
</div>