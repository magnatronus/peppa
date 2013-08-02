<?php

/**
 * @file
 * the Apps Admin Forms and handlers
 * 
 * @author: Stephen Rogers (srogers.uk@gmail.com) - www.spiralarm.co.uk @sarmcon
 * 
 * This work is licensed under a Creative Commons Attribution-NonCommercial 3.0 Unported License.
 * http://creativecommons.org/licenses/by-nc/3.0
 * 
 */

// main Peppa Apps form 
function peppa_apps_form($form, &$form_state){
 	
	// Add App Link
	$form['add']=array(
		'#markup' => sprintf('<p>%s</p>',l(t('Add Application'),"peppa/application/add",array('query' => drupal_get_destination())))
	);		

	// application options
	$form['options'] = array(
		'#title' => t('Options'),
		'#type' => 'fieldset'
	);
	
	// is a key required for access
	$form['options']['keyrequired'] = array(
		'#title' => t('Key Required for API access'),
		'#description' => t('It is recommended that this option be enabled for ALL production systems.'),
		'#type' => 'checkbox',
		'#default_value' => variable_get('peppa_validate_key', false),
	);	

	// is new user created active or blocked (can use API but cannot log into web site)
	$form['options']['userblocked'] = array(
		'#title' => t('New User Status Blocked'),
		'#description' => t('If a user account is Blocked then they can login via the Peppa REST API but not into the host web site.'),
		'#type' => 'checkbox',
		'#default_value' => variable_get('peppa_user_blocked', true),
	);	

	// can the api create  a user
	$form['options']['usercreate'] = array(
		'#title' => t('Enable API User create '),
		'#description' => t('Can the API be used to create a basic Drupal user account.'),
		'#type' => 'checkbox',
		'#default_value' => variable_get('peppa_user_create', false),
	);	

	// save options
	$form['options']['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Apply'),
	);	
	
	
	// current apps
	$form['apps'] = array(
		'#title' => t('Current Applications'),
		'#type' => 'fieldset'
	);
	
	$form['apps']['list']=array(
		'#markup' => sprintf('<p>%s</p>',peppa_apps_table())
	);

	return $form;
 	
}

// Handler for the application options submit
function peppa_apps_form_submit($form, &$form_state){
	
	// is a app key required for API access	
	variable_set('peppa_validate_key', $form_state['values']['keyrequired']);

	// can the API be used to create a Drupal user account
	variable_set('peppa_user_create', $form_state['values']['usercreate']);
	
	// IS the New USer Status Blocked
	variable_set('peppa_user_blocked', $form_state['values']['userblocked']);	
	
}

// form for create and update of a Peppa Application
function peppa_apps_detail_form($form, &$form_state,$appid=false){

	// if this is an edit lets get the app detail
	$app = false;
	if($appid){
		$form['appid']=array('#type'=>'value','#value'=>$appid);		
		$app = peppa_apps_get_byappid($appid);
	}

	// container
	$form['apps'] = array(
		'#title' => ($app)?t('Edit Application'):t('Add Application'),
		'#type' => 'fieldset'
	);

	// apikey	
	$form['apps']['apikey']=array(
		'#markup' => sprintf('<p>API Key: %s</p>', ($app)?$app->apikey:'The API Key will be automatically generated when the new Application entry has been created.')
	);

	// name of app
	$form['apps']['name'] = array(
		'#title' => t('Application name'),
		'#type' => 'textfield',
		'#required' => 'true',
		'#default_value' => ($app)?$app->name:'',
	);	

	// status of app
	$form['apps']['status'] = array(
		'#title' => t('Status'),
		'#type' => 'select',
		'#options' => array(
			0 => t('Disabled'),
			1 => t('Enabled'),
		),
		'#default_value' => ($app)?$app->status:0,
	);	

	// app info
	$form['apps']['info'] = array(
		'#title' => t('Information'),
		'#type' => 'textarea',
		'#default_value' => ($app)?$app->info:'',
		'#description' => t('option detailed information about the application.'),
	);	

	// submit
	$form['apps']['submit'] = array(
		'#type' => 'submit',
		'#value' => ($appid)?t('Update'):t('Add'),
	);	
	
	return $form;

}

// submit handler for the app detail form
function peppa_apps_detail_form_submit($form, &$form_state){

	$values = $form_state['values'];
	
	if($values['appid']){
		peppa_apps_update($values['appid'],$values['name'],$values['status'],$values['info']);		
	}
	else{
		peppa_apps_create($values['name'],$values['status'],$values['info']);		
	}
	
}


// Delete confirmation form 
function peppa_apps_delete_form($form, &$form_state,$appid=0){
 	
	$form['appid']=array('#type'=>'value','#value'=>$appid);		
	$app = peppa_apps_get_byappid($appid);

	// application options
	$form['delete'] = array(
		'#title' => t('Delete this application?'),
		'#type' => 'fieldset'
	);
	
	$form['delete']['name']=array(
		'#markup' => sprintf('<p><strong>Name: %s</strong></p>', $app->name)
	);

	// confirm delete
	$form['delete']['confirm'] = array(
		'#title' => t('Confirm'),
		'#description' => t('You need to tick here to confirm DELETE'),
		'#type' => 'checkbox'
	);	
	
	// submit
	$form['delete']['submit'] = array(
		'#type' => 'submit',
		'#value' => t('Delete'),
	);	

	return $form;
 	
}

// validate handler for the app delete confirmation
function peppa_apps_delete_form_validate($form, &$form_state){
	// ensure that the confirm is selected
	if(!$form_state['values']['confirm']){
		form_set_error('confirm', t('You need to tick the Confirm box to delete the application.'));	
	}
}

// submit handler for the app delete confirmation
function peppa_apps_delete_form_submit($form, &$form_state){
	
	peppa_apps_delete($form_state['values']['confirm']);

}

// helper function to generate the paged applications table
function peppa_apps_table(){
	
	$apps = peppa_apps_get_all();	
	$header=array();
	$rows=array();
	
	$header[] = array('data'=>'App Name');
	$header[] = array('data'=>'API Key');
	$header[] = array('data'=>'Created');
	$header[] = array('data'=>'Status');
	$header[] = array('data'=>'Options');


	foreach($apps as $app) {
		
 		//create the edit and delete links
		$editlink = l($app->name,"peppa/application/update/$app->appid",array('query' => drupal_get_destination()));
		$deletelink = l(t('delete'),"peppa/application/delete/$app->appid",array('query' => drupal_get_destination()));
		  		
   		$row = array($editlink, $app->apikey, format_date($app->created,'long'),(!$app->status)?'Disabled':'Enabled',sprintf(' %s ', $deletelink));
   		$rows[] = $row;
	}

	// Depending on the context there may be a better choice than this
	$table = theme('table',array('header'=>$header,'rows'=>$rows));
	
	// Add our pager
	$table .= theme('pager');
	
	return $table;
	
}
