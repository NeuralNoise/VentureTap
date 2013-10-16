<?php
// $Id: settings.php,v 1.39.2.3 2007/07/09 04:28:12 drumm Exp $

/**
 * @file
 * Drupal site-specific configuration file.
 *
 * IMPORTANT NOTE:
 * This file may have been set to read-only by the Drupal installation
 * program. If you make changes to this file, be sure to protect it again
 * after making your modifications. Failure to remove write permissions
 * to this file is a security risk.
 *
 * The configuration file to be loaded is based upon the rules below.
 *
 * The configuration directory will be discovered by stripping the
 * website's hostname from left to right and pathname from right to
 * left. The first configuration file found will be used and any
 * others will be ignored. If no other configuration file is found
 * then the default configuration file at 'sites/default' will be used.
 *
 * For example, for a fictitious site installed at
 * http://www.drupal.org/mysite/test/, the 'settings.php'
 * is searched in the following directories:
 *
 *  1. sites/www.drupal.org.mysite.test
 *  2. sites/drupal.org.mysite.test
 *  3. sites/org.mysite.test
 *
 *  4. sites/www.drupal.org.mysite
 *  5. sites/drupal.org.mysite
 *  6. sites/org.mysite
 *
 *  7. sites/www.drupal.org
 *  8. sites/drupal.org
 *  9. sites/org
 *
 * 10. sites/default
 *
 * If you are installing on a non-standard port number, prefix the
 * hostname with that number. For example,
 * http://www.drupal.org:8080/mysite/test/ could be loaded from
 * sites/8080.www.drupal.org.mysite.test/.
 */

/**
 * Database settings:
 *
 * Note that the $db_url variable gets parsed using PHP's built-in
 * URL parser (i.e. using the "parse_url()" function) so make sure
 * not to confuse the parser. If your username, password
 * or database name contain characters used to delineate
 * $db_url parts, you can escape them via URI hex encodings:
 *
 *   : = %3a   / = %2f   @ = %40
 *   + = %2b   ( = %28   ) = %29
 *   ? = %3f   = = %3d   & = %26
 *
 * To specify multiple connections to be used in your site (i.e. for
 * complex custom modules) you can also specify an associative array
 * of $db_url variables with the 'default' element used until otherwise
 * requested.
 *
 * You can optionally set prefixes for some or all database table names
 * by using the $db_prefix setting. If a prefix is specified, the table
 * name will be prepended with its value. Be sure to use valid database
 * characters only, usually alphanumeric and underscore. If no prefixes
 * are desired, leave it as an empty string ''.
 *
 * To have all database names prefixed, set $db_prefix as a string:
 *
 *   $db_prefix = 'main_';
 *
 * To provide prefixes for specific tables, set $db_prefix as an array.
 * The array's keys are the table names and the values are the prefixes.
 * The 'default' element holds the prefix for any tables not specified
 * elsewhere in the array. Example:
 *
 *   $db_prefix = array(
 *     'default'   => 'main_',
 *     'users'     => 'shared_',
 *     'sessions'  => 'shared_',
 *     'role'      => 'shared_',
 *     'authmap'   => 'shared_',
 *     'sequences' => 'shared_',
 *   );
 *
 * Database URL format:
 *   $db_url = 'mysql://username:password@localhost/databasename';
 *   $db_url = 'mysqli://username:password@localhost/databasename';
 *   $db_url = 'pgsql://username:password@localhost/databasename';
 */
$db_url = 'mysqli://PLACEHOLDER:PLACEHOLDER@localhost/venture';
$db_prefix = '';

/**
 * Base URL (optional).
 *
 * If you are experiencing issues with different site domains,
 * uncomment the Base URL statement below (remove the leading hash sign)
 * and fill in the URL to your Drupal installation.
 *
 * You might also want to force users to use a given domain.
 * See the .htaccess file for more information.
 *
 * Examples:
 *   $base_url = 'http://www.example.com';
 *   $base_url = 'http://www.example.com:8888';
 *   $base_url = 'http://www.example.com/drupal';
 *   $base_url = 'https://www.example.com:8888/drupal';
 *
 * It is not allowed to have a trailing slash; Drupal will add it
 * for you.
 */
# $base_url = 'http://www.example.com';  // NO trailing slash!

/**
 * PHP settings:
 *
 * To see what PHP settings are possible, including whether they can
 * be set at runtime (ie., when ini_set() occurs), read the PHP
 * documentation at http://www.php.net/manual/en/ini.php#ini.list
 * and take a look at the .htaccess file to see which non-runtime
 * settings are used there. Settings defined here should not be
 * duplicated there so as to avoid conflict issues.
 */
ini_set('arg_separator.output',     '&amp;');
ini_set('magic_quotes_runtime',     0);
ini_set('magic_quotes_sybase',      0);
ini_set('session.cache_expire',     200000);
ini_set('session.cache_limiter',    'none');
ini_set('session.cookie_lifetime',  2000000);
ini_set('session.gc_maxlifetime',   200000);
ini_set('session.save_handler',     'user');
ini_set('session.use_cookies',      1);
ini_set('session.use_only_cookies', 1);
ini_set('session.use_trans_sid',    0);
ini_set('url_rewriter.tags',        '');

/**
 * Drupal automatically generates a unique session cookie name for each site
 * based on on its full domain name. If you have multiple domains pointing at
 * the same Drupal site, you can either redirect them all to a single domain
 * (see comment in .htaccess), or uncomment the line below and specify their
 * shared base domain. Doing so assures that users remain logged in as they
 * cross between your various domains.
 */
