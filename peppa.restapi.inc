<?php

/**
 * @file
 * 
 * class definition that provides the REST API access for Peppa. There are 2 main verbs used in the API
 * 
 * User - this relates to user methods such as create, login and logout
 * Object - this relates to our object storage and manipulation
 * 
 * @author: Stephen Rogers (srogers.uk@gmail.com) - www.spiralarm.co.uk @sarmcon
 * 
 * This work is licensed under a Creative Commons Attribution-NonCommercial 3.0 Unported License.
 * http://creativecommons.org/licenses/by-nc/3.0
 *
 */
 
 
// This is Version 1 of our API  
class PeppaREST{
	
	// some protected vars
	protected $apikey = false;
	protected $sessionid = false;
	protected $payload = false;
	 
	/**
	 * Our constructor - if key check enabled and not valid this is where it all stops
	 */ 
	public function __construct(){
			
		
		// grab any passed apikey		
		$this->apikey = $_SERVER['HTTP_X_PEPPA_API_KEY'];

		// if required validate the passed App API Key
		if(variable_get('peppa_validate_key', false)){
			if(!(peppa_apps_validate_key($this->apikey))){
				watchdog('peppa', 'Unauthorised Peppa API Access -  Key Used: %key', array('%key' => $this->apikey), WATCHDOG_ERROR);							
				drupal_add_http_header('Status', '403 Forbidden');
				exit;			
			}
		}
		
		// Grab the sessionId if one was passed
		$this->sessionid = $_SERVER['HTTP_X_PEPPA_SESSIONID'];
		
		// define default response
		$this->payload = new stdClass();
		$this->payload->success = false;
		$this->payload->msg = null;
		
				
	}
	
	
	/**
	 * Lets process the request
	 * evey result that is NOT an HTTP error has a standard structure
	 */
	final public function handleRequest(){
				
		// we trap the whole request to handle unknown errors
		try{
			
			$apiargs = explode('/',request_path());
			$args = array_slice($apiargs,3);
			switch($args[0]){
				
				case 'user': // a user based api call
					$this->user_handler($args);
					break;
					
				case 'object': // these are object related function calls
					$this->object_handler($args);
					break;
			
				default: // everything else should be treated as not found
					watchdog('peppa', 'Unknown API verb used: %verb with API Key: %key', array('%verb' => $apiargs[2], '%key'=>$this->apikey), WATCHDOG_WARNING);							
					drupal_add_http_header('Status', '404 Not Found');
					exit;
			}
		}
		catch(Exception $e){
			watchdog('peppa', 'Bad API Request: %msg with APIKey: %key', array('%msg' => $e->getMessage(), '%key'=>$this->apikey), WATCHDOG_ERROR);							
			drupal_add_http_header('Status', '400 Bad Request');
			exit;
		}
		
		// now return 
		echo json_encode($this->payload);			 
		exit;
	}
	
	
	
