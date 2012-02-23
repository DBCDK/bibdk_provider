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
	    array('display_name' => 'HEST',),
	    );
    $return['success'] = TRUE;
  }

  return $return;
}


