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
 * @return array; an array as expected by ding_user (@see ding_user_user_login_validate)
 */
function bibdk_provider_wayf_authenticate($name, $wayfid) {
  $return = array(
    'success' => FALSE,
  );
  // loginWayf-method throws an exception if something (anything) goes wrong
  try {
    $loginType = isset($_SESSION['wayf_login']['login_type']) ? $_SESSION['wayf_login']['login_type'] : 'wayf_id';
    BibdkUser::instance()->loginWayf($name, $wayfid, $loginType);
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
  } catch (Exception $e) {
    $enable_logging = variable_get('bibdk_provider_enable_logging');
    if ($enable_logging) {
      watchdog('bibdk_wayf', 'login failed %userid; %wayfid message: %message', array('%userid' => $name, '%wayfid' => $wayfid, '%message' => $e->getMessage()), WATCHDOG_NOTICE);
    }
  }

  return $return;
}

/** Implements hook_user_login
 *  unset wayf-SESSION variables - we don't want them once user has logged in
 */
function bibdk_provider_user_login(&$edit, $account){
  if(isset($_SESSION['wayf_login'])){
    unset($_SESSION['wayf_login']);
  }
  if(isset($_SESSION['accept_wayf'])) {
    unset($_SESSION['accept_wayf']);
  }
}

/* \brief custom validator for user_login form (@see ding_wayf.module)
 * 
 * PRE: user is authenticated with WAYF
 */

function bibdk_provider_wayf_validate(&$form, &$form_state) {
  // check if validation comes from wayf
  if ($form_state['values']['authentication_provider'] != 'wayf') {
    return;
  }

  // use case: user exist, but has not yet validated wayf binding
  $url = bibdk_provider_wayf_validate_no_binding($form, $form_state);
  if ($url !== FALSE) {
    // somewhere along the way $_GET['destination'] is set
    // and that overrules drupal_goto; .. unset
    if (isset($_GET['destination'])){
      unset($_GET['destination']);
    }
    drupal_goto($url);
  }


  //second use case; user has already wayf permissions (user is registred, and
  // wayf_id has been set)
  $url = bibdk_provider_wayf_validate_wayf_user($form, $form_state);
  if ($url !== FALSE) {
    // somewhere along the way $_GET['destination'] is set
    // and that overrules drupal_goto; .. unset
    if (isset($_GET['destination'])){
      unset($_GET['destination']);
    }
    drupal_goto($url);
    exit;
  }
}

/** check if user exists. return url to accept page if user is not found
 * set $form_state['values']['name'] if user is found
 *
 * @global string $base_url
 * @param array $form
 * @param array $form_state
 * @return string|boolean 
 */
function bibdk_provider_wayf_validate_no_binding(&$form, &$form_state) {
  $pass = isset($form_state['input']['pass']) ? $form_state['input']['pass'] : FALSE;

  if (!$pass) {
    // @TODO handle this in a proper way
    // this should not happen since user has
    // been validated with wayf and thus has a wayfid
    die('NO PASSWORD');
    return;
  }
  // try to get username from provider
  $username = ding_provider_invoke('wayf', 'verify', $pass);
  
  if ($username === FALSE) {
    // user does either not exist or has not accepted wayf binding
    bibdk_provider_wayf_set_session($username, $pass, 'no_user');
    global $base_url;
    $url = $base_url . '/wayf/accept';
    return $url;
  }
  else {
    $form_state['values']['name'] = $username;
  }
  return FALSE;
}

/** 
 *
 * @global type $base_url
 * @param type $form
 * @param type $form_state
 * @return boolean|string 
 */
