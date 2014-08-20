<?php

$fields = array(
  'Date' => array('dbcolumn' => 'date', 'xmlfield' => 'day'),
  'AccountDescriptiveName' => array('dbcolumn' => 'acctname', 'xmlfield' => 'account'),  
  'ExternalCustomerId' => array('dbcolumn' => 'customerid', 'xmlfield' => 'customerID'), 
  'AccountCurrencyCode' => array('dbcolumn' => 'currCode', 'xmlfield' => 'currency'),
  'Impressions' => array('dbcolumn' => 'imps', 'xmlfield' => 'impressions'), 
  'Clicks' => array('dbcolumn' => 'clicks', 'xmlfield' => 'clicks'), 
  'Ctr' => array('dbcolumn' => 'ctr', 'xmlfield' => 'ctr', 'format' => 'percentage'), 
  'AverageCpc' => array('dbcolumn' => 'cpc', 'xmlfield' => 'avgCPC'), 
  'Cost' => array('dbcolumn' => 'cost', 'xmlfield' => 'cost'), 
  'AveragePosition' => array('dbcolumn' => 'pos', 'xmlfield' => 'avgPosition'), 
  'Conversions' => array('dbcolumn' => 'conv', 'xmlfield' => 'convertedClicks'),
  'ConversionRate' => array('dbcolumn' => 'convRate', 'xmlfield' => 'clickConversionRate', 'format' => 'percentage'),
  'CostPerConversion' => array('dbcolumn' => 'costPerConv', 'xmlfield' => 'costConvertedClick'),
  /* 'SalesCount' => array('dbcolumn' => 'purchases', 'xmlfield' => 'salesConvManyPerClick'),
  'PageViewCount' => array('dbcolumn' => 'pageviews', 'xmlfield' => 'pageViewConvManyPerClick'),
  'LeadCount' => array('dbcolumn' => 'leads', 'xmlfield' => 'leadsConvManyPerClick'),
  'SignupCount' => array('dbcolumn' => 'signups', 'xmlfield' => 'signUpConvManyPerClick'),
  'OtherConversionCount' => array('dbcolumn' => 'defaultConv', 'xmlfield' => 'otherConvManyPerClick'), */
);

$segmentation_fields = array(
  'Date' => array('dbcolumn' => 'date', 'xmlfield' => 'day'),
  'AccountDescriptiveName' => array('dbcolumn' => 'acctname', 'xmlfield' => 'account'),  
  'ExternalCustomerId' => array('dbcolumn' => 'customerid', 'xmlfield' => 'customerID'), 
  'AccountCurrencyCode' => array('dbcolumn' => 'currCode', 'xmlfield' => 'currency'),
  /* Add the ConversionCategoryName field to get other metrics.
   * See http://code.google.com/apis/adwords/docs/reportguide.html#conversion_metrics
   */
  'ConversionCategoryName' => array(),
  'Conversions' => array(),
  'ConversionValue' => array(),
);

$segmentation_data = array(
  'View of a key page' => 'pageviews',
  'Lead' => 'leads',
  'Signup' => 'signups',
  'Purchase/Sale' => 'purchases',
  'Other' => 'defaultConv',
);

define('FIELDS', serialize($fields));
define('SEGMENTATION_FIELDS', serialize($segmentation_fields));
define('SEGMENTATION_DATA', serialize($segmentation_data));
define('ADWORDS_VERSION', 'v201406');

function _adwords_process($form_state) {
  $form['adwords']['description'] = array(
    '#value' => t('<div>Use the "Process" button below to login to adwords and populate the database.</div>'),
  );
  
  $form['adwords']['process'] = array(
    '#type' => 'submit',
    '#value' => t('Process'),
    '#submit' => array(_adwords_process_submit),
  );
  
  $form['adwords']['result'] = array(
    '#type' => 'markup',
    '#value' => $form_state['storage']['result'],
  );
  
  return $form;
}

function _adwords_process_submit(&$form, &$form_state) {
    $form_state['storage']['result'] = adwords_get_data();
}

