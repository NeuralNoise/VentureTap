<div class="search-basic">
  <?php
    print drupal_render($form['filter0']);
    print drupal_render($form['submit']);
  ?>
  <div class="search-adv-link <?php print $advanced ? 'search-hidden' : '' ?>">
    <div>Advanced Search</div>
    <?php print theme('image', 'misc/arrow-asc.png') ?>
  </div>
  <div class="search-basic-link <?php print !$advanced ? 'search-hidden' : '' ?>">
    <div>Basic Search</div>
    <?php print theme('image', 'misc/arrow-desc.png') ?>
  </div>
</div>
<div class="search-adv <?php print !$advanced ? 'search-hidden' : '' ?>">
  <em>These advanced filters will be applied to your search query.</em>
  <div class="search-row">
    <?php
      print drupal_render($form['filter1']);
      print drupal_render($form['filter2']);
      print drupal_render($form['filter3']);
      print drupal_render($form['filter4']);
    ?>
  </div>
  <?php
    print drupal_render($form);
  ?>
</div>