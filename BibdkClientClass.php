<?php

class BibdkClient {
  const SERVICE_URL = 'http://mobilebib.addi.dk/1.1/';

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
    // @todo; errorhandling
    $dom = new DomDocument();
    $dom->loadXML($xml);
    $this->xpath = new DomXPATH($dom);
  }

  public function login($name,$pass) {
    $response = BibdkClient::request('login',array('user'=>$name,'psw'=>$pass,'outputType'=>'xml'));
    $this->set_xpath($response);
    $query = '//errorCode';
    $status = $this->xpath->query($query)->item(0)->nodeValue;
       
    if( $status != '200' ) {
      // @todo; log?
      return false;
    }
    return true;   
  } 
}