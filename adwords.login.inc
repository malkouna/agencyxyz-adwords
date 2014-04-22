<?php
function _adwords_login($form_state) {
  $form['adwords'] = array(
    '#type' => 'fieldset',
    '#title' => 'Google AdWords Login Details',
  );
  $form['adwords']['email'] = array(
    '#type' => 'textfield',
    '#title' => t('Email'),
  );
  
  $form['adwords']['password'] = array(
    '#type' => 'password',
    '#title' => t('Password'),
  );
  
  $form['adwords']['login'] = array(
    '#type' => 'submit',
    '#value' => t('Login'),
    '#submit' => array(_adwords_login_submit),
  );
  return $form;
}

function _adwords_login_submit(&$form, &$form_state) {
  // print_r($form);
  require_once 'src/Google/Api/Ads/AdWords/Lib/AdWordsUser.php';
  // drupal_set_message('arg_separator.output=' . ini_get('arg_separator.output'), 'status', false);
  ini_set('arg_separator.output', '&');

  try {
    // Get AdWordsUser from credentials in "../auth.ini"
    // relative to the AdWordsUser.php file's directory.
    $adworduser = new AdWordsUser();
    
    // save this in the session for later
    $_SESSION['adworduser'] = true;
  
    // Log SOAP XML request and response.
    $adworduser->LogDefaults();
  
    // Got to here can redirect to client list
    $form['#redirect'] = 'adwords/clients';
//    // Get the CampaignService.
//    $campaignService = $adworduser->GetCampaignService('v200909');
//  
//    // Create selector.
//    $selector = new CampaignSelector();
//  
//    // Get all campaigns.
//    $page = $campaignService->get($selector);
//  
//    // Display campaigns.
//    if (isset($page->entries)) {
//      foreach ($page->entries as $campaign) {
//        drupal_set_message('Campaign with name "' . $campaign->name . '" and id "'
//            . $campaign->id . "\" was found.", 'status', false);
//      }
//    } else {
//      drupal_set_message("No campaigns were found.", 'status', false);
//    }
  } catch (Exception $e) {
    drupal_set_message(print_r($e), 'error', false);
  }
}
