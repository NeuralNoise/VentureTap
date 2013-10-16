<div id="deal-view">
  <div id="main-left">
    <div class="page-title">
      <h1><?php print $title ?></h1>
      <?php
        $funding_type = strtolower(venture_render_field($deal, 'field_deal_funding_type'));
        print "<div>seeking <strong>$$amount</strong> in $funding_type funding</div>";
      ?>
    </div>
    <div class="section">
      <?php
        $fields = venture_element_get_deal_details($deal);
        foreach ($fields as $title => $field) {
          print "<h2>$title</h2>$field"; 
        }
        
        $attachments = venture_render_field($deal, 'field_deal_attachments', TRUE);
        if ($attachments) {
          print '<h1>Attachments</h1>' . implode('', $attachments);
        }
      ?>
    </div>
  </div>
  <div id="main-right">
    <div id="headline"><?php print venture_render_field($deal, 'field_deal_headline') ?></div>
    <div class="page-title">
      <h1>Company Summary</h1>
    </div>
    <div class="section">
      <table>
        <?php
          $fields = venture_element_get_deal_summary($deal);
          foreach ($fields as $title => $value) {
            print "<tr><td>$title:</td><td><div class='summary-value'>$value</div></td></tr>";
          }
        ?>
      </table>
      <?php
        if ($user->uid == $deal->uid) {
          $click = l('Click here', "deal/{$deal->nid}/edit");
          $msg = "$click to edit or delete this deal.<br/>";
          $msg .= '<span>Others will see a link for contacting you here.</span>';
        }
        else {
          $click = l('Click here', "pm/new/{$deal->uid}");
          $msg = "Interested? $click to contact the deal author.";
        }
        if (user_load(array('uid' => $deal->uid))->status) {
          print "<div id='deal-action'>$msg</div>";
        }
        
        $created = format_date($deal->created, 'custom', 'm/d/Y');
        $title = venture_deal_get_node_title($deal);
        $users = venture_render_field($deal, 'field_deal_users', TRUE);
        $groups = venture_render_field($deal, 'field_deal_groups', TRUE);
        print venture_element_render_deal_posted($deal->nid, $deal->uid, $created, $title, $users, $groups);
      ?>
    </div>
  </div>
</div>