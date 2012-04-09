<?php
function adwords_admin_settings() {

  $form['adwords'] = array(
    '#type' => 'fieldset',
    '#title' => t('AdWords API settings'),
  );

  $form['adwords']['adwords_sandbox'] = array(
    '#type' => 'checkbox',
    '#title' => t('Use Sandbox?'),
    '#default_value' => variable_get('adwords_sandbox', 1),
  );

  $form['adwords']['adwords_email_address'] = array(
    '#type' => 'textfield',
    '#title' => t('Email Address'),
    '#default_value' => variable_get('adwords_email_address', ''),
  );

  $form['adwords']['adwords_password'] = array(
    '#type' => 'password',
    '#title' => t('Password'),
    '#default_value' => variable_get('adwords_password', ''),
  );

  return system_settings_form($form);
}