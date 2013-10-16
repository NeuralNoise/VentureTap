<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Strict//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd"> 
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
  <title><?php print $head_title ?></title>
  <meta name="description" content="Tap into the venture community with VentureTap.com,
    a private social network for business angels and venture capitalists." />
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
    <?php
      if (!$offline) {
        print l('Sign In', 'login', array('id' => 'menu-signin'));
      }
    ?>
    <div id="menu-slogan">Tap into the venture community</div>
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
  <?php if ($messages) print $messages ?>
  <div id="content"><?php print $content ?></div>
  <div id="footer">
    <div id="copyright">Copyright &copy; 2010 VentureTap.com</div>
    <div id="footer-links">
      <?php
        if (!$offline) {
          $home = l('Home', '');
          $blogs = l('Blogs', 'blogs');
          $about = l('About Us', 'about');
          $contact = l('Contact Us', 'contact');
          $legal = l('Legal', 'legal');
          print "$home | $blogs | $about | $contact | $legal";
        }
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