function bibdk_provider_wayf_validate_wayf_user(&$form, &$form_state) {
  $pass = isset($form_state['values']['pass']) ? $form_state['values']['pass'] : FALSE;
  if (!$pass) {
    return FALSE;
  }
  $loginType = $_SESSION['wayf_login']['login_type'];
  $username = ding_provider_invoke('wayf', 'verify', $pass, $loginType);
  if ($username !== FALSE) {
    $errors = form_get_errors();
    // we got the username from webservice
    // thus user is okay for logging in 
    bibdk_provider_wayf_set_session($username, $pass, 'bibdk_user');
    // set username in form_state
    if (isset($_SESSION['accept_wayf']['name'])) {
      // clear errors for form_element['name']
      // errors are passed from includes/form.inc::_form_validate which is called
      // before custom validate functions.
      $errors = form_get_errors();
      bibdk_provider_wayf_clear_element_error($form, $errors, 'name');

      $form_state['values']['name'] = $form_state['values']['mail'] = $_SESSION['accept_wayf']['name'];
      $form_state['input']['name'] = $form_state['input']['mail'] = $_SESSION['accept_wayf']['name'];
    }
    return FALSE;
  }
  else {
    // user has no wayf binding, or user does not exist
    global $base_url;
    $url = $base_url . '/wayf/accept';
    return $url;
  }
}

/** Remove form error for given element
 *
 * @param type $form
 * @param type $errors
 * @param type $element
 * @return type 
 */
function bibdk_provider_wayf_clear_element_error($form, $errors, $element) {
  if (empty($errors)) {
    return;
  }
  // clear form errors
  form_clear_error();
  // clear error_messages
  drupal_get_messages('error');
  $remove_me = array();
  foreach ($errors as $name => $error) {
    if ($name == $element) {
      $remove_me[] = $name;
    }
  }

  if (empty($remove_me)) {
    return;
  }

  foreach ($remove_me as $name) {
    unset($errors[$name]);
  }

  // rebuild form_errors and messages
  foreach ($errors as $name => $error) {
    form_set_error($name, $error);
    drupal_set_message($error, 'error');
  }
}


function bibdk_provider_wayf_login_creds($authentication_attributes, $notify_by_mail) {
  // set authentication provider for ding_user (@see ding_user_user_login_validate)
  $form_state['values']['authentication_provider'] = 'wayf';

// set name and password for login
  $form_state['values']['name'] = $authentication_attributes['mail'][0];
  $form_state['values']['mail'] = $authentication_attributes['mail'][0];
  $form_state['values']['pass'] = $authentication_attributes['eduPersonTargetedID'][0];
  if (isset($notify_by_mail)) {
    $form_state['mail_me'] = $notify_by_mail;
  }
  return $form_state;
}

/**
 * 
 */
function bibdk_provider_wayf_accept_form() {
  return 'user_register_form';
  //return '_bibdk_provider_wayf_accept_form';
}

/**
 * Custom form alter - called from bibdk_provider.module::bibdk_provider_form_user_register_form_alter
 * @param type $form 
 */
function bibdk_provider_wayf_alter_user_register_form(&$form, &$form_state) {
  // this belongs in bibdk_provider.wayf.inc
  if (module_exists('ding_wayf')) {
    if (isset($_SESSION['wayf_login']['login_type'])) {
      // this is a wayf login
      // save submit and validate functions for later
      $form_state['process_later']['submit'] = $form['#submit'];
      // add a submit handler to save wayf data on temporary user
      $form_state['process_later']['submit'][] = 'bibdk_provider_wayf_user_register_add_wayf_data';
      $form_state['process_later']['validate'] = $form['#validate'];
      unset($form['#submit']);
      unset($form['#validate']);
      // new text on submit button 
      $form['actions']['submit']['#value'] = t('Update/create account', array(), array('context' => 'bibdk_provider'));
      $form['account']['mail']['#default_value'] = !empty($_SESSION['wayf_login']['mail'][0]) ? $_SESSION['wayf_login']['mail'][0] : '';
      $form['wayf_accept'] = array(
        'messages' => array(
          'message' => array(
            '#markup' => t('You have been authenticated by wayf, but you either have no account or has not yet accepted wayfbinding'),
          ),
          'accept' => array(
            '#type' => 'checkbox',
            '#default_value' => FALSE,
            '#title' => t('accept_wayf_binding', array(), array('context' => 'ding_wayf')),
          ),
        ),
        // put checkbox on top of form
        '#weight' => -100,
      );
      //add validation handler for checkbox
      //array_unshift($form['#validate'], 'bibdk_provider_accept_form_validate');
      $form['#validate'][] = 'bibdk_provider_accept_form_validate';
    }
  }
}

