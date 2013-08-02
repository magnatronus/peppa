<?php

/**
 * @file
 * 
 * class definition that provides the Version 2 REST API access for Peppa.
 * 
 * @author: Stephen Rogers (srogers.uk@gmail.com) - www.spiralarm.co.uk @sarmcon
 * 
 * This work is licensed under a Creative Commons Attribution-NonCommercial 3.0 Unported License.
 * http://creativecommons.org/licenses/by-nc/3.0
 *
 */
 
 
// This is Version 2 of our API  
class PeppaV2REST extends PeppaREST{
	
	/**
	 * Handle all the object V2 API requests - ALL Requests MUST have a valid user session id
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
				
				// first we use an object and remove the any objects not allowed at top level
				// these are oid, created and updated as they are part of generated object
				$object = $this->get_json_data();
				unset($object->oid);			// oid not allowed
				unset($object->created);		// created not allowed
				unset($object->updated);		// updated not allowed
				$data = json_encode($object);
				
				// now do the update
				peppa_object_update($name,$data,$user->uid,$oid);
				$object = peppa_object_get_byoid($oid,$user->uid);
				if($object){
					$this->payload->data = json_decode($object->data);
					$this->payload->data->created = $object->created;
					$this->payload->data->updated = $object->updated;
					$this->payload->data->oid = $oid;
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
					$this->payload->data = json_decode($object->data);
					$this->payload->data->created = $object->created;
					$this->payload->data->updated = $object->updated;
					$this->payload->data->oid = $oid;
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
						$this->payload->data = json_decode($object->data);					
						$this->payload->data->created = $object->created;
						$this->payload->data->updated = $object->updated;
						$this->payload->data->oid = $object->oid;	
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
					$this->payload->data = $data;					
				}
				break;
				
				
		}
		
	}
	 

} 
