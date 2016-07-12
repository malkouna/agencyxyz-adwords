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

function adwords_company_list_report() {
  /* Log into adwords, no client_login as it's over the entire MCC */
  $user = adwords_create_adwords_user();
  if (!$user) {
    watchdog('adwords', t('Cannot log into AdWords server'), NULL, WATCHDOG_ERROR);
    return;
  }
  /* Get all the accounts defined in MCC */
  // Get the service, which loads the required classes.
  $managedCustomerService =
    $user->GetService('ManagedCustomerService', ADWORDS_VERSION);

  // Create selector.
  $selector = new Selector();
  // Specify the fields to retrieve.
  $selector->fields = array('CustomerId', 'Name');

  // Make the get request.
  $graph = $managedCustomerService->get($selector);
  foreach ($graph->entries as $account) {
    $detail_link = l($account->customerId, 'admin/reports/adwords-customer-detail/' . $account->customerId);
    $rows[] = array($detail_link, $account->name);
  }
  $output = theme_table(array('Customer Id', 'Customer Name'), $rows);

  return $output;
}

