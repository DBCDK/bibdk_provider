<?php

/**
 * Implements provider user authenticate
 */
function bibdk_provider_user_authenticate($name, $pass) {
  $return = array(
    'success' => FALSE,
  );

  if (BibdkUser::instance()->login($name, $pass)) {
    $return['creds'] = array(
      'name' => $name,
      'pass' => $pass,
    );

    $return['user'] = array(
      'data' => array('display_name' => 'HEST'),
      'mail' => $name,
    );
    $return['success'] = TRUE;
    $return['messages'] = array('HEJ', 'notice');

    // @TODO; set a proper authname - some hash of mail
    // $return['authname'] = 'HEST';
  }
  else {
    /* advanced regex to match e-mail addresses including obscure usernames and
      at server which only has an IPv4 address */
    $is_email = !preg_match("/[a-z0-9!#$%&'*+\/=?^_`\{|\}~-]+(?:\.[a-z0-9!#$%&'*+\/=?^_`\{|\}~-]+)*@(?:(?:[a-z0-9](?:[a-z0-9-]*[a-z0-9])?\.)+[a-z0-9](?:[a-z0-9-]*[a-z0-9])?)/", strtolower($name));

    /* if all characters in username or password is capitalized caps lock may be on */
    $capslock_on = preg_match('/[[:alpha:]]/', $name) && $name == strtoupper($name) || preg_match('/[[:alpha:]]/', $pass) && $pass == strtoupper($pass);

    /* if password is 4 or 5 digits */
    $is_pincode = preg_match('/^[[:digit:]]{4,5}$/', $pass);

    if ($is_email) {
      $return['messages'][] = t('Your username is your email-address.');
    }
    if ($capslock_on) {
      $return['messages'][] = t('Remember to turn off caps lock.');
    }
    if ($is_pincode) {
      $return['messages'][] = t('Use your Bibliotek.dk password, not your PIN code for your local library.');
    }
  }
  return $return;
}

/**
 * pjo testing
 * Implements hook_user_view
 * NOTE; this skips the userview page and redirects to user edit page.
 * what do we need the userview page for anyway ..
 * */
function bibdk_provider_user_view($account, $view_mode, $langcode) {
  drupal_goto('user/' . $account->uid . '/edit');
}

// @TODO this function should do something
function bibdk_provider_is_authenticated() {
  return TRUE;
}

/** \brief check if user is logged in and if so if user is provider user
 * 
 * @global type $user
 * @return boolean 
 */
function _bibdk_provider_user_check() {
  global $user;
  if ($user->uid) {
    if (ding_user_is_provider_user($user)) {      
      // user is logged in and is provider user
      return TRUE;
    }
    // user is not provider user    
    return FALSE;
  }
  // user is not logged in  
  return FALSE;
}

function bibdk_provider_user_set_favourite($name, $agencyid) {
  if( !(_bibdk_provider_user_check()) ) {
    return FALSE;
  }
  return BibdkUser::instance()->setFavourite($name, $agencyid);
}

function bibdk_provider_user_add_favourite($name, $agencyid) {
  if( !(_bibdk_provider_user_check()) ) {
    return FALSE;
  }
  
  return BibdkUser::instance()->addFavourite($name, $agencyid);
}

function bibdk_provider_user_delete_favourite($name, $agencyid) {
  if( !(_bibdk_provider_user_check()) ) {
    return FALSE;
  }
  return BibdkUser::instance()->deleteFavourite($name, $agencyid);
}

/**
 * \brief get favourite agencies for given user
 * @param name; userId (email)
 * return array (might be empty)
 * */
function bibdk_provider_user_get_favourites($name) {
  static $response;
  if (!isset($response)) {
    $response = BibdkUser::instance()->getFavourites($name);
  }
  return $response;
}

function bibdk_provider_user_verify($name) {
  //$response = NULL;
  static $response;
  if (!isset($response)) {
    $response = BibdkUser::instance()->verify($name);
  }
  return $response;
}

function bibdk_provider_user_create_account($name, $pass) {
  return BibdkUser::instance()->create($name, $pass);
}

function bibdk_provider_user_update_password($name, $pass) {
  return BibdkUser::instance()->update_password($name, $pass);
}

function bibdk_provider_user_delete_account($name) {
  return BibdkUser::instance()->delete($name);
}

function bibdk_provider_user_save_favourite_data($name, $agencyid, $data) {
  return BibdkUser::instance()->saveFavouriteData($name, $agencyid, $data);
}