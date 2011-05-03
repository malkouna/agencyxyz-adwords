<?php
function adwords_monthly_list() {
	
}

function adwords_monthly_add($form_state) {
   $query = db_query("
       SELECT td.tid, td.name
       FROM {term_data} td
       JOIN {vocabulary} v
       ON td.vid = v.vid
       AND v.name = 'Clients'");
    while($client = db_fetch_array($query)) {
        $clients[$client['tid']] = $client['name'];
    }
	$form['client_id'] = array(
	   '#type' => 'select',
	   '#options' => $clients,
	   '#title' => 'Client',
	);
	$form['date'] = array(
	   '#type' => 'date_select',
	   '#date_format' => 'm-Y',
	);
	$form['save'] = array(
	   '#type' => 'submit',
	   '#value' => t('Save'),
	   '#submit' => array('adwords_monthly_add_submit'),
	);
	return $form;
}

function adwords_monthly_add_submit($form, &$form_state) {
    db_query("INSERT INTO adwords_daily_note VALUES (%d, '%s', '%s')",
    $form['client_id']['#value'], $form['date']['#value'], $form['note']['#value']);
    $form_state['redirect'] = 'adwords/daily/detail/' . $form['client_id']['#value'];
	
}
function adwords_monthly_submit() {
	
}

function adwords_montly_approve() {
    
}

function adwords_montly_view() {
    
}