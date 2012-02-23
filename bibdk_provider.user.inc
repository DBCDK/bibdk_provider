<?php

/**
 * Implements provider user authenticate
 */

function bibdk_provider_user_authenticate($name, $pass) {
  $return = array(
    'success' => FALSE,
  );

  // FAKE A LOGIN;   
  //@todo; do a proper login
  $return['creds'] = array(
    'name' => 'pjo@dbc.dk',
    'pass' => '2222',
  );

  $return['user'] = array(
      'data' => array(
        'display_name' => 'HEST',
      ),
    );
  $return['success'] = TRUE;

  dpm($return);

  return $return;
}


