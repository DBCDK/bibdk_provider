<?php

class BibdkClient {
private static $SERVICE_URL;
private static $SECURITY_CODE;

/** \brief
 * Send a request to bibdk provider using nanosoap
 */
public static function request($action,$params) {
  if( !isset(self::$SERVICE_URL) ) {
    self::$SERVICE_URL = variable_get('bibdk_provider_wsdl_url');

    if( !isset(self::$SERVICE_URL) ) {
      // somehow bibdk_provider url is not set - FATAL
      watchdog('BIBDK_PROVIDER',t('Provider url is not set'),array(),WATCHDOG_ERROR,l(t('Set provider url'),'admin/config/ding/provider/bibdk_provider'));
    }
  }

  if( !isset(self::$SECURITY_CODE) ) {
    self::$SECURITY_CODE = variable_get('bibdk_provider_security_code');

    if( !isset(self::$SECURITY_CODE) ) {
      // somehow bibdk_provider security code is not set - FATAL
      watchdog('BIBDK_PROVIDER',t('Security code is not set'),array(),WATCHDOG_ERROR,l(t('Set securitycode'),'admin/config/ding/provider/bibdk_provider'));
    }
  }    

  if( !is_array($params) ) {
    return false;
  }
  
  $request = '?action='.$action;
  foreach( $params as $key=>$value ) {
    $request .= '&';
    // encode specialchars
    $request .= $key.'='.htmlspecialchars($value);
  }
  // add securitycode
  if( isset(self::$SECURITY_CODE) ) {
    $request .= '&';
    $request .= 'securityCode='.self::$SECURITY_CODE;
  }

  $url = self::$SERVICE_URL.$request;

  $nano = new NanoSOAPClient(self::$SERVICE_URL);
  $response = $nano->curlRequest($url);

  return $response;
}

}

class BibdkUser {
  private static $instance;
  private function __construct(){}
  private $xpath;
  
  public static function instance(){
    if( !isset(self::$instance) ) {
      self::$instance = new BibdkUser();
    }
    return self::$instance;
  }
  
  private function set_xpath($xml){
    $dom = new DomDocument();
    if( !@$dom->loadXML($xml) ) {
      watchdog('BIBKD client could not load response',$xml,array(),WATCHDOG_ERROR);
      return false;
    }
    $this->xpath = new DomXPATH($dom);
    return true;
  }

  public function login($name,$pass) {
    $response = BibdkClient::request('login',array('userId'=>$name,'userPinCode'=>$pass,'outputType'=>'xml'));
    if( !$this->set_xpath($response) ) {
      return false;
    }
    $query = '//error';
    $status = $this->xpath->query($query);
    $ok = isset($status->item(0)->nodeValue) ? FALSE : TRUE;
    
    return $ok;   
  } 

  public function create($name,$pass) {
    $response = BibdkClient::request('createUser', array('userId'=>$name,'userPinCode'=>$pass,'outputType'=>'xml'));
    if( !$this->set_xpath($response) ) {
      return false;
    }
    $query = '//error';
    
    $status = $this->xpath->query($query);
    $ok = isset($status->item(0)->nodeValue) ? FALSE : TRUE;
        
    return $ok;  
  }

  public function verify($name) {
    $response =  BibdkClient::request('verifyUser', array('userId'=>$name,'outputType'=>'xml'));
    if( !$this->set_xpath($response) ) {
      return false;
    }
    $query = "//verified";
    $status = $this->xpath->query($query);
    return $status->item(0)->nodeValue == 'TRUE';       
  }

  public function update_password( $name, $pass ) {
    $response =  BibdkClient::request('updatePassword', array('userId'=>$name,'userPinCode'=>$pass,'outputType'=>'xml'));
    if( !$this->set_xpath($response) ) {
      return false;
    }
    $query = '//error';
    $status = $this->xpath->query($query);
    $ok = isset($status->item(0)->nodeValue) ? FALSE : TRUE;
    
    
    return $ok;   
  }

  public function delete($name, $password) {
     $response =  BibdkClient::request('deleteUser', array('userId'=>$name,'userPinCode'=>$pass,'outputType'=>'xml'));
    if( !$this->set_xpath($response) ) {
      return false;
    }
    $query = '//error';
    $status = $this->xpath->query($query);
    $ok = isset($status->item(0)->nodeValue) ? FALSE : TRUE;
    
    
    return $ok;
  }
}
