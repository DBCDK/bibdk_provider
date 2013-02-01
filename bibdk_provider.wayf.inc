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

/* if (BibdkUser::instance()->loginWayf($name, $wayfid)) {
  $cred = array('');
  ding_user_save_creds($cred, $account);
  }
 */

# check if user exists?
# has accepted bind?
# else ask for bind
# otherwise
# ask for creating a new user

/**
 *
 * @param type $name
 * @param type $wayfid
 * @return type
 */
function bibdk_provider_wayf_bind($name, $wayfid) {
  return BibdkUser::instance()->bindWayf($name, $wayfid);
}

/** \brief Custom validator for wayf login
 *
 * @param type $form
 * @param type $form_state 
 */
function bibdk_provider_wayf_validate($form, $form_state) {
  // @TODO; VALIDATE
  // verify user
  // check way bind
  // set form if necessary
  // etc..  
}

/**
 *
 * @param type $name
 * @return type
 */
function bibdk_provider_wayf_delete($name) {
  return BibdkUser::instance()->deleteWayf($name);
}
