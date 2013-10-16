function actions_init() {
  $(".record-actions a[@id^=action]").click(function(){
    var dialog = $("#" + this.id + "-dialog");
    var ok_func = function(caller) {
      load_async(caller);
      return false;
    };
    var title = $(this).attr("title") ? $(this).attr("title") : $(this).text();
    create_dialog(dialog, this, title, ok_func);
    return false;
  });
}
function create_dialog(item, caller, title, ok_func) {
  $(item).dialog({
    modal: true,
    overlay: { opacity: 0.5, background: "black" },
    height: "auto",
    title: title.substring(0, 1).toUpperCase() + title.substring(1, title.length),
    autoopen: false,
    buttons: {
      "OK": function(){
              var close = true;
              ok_button.attr("disabled", true);
              if (ok_func) close = ok_func(caller, item);
              if (close) $(this).dialog("close");
            },
      "Cancel": function(){ $(this).dialog("close"); }
    }
  });
  $(item).dialog('open');
  var ok_button = $(".ui-dialog:visible button:first");
  ok_button.focus();
}
function create_submit_dialog(btn, title, msg) {
  var dialog = $("<div>" + msg + "</div>");
  var ok_func = function(caller) {
    caller.unbind('click');
    caller.click();
    return false;
  };
  create_dialog(dialog, btn, title, ok_func);
}
function create_tooltips(item) {
  item.each(function(){
    var title = $(this).attr("title");
    if (title) {
      $(this).attr("title", "");
      $(this).parent().attr("title", title).tooltip({delay: 0, track: true, showBody: " - "});
    }
  });
}
function cur_view_init(path) {
  if (path == undefined) {
    path = "#current-view";
  }
  $(path).change(function(){
    window.location.href = $(this).val();
  });
}
function enable_media(player) {
  var audioRegex = new RegExp('\.(aif|aiff|aac|au|gsm|mid|midi|mp3|m4a|snd|ra|ram|wav|wma)$');
  var videoRegex = new RegExp('\.(asf|avi|flv|mov|mpg|mpeg|mp4|qt|rm|rv|rpm|smi|smil|swf|wmv|3g2|3gp|xaml)$');
  
  $.fn.media.defaults.flvPlayer = player;
  $.fn.media.defaults.mp3Player = player;

  $('.filefield-item a').each(function(){
    var height = 0;
    if (audioRegex.test($(this).text())) {
      height = 20;
    }
    else if (videoRegex.test($(this).text())) {
      height = 300;
    }
    
    if (height) {
      $(this).clone().insertAfter($(this)).media({
        width: 400,
        height: height,
        params: { allowfullscreen: 'true' } 
      });
    }
  });
}
function init_account_profile() {
  init_shared_profile();
  submit_tab_on_enter("tab-container");
  
  var tabs = $(".tab-container");
  var tab = tabs.filter(":has(.error):first");
  if (tab.size()) {
    $(".tabset").tabs({selected: tabs.index(tab)});
  }
  else {
    $(".tabset").tabs();
  }
}
function init_deal_edit() {
  actions_init();
  submit_tab_on_enter("section");
  states_init("#edit-field-deal-country-key", "#edit-field-deal-state-key");
  create_tooltips($(".form-radios input"));
  
  var investorsId = "#edit-field-deal-prior-investors-keys";
  $(investorsId).change(function(){ toggle_investors("#" + this.id); }); 
  toggle_investors(investorsId);
  
  $("#edit-delete").click(function(){
    create_submit_dialog($(this), $(this).val(), "Are you sure you want to delete this deal?");
    return false;
  });
}
function init_deal_view() {
  actions_init();
  set_external_links();
}
function init_deals() {
  actions_init();
  search_init();
  cur_view_init("#groups select");
}
function init_front() {
  set_external_links();
}
function init_home() {
  actions_init();
  load_accordion();
  set_external_links();
}
function init_group_edit() {
  submit_tab_on_enter("section");
  states_init("#edit-field-group-country-key", "#edit-field-group-state-key");
  
  $("input[name=og_selective]").click(function(){ toggle_membership(); }); 
  toggle_membership();
}
function init_group_view() {
  actions_init();
}
function init_groups() {
  actions_init();
  search_init();
  cur_view_init();
}
function init_invite() {
  set_external_links();
}
function init_invite_history() {
  $("a.withdraw").click(function(){
    var email = $(this).parent().parent().find(".email").text();
    var dialog = $("<div>You are about to withdraw your invitation to " + email + ". Proceed?</div>");
    var ok_func = function(caller) {
      load_async(caller);
      return false;
    };
    create_dialog(dialog, this, "Withdraw Invitation", ok_func);
    return false;
  });
  $("a.delete").click(function(){
    load_async(this);
    return false;
  });
}
function init_network() {
  actions_init();
  search_init();
  cur_view_init();
}
function init_pm_list() {
  $("#edit-delete").attr("msg", "Are you sure you want to delete ");
  $("#edit-permanent").attr("msg", "Are you sure you want to permanently delete ");
  $("input[@msg]").click(function(){
    var checked = $("td input[@type='checkbox']:checked").size();
    if (checked) {
      var msg = $(this).attr("msg") + ((checked > 1) ? "these messages?" : "this message?");
      create_submit_dialog($(this), $(this).val(), msg);
      return false;
    }
  });
  $("#edit-empty").click(function(){
    create_submit_dialog($(this), $(this).val(),
      "Are you sure you want to permanently delete all messages in the Recycle Bin?");
    return false;
  });
}
function init_pm_view() {
  var msg = ($(".page-title h1").text().indexOf("deleted") == -1) ? "delete" : "permanently delete";
  msg = "Are you sure you want to " + msg + " this message?";
  $("#edit-delete").click(function(){
    create_submit_dialog($(this), "Delete Message", msg);
    return false;
  });
}
function init_post_view() {
  actions_init();
}
function init_profile() {
  set_external_links();
  actions_init();
}
function init_register_profile() {
  init_shared_profile();
  set_external_links();
  
  var tzOffset = new Date().getTimezoneOffset() * -60;
  $("#edit-user-register-timezone").val(tzOffset);
}
function init_search() {
  search_init();
}
function init_shared_profile() {
  states_init("#edit-field-country-key", "#edit-field-state-key");
  states_init("#edit-field-countries-keys", "#edit-field-states-keys");
  scroll_init("#edit-field-industries-keys");
  scroll_init("#edit-field-countries-keys");
  scroll_init("#edit-field-states-keys");  
  create_tooltips($(".form-checkboxes input"));
}
function load_accordion() {
  $(window).load(function(){
    $("#accordion").show();
    var accordionFooterHeight = $(".accordion-footer").outerHeight();
    $("#accordion").accordion({
      event: "mousemove click",
      header: ".accordion-header",
      clearStyle: true
    });
    $(".accordion-content").each(function(){
      $(this).height($(this).parent().outerHeight() - accordionFooterHeight);
    });
  });
}
function load_async(link) {
  var cb = function(){ window.location.reload(); };
  $.get(link.href, null, cb);
}
function scroll_init(selectId) {
  $(selectId).resize(function(){
    scroll_to_selected("#" + this.id);
  });
  setTimeout("scroll_to_selected('" + selectId + "')", 0);
}
function scroll_to_selected(selectId) {
  var selected = $(selectId + " :selected");
  for(var i = selected.length - 1; i >= 0; i--) {
    selected.get(i).selected = true;
  }
}
function search_init() {
  $(".search-adv-link div, .search-basic-link div").click(function(){
    $(this).parent().hide();
    $(this).parent().siblings(":hidden").show();
    $(".search-adv").toggle();
    return false;
  });
}
function set_external_links() {
  $("a[@rel=external]").click(function(){
    window.open($(this).attr("href"), '_blank');
    return false;
  });
}
function states_init(countriesId, statesId) {
  $(countriesId).attr("statesId", statesId).change(function(){ toggle_states("#" + this.id); }); 
  toggle_states(countriesId);
}
function submit_tab_on_enter(className) {
  $("." + className).keydown(function(event){
    if (event.keyCode == 13 && event.target.type != "textarea") {
      $("#edit-submit")[0].click();
      return false;
    }
  });
}
function toggle_investors(investorsId) {
  var selectedValue = "";
  $(investorsId + " :selected").each(function(){
    if ($(this)[0].selected) {
      selectedValue += $(this).val();
    }
  });
  var raised = $("#edit-field-deal-capital-raised-0-value-wrapper");
  var valuation = $("#edit-field-deal-current-valuation-0-value-wrapper");
  if (selectedValue == "None") {
    raised.hide();
    valuation.hide();
  }
  else {
    raised.show();
    valuation.show();
  }
}
function toggle_membership() {
  var membership = $("input[name=og_selective]:checked").val();
  membership == 1 ? $("#submission").show() : $("#submission").hide();
}
function toggle_states(countriesId) {
  var selectedValue = "";
  $(countriesId + " :selected").each(function(){
    if ($(this)[0].selected) {
      selectedValue += $(this).val();
    }
  });
  var states = $($(countriesId).attr("statesId")).parent();
  selectedValue == "United States" ? states.show() : states.hide();
}
function twitter_search() {
  new TWTR.Widget({
    version: 2,
    type: 'search',
    search: '\"angel investor\" OR \"angel investors\" OR \"angel investment\" OR ' +
            '\"venture capital\" OR \"venture capitalist\" OR \"venture capitalists\"',
    interval: 5000,
    title: 'Venture buzz',
    width: 195,
    height: 300,
    features: {
      scrollbar: false,
      loop: true,
      live: true,
      hashtags: true,
      timestamp: true,
      avatars: true,
      behavior: 'default'
    }
  }).render().start();
}