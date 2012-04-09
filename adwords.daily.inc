<?php
/**
 * adwords_daily_overview()
 * 
 * If no $adwords_user specified get all accounts, otherwise
 * restrict to just those that are set up for this user.
 */
function adwords_daily_overview($account = false) {
  global $user;	
  // Accounts are defined in the clients vocab
  if(!$account) { // Want all clients
	  $clients = db_query("
	    SELECT td.*
	    FROM {term_data} td
	    JOIN {vocabulary} v
	    ON td.vid = v.vid
	    AND v.name = 'Clients'
	  ");
  }
  else { // Just clients defined for this user
  	 $clients = db_query("
  	     SELECT td.*
  	     FROM {node} n
  	     JOIN {content_field_account_clients} c
  	     ON n.nid = c.nid
  	     JOIN {term_data} td
  	     ON c.field_account_clients_value = td.tid
  	     WHERE n.type = 'account_clients'
  	     AND n.uid = %d
  	 ", $user->uid);	  
  }
  
  /*
   * For each client:
   *    Get the start date of the current month and check if there are campaign values defined
   *    If there isn't a campaign month defined, try and grab from a previous month and use those values
   *    If there isn't a previous month display message for this client
   *    
   *    Always show link to edit/add campaign values
   */
  
  $reporting_start = date("Y-m-01");
  $day_of_month = date("j") - 1; // Todays values won't be in yet, work from yesterday
  /* @AM 1/2/2012 If $day_of_month is 0 then show complete previous month */
  if ($day_of_month == 0) {
    /* Take one month off $reporting_start */
    $reporting_start = date("Y-m-01", strtotime('last month'));
    /* And make $day_of_month end day of that month ('t' is number of days in month */
    $day_of_month = date('t', strtotime($reporting_start));
  }
  $output = "Reporting for month starting " . $reporting_start . " at day " . $day_of_month;
  while($result = db_fetch_object($clients)) {
    /*
     * @AM 7/1/11 check if there is any data in the adwords table before doing anything else
     * if there isn't just go to next client.
     */
    $query = db_query("
      SELECT *
      FROM adwords_data
      WHERE acctname = '%s'
      AND date >= '%s'", $result->name, $reporting_start
    );
    
    if(db_affected_rows() == 0) {
      continue;
    }
  	// Get current targets
  	$query = db_query("
	   SELECT * 
	   FROM adwords_campaign_targets
	   WHERE client_id = %d
	   AND campaign_date = '%s'", $result->tid, $reporting_start);
  	$targets = db_fetch_object($query);
  	
  	// If there are no targets defined see if we can grab some defaults from previous months
  	if (!$targets) {
  		$query = db_query("
  		    SELECT *
	        FROM adwords_campaign_targets
	        WHERE client_id = %d
	        ORDER by campaign_date DESC  		    
  		", $result->tid);
  		$targets = db_fetch_object($query);
  	}
  	
  	// If still don't have targets defined then don't show results
    $options = array(
        'html' => true,
        'attributes' => array(
            'title' => t('Define Campaign Budget'),
        )
    );
  	if (!$targets) {
  		$output .= '
        <tr>
            <td>' . l($result->name, 'adwords/daily/detail/' . $result->tid) . '</td>
            <td colspan="12"> No campaign budgets have been defined for this client</td>';
      if (user_access('administer adwords')) {
        $output .= '
            <td class="define-campaign-budget">' . l('<span>Define Campaign Budget</span>', 'adwords/budgets/' . $result->tid, $options) . '</td>';
      }
      $output .= '
         </tr>';
  	}
  	else {
  		$totals = _adwords_daily_totals($result->tid, $targets);
  		$totals_string = '';

  		foreach($totals as $total) {
  		  $total = doubleval($total);
  			if ($total > 0) {
  				$class = 'green';
  			}
  			else {
  				$class = 'red';
  			}
  			$totals_string .= '<td class="' . $class . '">' . number_format($total, 2, '.',  ',') . '</td>'; 
  		}
        $output .= '
            <tr>
                <td>' . l($result->name, 'adwords/daily/detail/' . $result->tid) . '</td>' .
                $totals_string;
          if (user_access('administer adwords')) {
            $output .= '
                <td class="define-campaign-budget">' . l('<span>Define Campaign Budget</span>', 'adwords/budgets/' . $result->tid, $options) . '</td>';
          }
          $output .= '
            </tr>';
  	}
  }
  
  if($output) {
    $result = '
        <table>
            <thead>
                <tr>
                    <td>Client</td>
                    <td>Clicks on Target?</td>
                    <td>Clicks Delivered</td>
                    <td>Clicks Target</td>
                    <td>Clicks Change Required</td>
                    <td>Clicks Change Required</td>
                    <td>Budget on Track?</td>
                    <td>Spend Actual</td>
                    <td>Spend Target</td>
                    <td>Spend Change Required</td>
                    <td>Spend Change Required</td>
                    <td>Average Position Overall</td>
                    <td>Target Position Overall</td>';
    if(user_access('administer adwords')) {
      $result .= '
                    <td>Edit Budget</td>';
    }
    $result .= '
                </tr>
            </thead>
            <tbody>' . $output . '
            </tbody>
        </table>';
  }
  return $result;
}

function _adwords_daily_totals($client_id, $targets) {
	$yesterday = date("Y-m-d", mktime(0, 0, 0, date("m"), date("d")-1, date("Y"))); // Todays values won't be in yet, so work from yesterday
  $campaign_date = date("Y-m-01");
  $day_of_month = date("j") - 1;  // Use yesterday 
  /* @AM 1/2/2012 If $day_of_month is 0 then show complete previous month */
  if ($day_of_month == 0) {
    /* "Yesterday" will be last day of the previous month */
    $yesterday = date("Y-m-d", strtotime('yesterday', $campaign_date));
    /* Take one month off $campaign_date */
    $campaign_date = date("Y-m-01", strtotime('last month'));
    /* And make $day_of_month end day of that month ('t' is number of days in month */
    $day_of_month = date('j', strtotime($reporting_start));

  }


  $inventory_budget = $targets->total_budget/((100 + $targets->service_fee)/100.0);
  $days_in_month = date('t', strtotime($campaign_date));
  $days_left_in_month = $days_in_month - $day_of_month + 1; // +2 as want the day before yesterday
  $daily_spend = $inventory_budget/$days_in_month;

  $target_monthly_clicks = $inventory_budget/$targets->target_cpc;
	$target_daily_average_clicks = $target_monthly_clicks/$days_in_month;  // date('t') gives days in current month
	// Get aggregate data
	$totals_data = db_fetch_object(db_query("
	   SELECT SUM(ad.clicks) AS clicks, SUM(ad.cost)/1000000.0 AS cost,
	       AVG(ad.pos) AS average_position
	   FROM adwords_data ad
	   JOIN {term_data} td
	   ON ad.acctname = td.name
	   WHERE ad.date >= '%s' 
	   AND ad.date < '%s'
	   AND td.tid = %d
	", $campaign_date, $yesterday, $client_id));
	// Get todays data
    $yesterdays_data = db_fetch_object(db_query("
       SELECT clicks, cost/1000000 as cost
       FROM adwords_data ad
       JOIN {term_data} td
       ON ad.acctname = td.name
       WHERE ad.date = '%s' 
       AND td.tid = %d
    ", $yesterday, $client_id));
    
    $results['clicks_on_target'] = ($totals_data->clicks + $yesterdays_data->clicks) - $day_of_month*$target_daily_average_clicks;
    $results['daily_clicks_delivered'] = $yesterdays_data->clicks;
    $results['daily_clicks_target'] = ($target_monthly_clicks - $totals_data->clicks)/$days_left_in_month;
    $results['daily_clicks_change_number'] = $results['daily_clicks_target'] - $results['daily_clicks_delivered'];
    if ($results['daily_clicks_delivered'] > 0)
        $results['daily_clicks_change_percent'] = $results['daily_clicks_change_number']/$results['daily_clicks_delivered'] * 100;
    else
        $results['daily_clicks_change_percent'] = '';
        
    $results['budget_on_track'] = $day_of_month*$daily_spend - $totals_data->cost;
    $results['daily_spend_actual'] = $yesterdays_data->cost;
    $results['daily_spend_target'] = ($inventory_budget - $totals_data->cost)/$days_left_in_month;
    $results['daily_spend_change_number'] = $results['daily_spend_target'] - $results['daily_spend_actual'];
    if ($results['daily_spend_actual'] > 0)
        $results['daily_spend_change_percent'] = $results['daily_spend_change_number']/$results['daily_spend_actual'] * 100;
    else
        $results['daily_spend_change_percent'] = '';
    
    $results['average_position_overall'] = $totals_data->average_position;
    $results['target_average_position'] = $targets->target_avg_pos;
       	
	return $results;
}

function adwords_daily_traffic($client_id, $start_date = NULL, $end_date = NULL) {
  if (!is_numeric($client_id)) {
    return drupal_access_denied();
  }

  if (!$start_date) {
     $start_date = date("Y-m-01");
  }
  if (!$end_date) {
    $end_date = date("Y-m-d");
  }
  
  /* if we're at the start of the month rewind to previous month */
  if (date('j', strtotime($end_date)) == 1) {
    $start_date = date("Y-m-01", strtotime('last month', $start_date));
    $end_date = date("Y-m-d", strtotime('last month', $end_date));
  }  
  
  // Get and display all notes for this client
  $query = db_query("
      SELECT *
      FROM adwords_daily_note adn
      WHERE client_id = %d
      AND date >= '%s'
      AND date <= '%s'
      ORDER BY date", $client_id, $start_date, $end_date
  );
  $order = 0;
  while($note = db_fetch_array($query)) {
    $note['zebra'] = $order %2==0?'odd':'even';
    $notes[] = $note;
    $order++;
  }

  $result = db_fetch_object(
    db_query("
      SELECT name
      FROM {term_data}
      WHERE tid = %d", $client_id));

  drupal_set_title(t('Adwords Daily Traffic Report for @client', array('@client' => $result->name)));
  // $output = 'Hello World' . $client_id;

  $results = adwords_daily_traffic_calc($client_id, $result->name, $start_date, $end_date);
  
  // Now theme the results
  $output = theme('adwords_notes', $notes);
  if (user_access('administer adwords')) {
    $output .= l(t('Edit Columns'), 'adwords/daily/detail/' . $client_id . '/configure');
  }
  
  $output .= theme('adwords_results', $results['rows'], $results['totals'], $client_id);
  
  /* @AM 09/04/2012 Show last months details as well */
  $archive_start_date = date('Y-m-d', strtotime("-1 month", strtotime($start_date)));;
  $archive_end_date = date('Y-m-d', strtotime("-1 day", strtotime($start_date)));
  $archive_results  = adwords_daily_traffic_calc($client_id, $result->name, $archive_start_date, $archive_end_date);
  $output .= '
  	<fieldset class="collapsible collapsed">
    	<legend>Last Month Overview</legend>' .
      theme('adwords_results', $archive_results['rows'], $archive_results['totals'], $client_id) .
    '</fieldset>';
  
  return $output;
}

function adwords_daily_traffic_calc($client_id, $client_name, $start_date, $end_date) {
  $start_datestamp = strtotime($start_date);
  
  $budget = db_fetch_object(db_query("
      SELECT * 
      FROM adwords_campaign_targets
      WHERE client_id = %d
      AND campaign_date = '%s'", $client_id, $start_date));


  //    $today = date("Y-m-d");
  //    $day_of_month = date("j");
  $days_in_month = date('t', strtotime($start_date));
  //     $days_left_in_month = $days_in_month - $day_of_month;


  if($budget) {
    $inventory_budget = $budget->total_budget / ((100.0 + $budget->service_fee)/100.0);
    $daily_spend = $inventory_budget/$days_in_month;
    $target_monthly_clicks = $inventory_budget/$budget->target_cpc;
    $target_daily_average_clicks = $target_monthly_clicks/$days_in_month;  // date('t') gives days in current month
  }
  
 
  $rows = array();

  $query = db_query("
      SELECT ad.*
      FROM adwords_data ad
      WHERE ad.acctname = '%s'
      AND ad.date >= '%s'
      AND ad.date <= '%s'
      ORDER by ad.date", $client_name, $start_date, $end_date);

  $daynum = 1;
  $actual_cumulative_clicks = 0;
  $actual_cumulative_spend = 0;
  
  /*
   * @AM 18/3/2011 Need to step through every day of month to set up
   * average data for every selected date
   */

  for($daynum = 1; $daynum <= $days_in_month; $daynum++) {
  	$rowdatestamp = mktime(0, 0, 0, date("m", $start_datestamp), date("d", $start_datestamp) + $daynum - 1, date("Y", $start_datestamp));
  	$rowdate = date("Y-m-d", $rowdatestamp);
  	
    /*
     *  @AM 10/8/2010 Set up the "add notes" link to be a different image
     *  and have hover text if there are notes for that day.
     */
    $notetext = '';
    $class = 'add-note';
      $hover = db_query("
          SELECT note
          FROM adwords_daily_note adn
          WHERE client_id = %d
          AND date = '%s'", $client_id, $rowdate
      );
      while($note = db_fetch_array($hover)) {
          $notetext .= '<li>' . $note['note'] . '</li>'; 
      }
      
      $alt = $title = t('Add Note');
    if (!empty($notetext)) {
      $class .= ' saved-note';
    }
    $options = array(
        'html' => true,
        'attributes' => array(
            'title' => $title,
            'class' => $class,
          'alt' => $alt,
      )
    );
    
    $columns['date'] = str_replace('-', '&nbsp;', date('D-d/m', $rowdatestamp));
    $columns['imps'] = '';
    $columns['clicks'] = '';
    $columns['ctr'] = '';
    $columns['avg_cpc'] = '';
    $columns['daily_spend'] = '';
    $columns['avg_pos'] = '';
    $columns['conv'] = '';
    $columns['conv_rate'] = '';
    $columns['cpa'] = '';
    $columns['target_daily_average_clicks'] = number_format($target_daily_average_clicks, '0');
    $columns['revised_average_daily_clicks_target'] = '';
    $columns['target_daily_spend'] = '$' . number_format($daily_spend, '0');
    $columns['revised_daily_spend_target'] = '';
    $columns['target_cumulative_spend'] = '$' . number_format($daily_spend * $daynum, '0');
    $columns['actual_cumulative_spend'] = '';  
    $columns['under_or_over_cumulative_budget'] = '';
    $columns['remaining_budget'] = '';
    $columns['target_cumulative_clicks']  = number_format($target_daily_average_clicks * $daynum, '0');
    $columns['actual_cumulative_clicks'] = '';
    $columns['purchases'] = '';
    $columns['leads'] = '';
    $columns['signups'] = '';
    $columns['pageviews'] = '';
    $columns['default_conv'] = '';
    if (user_access('administer adwords')) {
      $columns['add_note'] = l('<span>Add Note</span>', 'adwords/daily/note/add/' . $client_id . '/' . $rowdate, $options);
      $columns['add_note'] .= '<ul class="notetext">' . $notetext . '</ul>';
    }
     
    //    foreach($row as $column => $value) {
    //      $columns[$column] = $value;
    //    }
    $rows[$rowdate] = $columns;
  }

  /*
   * @AM 18/3/2011 Now look at data and get actual data
   */
  while($row = db_fetch_object($query)) {
    $daynum = date("j", strtotime($row->date));
    $rows[$row->date]['imps'] = number_format($row->imps, '0');
    $total_imps += $row->imps;
    $rows[$row->date]['clicks'] = number_format($row->clicks, '0');
    $total_clicks += $row->clicks;
    $rows[$row->date]['ctr'] = number_format(($row->ctr)*100, '2') . '%';
    $rows[$row->date]['avg_cpc'] = '$' . number_format(($row->cpc)/1000000, '3');
    $rows[$row->date]['daily_spend'] = '$' . number_format($row->cost/1000000, '0');
    $total_daily_spend += $row->cost;
    $rows[$row->date]['avg_pos'] = number_format($row->pos, '1');
    $total_avg_pos += $row->pos;
    $rows[$row->date]['conv'] = number_format($row->conv, '0');
    $total_conversions += $row->conv;
    $rows[$row->date]['conv_rate'] = number_format(($row->convRate)*100, '2') . '%';
    $total_conversion_rate += $row->convRate;
    $rows[$row->date]['cpa'] = '$' . number_format($row->costPerConv/1000000, '3');
    $total_cpa += $row->costPerConv;
    $rows[$row->date]['revised_average_daily_clicks_target'] = number_format(($target_monthly_clicks - $actual_cumulative_clicks)/($days_in_month - $daynum + 1), '0');
    $rows[$row->date]['revised_daily_spend_target'] = '$' . number_format(($inventory_budget - $actual_cumulative_spend) / ($days_in_month - $daynum + 1), '0');

    $rows[$row->date]['under_or_over_cumulative_budget'] = '$' . number_format(($daily_spend * $daynum) - $actual_cumulative_spend - $row->cost/1000000, '0');
    $rows[$row->date]['remaining_budget'] = '$' . number_format($inventory_budget - $actual_cumulative_spend, '2');

    // Format required columns and add up totals now that the previous data not needed
    $actual_cumulative_spend += $row->cost/1000000;  // Add cost after doing revised target
    $rows[$row->date]['actual_cumulative_spend'] = '$' . number_format($actual_cumulative_spend, '0');  // Format after I don't need number
    $actual_cumulative_clicks += $row->clicks;  // Add clicks after doing revised target
    $rows[$row->date]['actual_cumulative_clicks'] = number_format($actual_cumulative_clicks, '0');
    $rows[$row->date]['purchases'] = number_format($row->purchases, 0);
    $rows[$row->date]['leads'] = number_format($row->leads, 0);
    $rows[$row->date]['signups'] = number_format($row->signups, 0);
    $rows[$row->date]['pageviews'] = number_format($row->pageviews, 0);
    $rows[$row->date]['default_conv'] = number_format($row->defaultConv, 0);
     
    //    foreach($row as $column => $value) {
    //      $columns[$column] = $value;
    //    }
  }
  // Build totals array for disaply
  $totals['date'] = '';
  $totals['imps'] = number_format($total_imps, '0');
  $totals['clicks'] = number_format($total_clicks, '0');
  $totals['ctr'] = number_format($total_clicks/$total_imps*100/($daynum-1), '2') . '%';
  $totals['avg_cpc'] = '$' . number_format($total_daily_spend/($total_clicks*1000000), '3');
  $totals['daily_spend'] = '$' . number_format($total_daily_spend/1000000, '0');
  $totals['avg_pos'] = number_format($total_avg_pos/$daynum, '1');
  $totals['conv'] = number_format($total_conversions, '0');
  $totals['conv_rate'] = number_format($total_conversion_rate*100/($daynum-1), '2') . '%';
  $totals['cpa'] = '$' . number_format($total_cpa/(($daynum-1)*1000000), '3');
  $totals['target_daily_average_clicks'] = number_format($target_monthly_clicks);
  
  $results['totals'] = $totals;
  $results['rows'] = $rows;
  
  return $results;
}

function adwords_campaign_budget($form_state, $client_id) {
  // Get client name for title
  $result = db_fetch_object(db_query("SELECT name FROM {term_data} WHERE tid = %d", $client_id));
  drupal_set_title('AdWords Campaign Budget for ' . $result->name);
  $budget = db_fetch_object(db_query("
      SELECT * 
      FROM adwords_campaign_targets
      WHERE client_id = %d
      AND campaign_date = '%s'", $client_id, date("Y-m-01")));

  // If there is no budget defined see if we can grab some defaults from previous months
  if (!$budget) {
    $query = db_query("
        SELECT *
        FROM adwords_campaign_targets
        WHERE client_id = %d
        ORDER by campaign_date DESC         
    ", $client_id);
    $budget = db_fetch_object($query);
  }
  
  $form['client_id'] = array(
      '#type' => 'hidden',
      '#value' => $client_id,
  );

  $form['total_budget'] = array(
      '#type' => 'textfield',
      '#title' => t('Total Budget'),
      '#required' => true,
      '#size' => 10,
      '#default_value' => $budget?$budget->total_budget:'',
  );
  $form['service_fee'] = array(
      '#type' => 'textfield',
      '#title' => t('Service Fee'),
      '#required' => true,
      '#size' => 10,
      '#default_value' => $budget?$budget->service_fee:'',
  );
  $form['target_cpc'] = array(
      '#type' => 'textfield',
      '#title' => t('Target CPC'),
      '#required' => true,
      '#size' => 10,
      '#default_value' => $budget?$budget->target_cpc:'',
  );
  $form['target_avg_pos'] = array(
      '#type' => 'textfield',
      '#title' => t('Target Average Position'),
      '#required' => true,
      '#size' => 10,
      '#default_value' => $budget?$budget->target_avg_pos:'',
  );
    $form['target_cpa'] = array(
        '#type' => 'textfield',
        '#title' => t('Target Cost Per Acquisition'),
        '#required' => true,
        '#size' => 10,
        '#default_value' => $budget?$budget->target_cpa:'',
    );
  $form['save'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
      '#submit' => array('adwords_campaign_budget_submit'),
  );
  return $form;
}

function adwords_campaign_budget_submit($form, &$form_state) {
  // print_r($form);
  $campaign_date = date("Y-m-01");
  db_query("DELETE FROM adwords_campaign_targets WHERE campaign_date = '%s' AND client_id = %d", $campaign_date, $form['client_id']['#value']);
  db_query("INSERT INTO adwords_campaign_targets VALUES ('%s', %d, %f, %f, %f, %f, %f)",
     $campaign_date, $form['client_id']['#value'], $form['total_budget']['#value'], $form['service_fee']['#value'],
     $form['target_cpc']['#value'], $form['target_avg_pos']['#value'], $form['target_cpa']['#value']);
  $form_state['redirect'] = 'adwords/daily';
}

function adwords_daily_note($form_state, $action, $id, $date = NULL) {
  $form['action'] = array(
    '#type' => 'hidden',
    '#value' => $action,
  );
  
  $form['id'] = array(
    '#type' => 'hidden',
    '#value' => $id,
  );
  
  $form['date'] = array(
    '#type' => 'hidden',
    '#value' => $date,  
  );
  
  if($action == 'delete') {
    $form['message'] = array(
      '#type' => 'markup',
      '#value' => '<div class="confirm-delete">This will permanently delete this note. Press <i>Delete</i> to continue.</div>',
    );
    $form['delete'] = array(
      '#type' => 'submit',
      '#value' => 'Delete',
    );
  }
  else {
    if ($action == 'edit') {
      /* get text of current note */
      $default_text = db_fetch_object(db_query("SELECT note FROM adwords_daily_note WHERE nid = %d", $id))->note;
    }

    $form['date_display'] = array(
      '#value' => $date,
    );
    $form['note'] = array(
      '#type' => 'textarea',
      '#title' => t('Note'),
      '#default_value' => $default_text,
    );
    $form['save'] = array(
      '#type' => 'submit',
      '#value' => t('Save Note'),
    );
  }  
  
  return $form;
}

function adwords_daily_note_submit($form, &$form_state) {
  if ($form_state['values']['action'] == 'add') {
    $client_id = $form_state['values']['id'];
  	db_query("INSERT INTO adwords_daily_note (client_id, date, note) VALUES (%d, '%s', '%s')", 
	     $form_state['values']['id'], $form_state['values']['date'], $form_state['values']['note']);
  }
  else {
    $client_id = db_fetch_object(db_query("SELECT client_id FROM adwords_daily_note WHERE nid = %d", $form_state['values']['id']))->client_id;
    if ($form_state['values']['action'] == 'delete') {
      db_query("DELETE FROM adwords_daily_note WHERE nid = %d", $form_state['values']['id']);
    }
    else if ($form_state['values']['action'] == 'edit') {
      db_query("UPDATE adwords_daily_note SET note = '%s' WHERE nid = %d", $form_state['values']['note'], $form_state['values']['id']); 
    }
  }

  $form_state['redirect'] = 'adwords/daily/detail/' . $client_id;
}

function adwords_archived_data($form_state) {
  $output = '<div>Enter in selection criteria and press "Go!" to get archived data.</div>';
  if (!empty($form_state['storage']['start_date'])) {
    $output = adwords_daily_traffic($form_state['storage']['client'], $form_state['storage']['start_date'], $form_state['storage']['end_date']);
  }
  $query = db_query("
     SELECT td.tid, td.name
     FROM {term_data} td
     JOIN {vocabulary} v
     ON td.vid = v.vid
     AND v.name = 'Clients'");
  while($client = db_fetch_array($query)) {
    $clients[$client['tid']] = $client['name'];
  }
  $form['client'] = array(
     '#type' => 'select',
     '#title' => t('Client'),
     '#options' => $clients,
  );
  
  $form['start_date'] = array(
     '#type' => 'date_popup',
     '#title' => t('Start Date'),
     '#date_format' => DATE_FORMAT_DATE,
  );
  
  $form['end_date'] = array(
       '#type' => 'date_popup',
       '#title' => t('End Date'),
       '#date_format' => DATE_FORMAT_DATE,
    );
    
    $form['submit'] = array(
        '#type' => 'submit',
        '#value' => t('Go!'),
    );
    
    $form['data'] = array(
        '#value' => $output,
    );
  return $form;
}


function adwords_archived_data_submit($form, &$form_state) {
  // Save data in storage and let redirect to itself
  // print_r($form);
  $form_state['storage']['client'] = $form['client']['#value'];
    $form_state['storage']['start_date'] = $form['start_date']['#value']['date'];
  $form_state['storage']['end_date'] = $form['end_date']['#value']['date'];
    
}