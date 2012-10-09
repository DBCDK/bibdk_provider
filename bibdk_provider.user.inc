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

// @TODO this function should do something
function bibdk_provider_is_authenticated(){
  return TRUE;
}

function bibdk_provider_set_favourite($name, $agencyid){
  return BibdkUser::instance()->setFavourite($name,$agencyid);
}

function bibdk_provider_add_favourite($name,$agencyid){
  return BibdkUser::instance()->addFavourite($name, $agencyid);
}

function bibdk_provider_delete_favourite($name,$agencyid){
  return BibdkUser::instance()->deleteFavourite($name,$agencyid);
}

/**
 * \brief get favourite agencies for given user
 * @param name; userId (email)
 * return array (might be empty)
 **/
function bibdk_provider_get_favourites($name) {
  static $response;
  if( !isset($response) ) {
    $response =  BibdkUser::instance()->getFavourites($name);    
  }
  return $response;
}

function bibdk_provider_user_verify($name) {
  //$response = NULL;
  static $response;
  if( !isset($response) ) {
    $response = BibdkUser::instance()->verify($name);
  }
  return $response;
}

function bibdk_provider_user_create($name, $pass) {
  return BibdkUser::instance()->create($name, $pass);
}

function bibdk_provider_user_update_password($name, $pass) {
  return BibdkUser::instance()->update_password($name, $pass);
}

function bibdk_provider_client_user_delete($name, $pass) {
  return BibdkUser::instance()->delete($name, $pass);
}

