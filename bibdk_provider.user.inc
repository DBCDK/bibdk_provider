<?php



/**
 * Implements provider user authenticate
 */

function bibdk_provider_user_authenticate($name, $pass) {
  $return = array(
    'success' => FALSE,
  );

  if( BibdkUser::instance()->login($name,$pass) ) {
    $return['creds'] = 
      array(
	    'name' => $name,
	    'pass' => $pass,
	    );

    $return['user'] = 
      array(
	    'data' => 
	    array('display_name' => 'HEST',
		  'mail' => $name ),
	    );
    $return['success'] = TRUE;

    // @TODO; set a proper authname - some hash of mail
    // $return['authname'] = 'HEST';
    
  }
  return $return;
}

function bibdk_provider_user_verify( $name ) {
  return BibdkUser::instance()->verify($name);
}


function bibdk_provider_user_create( $name, $pass ) {
  if( BibdkUser::instance()->create( $name, $pass ) ) {
      return TRUE;
    }
    return FALSE;
}

function bibdk_provider_user_update_password( $name, $pass ) {
  return BibdkUser::instance()->update_password($name, $pass);
}


