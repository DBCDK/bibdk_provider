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
  $args = func_get_args();
  // real arguments (uid, timestamp, and hash) are in [2]
  $real_args = $args[2];

  // set form_state
  if( !isset($form_state) ) {
    $form_state = array();
  }
  // include form file
  form_load_include($form_state, 'inc', 'user','user.pages');

  if (isset($real_args[0]) && isset( $real_args[1] ) && isset($real_args[2])) {
    $block->content = drupal_get_form('user_pass_reset', $real_args[0], $real_args[1], $real_args[2]);
  }

  return $block;
}
