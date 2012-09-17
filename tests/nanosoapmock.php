<?php

class NanoSOAPClient {

  private $request;
  private $requestType;

  public function __construct($url) {
  }

  private function soapEnveloping($bodyTag, $message) {
    return '<?xml version="1.0" encoding="utf-8"?>'
        . '<SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope"><SOAP-ENV:Body>'
        . "<$bodyTag>"
        . $message
        . "</$bodyTag>"
        . '</SOAP-ENV:Body></SOAP-ENV:Envelope>';
  }

  private function soapFault() {
    return '<?xml version="1.0" encoding="utf-8"?><SOAP-ENV:Fault xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope"><faultcode>SOAP-ENV:Server</faultcode><faultstring>Incorrect SOAP envelope or wrong/unsupported request</faultstring></SOAP-ENV:Fault>';
  }

  public function call($requestType, $request) {
    $this->requestType = $requestType;
    $this->request = $request;

    return $this->$requestType($request);
  }

  public function getRequest() {
    return $this->request;
  }

  public function getAction() {
    return $this->requestType;
  }

  private function verifyUserRequest($request) {
    $userId = $request['userId'];

    if (empty($userId)) {
      $response = '<error>Missing</error>';
    }
    else {
      if ($userId == 'validUser') {
        $response = '<verified>true</verified>';
      }
      else {
        $response = '<verified>false</verified>';
      }
    }

    return $this->soapEnveloping('verifyUserResponse', $response);
  }

  private function loginRequest($request) {
    $userId = $request['userId'];
    $password = $request['userPinCode'];

    if (empty($userId) || empty($password)) {
      $response = '<error>Missing</error>';
    }
    else {
      if ($userId == 'validUser' && $password == '123456') {
        $response = "<userId>$userId</userId>";
      }
      else {
        $response = '<error>Wrong userid or password</error>';
      }
    }

    return $this->soapEnveloping('loginResponse', $response);
  }

  private function createUserRequest($request) {
    $userId = $request['userId'];
    $password = $request['userPinCode'];

    if (empty($userId) || empty($password)) {
      $response = '<error>Missing</error>';
    }
    else {
      if ($userId == 'createUser' && $password == '123456') {
        $response = "<userId>$userId</userId>";
      }
      else {
        $response = '<error>user already exists</error>';
      }
    }

    return $this->soapEnveloping('createUserResponse', $response);
  }

  private function updatePasswordRequest($request) {
    $userId = $request['userId'];
    $password = $request['userPinCode'];

    if (empty($userId) || empty($password)) {
      $response = '<error>Missing</error>';
    }
    else {
      if ($userId == 'updateUser') {
        $response = "<userId>$userId</userId>";
      }
      else {
        $response = '<error>can\'t update user</error>';
      }
    }

    return $this->soapEnveloping('updatePasswordResponse', $response);
  }

  private function deleteUserRequest($request) {
    $userId = $request['userId'];

    if (empty($userId)) {
      $response = '<error>Missing</error>';
    }
    else {
      $response = "<userId>$userId</userId>";
    }

    return $this->soapEnveloping('deleteUserResponse', $response);
  }
}
