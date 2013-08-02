<?php

/**
 * @file
 * This file conatins a  the data access function required to store and retrieve data from our selected perm storage
 * 
 * @author: Stephen Rogers (srogers.uk@gmail.com) - www.spiralarm.co.uk @sarmcon
 * 
 * This work is licensed under a Creative Commons Attribution-NonCommercial 3.0 Unported License.
 * http://creativecommons.org/licenses/by-nc/3.0
 * 
 * 
 */
 

/**
 * Peppa Object Functions
 */
function peppa_object_create($name,$data,$uid){
		
	// create an objectId
	$oid = md5(uniqid($uid.$name));
	
	// now create the object record
	db_insert('peppa_object')->fields(array(
		'uid' => $uid,
		'oid' => $oid,
		'name' => $name,
		'created'=>time(),
		'updated'=>time(),
		'data'=>$data
	))->execute();
		
	return $oid;			
	
}

// update an existing object
function peppa_object_update($name,$data,$uid,$oid){
	
	return db_update('peppa_object')->fields(array(
		'updated'=>time(),
		'data'=>$data
	))->condition('name', $name)->condition('uid',$uid)->condition('oid',$oid)->execute();
	
}

// get all the named objects owned by the specified user
function peppa_object_get_byuid($name,$uid){
	return db_query('SELECT * FROM {peppa_object} WHERE name = :name AND uid = :uid ORDER BY updated DESC', array(':name' => $name,':uid' => $uid))->fetchAll();		
}

// get an object by its object id and owner
function peppa_object_get_byoid($oid,$uid){
	return db_query('SELECT * FROM {peppa_object} WHERE oid = :oid AND uid = :uid', array(':oid' => $oid,':uid' => $uid))->fetch();	
}
 
// get an object by its object id, owner and name
function peppa_object_getnamed_byoid($name,$oid,$uid){
	return db_query('SELECT * FROM {peppa_object} WHERE oid = :oid AND uid = :uid AND name = :name', array(':oid' => $oid,':uid' => $uid, ':name' => $name))->fetch();	
}
  
// delete a named object  
function peppa_object_deletenamed_byoid($name,$oid,$uid){
	return db_delete('peppa_object')->condition('oid', $oid)->condition('name', $name)->condition('uid', $uid)->execute();			
}  
  
  
/**
 * Peppa  Session functions
 */ 
 
// create a new user session and remove any old one if found 
function peppa_session_create($uid,$apikey){

	// first look for an old session and remove if found
	peppa_session_delete_by_uid($uid);
	
	// now create a new session id
	$sid = md5(uniqid($uid));
	
	// now create the object record
	$res = db_insert('peppa_session')->fields(array(
		'uid' => $uid,
		'sid' => $sid,
		'created'=>time(),
		'apikey'=> $apikey,
		'lastcall' => time()
	))->execute();
	
	return $sid;
}
 
// save the last API call time for the specified session 
function peppa_session_update_lastcall($sid){
	db_update('peppa_session')->fields(array(
		'lastcall' => time()
	))->condition('sid', $sid)->execute();	
} 
 
// get a session id details by the session id 
function peppa_session_get_by_sid($sid){
	return db_query('SELECT * FROM {peppa_session} WHERE sid = :sid', array(':sid' => $sid))->fetch();	
} 
 
// delete the session with the specified sessionid 
function peppa_session_delete_by_sid($sid){
	return db_delete('peppa_session')->condition('sid', $sid)->execute();		
}
 
// delete the session associated with a drupal user id 
function peppa_session_delete_by_uid($uid){
	return db_delete('peppa_session')->condition('uid', $uid)->execute();		
}
 
/**
 * peppa_apps functions
 */ 
 
// validate an app key 
function peppa_apps_validate_key($key){
	return db_query('SELECT status FROM {peppa_apps} WHERE apikey = :apikey', array(':apikey' => $key))->fetchField();
}

// get all a paged list of the registered peppa apps
function peppa_apps_get_all($limit=10){
  $query = db_select('peppa_apps', 'pa')->extend('PagerDefault');
  $query->fields('pa', array('appid','name','apikey','status','created'))
        ->orderBy('name', 'ASC')
        ->limit($limit);
  return $query->execute();	
}
 
// get an app by it's appid 
function peppa_apps_get_byappid($appid){
	return db_query('SELECT * FROM {peppa_apps} WHERE appid = :appid', array(':appid' => $appid))->fetch();
} 

// create a new app
function peppa_apps_create($name,$status,$info){
	
	// create an app key
	$appkey = md5(uniqid($name));
	
	// now create the object record
	return db_insert('peppa_apps')->fields(array(
		'apikey' => $appkey,
		'name' => $name,
		'created'=>time(),
		'info'=> $info,
		'status' => $status
	))->execute();
		
}

// update app
function peppa_apps_update($appid,$name,$status,$info){
	
	return db_update('peppa_apps')->fields(array(
		'name' => $name,
		'info'=> $info,
		'status' => $status
	))->condition('appid',$appid,"=")->execute();
		
}

// delete app
function peppa_apps_delete($appid){
	return db_delete('peppa_apps')->condition('appid', $appid)->execute();		
}