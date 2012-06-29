<?php

/**
 * Singleton class for querying the Bibdk provider webservice.
 */
class BibdkClient {
  private static $instance;
  private static $service_url;
  private static $security_code;

  /**
   * Private constructor so a static function must be call to create an instance of the class.
   */
  private function __construct() {
    self::$service_url = variable_get('bibdk_provider_webservice_url', '');
    self::$security_code = variable_get('bibdk_provider_security_code', '');

    if (empty(self::$service_url)) {
      watchdog('bibdk_provider', t('Provider url is not set'), array(), WATCHDOG_ERROR, l(t('Set provider url'), 'admin/config/ding/provider/bibdk_provider'));
    }

    if (empty(self::$security_code)) {
      // somehow bibdk_provider security code is not set - FATAL
      watchdog('bibdk_provider', t('Security code is not set'), array(), WATCHDOG_ERROR, l(t('Set securitycode'), 'admin/config/ding/provider/bibdk_provider'));
    }
  }

  /**
   * Request function for querying the webservice.
   *
   * @param $action
   *   The request type as a string.
   * @param $params
   *   Array containing the structure of the request.
   *
   * @return
   *   The response which is an XML string.
   */
  public static function request($action, $params) {
    if (!isset(self::$instance)) {
      self::$instance = new BibdkClient();
    }

    if (!is_array($params)) {
      return FALSE;
    }

    $request = array();

    foreach ($params as $key => $value) {
      $request[$key] = htmlspecialchars($value);
    }

    // add securitycode
    if (isset(self::$security_code)) {
      $request['securityCode'] = htmlspecialchars(self::$security_code);
    }

    $nano = new NanoSOAPClient(self::$service_url);
    return $nano->call($action, $request);
  }
}

/**
 * A wrapper class for calls to the Bibdk provider webservice.
 *
 * This is a singleton class.
 */
class BibdkUser {
  private static $instance;
  private $xpath;

  /**
   * Private constructor so a static function must be call to create an instance of the class.
   */
  private function __construct() {}

  /**
   * Function to get a BibdkUser object.
   *
   * @return
   *   Singleton BibdkUser object.
   */
  public static function instance() {
    if (!isset(self::$instance)) {
      self::$instance = new BibdkUser();
    }

    return self::$instance;
  }

  /**
   * Create a DomXPATH object for parsing XML.
   *
   * @param $xml
   *   String containing XML.
   *
   * @return
   *   Boolean indicating if DomXPATH object is created.
   */
  private function set_xpath($xml) {
    $dom = new DomDocument();
    if (!@$dom->loadXML($xml)) {
      watchdog('bibdk_provider', t('BIBDK client could not load response: %xml', array('%xml' => var_export($xml, TRUE))), array(), WATCHDOG_ERROR);
      return FALSE;
    }
    $this->xpath = new DomXPATH($dom);
    return TRUE;
  }

  /**
   * Function which hands the request to the BibdkClient and parses the response.
   *
   * @param $action
   *   The request type as a string.
   * @param $params
   *   An array structure representing the request parameters.
   * @param $verifyPresence
   *   Which XPath element should be present in the response and neither be
   *   empty nor has a string value of '0' or 'false'.
   *
   * @return
   *   Boolean telling if request was a success and return a positive response.
   */
  private function requestHelper($action, $params, $verifyPresence) {
    $response =  BibdkClient::request($action, $params);

    if (!$this->set_xpath($response)) {
      return FALSE;
    }

    $status = $this->xpath->query($verifyPresence);
    return !empty($status->item(0)->nodeValue);
  }

  /**
   * Function to logging in a user.
   *
   * @param $name
   *   Username (e-mail address).
   * @param $pass
   *   Password for the user.
   *
   * @return
   *   Boolean telling if the login attempt was successful.
   */
  public function login($name, $pass) {
    $params = array(
      'userId' => $name,
      'userPinCode' => $pass,
      'outputType' => 'xml',
    );
    return $this->requestHelper('loginRequest', $params, '//userId');
  }

  /**
   * Function for creating a new user
   *
   * @param $name
   *   E-mail address which is used as username.
   * @param $pass
   *   Password for the user.
   *
   * @return
   *   Boolean telling whether the user was created or not.
   */
  public function create($name, $pass) {
    $params = array(
      'userId' => $name,
      'userPinCode' => $pass,
      'outputType' => 'xml',
    );
    return $this->requestHelper('createUserRequest', $params, '//userId');
  }

  /**
   * Function to check if the username already exists.
   *
   * @param $name
   *   E-mail address which is used as username.
   *
   * @return
   *   Boolean telling if the user already exists.
   */
  public function verify($name) {
    $params =  array(
      'userId' => $name,
      'outputType' => 'xml',
    );
    $response =  $this->requestHelper('verifyUserRequest', $params, '//verified');
  }

  /**
   * Function to check if the username already exists.
   *
   * @param $name
   *   Username for which the password is changed.
   * @param $pass
   *   New password for the user.
   *
   * @return
   *   Boolean telling if the change of password succeed.
   */
  public function update_password($name, $pass) {
    $params = array(
      'userId' => $name,
      'userPinCode' => $pass,
      'outputType' => 'xml',
    );
    return $this->requestHelper('updatePasswordRequest', $params, '//userId');
  }

  /**
   * Function to delete a user.
   *
   * @param $name
   *   Username which should be deleted.
   * @param $pass
   *   Password for the user.
   *
   * @return
   *   Boolean telling if the deletion was successful.
   */
  public function delete($name, $password) {
    $params = array(
      'userId' => $name,
      'userPinCode' => $pass,
      'outputType' => 'xml',
    );
    return $this->requestHelper('deleteUserRequest', $params, '//userId');
  }
}
