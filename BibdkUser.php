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
      if (variable_get('bibdk_provider_enable_logging')) {
        watchdog('bibdk_provider', t('Provider url is not set'), array(), WATCHDOG_ERROR, l(t('Set provider url'), 'admin/config/ding/provider/bibdk_provider'));
      }
    }

    if (empty(self::$security_code)) {
      // somehow bibdk_provider security code is not set - FATAL
      if (variable_get('bibdk_provider_enable_logging')) {
        watchdog('bibdk_provider', t('Security code is not set'), array(), WATCHDOG_ERROR, l(t('Set securitycode'), 'admin/config/ding/provider/bibdk_provider'));
      }
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
      $request['oui:' . $key] = htmlspecialchars($value);
    }

    // add securitycode
    if (isset(self::$security_code)) {
      $request['oui:' . 'securityCode'] = htmlspecialchars(self::$security_code);
    }

    $nano = new NanoSOAPClient(self::$service_url, array('namespaces' => array('oui' => 'http://oss.dbc.dk/ns/openuserinfo')));

    if ($simpletest_prefix = drupal_valid_test_ua()) {
      NanoSOAPClient::setUserAgent(drupal_generate_test_ua($simpletest_prefix));
    }

    return $nano->call('oui:' . $action, $request);
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
      if (variable_get('bibdk_provider_enable_logging')) {
        watchdog('bibdk_provider', t('BIBDK client could not load response: %xml', array('%xml' => var_export($xml, TRUE))), array(), WATCHDOG_ERROR);
      }
      return FALSE;
    }
    $this->xpath = new DomXPATH($dom);
    $this->xpath->registerNamespace('oui', 'http://oss.dbc.dk/ns/openuserinfo');
    return TRUE;
  }

  /**
   * Function which hands the request to the BibdkClient.
   *
   * @param $action
   *   The request type as a string.
   * @param $params
   *   An array structure representing the request parameters.
   * @return
   *   Response from BibdkClient as xml string.
   */
  private function makeRequest($action, $params) {
    return BibdkClient::request($action, $params);
  }

  /**
   * Function which parses the response.
   *
   * @param $xmlstring
   *   The reponse as a xml string.
   * @param $xmltag
   *   Which XPath element should extracted from the response.
   *
   * @return
   *   FALSE if the xpath can't be set otherwise value of the xmltag to be
   *   extracted.
   */
  private function responseExtractor($xmlstring, $xmltag) {
    if (!@$this->set_xpath($xmlstring)) {
      return FALSE;
    }
    $pos = strpos($xmltag, 'oui:');
    if ($pos === FALSE) {
      $xmltag = 'oui:' . $xmltag;
    }
    $query = '//' . $xmltag;

    $tagcontent = $this->xpath->query($query);

    if ($tagcontent->item(0)) {
      return $tagcontent->item(0)->firstChild;
    }
    else {
      return FALSE;
    }
  }

  /*   * **************  FAVOURITES ************** */

  public function setFavourite($username, $agencyid) {
    $params = array('userId' => $username, 'agencyId' => $agencyid);
    $response = $this->makeRequest('setFavouriteRequest', $params);

    $xmlmessage = $this->responseExtractor($response, 'setFavouriteResponse');

    if ($xmlmessage != FALSE && $xmlmessage->nodeName == 'oui:userId') {
      return TRUE;
    }
    else {
      return FALSE;
    }
  }

  /** \brief get all favourite agencies for a given user
   * @staticvar type $response
   * @param type $username
   * @return type xml
   */
  public function getFavourites($username) {
    static $response;
    $params = array('userId' => $username);
    $response = $this->makeRequest('getFavouritesRequest', $params);

    return $response;
  }

  public function getCart($username){
    static $response;
    $params = array('oui:userId' => $username);
    $response = $this->makeRequest('getCartRequest', $params);

    $xmlmessage = $this->responseExtractor($response, 'getCartResponse');

    $ret = array('status' => 'error', 'response' => '');
    if ($xmlmessage->nodeName != 'oui:error') {
      $ret['status'] = 'success';
      $ret['response'] = $response;
    }
    else {
      $ret['response'] = $xmlmessage->nodeValue;
    }
    return $ret;
  }

  public function addCartContent($username, $content){
    static $response;
    $params = array(
      'oui:userId' => $username,
      'oui:cartContent' => $content
    );
    $response = $this->makeRequest('addCartContentRequest', $params);
    $xmlmessage = $this->responseExtractor($response, 'addCartContentResponse');

    $ret = array('status' => 'error', 'response' => '');

    if ($xmlmessage->nodeName != 'oui:error') {
      $ret['status'] = 'success';
      $ret['response'] = $response;
    }
    else {
      $ret['response'] = $xmlmessage->nodeValue;
    }
    return $ret;
  }

  public function removeCartContent($username, $content){
    static $response;
    $params = array(
      'oui:userId' => $username,
      'oui:cartContent' => array(
        'oui:cartContentElement' => $content,
      )
    );
    $response = $this->makeRequest('removeCartContentRequest', $params);

    $xmlmessage = $this->responseExtractor($response, 'removeCartContentResponse');

    $ret = array('status' => 'error', 'response' => '');

    if ($xmlmessage->nodeName != 'oui:error') {
      $ret['status'] = 'success';
      $ret['response'] = $response;
    }
    else {
      $ret['response'] = $xmlmessage->nodeValue;
    }
    return $ret;
  }

  /**
   * \brief add an agency to favourites for given user
   * @param type $username
   * @param type $agencyid
   * @return type xml
   */
  public function addFavourite($username, $agencyid) {
    $params = array('userId' => $username, 'agencyId' => $agencyid);
    $response = $this->makeRequest('addFavouriteRequest', $params);

    $xmlmessage = $this->responseExtractor($response, 'addFavouriteResponse');

    $ret = array('status' => 'error', 'response' => '');

    if ($xmlmessage->nodeName != 'oui:error') {
      $ret['status'] = 'success';
      $ret['response'] = $response;
    }
    else {
      $ret['response'] = $xmlmessage->nodeValue;
    }
    return $ret;
  }

  /**
   * \brief delete an agency on a given user
   * @param type $username
   * @param type $agencyid
   * @return type xml
   */
  public function deleteFavourite($username, $agencyid) {
    $params = array('userId' => $username, 'agencyId' => $agencyid);
    $response = $this->makeRequest('deleteFavouriteRequest', $params);

    $xmlmessage = $this->responseExtractor($response, 'deleteFavouriteResponse');

    if ($xmlmessage != FALSE && $xmlmessage->nodeName == 'oui:userId') {
      return TRUE;
    }
    else {
      return FALSE;
    }
  }

  public function saveFavouriteData($name, $agencyid, $data) {
    $params = array('userId' => $name, 'agencyId' => $agencyid, 'favouriteData' => $data);
    $response = $this->makeRequest('setFavouriteDataRequest', $params);

    return $response;
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
    $response = $this->makeRequest('loginRequest', $params);
    $xmlmessage = $this->responseExtractor($response, 'loginResponse');

    if ($xmlmessage != FALSE && $xmlmessage->nodeName == 'oui:userId') {
      return TRUE;
    }
    else {
      return FALSE;
    }
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
    $response = $this->makeRequest('createUserRequest', $params);
    $xmlmessage = $this->responseExtractor($response, 'createUserResponse');

    if ($xmlmessage != FALSE && $xmlmessage->nodeName == 'oui:userId') {
      return TRUE;
    }
    else {
      return FALSE;
    }
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
// only verify once - that should be enough
    static $response;

    if (empty($response)) {
      $params = array(
        'userId' => $name,
        'outputType' => 'xml',
      );
      $response = $this->makeRequest('verifyUserRequest', $params);
    }

    $xmlmessage = $this->responseExtractor($response, 'verifyUserResponse');

    if ($xmlmessage != FALSE && $xmlmessage->nodeName == 'oui:verified') {
      return preg_match('/true/i', $xmlmessage->nodeValue);
    }
    else {
      return FALSE;
    }
  }

  /**
   * Function to update password.
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
    $response = $this->makeRequest('updatePasswordRequest', $params);
    $xmlmessage = $this->responseExtractor($response, 'updatePasswordResponse');

    if ($xmlmessage != FALSE && $xmlmessage->nodeName == 'oui:userId') {
      return TRUE;
    }
    else {
      return FALSE;
    }
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
  public function delete($name) {
    $params = array(
      'userId' => $name,
      'outputType' => 'xml',
    );
    $response = $this->makeRequest('deleteUserRequest', $params);
    $xmlmessage = $this->responseExtractor($response, 'deleteUserResponse');

    if ($xmlmessage != FALSE && $xmlmessage->nodeName == 'oui:userId') {
      return TRUE;
    }
    else {
      return FALSE;
    }
  }

  /**
   * Login using WAYF.
   *
   * Throws an exception if an error is return from the web service.
   *
   * @param $name
   *   Email address of the user.
   * @param $wayfId
   *   Unique id return by WAYF
   * @return boolean
   *   Indicates if operation was successful.
   * @throws Exception
   *   If web service returns an error.
   */
  public function loginWayf($name, $wayfId) {
    $params = array(
      'userId' => $name,
      'wayfId' => $wayfId,
      'outputType' => 'xml',
    );
    $response = $this->makeRequest('loginWayfRequest', $params);
    $xmlmessage = $this->responseExtractor($response, 'loginWayfResponse');

    if ($xmlmessage != FALSE && $xmlmessage->nodeName == 'oui:userId') {
      return TRUE;
    }
    else {
      if ($xmlmessage->nodeName == 'oui:error') {
        throw new Exception($xmlmessage->nodeValue);
      }
      else {
        return FALSE;
      }
    }
  }

  /**
   * Binds WAYF id to a user.
   *
   * Old WAYF id will be overwritten.
   *
   * @param $name
   * @param $wayfId
   *   WAYF id of user.
   * @return boolean
   *   Indicates if operation was successful.
   * @throws Exception
   *   If web service returns an error.
   */
  public function bindWayf($name, $wayfId) {
    $params = array(
      'userId' => $name,
      'wayfId' => $wayfId,
      'outputType' => 'xml',
    );
    $response = $this->makeRequest('bindWayfRequest', $params);
    $xmlmessage = $this->responseExtractor($response, 'bindWayfResponse');

    if ($xmlmessage != FALSE && $xmlmessage->nodeName == 'oui:userId') {
      return TRUE;
    }
    else {
      if ($xmlmessage->nodeName == 'oui:error') {
        throw new Exception($xmlmessage->nodeValue);
      }
      else {
        return FALSE;
      }
    }
  }

  /**
   * Deletes WAYF binding for a user.
   *
   * @param $name
   *   User who should have removed binding.
   * @return boolean
   *   Indicates if operation was successful.
   * @throws Exception
   *   If web service returns an error.
   */
  public function deleteWayf($name) {
    $params = array(
      'userId' => $name,
      'outputType' => 'xml',
    );
    $response = $this->makeRequest('deleteWayfRequest', $params);
    $xmlmessage = $this->responseExtractor($response, 'deleteWayfResponse');

    if ($xmlmessage != FALSE && $xmlmessage->nodeName == 'oui:userId') {
      return TRUE;
    }
    else {
      if ($xmlmessage->nodeName == 'oui:error') {
        throw new Exception($xmlmessage->nodeValue);
      }
      else {
        return FALSE;
      }
    }
  }

}
