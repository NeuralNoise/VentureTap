<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"> 
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
  <title><?php print $head_title ?></title>
  <?php 
    print $styles;
    print $scripts;
  ?>
</head>
<body>
<div id="border">
<div id="container">
  <div id="menu">
    <a href="<?php print $base_path ?>">
      <img src="<?php print $image_path ?>/logo.png" alt="VentureTap" title="" />
    </a>
    <div id="menu-acct">
      <div><?php print venture_profile_get_name($user_profile) ?></div>
      <?php
        $new_msgs = venture_pm_get_msg_count();
        $inbox = ($new_msgs > 0) ? "Inbox ($new_msgs)" : 'Inbox';
        print l($inbox, 'pm') . ' | ' . l('Account', 'account') . ' | ' . l('Sign Out', 'logout');
      ?>
    </div>
    <div id="menu-links"><?php
      $menu_items = array('home', 'profile', 'network', 'groups', 'deals');
      print venture_element_render_menu($menu_items);
    ?></div>
    <?php print drupal_get_form('search_block_form') ?>
  </div>
  <div id="shadow"></div>
  <!--[if lt IE 7]>
    <div class="messages warning">
      This browser is no longer supported. Please upgrade to the latest 
        <a href="http://www.microsoft.com/windows/Internet-explorer">Internet Explorer</a>, 
        <a href="http://www.mozilla.com/firefox">Mozilla Firefox</a>, or 
        <a href="http://www.google.com/chrome">Google Chrome</a>.
    </div>
  <![endif]-->
  <?php 
    if ($messages) print $messages;
    if ($tabs) print $tabs;
    if (isset($tabs2)) print $tabs2;
  ?>
  <div id="content"><?php print $content ?></div>
  <div id="footer">
    <div id="copyright">Copyright &copy; 2010 VentureTap.com</div>
    <div id="footer-links">
      <?php
        $home = l('Home', '');
        $blogs = l('Blogs', 'blogs');
        $about = l('About Us', 'about');
        $contact = l('Contact Us', 'contact');
        $legal = l('Legal', 'legal');
        print "$home | $blogs | $about | $contact | $legal";
      ?>
    </div>
  </div>
</div>
</div>
<div id="corner"></div>
<?php print $closure ?>

<script type="text/javascript">
var gaJsHost = (("https:" == document.location.protocol) ? "https://ssl." : "http://www.");
document.write(unescape("%3Cscript src='" + gaJsHost + "google-analytics.com/ga.js' type='text/javascript'%3E%3C/script%3E"));
</script>
<script type="text/javascript">
try {
var pageTracker = _gat._getTracker("PLACEHOLDER");
pageTracker._trackPageview();
} catch(err) {}
</script>

</body>
</html>