<div id="user-home">
  <div class="page-title">
    <h1>Welcome to VentureTap!</h1>
    <div id="title-extra"><strong>My stats:</strong>
      <?php
        $connections = count($buddies);
        $groups = count($account->og_groups);
        $posts = venture_group_get_post_count($account->uid);
        $deals = venture_deal_get_count();
        
        $stats = "$connections " . format_plural($connections, 'connection', 'connections');
        $stats .= ", $groups " . format_plural($groups, 'group', 'groups');
        $stats .= ", $posts group " . format_plural($posts, 'post', 'posts');
        $stats .= ", $deals " . format_plural($deals, 'deal', 'deals');
        
        print $stats;
      ?>
    </div>
  </div>
  <div class="desc-container">
    <img src="<?php print $image_path ?>/home.png" alt="" title="" />
    <div class="page-desc">
      <?php
        $complete = 'Complete your ' . l('profile', 'profile') . ' with a description of your ' .
          l('experience', 'account', array(), NULL, 'experience') . ' and a profile ' .
          l('photo', 'account', array(), NULL, 'photo');
        $explore = 'Explore the existing ' . l('groups', 'groups') . ' or ' .
          l('create', 'group/new') . ' your own';
        $search = 'Search our ' . l('network', 'network') . ' for investment partners';
        $discover = 'Discover quality ' . l('deals', 'deals') . ' or ' . l('post', 'deal/new') . ' a new one';
        $grow = 'Grow your personal network by ' . l('inviting', 'invite') . ' investors you know';
      ?>
      Not sure what to do next? Try the following:
      <ul>
        <li><?php print $complete ?></li>
        <li><?php print $explore ?></li>
        <li><?php print $search ?></li>
        <li><?php print $discover ?></li>
        <li><?php print $grow ?></li>
      </ul>
    </div>
  </div>
  <div id="home-left">
    <?php
      if (venture_pm_get_msg_count()) {
        $messages = venture_element_render_new_messages($account->uid);
        print "<h1>New Messages</h1>$messages";
      }
    ?>
    <h1>Activity Feed</h1>
    <?php
      $requesters = venture_network_get_requesting_users($account->uid);
      print venture_element_render_requests($requesters, 'connection', "network/accept", "network/deny");
      print venture_element_render_activities($account->uid, $buddies);
    ?>
    <h1>Site Updates</h1>
    <div id="accordion">
      <div class="accordion-header">
        <div class="accordion-icon"></div>
        <div class="accordion-title">
          <strong>Members</strong> updated on
          <?php
            $current_view = views_build_view('embed', $network_view,  array(), TRUE, 5);
            print $network_view->latest_date;
          ?>
        </div>
        <div class="accordion-latest">
          <?php print venture_trim_string($network_view->latest, 40) ?>
        </div>
      </div>
      <div class="accordion-container">
        <div class="accordion-content"><?php print $current_view ?></div>
        <div class="accordion-footer">
          <?php
            print $network_view->total_rows . ' ' . format_plural($network_view->total_rows, 'member', 'members');
            print ' - ' . l('invite', 'invite') . ' - ' . l('view all', 'network/all'); 
          ?>
        </div>
      </div>
      <div class="accordion-header">
        <div class="accordion-icon"></div>
        <div class="accordion-title">
          <strong>Group Posts</strong>
          <?php
            $view_arg = $is_admin ? 'all' : $account->uid;
            $current_view = views_build_view('embed', $topic_view,  array($view_arg), TRUE, 5);
            if ($topic_view->total_rows) {
              print 'updated on ' . $topic_view->latest_date;
            }
          ?>
        </div>
        <div class="accordion-latest">
          <?php print venture_trim_string($topic_view->latest, 40) ?>
        </div>
      </div>
      <div class="accordion-container">
        <div class="accordion-content"><?php print $current_view ?></div>
        <div class="accordion-footer">
          <?php
            $posts = venture_group_get_post_count();
            print $posts . ' group ' . format_plural($posts, 'post', 'posts');
            print ' - ' . l('view my groups', 'groups'); 
          ?>
        </div>
      </div>
      <div class="accordion-header">
        <div class="accordion-icon"></div>
        <div class="accordion-title">
          <strong>Groups</strong> updated on
          <?php
            $current_view = views_build_view('embed', $group_view,  array(), TRUE, 5);
            print $group_view->latest_date;
          ?>
        </div>
        <div class="accordion-latest">
          <?php print venture_trim_string($group_view->latest, 40) ?>
        </div>
      </div>
      <div class="accordion-container">
        <div class="accordion-content"><?php print $current_view ?></div>
        <div class="accordion-footer">
          <?php
            print $group_view->total_rows . ' ' . format_plural($group_view->total_rows, 'group', 'groups');
            print ' - ' . l('create new', 'group/new') . ' - ' . l('view all', 'groups/all');
          ?>
        </div>
      </div>
      <div class="accordion-header">
        <div class="accordion-icon"></div>
        <div class="accordion-title">
          <strong>Deals</strong>
          <?php
            $current_view = views_build_view('embed', $deal_view,  array(), TRUE, 5);
            if ($deal_view->total_rows) {
              print 'updated on ' . $deal_view->latest_date;
            }
          ?>
        </div>
        <div class="accordion-latest">
          <?php print venture_trim_string($deal_view->latest, 40) ?>
        </div>
      </div>
      <div class="accordion-container">
        <div class="accordion-content"><?php print $current_view ?></div>
        <div class="accordion-footer">
          <?php
            print $deal_view->total_rows . ' ' . format_plural($deal_view->total_rows, 'deal', 'deals');
            print ' - ' . l('post new', 'deal/new') . ' - ' . l('view all', 'deals');
          ?>
        </div>
      </div>
    </div>
    <div id="home-bottom">
      <?php
        $send_img = "<img src='$image_path/pm-small.png' alt='' title='' />";
        print l($send_img, 'pm/new/1', array(), NULL, NULL, FALSE, TRUE);
        print l('Send us a message', 'pm/new/1');
      ?>
      <a href="http://twitter.com/VentureTap" rel="external">
        <img id="twitter" src="<?php print $image_path ?>/twitter.png" alt="" title="" /></a>
      <a href="http://twitter.com/VentureTap" rel="external">Follow us on Twitter</a>
    </div>
  </div>
  <div id="home-right">
    <a href="<?php print url('invite') ?>">
      <img id="growth" src="<?php print $image_path ?>/growth.png" alt="" title="" />
    </a>
    <h2>Expand your network</h2>
    <div class="right-section">
      <div class="right-group">
        As a private community, the quality as well as quantity of our members depend on you.
      </div>
      <div class="right-group">  
        Get involved in growing our investor community! 
      </div>
      <div class="right-group">
        <?php print l('Invite qualified investors now', 'invite') ?>
      </div>
    </div>
    <div>
      <script type="text/javascript" src="http://widgets.twimg.com/j/2/widget.js"></script>
      <script type="text/javascript">twitter_search();</script>
    </div>
  </div>
</div>