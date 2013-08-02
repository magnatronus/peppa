<?php

/**
 * @file
 * The permissions definitions and set up for the peppa module
 * 
 * @author: Stephen Rogers (srogers.uk@gmail.com) - www.spiralarm.co.uk @sarmcon
 * 
 * This work is licensed under a Creative Commons Attribution-NonCommercial 3.0 Unported License.
 * http://creativecommons.org/licenses/by-nc/3.0
 * 
 * 
 * 
 */
 
/**
 * Implement hook_permission
 */
function peppa_permission(){
	return array(
		'peppa api' => array(
			'title' => 'Access Peppa endpoint'
		)
	);	
} 
