<?php
function adwords_admin_settings() {
	$form['adwords_email_server'] = array(
		'#type' => 'textfield',
		'#title' => t('Email server'),
		'#description' => t('Use a string in the form <i>{codeontheroad.com.au:110/pop3}INBOX</i>'),
		'#default_value' => variable_get('adwords_email_server', '{codeontheroad.com.au:110/pop3}INBOX'),
	);

	$form['adwords_email_address'] = array(
		'#type' => 'textfield',
		'#title' => t('Email Address'),
		'#default_value' => variable_get('adwords_email_address', ''),
	);
	
	$form['adwords_email_password'] = array(
		'#type' => 'password',
		'#title' => t('Password'),
		'#default_value' => variable_get('adwords_email_password', ''),
	);
	return system_settings_form($form);
}