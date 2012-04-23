<?php

class BibdkClient {
  const SERVICE_URL = 'http://metode.dbc.dk/~pjo/webservices/DDBUserInfo/trunk/server.php';

  public static function request($action,$params) {
    if( !is_array($params) ) {
      return false;
    }
    
    $request = '?action='.$action;
    foreach( $params as $key=>$value ) {
      $request .= '&';
      $request .= $key.'='.$value;
    }

    $url = self::SERVICE_URL.$request;

    $nano = new NanoSOAPClient(self::SERVICE_URL);
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
      watchdog('CLIENT',$xml);
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
