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
  if (BibdkUser::instance()->loginWayf($name, $wayfid)) {
    $cred = array('');
    ding_user_save_creds($cred, $account);




# check if user exists?
# has accepted bind?
# else ask for bind
# otherwise
# ask for creating a new user
  }
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
