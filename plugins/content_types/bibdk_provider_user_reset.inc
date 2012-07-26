<?php
/**
* This plugin array is more or less self documenting
*/
$plugin = array(
  // the title in the admin
  'title' => t('User reset'),
  // no one knows if "single" defaults to FALSE...
  'single' => TRUE,
  // oh joy, I get my own section of panel panes
  'category' => array(t('User Login CRUD'), -9),
  'render callback' => '_bibdk_provider_get_drupal_user_pass_reset_form',
);

/**
 * Get drupal user_pass_reset form from core user.module 
 * Use form_load_include to ensure the form is included also if it is cached
 */
function _bibdk_provider_get_drupal_user_pass_reset_form($form, &$form_state) {
  $block = new stdClass();
  // get arguments
  $args = func_get_args();
  // real arguments are in [2]
  $real_args = $args[2];
  $path = base_path();
  // remove front and tailing '/'
  $path = str_replace('/','',$path);
  // also remove language prefixes if any
  $go_away = array('en','da',$path);
  
  foreach($go_away as $away) {
    $key = array_search($away, $real_args);
    // key might be 0
    if( $key !== FALSE ) {
      unset($real_args[$key]);
    }
  }
  
  // reset array keys
  $real_args = array_values($real_args);
 
  // set form_state
  if( !isset($form_state) ) {
    $form_state = array();
  }
  // include form file
  form_load_include($form_state, 'inc', 'user','user.pages');
  // get form(user_pass_reset) @args; $uid:$real_args[2], $timestamp:$real_args[3], $hashed_pass:$real_args[4]
  if( isset($real_args[2]) && isset( $real_args[3] ) && isset($real_args[4]) ) {
    $block->content = drupal_get_form('user_pass_reset',$real_args[2],$real_args[3],$real_args[4]);
  }
  return $block;
}