function adwords_get_data() {
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
  $selector->fields = array('Login', 'CustomerId',  'Name');

  // Make the get request.
  $graph = $managedCustomerService->get($selector);
  if (isset($graph->entries)) {
    $count = count($graph->accounts);
    /* We have some accounts do report definition once */
    // Load the service, so that the required classes are available.
    $user->LoadService('ReportDefinitionService', ADWORDS_VERSION);
    
    // Create selector.
    $selector = new Selector();
    $segmentation_selector = new Selector();
    
    $fields = unserialize(FIELDS);
    $segmentation_fields = unserialize(SEGMENTATION_FIELDS);
    $segmentation_data = unserialize(SEGMENTATION_DATA);
    
    foreach($fields as $field) {
      $mappings[$field['xmlfield']] = $field['dbcolumn'];
      if(isset($field['format'])) {
        $formats[$field['xmlfield']] = $field['format'];
      }
    }
    $selector->fields = array_keys($fields);
    $segmentation_selector->fields = array_keys($segmentation_fields);

    
    // Create report definition.
    $reportDefinition = new ReportDefinition();
    $reportDefinition->selector = $selector;
    $reportDefinition->reportName = 'AdWords Automated Processing with Conversions';
    $reportDefinition->dateRangeType = 'LAST_30_DAYS';
    $reportDefinition->reportType = 'ACCOUNT_PERFORMANCE_REPORT';
    $reportDefinition->downloadFormat = 'XML';
  
    // Exclude criteria that haven't recieved any impressions over the date range.
    $reportDefinition->includeZeroImpressions = FALSE;
    
    // Create report definition.
    $segmentation_report_definition = new ReportDefinition();
    $segmentation_report_definition->selector = $segmentation_selector;
    $segmentation_report_definition->reportName = 'AdWords Automated Processing with Segmentation';
    $segmentation_report_definition->dateRangeType = 'LAST_30_DAYS';
    $segmentation_report_definition->reportType = 'ACCOUNT_PERFORMANCE_REPORT';
    $segmentation_report_definition->downloadFormat = 'XML';
  
    // Exclude criteria that haven't recieved any impressions over the date range.
    $segmentation_report_definition->includeZeroImpressions = FALSE;
        
    foreach ($graph->entries as $index => $account) {
  		/* For each of these do an ad hoc report, getting data in XML and saving to db */
      //$xml .= print_r($account, TRUE);
      /* Testing
      if ($account->customerId != 9658854421) {
            continue;
        }
      */
      // Set additional options.
      $options = array('version' => ADWORDS_VERSION);
    
      /* Set the client Id for this report */
      $user->SetClientCustomerId($account->customerId);
      /* Validate report before downloading */
      // Download report. Set second param $file to null to get xml directly
      $vars = array(
        '!index' => $index + 1,
        '!count' => $count + 1,
        '!account' => $account->customerId . ':' . $account->login . ':' . $account->companyName,
      );
      try {
        $xml = new SimpleXMLElement(ReportUtils::DownloadReport($reportDefinition, NULL, $user, $options));
        if(isset($xml->table->row)) {
          foreach($xml->table->row as $row) {
            $columns = null;
            $values = null;
            // insert each row
            foreach ($row->attributes() as $attribute => $value) {
              if(isset($mappings[$attribute])) {
                $columns[] = $mappings[$attribute];

                if (isset($formats[$attribute])) {
                  if ($formats[$attribute] == 'percentage') {
                    /* will get string '2.08%', want value 0.028 */
                    $value = str_replace('%', '', $value);
                    $value = $value / 100.0;
                  }
                }

                $values[$attribute] = "'" . (string)$value . "'";
              }
            }
            $column_statement = implode(',', $columns);
            $value_statement = implode(',', $values); 
            // Data comes with single quotes surrounding the values and db_query tries to escape them
            // so build up queries with concatenation rather than best practice of passing parameters.
            // Key for the data is the acctname and date so delete this row first
            db_query("
                DELETE FROM {adwords_data}
                WHERE date = " . $values['day'] . "
                AND customerid = " . $values['customerID']);
            // Now insert
            $insert = "INSERT INTO {adwords_data} (" . $column_statement . ") VALUES (" . $value_statement . ")";
            db_query($insert); 
            
          }
          /* Get segmenation data as well */
          $segmentation_xml = new SimpleXMLElement(ReportUtils::DownloadReport($segmentation_report_definition, NULL, $user, $options));
          /* Update values in adwords_data */
          if(isset($segmentation_xml->table->row)) {
            foreach($segmentation_xml->table->row as $conversion_row) {
              $column = (string)$conversion_row['conversionTrackingPurpose'];
              $sql = "UPDATE {adwords_data} SET " . $segmentation_data[(string)$conversion_row['conversionCategory']] . " = '%s' WHERE date = '%s' AND acctname = '%s'";
              /* Run query */
              db_query($sql, 
                (string)$conversion_row['convertedClicks'],
                (string)$conversion_row['day'],
                (string)$conversion_row['account']
              );
            }
          }
          watchdog('adwords', t('Report data updated: !indecFx of !count successful for account !account', $vars));
        }
      } catch (Exception $e) {
        $vars['!exception'] = $e->getMessage;
        watchdog('adwords', t('Report !index of !count for account !account generated exception !exception', $vars)); 
      }
    }
  }
  /* Do some admin on account names as they can be changed in the MCC interface */
  /* @AM 17/07/2012 Update the name for any data that has an old customer name */
  db_query("
  	UPDATE adwords_data d
    JOIN (
        SELECT d.acctname, d.customerid
        FROM adwords_data d
        JOIN (
            SELECT max(date) as date, customerid
            FROM adwords_data
            GROUP BY customerid
        ) d1
        ON d1.date = d.date
        AND d1.customerid = d.customerid
    ) d1
    ON d.customerid = d1.customerid
    SET d.acctname = d1.acctname");
	/*
   * @AM 6/1/2011 Get the unique list of clients and generate taxonomy terms if needed
   */
  db_query("
    INSERT INTO {term_data}
    SELECT DISTINCT null as tid, 1 as vid, acctname as name, customerid as description, 0 as weight
    FROM adwords_data  ad
    LEFT OUTER JOIN {term_data} td
    ON ad.customerid = td.description
    AND ad.acctname = td.name
    WHERE td.name IS NULL");
  
  db_query("
    INSERT INTO {term_hierarchy}
    SELECT DISTINCT td.tid, 0 as parent
    FROM {term_data}  td
    LEFT OUTER JOIN {term_hierarchy} th
    ON td.tid = th.tid
    WHERE th.tid IS NULL");
  
  /* @AM 17/07/2012 Delete unused terms */

  
}