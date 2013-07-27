<?php
function adwords_daily_traffic_configure(&$form_state, $client_id) {
	// Example http://www.computerminds.co.uk/quick-guide-using-drupal-add-tabledrag-and-enjoying-jquery-drag-and-drop-loveliness
	// http://coffeeshopped.com/2010/09/drupal-drag-and-drop-tables-in-custom-modules
	$form['rows']['#tree'] = true;
	
	/*
	 * Get columns for this client_id if they exist otherwise
	 * use defaults (client_id = 0) 
	 */
	$form_state['client_id'] = $client_id;
	$query = db_query("SELECT * from adwords_report_columns WHERE client_id = %d ORDER BY weight", $client_id);
	if(db_affected_rows() == 0) {
	  /* Get defaults instead */
	   $query = db_query("SELECT * from adwords_report_columns WHERE client_id = 0 ORDER BY weight");
	}

	while($row = db_fetch_object($query)) {
	  $form['rows'][$row->id]['hide'] = array(
      '#type' => 'checkbox',
	    '#default_value' => $row->hide,
	  );
    $form['rows'][$row->id]['column'] = array(
      '#type' => 'markup',
      '#value' => $row->column,
    );
    
    $form['rows'][$row->id]['column_hidden'] = array(
      '#type' => 'hidden',
      '#value' => $row->column,
    );
    
    $form['rows'][$row->id]['title'] = array(
      '#type' => 'textfield',
      '#default_value' => $row->title,
    );

//    $form['rows'][$row_id]['data'] = array(
//      '#type' => 'value',
//      '#value' => $data,
//    );
    $form['rows'][$row->id]['weight'] = array(
      '#type' => 'weight',
      '#delta' => 20,
      '#default_value' => $row->weight,
      '#attributes' => array('class' => 'weight'),
    );	  
	}
	$form['submit']=array(
      '#type'=>'submit',
      '#value'=>t('Save changes'),
    );
    
    return $form;
}

function adwords_daily_traffic_configure_submit($form, &$form_state) {
  $form_state['redirect'] = 'adwords/daily/detail/' . $form_state['client_id'];
  db_query("DELETE FROM adwords_report_columns WHERE client_id = %d", $form_state['client_id']);
  foreach($form_state['values']['rows'] as $id => $item) {
    db_query("INSERT INTO adwords_report_columns VALUES (null, %d, '%s', '%s', %d, %d)",
      $form_state['client_id'], $item['column_hidden'], $item['title'], $item['hide'], $item['weight']);
  }
}