<?php

require_once 'template-element.inc';
require_once 'template-form.inc';
require_once 'template-page.inc';
require_once 'template-view.inc';

global $base_path, $image_path, $script_path;
$image_path = $base_path . path_to_theme() . '/images';
$script_path = $base_path . path_to_theme() . '/scripts';

/******** BUILT-IN FUNCTIONS ********/

/**
* Add custom variables
*/
function _phptemplate_variables($hook, $vars = array()) {
  global $user, $image_path, $script_path;
  $vars['image_path'] = $image_path;
  $vars['script_path'] = $script_path;
  
  if ($hook == 'page') {
    // Leave tabs for admin user only
    if (!venture_profile_is_admin($user)) {
      $vars['tabs'] = $vars['tabs2'] = NULL;
    }
    
    if ($user->uid) {
      // Load the user's profile data
      $vars['user_profile'] = venture_profile_retrieve();
    }
    else {
      // Load the template for anonymous users
      $vars['template_files'] = array('page-anonymous');
      $vars['offline'] = variable_get('site_offline', 0);
    }
    
    // Enable Javascript actions
    venture_add_js('jquery.tooltip.pack.js');
    venture_add_js('jquery-ui-1.6.custom.pack.js');
    venture_add_js('script.js');

    // Aggregate the Javascript files - D6 should be able to do this
    $vars['scripts'] = javascript_aggregator_cache(drupal_get_js());
  }
  
  return $vars;
}

/******** THEME OVERRIDE FUNCTIONS ********/

/**
* When theming multi-value CCK view fields, return the
* actual array (compacted) instead of the themed output
*/
function venture_content_view_multiple_field($items, $field, $data) {
  return array_values(array_filter($items));
}

/**
* Override the widget for CCK links (remove extra markup)
*/
function venture_link_widget_form_row($element) {
  return drupal_render($element['url']);
}

/**
* Theme the pager
*/
function venture_pager($tags = array(), $limit = 10, $element = 0, $parameters = array()) {
  global $pager_page_array, $pager_total, $pager_total_items;
  $num_total = $pager_total_items[$element];
  
  // The tags variable is hijacked to allow for custom pagers.
  $single = $tags['single'] ? $tags['single'] : 'Result';
  $plural = $tags['plural'] ? $tags['plural'] : 'Results';
  
  // Create the pager status
  if ($num_total > 1) {
    $cur_page = $pager_page_array[$element];
    $num_first = $limit * $cur_page + 1;
    $num_last = min($num_first + $limit - 1, $num_total);
    $output = "$plural $num_first-$num_last of $num_total";
  }
  else if ($num_total == 1) {
    $output = "$single 1 of 1";
  }
  else {
    // No items - no pager
    return;
  }
  
  if ($tags['link']) {
    $output = "<div>{$tags['link']}</div>$output";
  }
  
  // Create the pager links
  if ($pager_total[$element] > 1) {
    $first = theme('pager_first', 'First', $limit, $element, $parameters);
    $previous = theme('pager_previous', 'Previous', $limit, $element, 1, $parameters);
    $next = theme('pager_next', 'Next', $limit, $element, 1, $parameters);
    $last = theme('pager_last', 'Last', $limit, $element, $parameters);
    
    $first = $first ? $first : 'First';
    $previous = $previous ? $previous : 'Previous';
    $next = $next ? $next : 'Next';
    $last = $last ? $last : 'Last';
    
    $output .= "<span class='pager-links'>$first | $previous | $next | $last</span>";
  }
  
  $output = "<div class='pager'>$output</div>";
  return $output;
}

/**
* Override username rendering
*/
function venture_username($object) {
  if ($object->uid) {
    $name = venture_profile_get_name($object->uid, FALSE);
    if (venture_profile_is_valid($object)) {
      $output = l($name, 'user/'. $object->uid);
    }
    else {
      $output = check_plain($name);
    }
  }
  else {
    $output = variable_get('anonymous', 'Anonymous');
  }
  return $output;
}

/******** UTILITY FUNCTIONS ********/

/**
* Add Javascript file
*/
function venture_add_js($name) {
  drupal_add_js(path_to_theme() . "/scripts/$name");
}

/**
* Enable JQuery Media plugin
*/
function venture_enable_media() {
  global $script_path;
  venture_add_js('jquery.media.pack.js');
  venture_add_js('swfobject2.pack.js');
  drupal_add_js("$(document).ready(function(){ enable_media('$script_path/jwplayer.swf'); })", 'inline');
}

/**
* Render a CCK field
*/
function venture_render_field($node, $field_name, $multiple = FALSE) {
  if (!$node->{$field_name}) return $multiple ? array() : NULL;
  
  $values = array();
  $field = content_fields($field_name, $node->type);
  $format = $node->teaser ? 'teaser' : 'full';
  $formatter = $field['display_settings'][$format]['format'];
  
  foreach($node->{$field_name} as $value) {
    $view = content_format($field, $value, $formatter);
    if (!$multiple) return $view;
    if ($view) $values[] = $view;
  }
  
  return $values;
}

/**
* Render a Views field
*/
function venture_render_view_field($view, $node, $index, $format = TRUE) {
  $fields = _views_get_fields();
  $field = $view->field[$index];
  if ($format) {
    return views_theme_field('views_handle_field', $field['queryname'], $fields, $field, $node, $view);
  }
  else {
    return $node->$field['queryname'];
  }
}

/**
* Trim a string to a given number of letters
*/
function venture_trim_string($string, $length, $end = '...') {
  if (strlen($string) > $length) {
    $length -=  strlen($end);
    $string = substr($string, 0, $length);
    $string .= $end;
  }
  return $string;
}