	/**
	 * Handle all the object API requests - ALL Requests MUST have a valid user session id
	 * - Create (POST)
	 * - Read (GET)
	 * - Update (PUT)
	 * - Delete (DELETE)
	 */
	protected function object_handler($args){
		
		// check for a valid session ID - if not found deny access
		$user = peppa_session_get_by_sid($this->sessionid);
		if(!$user){
			watchdog('peppa', 'Class API Call error - invalid or no session ID provided.', null, WATCHDOG_WARNING);							
			drupal_add_http_header('Status', '403 Access Denied');
			exit;			
		}
		
		// now record the API call attempt time
		peppa_session_update_lastcall($this->sessionid);
		
		// If here then we have a valid session ID
		switch($_SERVER['REQUEST_METHOD']){
			
			case 'DELETE': // delete the selected object
				$name = $args[1];
				$oid = $args[2];
				if(peppa_object_deletenamed_byoid($name,$oid,$user->uid)){
					$this->payload->success=true;
					$this->payload->msg = sprintf('Deleted object with oid: %s', $oid);	
				}
				break;
						

			case 'PUT': // Update an Object
				$name = $args[1];
				$oid = $args[2];
				$data = $this->get_json_data(false);
				peppa_object_update($name,$data,$user->uid,$oid);
				$object = peppa_object_get_byoid($oid,$user->uid);
				if($object){
					$this->payload->created = $object->created;
					$this->payload->updated = $object->updated;
					$this->payload->oid = $oid;
					$this->payload->{$name} = json_decode($object->data);
					$this->payload->success = true;
				}
				else{
					$this->payload->success = false;
					$this->payload->msg = 'error updating the specified object';					
				}
				break;		

			case 'POST': // Add a new object
				$name = $args[1];
				$data = $this->get_json_data(false);
				$oid = peppa_object_create($name,$data,$user->uid);
				$object = peppa_object_get_byoid($oid,$user->uid);
				if($object){
					$this->payload->created = $object->created;
					$this->payload->updated = $object->updated;
					$this->payload->oid = $oid;
					$this->payload->{$name} = json_decode($object->data);
					$this->payload->success = true;
				}
				else{
					$this->payload->success = false;
					$this->payload->msg = 'error creating the specified object';					
				}
				break;		
			
			case 'GET': // Get one or ALL requested objects
				$name = $args[1];
				$oid = $args[2];
				if($oid){
					$object = peppa_object_getnamed_byoid($name,$oid,$user->uid);
					if($object){
						$this->payload->success = true;
						$this->payload->created = $object->created;
						$this->payload->updated = $object->updated;
						$this->payload->oid = $object->oid;	
						$this->payload->{$name} = json_decode($object->data);					
					}
					else{
						$this->payload->msg = sprintf('Unable to located the specified data with object ID: %s',$oid);
					}				
				}
				else{
					$objects = peppa_object_get_byuid($name,$user->uid);
					$data = array();
					foreach($objects as $obj){
						$r = json_decode($obj->data);
						$r->created = $obj->created;
						$r->updated = $obj->updated;
						$r->oid = $obj->oid;
						$data[] = $r;
					}
					$this->payload->success = true;
					$this->payload->{$name} = $data;					
				}
				break;
				
				
		}
		
	}	 
	 
	
	
