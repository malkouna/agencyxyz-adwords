<?php

function _adwords_clients($form_state) {
  // If I don't have a google login then redirect to login
  if (empty($_SESSION['adworduser'])) {
    $form['#redirect'] = 'adwords/login';
    return $form;
  }
  
  // for 
  $form['client'] = array(
    '#value' => l('Client Report', 'adwords/listreports'),
  );
  return $form;
}