/** Custom validator for accept_form
 *
 * @param type $form
 * @param type $form_state
 * @return type 
 */
function bibdk_provider_accept_form_validate($form, &$form_state) {
  // check if $_SESSION variables are set
  if (!isset($_SESSION['accept_wayf'])) {
    // all of this is of no good if this variable is not set - redirect user to try again
    drupal_set_message('ding_wayf',t('Something went wrong. try logging in with wayf again', array(), array('context' => 'ding_wayf')));
    form_set_error('form', t('Something went wrong. try logging in with wayf again', array(), array('context' => 'ding_wayf')));
    drupal_goto('user');
  }

  //validate that accept has been checked
  if (!$form_state['values']['accept']) {
    form_set_error('accept', t('You must accept binding your 
      bibliotek.dk username with wayf id', array(), array('context' => 'ding_wayf')));
    return;
  }

  // check userstatus 
  if (!bibdk_provider_wayf_accept_validate_check_user($form_state)) {
    // double check. this should not happen since mail is required
    form_set_error('form', t('Something went wrong. try logging in with wayf again', array(), array('context' => 'ding_wayf')));
  }

  // process the form
  bibdk_provider_wayf_process_user_register_form($form, $form_state);
}

/** Check if given user $form_state ($form_state['values']['mail']) exists.
 * and update SESSION variables accordingly
 *
 * @param array 
 * @return boolean 
 */
function bibdk_provider_wayf_accept_validate_check_user($form_state) {
  
  
  
  if (!isset($form_state['values']['mail'])) {
    return FALSE;
  }

  $username = $form_state['values']['mail'];
  // set SESSION variables
  $_SESSION['accept_wayf']['name'] = $username;
  // we know that user has not yet logged on via way or we wouldn't be here
  // check if user is known
  $status = ding_provider_invoke('user', 'verify', $username);
  
  if (!$status) {
    // no bibdk user
    $_SESSION['accept_wayf']['status'] = 'no_user';
  }
  else {
    // existing bibdk_user
    $_SESSION['accept_wayf']['status'] = 'bibdk_user';
  }
  return TRUE;
}

/** Process user register form. Validate and submitfunctions for wayf-login
 * are stored in form_state['process_later']
 *
 * @param type $form
 * @param type $form_state
 * @return type 
 */
function bibdk_provider_wayf_process_user_register_form($form, &$form_state) {
  // check if $_SESSION variables are set
  if (!isset($_SESSION['accept_wayf'])) {    
    return;
  }
  $status = $_SESSION['accept_wayf']['status'];  
  if ($status == 'bibdk_user') {
    // this is a first time login with wayf. 
    $errors = form_get_errors();
    if (empty($errors)) {
      // user is already bibdk_user; bind wayf_id to user and log in
      bibdk_provider_accept_form_bind_and_login();
    }
    else {
      return;
    }
  }
  if ($status == 'no_user') { 
    bibdk_provider_wayf_accept_create_new_user($form, $form_state);
  }
}

/** Handle registration of users logged in via wayf (or nemlogin).
 *
 * @param array $form
 * @param array $form_state 
 */
function bibdk_provider_wayf_accept_create_new_user($form, &$form_state) {
  // run validation handlers before submitting
  // @see form.inc::form_execute_handlers
  $form_state['validate_handlers'] = $form_state['process_later']['validate'];
  form_execute_handlers('validate', $form, $form_state);
  $errors = form_get_errors();
  if (empty($errors)) {
    // form validated now execute submit handlers (user_register)
    $form_state['submit_handlers'] = $form_state['process_later']['submit'];
    form_execute_handlers('submit', $form, $form_state);
    drupal_goto();
    exit;
  }
}

/** Submit handler (user_register_form). Add wayf data to user. 
 * When user resets password wayf data will be used to bind wayf_id to user 
 * (@see bibdk_provider::_bibdk_provider_user_profile_form_login)
 *
 * @param array $form
 * @param array $form_state
 */
function bibdk_provider_wayf_user_register_add_wayf_data($form, $form_state) {
  if (isset($_SESSION['wayf_login'])) {
    // get the user
    $account = $form['#user'];
    // add wayf-data
    $account->data = array('wayf'=>$_SESSION['wayf_login']);
    // save user
    user_save($account);
  }    
}

/** For existing bibdk_user. Bind wayf_id and log user in
 * 
 */
function bibdk_provider_accept_form_bind_and_login() {
  $username = $_SESSION['accept_wayf']['name'];
  // pass is wayf_id; user has been validated by wayf
  $wayf_id = $_SESSION['accept_wayf']['pass'];
  $login_type = isset($_SESSION['wayf_login']['login_type']) ? $_SESSION['wayf_login']['login_type'] : 'wayf_id';
  
  // @TODO verify user password before binding
    
  // bind wayf_id to user
  if (ding_provider_invoke('wayf', 'bind', $username, $wayf_id, $login_type)) {
    $attributes['eduPersonTargetedID'][0] = $wayf_id;
    $attributes['mail'][0] = $username;

    $function = 'ding_wayf_login';
    if (function_exists($function)) {
      // log user in
      $function($attributes);
    }
    else {
      // this is an ERROR - login function was not found
      drupal_set_message(t('Internal error please contact administrator if problem persists',array(),array('context'=>'ding_wayf')), 'error');
      watchdog('wayf login', $function.' WAS NOT FOUND ', array(), WATCHDOG_ERROR);
    }
  }
  else {
    // wayf bind did not succeed. 
    // notice for the user
    drupal_set_message(t('Sorry we could not bind your wayf_id with your account',array(),array('context'=>'ding_wayf')), 'warning');
    // error for the log
    watchdog('wayf login', 'bibdk_provider::bind_wayf failed in binding %username with %wayf_id' , array('%username'=>$username, '%wayf_id'=>$wayf_id), WATCHDOG_ERROR);
  }
}

/**
 * Add wayf parameters to SESSION
 * */
function bibdk_provider_wayf_set_session($name, $pass, $status = 'no_user') {
  if (!isset($_SESSION)) {
    drupal_session_initialize();
    $_SESSION['accept_wayf'] = array();
  }
  $_SESSION['accept_wayf']['name'] = $name;
  $_SESSION['accept_wayf']['pass'] = $pass;
  $_SESSION['accept_wayf']['status'] = $status;
}

function bibdk_provider_wayf_bind_wayf($username) {
  $wayfid = isset($_SESSION['wayf_login']['eduPersonTargetedID'][0]) ? $_SESSION['wayf_login']['eduPersonTargetedID'][0] : NULL;
  if (empty($wayfid)) {
    return FALSE;
  }

  if (ding_provider_implements('wayf', 'bind')) {
    $success = ding_provider_invoke('wayf', 'bind', $username, $wayfid);
  }

  return $success;
}

/**
 *
 * @param type $name
 * @param type $wayfid
 * @return type
 */
function bibdk_provider_wayf_bind($name, $wayfid, $loginType = 'wayf_id') {
  try {
    $ret = BibdkUser::instance()->bindWayf($name, $wayfid, $loginType);
    return $ret;
  } catch (Exception $e) {
    return FALSE;
  }
}

/** Verify that a user with given $wayfid exists
 *
 * @param string $wayfId
 * @return mixed; string(userid) if user exists; bool(FALSE) if not 
 */
function bibdk_provider_wayf_verify($wayfId) {
  if (!isset($_SESSION['wayf_login']['login_type'])) {
    return FALSE;
  }
  try {
    $ret = BibdkUser::instance()->verifyWayf($wayfId, $_SESSION['wayf_login']['login_type']);
    return $ret;
  } catch (Exception $e) {
    return FALSE;
  }
}

/**
 *
 * @param type $name
 * @return type
 */
function bibdk_provider_wayf_delete($name) {
  return BibdkUser::instance()->deleteWayf($name);
}
