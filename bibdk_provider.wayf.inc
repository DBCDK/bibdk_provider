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
    BibdkUser::instance()->loginWayf($name, $wayfid);
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

/* \brief custom validator for user_login form (@see ding_wayf.module)
 * 
 * PRE: user is authenticated with WAYF
 */

function bibdk_provider_wayf_validate(&$form, &$form_state) {
  // check if validation comes from wayf
  if ($form_state['values']['authentication_provider'] != 'wayf') {
    return;
  }
  // first use case - user does not exist, but is validated with wayf
  $url = bibdk_provider_wayf_validate_no_user($form_state);
  if ($url !== FALSE) {
    drupal_goto($url);
  }

  // next use case; user exist, but has no wayf id. User is still validated with wayf
  $url = bibdk_provider_wayf_validate_bibdk_user($form_state);
  if ($url !== FALSE) {
    drupal_goto($url);
  }
}

/** \brief validation handler for existing bibdk_users logging on with wayf
 * for the first time.
 *
 * try to logon with wayf; if failed return url to accept page
 * 
 * @param type $form_state
 * @return boolean 
 */
function bibdk_provider_wayf_validate_bibdk_user($form_state) {
  $name = isset($form_state['input']['name']) ? $form_state['input']['name'] : FALSE;
  $pass = isset($form_state['input']['pass']) ? $form_state['input']['pass'] : FALSE;

  if (!$name || !$pass) {
    return FALSE;
  }

  $result = ding_provider_invoke('wayf', 'authenticate', $name, $pass);
  if (!$result['success']) {
    // set session variables for next step 
    bibdk_provider_set_session($name, $pass, 'bibdk_user');
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
  return '_bibdk_provider_wayf_accept_form';
}

/** \brief Accept form for binding wayf id to user
 * 
 * @return string 
 */
function _bibdk_provider_wayf_accept_form() {
  $form['container'] = array(
    '#type' => 'container',
    '#attributes' => array(
      'class' => array('element-section', drupal_html_class('wayf_accept_form')),
    ),
  );
  $form['container']['accept_form'] = array(
    'messages' => array(
      'message' => array(
        '#markup' => t('You have been authenticated by wayf, but you have no account etc...'),
      ),
      'accept' => array(
        '#type' => 'checkbox',
        '#default_value' => FALSE,
        '#title' => t('accept_wayf_binding', array(), array('context' => 'ding_wayf')),
      ),
    ),
  );
  $form['container']['actions'] = array(
    'abort' => array(
      '#name' => 'abort',
      '#type' => 'submit',
      '#value' => t('abort', array(), array('context' => 'ding_wayf')),
    ),
    'submit' => array(
      '#name' => 'continue',
      '#type' => 'submit',
      '#value' => t('continue', array(), array('context' => 'ding_wayf')),
    ),
  );

  $form['#validate'][] = 'bibdk_provider_accept_form_validate';
  $form['#submit'][] = 'bibdk_provider_accept_form_submit';
  return $form;
}

/** \brief wayf_accept_form submit handler 
 *
 * redirect to user page if abort is clicked
 * 
 * @param type $form
 * @param type $form_state 
 */
function bibdk_provider_accept_form_submit($form, $form_state) {
  if ($form_state['clicked_button']['#name'] == 'abort') {
    ding_wayf_user_logout(NULL, 'user');
  }
}

/** \brief validation handler for accept_form
 *
 * @param type $form
 * @param type $form_state 
 */
function bibdk_provider_accept_form_validate($form, &$form_state) {

  // check if $_SESSION variables are set
  if (!isset($_SESSION['accept_wayf'])) {
    // @TODO .. log; this method is of no use in this case
    return;
  }

// continue button clicked
  if ($form_state['clicked_button']['#name'] == 'continue') {
    if ($form_state['values']['accept']) {

// get name and pass from session; create a user if accept is set
// $_SESSION variables are set in $provider_wayf_validate
// @see bibdk_provider_wayf_validate

      $name = $_SESSION['accept_wayf']['name'];
      $pass = $_SESSION['accept_wayf']['pass'];
      $status = $_SESSION['accept_wayf']['status'];
      // we don't need the session variables anymore
      unset($_SESSION['accept_wayf']);

// prepare an array for ding_wayf_login
      $authentication_attributes['mail'][0] = $name;
      $authentication_attributes['eduPersonTargetedID'][0] = $pass;

      switch ($status) {
        // first use case - no account with bibliotek.dk
        case 'no_user':
          if (ding_provider_implements('user', 'create_account')) {
            if (ding_provider_invoke('user', 'create_account', $name, $pass)) {
              // user has been created with mail, wayfid.
              if (ding_provider_implements('wayf', 'bind')) {
                ding_provider_invoke('wayf', 'bind', $name, $pass);
              }
              // pass key for email to login method
              $notify_by_mail->key = 'wayf_new_user';
              // if user wishes to change password later on he can do so
              ding_wayf_login($authentication_attributes,  $notify_by_mail);
            }
          }
          break;
        // next use case - user has an bibdk account, but no wayfid
        case 'bibdk_user':
          if (ding_provider_implements('wayf', 'bind')) {
            if (ding_provider_invoke('wayf', 'bind', $name, $pass)) {
              ding_wayf_login($authentication_attributes);
            }
          }
          break;
        default:
          drupal_set_message(t('wayf_could_not_login', array(), array('context' => 'ding_wayf')));
          drupal_goto('user');
          break;
      }
    }
    else {
// user clicked continue, but did not check accept. maybe user forgot to check the box
// tell him to either check the box or press abort
      form_set_error('accept', t('either check accept or press abort', array(), array('context' => 'ding_wayf')));
    }
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
    bibdk_provider_set_session($name, $pass, 'no_user');
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
function bibdk_provider_set_session($name, $pass, $status = 'no_user') {
  if (!isset($_SESSION)) {
    drupal_session_initialize();
    $_SESSION['accept_wayf'] = array();
  }
  $_SESSION['accept_wayf']['name'] = $name;
  $_SESSION['accept_wayf']['pass'] = $pass;
  $_SESSION['accept_wayf']['status'] = $status;
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