# $cookie_domain = 'example.com';

/**
 * Variable overrides:
 *
 * To override specific entries in the 'variable' table for this site,
 * set them here. You usually don't need to use this feature. This is
 * useful in a configuration file for a vhost or directory, rather than
 * the default settings.php. Any configuration setting from the 'variable'
 * table can be given a new value.
 *
 * Remove the leading hash signs to enable.
 */
# $conf = array(
#   'site_name' => 'My Drupal site',
#   'theme_default' => 'minnelli',
#   'anonymous' => 'Visitor',
# );


/**
* Implementation of a special URL rewriting function
*/
function custom_url_rewrite($type, $path, $original) {
  // This path was already aliased, skip rewriting it
  if ($path != $original) {
    return $path;
  }
  
  // Check if the redirect has already been cached
  static $redirects;
  if ($redirects[$type][$path]) {
    return $redirects[$type][$path];
  }
  
  $redirect = $path;
  if ($type == 'source') {
    $redirect = custom_url_rewrite_source($path);
  }
  else if ($type == 'alias') {
    $redirect = custom_url_rewrite_alias($path);
  }
  
  // Cache the redirect
  $redirects[$type][$path] = $redirect;
  return $redirect;
}

/**
* Rewrite URL coming from the client
*/
function custom_url_rewrite_source($path) {
  global $user;
  
  // Restrict direct access to specific pages to non-admins
  if ($user->uid != 1) {
    $deny = array('node', 'comment', 'filter', 'privatemsg', 'invite/withdraw', 'og', 'activity');
    if ($user->uid) {
      // Must be open for anon users so that admin can login during maintenance
      $deny[] = 'user';
    }
    foreach($deny as $item) {
      if (strncmp($path, $item, strlen($item)) == 0) {
        return NULL;
      }
    }
  }
  
  // Redirect profile page to user page
  if (preg_match('!^profile(/\d+)?$!', $path, $matches)) {
    $append = count($matches) > 1 ? $matches[1] : "/{$user->uid}";
    return 'user' . $append;
  }
  
  // Redirect account page to user bio page
  if (preg_match('!^account(/\d+)?$!', $path, $matches)) {
    $append = count($matches) > 1 ? $matches[1] : "/{$user->uid}";
    return 'user' . $append . '/bio';
  }
  
  // Redirect anon user pages to their correct locations
  if (preg_match('!^(register|login|password|reset)(/.*)?$!', $path, $matches)) {
    return 'user/' . $matches[0];
  }
  
  // Redirect node search pages to the custom search
  if (preg_match('!^search(/.*)?$!', $path, $matches)) {
    $append = count($matches) > 1 ? $matches[1] : '';
    return 'advsearch/node' . $append;
  }
  
  // Redirect private message pages to correct location
  if (preg_match('!^pm(/.*)?$!', $path, $matches)) {
    $redirect = 'privatemsg';
    if (count($matches) > 1) {
      $match = $matches[1];
      if (strncmp($match, '/sent', 5) == 0) {
        $redirect .= '/list/1';
      }
      else if (strncmp($match, '/deleted', 8) == 0) {
        $redirect .= '/list/-1';
      }
      else if (preg_match('!^/(new|reply|view|delete)(/\d+)?$!', $match)) {
        $redirect .= $match;
      }
    }
    return $redirect;
  }
  
  // Redirect group members page to correct location
  if (preg_match('!^group/(\d+)/users$!', $path, $matches)) {
    return 'network/group/' . $matches[1];
  }
  
  return $path;
}

/**
* Rewrite URL going out to the client
*/
function custom_url_rewrite_alias($path) {
  global $user;
  
  if ($path == "home") {
    // Alias home page to root
    return NULL;
  }
  else if ($path == 'user') {
    // Password assistance page redirects to 'user'
    return 'login';
  }

  // Redirect user page to profile page
  if (preg_match('!^user/(\d+)$!', $path, $matches)) {
    $append = $matches[1] == $user->uid ? '' : "/{$matches[1]}";
    return 'profile' . $append;
  }
  
  // Redirect user bio page to account page
  if (preg_match('!^user/(\d+)/bio$!', $path, $matches)) {
    $append = $matches[1] == $user->uid ? '' : "/{$matches[1]}";
    return 'account' . $append;
  }
  
  // Redirect anon user pages to their correct locations
  if (preg_match('!^user/((register|login|password|reset)(/.*)?)$!', $path, $matches)) {
    return $matches[1];
  }
  
  // Clean up the varios URLs for current user
  if (preg_match('!^(network|groups)/(\d+)$!', $path, $matches)) {
    $append = $matches[2] == $user->uid ? '' : "/{$matches[2]}";
    return $matches[1] . $append;
  }
  
  // Redirect node search pages to the custom search
  if (preg_match('!^search/node(/.*)?$!', $path, $matches)) {
    $append = count($matches) > 1 ? $matches[1] : '';
    return 'search' . $append;
  }
  
  // Redirect private message pages to correct location
  if (preg_match('!^privatemsg(/.*)?$!', $path, $matches)) {
    $redirect = 'pm';
    if (count($matches) > 1) {
      $match = $matches[1];
      if (strncmp($match, '/list/1', 7) == 0) {
        $redirect .= '/sent';
      }
      else if (strncmp($match, '/list/-1', 8) == 0) {
        $redirect .= '/deleted';
      }
      else if (preg_match('!^/(new|reply|view|delete)(/\d+)?$!', $match)) {
        $redirect .= $match;
      }
    }
    return $redirect;
  }

  return $path;
}