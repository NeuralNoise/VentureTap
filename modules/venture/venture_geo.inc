<?php

/**
* Return the location string
*/
function venture_geo_get_location($city, $state, $country) {
  $location = "$city, ";
  $location .= ($state ? $state : $country);
  return $location;
}

/**
* Return the path of the map xml file
*/
function venture_geo_get_map_path() {
  $files = array_keys(file_scan_directory(file_directory_path(), '^map_[0-9]{10}\.xml$'));
  sort($files);
  return end($files);
}

/**
* Geocode location data and create a map file if needed
*/
function venture_geo_process() {
  $geocoded = venture_geo_process_geocodes();
  $existing_map = venture_geo_get_map_path();
  
  if ($geocoded || !$existing_map) {
    venture_geo_process_map($existing_map);
  }
}

/**
* Retrieve geocoded data (lat/long) for user location
*/
function venture_geo_process_geocodes() {
  $geocoded = false;
  
  $query = "SELECT nid FROM {content_type_profile} WHERE LENGTH(IFNULL(field_location_value, '')) = 0";
  $result = db_query($query);
  
  while ($row = db_fetch_object($result)) {
    $profile = node_load($row->nid);
    $city = $profile->field_city[0]['value'];
    $state = $profile->field_state[0]['value'];
    $country = $profile->field_country[0]['value'];
    
    $geo_location = $city . ',' . $state . ',' . $country;
    $geo_query = array('query' => $geo_location, 'maxRows' => 1);
    $geo_result = geonames_query('search', $geo_query);
    
    $location = 'invalid';
    
    if (!$geo_result) {
      watchdog('venture', "Unable to geocode $location");
      continue;
    }
    else if ($geo_result->results) {
      $geocode = $geo_result->results[0];
      $location = $geocode['name'] . '|' . $geocode['lat'] . '|' . $geocode['lng'];
      $geocoded = true;
    }
    
    $profile->field_location[0]['value'] = $location;
    node_save($profile);
  }
  
  return $geocoded;
}

/**
* Generate a map file
*/
function venture_geo_process_map($existing_map) {
  $query = "SELECT field_location_value, field_country_value, field_state_value, COUNT(nid) AS investor_count,
      MIN(field_minimum_investment_value) AS field_minimum_investment_value, 
      MAX(field_maximum_investment_value) AS field_maximum_investment_value
    FROM {content_type_profile} WHERE LENGTH(IFNULL(field_location_value, '')) > 0
      AND field_location_value != 'invalid'
    GROUP BY field_location_value ORDER BY investor_count";
  $result = db_query($query);
  
  $map_data = venture_geo_process_map_xml('default_color', array('color' => 'dadada'));
  $map_data .= venture_geo_process_map_xml('background_color', array('color' => 'ffffff'));
  $map_data .= venture_geo_process_map_xml('outline_color', array('color' => 'c4c4c4'));
  $map_data .= venture_geo_process_map_xml('default_point', array('color' => 'ff0000', 'size' => 7, 'opacity' => 50));
  $map_data .= venture_geo_process_map_xml('scale_points', array('data' => 25));
  $map_data .= venture_geo_process_map_xml('hover', array('font_size' => 13, 'font_color' => 'ffffff', 'background_color' => '51516d' ));
  
  while ($row = db_fetch_object($result)) {
    $geocode = explode('|', $row->field_location_value);    
    $location = venture_geo_get_location($geocode[0], $row->field_state_value, $row->field_country_value);
    $investors = "{$row->investor_count} venture investor";
    $investors = $row->investor_count > 1 ? $investors . 's' : $investors;
    
    $field_minimum_investment = content_fields('field_minimum_investment', 'profile');
    $field_maximum_investment = content_fields('field_maximum_investment', 'profile');
    $min_investment = content_format($field_minimum_investment, $row->field_minimum_investment_value);
    $max_investment = content_format($field_maximum_investment, $row->field_maximum_investment_value);
    
    $map_data .= venture_geo_process_map_xml('point', array(
      'name' => $location,
      'loc' => $geocode[1] . ',' . $geocode[2],
      'hover' => "\n$investors - \n  Minimum investment: $min_investment\n  Maximum investment: $max_investment"
    ));
  }
  
  $map_data = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<countrydata>\n$map_data</countrydata>";
  $map_destination = file_directory_path() . '/map_' . time() . '.xml';
  file_save_data($map_data, $map_destination, FILE_EXISTS_REPLACE);
  
  if ($existing_map) {
    file_delete($existing_map);
  }
}

/**
* Generate an XML node for the map file
*/
function venture_geo_process_map_xml($id, $params) {  
  $output = '';
  if ($params) {
    $output = "  <state id=\"$id\">\n";
    foreach ($params as $name => $value) {
      if($value) 
        $output .= "    <$name>$value</$name>\n";
    }
    $output .= "  </state>\n";
  }
  return $output;
}
  
?> 