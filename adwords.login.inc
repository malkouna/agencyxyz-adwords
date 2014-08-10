<?php

function _adwords_login() {
  adwords_create_adwords_user();

  $form = array();

  $form['adwords'] = array(
    '#type' => 'fieldset',
    '#title' => 'Get Refresh Token (OAuth2) for Google AdWords',
  );
  $form['adwords']['adwords_client_id'] = array(
    '#type' => 'textfield',
    '#title' => t('Client Id'),
    '#default_value' => variable_get('adwords_client_id', ADWORDS_CLIENT_ID),
  );

  $form['adwords']['adwords_client_secret'] = array(
    '#type' => 'textfield',
    '#title' => t('Client Secret/'),
    '#default_value' => variable_get('adwords_client_secret', ADWORDS_CLIENT_SECRET),
  );

  $form['adwords']['adwords_developer_token'] = array(
    '#type' => 'textfield',
    '#title' => t('Developer Token'),
    '#default_value' => variable_get('adwords_developer_token', ADWORDS_DEVELOPER_TOKEN),
  );

  $form['adwords']['adwords_user_agent'] = array(
    '#type' => 'textfield',
    '#title' => t('User Agent'),
    '#default_value' => variable_get('adwords_user_agent', ADWORDS_USER_AGENT),
  );

  $form['adwords']['adwords_refresh_token'] = array(
    '#type' => 'textfield',
    '#title' => t('Refresh Token'),
    '#default_value' => variable_get('adwords_refresh_token', ADWORDS_REFRESH_TOKEN),
  );

  return system_settings_form($form);
}

