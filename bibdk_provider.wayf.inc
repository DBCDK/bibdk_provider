<?php

/**
 * @file
 * Implements WAYF capabilities.
 */

/**
 * Authentication using WAYF
 *
 * @param type $name
 * @param type $wayfid
 * @return type
 */
function bibdk_provider_wayf_authenticate($name, $wayfid) {
  $return = array(
    'success' => FALSE,
  );

  if (BibdkUser::instance()->loginWayf($name, $wayfid)) {
     $return['creds'] = array(
      'name' => $name,
      'pass' => $wayfid,
    );

    $return['user'] = array(
      'data' => array('display_name' => $name),
      'mail' => $name,
    );
    $return['success'] = TRUE;
    $return['messages'] = array('HEJ', 'notice');
  }

  return $return;
}

/* \brief custom validator for user_login form (@see ding_wayf.module)
 * 
 * 
 */

function bibdk_provider_wayf_validate(&$form, &$form_state) {
   if ($form_state['values']['authentication_provider'] != 'wayf') {
    return;
  }
  // first use case - user does not exist, but is validated with wayf
  $url = bibdk_provider_wayf_validate_no_user($form_state);
  
  if ($url !== FALSE) {
    drupal_goto($url);
    exit;
  }
  // next use case; user exist, but has no wayf id. User is still validated with wayf
  else {
    drupal_goto('user');
    
    // @TODO bind wayf to user
    // try wayflogin - if fail go to accept page
  }
}

/** \brief validation handler for not existing user
 * 
 * check if user is known. if user is not known return 
 * url for wayf-accept;
 *
 * @global type $base_url
 * @param type $form_state
 * @return boolean|string 
 */
function bibdk_provider_wayf_validate_no_user($form_state) {
 
  $name = isset($form_state['input']['name']) ? $form_state['input']['name'] : FALSE;
  $pass = isset($form_state['input']['pass']) ? $form_state['input']['pass'] : FALSE;

  if (!$name || !$pass) {
    return FALSE;
  }
  // verify user
  if (!ding_provider_invoke('user', 'verify', $name)) {
    // set session variables for next step 
    bibdk_provider_set_session($name, $pass);
    // redirect to wayf_accept page
    global $base_url;
    $url = $base_url . '/wayf/accept';
    // we have to unset destination set in $_GET
    // for drupal to goto the correct page
    unset($_GET['destination']);
    return $url;
  }
  else {
    return FALSE;
  }
}

/**
 * Add wayf parameters to SESSION
 * */
function bibdk_provider_set_session($name, $pass) {
  if (!isset($_SESSION)) {
    drupal_session_initialize();
    $_SESSION['accept_wayf'] = array();
  }
  $_SESSION['accept_wayf']['name'] = $name;
  $_SESSION['accept_wayf']['pass'] = $pass;
}

/**
 *
 * @param type $name
 * @param type $wayfid
 * @return type
 */
function bibdk_provider_wayf_bind($name, $wayfid) {
  return BibdkUser::instance()->bindWayf($name, $wayfid);
}

/**
 *
 * @param type $name
 * @return type
 */
function bibdk_provider_wayf_delete($name) {
  return BibdkUser::instance()->deleteWayf($name);
}