	/**
	 * Handle the user API request
	 * - Login
	 * - Logout
	 * - Create (this is restricted and can be turned off)
	 * - Update (allows the user record to be updated - currently just a password change)
	 */
	private function user_handler($args){
		
		//process request by request method
		switch($_SERVER['REQUEST_METHOD']){
		
			case 'GET':
				$this->process_user_get($args[1]);
				break;
				
			case 'POST':
				$this->process_user_post();
				break;
				
			case 'PUT':
				$this->process_user_put();
				break;

			default:
				watchdog('peppa', 'Bad User Request: %argone , Method: %method  with APIKey: %key', array('%argone' => $args[1],'%method'=>$_SERVER['REQUEST_METHOD'],'%key'=>$this->apikey), WATCHDOG_WARNING);							
				drupal_add_http_header('Status', '400 Bad Request');
				exit;				
		}
		
	}
	
	
	/**
	 * Process our User PUT Request for this request we need to check for valid session id and current password match
	 * - Password update
	 */
	private function process_user_put(){
		
		// check for a valid session ID - if not found deny access
		$user = peppa_session_get_by_sid($this->sessionid);
		if(!$user){
			watchdog('peppa', 'Class API Call error - invalid or no session ID provided.', null, WATCHDOG_WARNING);							
			drupal_add_http_header('Status', '403 Access Denied');
			exit;			
		}
		
		// now record the API call attempt time
		peppa_session_update_lastcall($this->sessionid);
		
		// get our JSON payload and check current password is correct
		$data = $this->get_json_data();

		// we need the currrent Drupal user record to do the update
		$account = user_load($user->uid);
		if(!$account){
			watchdog('peppa', 'Unable to load user for UID: %uid.', array('%uid'=>$user->uid), WATCHDOG_WARNING);							
			drupal_add_http_header('Status', '403 Access Denied');
			exit;						
		}
		
		// Do our password update
		$updated_user = user_save($account, array('pass'=>$data->pass));
		if($user){				
			$this->payload->success = true;
			$this->payload->msg = 'User password updated.';
		}
		else{
			$this->payload->success = false;
			$this->payload->msg = 'Error updating the user password.';			
		}
		
	}

	
	/**
	 * Process our User POST request
	 * - Create
	 */ 
	private function process_user_post(){
		
		
		// first check to see if user create permission is turned on
		if(variable_get('peppa_user_create', false)){
							
			// get our JSON payload and generate our basic drupal user object
			$newuser = $this->get_json_data();
			$duser = array();
			$duser['name'] = $newuser->name;
			$duser['mail'] = $newuser->mail;
			$duser['pass'] = $newuser->pass;
			$duser['status'] = !variable_get('peppa_user_blocked', true);
			
			// create a new drupal user
			$user = user_save(null,$duser);
			if($user){				
				$this->payload->success = true;
				drupal_add_http_header('Status', '201 Created');
				
				// If auto login request do it other wise just return user account info
				if($newuser->login){
					$result = $this->create_peppa_session($user->uid);
					if($result){
						$this->payload->user = $result;	
					}
					else{
						$this->payload->success = false;
						$this->payload->msg = sprintf('user account %s created. But error logging in.', $user->name);
					}				
				}
				else{
					$result = new stdClass();
					$result->name = $newuser->name;
					$result->mail = $newuser->mail;
					$this->payload->user = $result;
				}
			}
			else{
				$this->payload->success = false;
				$this->payload->msg = sprintf('There was an error creating user account %s.', $user->name);
			}
		}
		else{
			watchdog('peppa', 'User Create Request while turned off with APIKey: %key', array('%key'=>$this->apikey), WATCHDOG_WARNING);							
			drupal_add_http_header('Status', '400 Bad Request');
			exit;				
		}
		
	}	
	
	
	/**
	 * Process our User GET request
	 *  - Login 
	 *  - Logout
	 */
	private function process_user_get($request){
		
		switch($request){
			
			case 'logout':
				peppa_session_delete_by_sid($this->sessionid);
				$this->payload->success=true;
				break;
				
			case false: // this is a login
				$name = trim($_REQUEST['name']);
				$password = trim($_REQUEST['pass']);
				$uid = user_authenticate($name, $password);
				if($uid){
					$result = $this->create_peppa_session($uid);
					if($result){
						$this->payload->user = $result;
						$this->payload->success = true;	
					}
					else{
						watchdog('peppa', 'User Login Error, Unable to create Peppa session for user : %userid  using APIKey: %key', array('%userid' => $uid,'%key'=>$this->apikey), WATCHDOG_ERROR);							
						drupal_add_http_header('Status', '403 Forbidden');
						exit;					
					}
				}
				else{
					watchdog('peppa', 'User Login Error, Unable to Authenticate user : %name  using APIKey: %key', array('%name' => $name,'%key'=>$this->apikey), WATCHDOG_ERROR);							
					drupal_add_http_header('Status', '403 Forbidden');
					exit;					
				}			
				break;
				
			default:
				watchdog('peppa', 'Bad User Request: %argone , Method: %method  with APIKey: %key', array('%argone' => $args[1],'%method'=>$_SERVER['REQUEST_METHOD'],'%key'=>$this->apikey), WATCHDOG_WARNING);							
				drupal_add_http_header('Status', '400 Bad Request');
				exit;								
		}
		
	}	


	/**
	 * helper to create a peppa session and return the user object
	 */
	private function create_peppa_session($uid){
		$sid = peppa_session_create($uid,$this->apikey);
		if($sid){
			$user = user_load($uid);
			$result = new stdClass();
			$result->sid = $sid;
			$result->name = $user->name;
			$result->mail = $user->mail;
			return $result;	
		}
		else{
			return false;
		}
	}
	
	/**
	 * helper to get the JSON payload
	 */
	protected function get_json_data($decode=true){
		$data = file_get_contents('php://input');
		return ($decode)?json_decode($data):$data;	
	}
	
	
	
} 
