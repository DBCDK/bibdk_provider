<?php

/**
 * Class BibdkUser
 * Provider test class
 */

class BibdkUser {

  private static $instance;
  protected $test_users = array();

  private function __construct() {
    $this->test_users = variable_get('bibdk_test_users', array());
  }

  public static function instance() {
    if (!isset(self::$instance)) {
      self::$instance = new BibdkUser();
    }

    return self::$instance;
  }

  public function create($name, $pass) {
    $this->test_users[$name] = $pass;
    variable_set('bibdk_test_users', $this->test_users);

    return TRUE;
  }

  public function verify($name) {
    return isset($this->test_users[$name]);
  }

  public function update_password($name, $pass) {
    $this->test_users[$name] = $pass;
    variable_set('bibdk_test_users', $this->test_users);
  }

  public function delete(){

  }

  /**
   * @return bool
   */
  public function deleteWayf(){
    return TRUE;
  }
}