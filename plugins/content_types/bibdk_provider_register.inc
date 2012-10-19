<?php
/**
* This plugin array is more or less self documenting
*/
$plugin = array(
  // the title in the admin
  'title' => t('User register'),
  // no one knows if "single" defaults to FALSE...
  'single' => TRUE,
  // oh joy, I get my own section of panel panes
  'category' => array(t('User Login CRUD'), -9),
  'render callback' => '_bibdk_provider_get_drupal_user_register_form',
);

/**
 * Get drupal user_register_form from core user.module 
 * Use form_load_include to ensure the form is included also if it is cached
 */
function _bibdk_provider_get_drupal_user_register_form($form, &$form_state) {
  $block = new stdClass();

  if( !isset($form_state) ) {
    $form_state = array();
  }
  form_load_include($form_state, 'module', 'user');

  $block->content = drupal_get_form('user_register_form');
  return $block;
}
