<div id="user-deals">
  <div class="page-title">
    <h1><?php print $title ?></h1>
  </div>
  <div class="desc-container">
    <img src="<?php print $image_path ?>/deals.png" alt="" title="" />
    <div class="page-desc">
      <?php
        print "<p>$page_desc</p>";
        print l('Post new public deal', 'deal/new') ;
      ?>
    </div>
  </div>
  <ul class="ui-tabs-nav">
    <?php
      foreach($tabs as $tab_name => $tab) {
        $tab_count = venture_views_get_node_count('deals', $tab['args']);
        $tab_link = l("<span>$tab_name ($tab_count)</span>", $tab['url'], array(), NULL, NULL, FALSE, TRUE);
        print "<li class='{$tab['class']}'>$tab_link</li>";
        
        if ($tab['class']) {
          $view_args = $tab['args'];
          if (arg(1) == 'group' && in_array(arg(2), $groups)) {
            $view_args[2] = array(arg(2));
          }
        }
      }
    ?>
  </ul>
  <div id="deal-main" class="ui-tabs-panel">
    <?php if ($groups): ?>
      <div id="groups">
        <img src="<?php print $image_path ?>/arrow-right.gif" alt="" title="" />
        Filter by group: 
        <select>
          <option value="<?php print url("deals/group") ?>">-- All Moderated Groups --</option>
          <?php
            $counts = venture_deal_get_group_counts($groups);
            foreach($groups as $gid) {
              $count = $counts[$gid] ? $counts[$gid] : 0;
              $group_path = url("deals/group/$gid");
              $group_name = check_plain($user->og_groups[$gid]['title']) . " ($count)";
              $selected = $gid == arg(2) ? 'selected="selected"' : '';
              if ($selected) {
                $selected_gid = $gid;
              }
              print "<option value='$group_path' $selected>$group_name</option>";
            }
          ?>
        </select>
        <?php
          if ($selected_gid) {
            print l('Group home', "group/$selected_gid");
            print l('Submit a deal', "group/$selected_gid/submit");
          }
        ?>
      </div>
    <?php
      elseif (is_array($groups)):
        print "Deals can be submitted to moderated groups only. You do not currently belong to a moderated group. ";
        print l("Click here", "groups/all", null, "filter1=1") . " to find one."; 
      endif;
      
      print theme('view', 'deals', 10, TRUE, 'embed', $view_args);
    ?>
  </div>
</div>