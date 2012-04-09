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
  'Conversions' => array('dbcolumn' => 'conv', 'xmlfield' => 'conv1PerClick'), 
  'ConversionRate' => array('dbcolumn' => 'convRate', 'xmlfield' => 'convRate1PerClick', 'format' => 'percentage'),
  'CostPerConversion' => array('dbcolumn' => 'costPerConv', 'xmlfield' => 'costConv1PerClick'),
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

function _adwords_process($form_state) {
  $form['adwords']['description'] = array(
    '#value' => t('<div>Use the "Process" button below to retrieve email reports and populate the database.</div>'),
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
	$user = adwords_login();
	if (!$user) {
		watchdog('adwords', t('Cannot log into AdWords server'), NULL, WATCHDOG_ERROR);
		return;
	}
  /* Get all the accounts defined in MCC */
  // Get the service, which loads the required classes.
  $servicedAccountService =
      $user->GetService('ServicedAccountService', 'v201109');

  // Create selector.
  $selector = new ServicedAccountSelector();
  // To get the links paging must be disabled.
  $selector->enablePaging = FALSE;

  // Make the get request.
  $graph = $servicedAccountService->get($selector);
  if (isset($graph->accounts)) {
    $count = count($graph->accounts);
    /* We have some accounts do report definition once */
    // Load the service, so that the required classes are available.
    $user->LoadService('ReportDefinitionService', 'v201109');
    
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
    $reportDefinition->dateRangeType = 'LAST_7_DAYS';
    $reportDefinition->reportType = 'ACCOUNT_PERFORMANCE_REPORT';
    $reportDefinition->downloadFormat = 'XML';
  
    // Exclude criteria that haven't recieved any impressions over the date range.
    $reportDefinition->includeZeroImpressions = FALSE;
    
    // Create report definition.
    $segmentation_report_definition = new ReportDefinition();
    $segmentation_report_definition->selector = $segmentation_selector;
    $segmentation_report_definition->reportName = 'AdWords Automated Processing with Segmentation';
    $segmentation_report_definition->dateRangeType = 'LAST_7_DAYS';
    $segmentation_report_definition->reportType = 'ACCOUNT_PERFORMANCE_REPORT';
    $segmentation_report_definition->downloadFormat = 'XML';
  
    // Exclude criteria that haven't recieved any impressions over the date range.
    $segmentation_report_definition->includeZeroImpressions = FALSE;
        
    foreach ($graph->accounts as $index => $account) {
  		/* For each of these do an ad hoc report, getting data in XML and saving to db */
      //$xml .= print_r($account, TRUE);

      // Set additional options.
      $options = array('version' => 'v201109', 'returnMoneyInMicros' => TRUE);
    
      /* Set the client Id for this report */
      $user->SetClientId($account->customerId);
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
              $columns[] = $mappings[$attribute];
              if(isset($formats[$attribute])) {
                if ($formats[$attribute] == 'percentage') {
                  /* will get string '2.08%', want value 0.028 */
                  $value = str_replace('%', '', $value);
                  $value = $value/100.0;
                }
              }
              $values[$attribute] = "'" . (string)$value . "'";
            }
            $column_statement = implode(',', $columns);
            $value_statement = implode(',', $values); 
            // Data comes with single quotes surrounding the values and db_query tries to escape them
            // so build up queries with concatenation rather than best practice of passing parameters.
            // Key for the data is the acctname and date so delete this row first
            db_query("DELETE FROM {adwords_data} WHERE date = " . $values['day'] . " AND acctname = " . $values['account']);
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
              $sql = "UPDATE {adwords_data} SET " . $segmentation_data[(string)$conversion_row['conversionTrackingPurpose']] . " = '%s' WHERE date = '%s' AND acctname = '%s'";
              /* Run query */
              db_query($sql, 
                (string)$conversion_row['conv1PerClick'],
                (string)$conversion_row['day'],
                (string)$conversion_row['account']
              );
            }
          }
          watchdog('adwords', t('Report data updated: !index of !count successful for account !account', $vars));
        }
      } catch (Exception $e) {
        $vars['!exception'] = $e->getMessage;
        watchdog('adwords', t('Report !index of !count for account !account generated exception !exception', $vars)); 
      }
    }
  }
	/*
   * @AM 6/1/2011 Get the unique list of clients and generate taxonomy terms if needed
   */
  db_query("
    INSERT INTO {term_data}
    SELECT DISTINCT null as tid, 1 as vid, acctname as name, '' as description, 0 as weight
    FROM adwords_data  ad
    LEFT OUTER JOIN {term_data} td
    ON ad.acctname = td.name
    WHERE td.name IS NULL");
  
  db_query("
    INSERT INTO {term_hierarchy}
    SELECT DISTINCT td.tid, 0 as parent
    FROM {term_data}  td
    LEFT OUTER JOIN {term_hierarchy} th
    ON td.tid = th.tid
    WHERE th.tid IS NULL");
}

function adwords_login($client_login = null) {
  $path = dirname(__FILE__) . '/../../libraries/adwords/v201109/src';
  set_include_path(get_include_path() . PATH_SEPARATOR . $path);

  require_once 'Google/Api/Ads/AdWords/Lib/AdWordsUser.php';
  require_once 'Google/Api/Ads/Common/Util/MapUtils.php';

  $ini_file = variable_get('adwords_sandbox', 1) == 1?'settings-sandbox.ini':'settings.ini';
  $ini_file = dirname(__FILE__) . '/' . $ini_file;
  $developer_token = variable_get('adwords_sandbox', 1) == 1?variable_get('adwords_email_address', '') . '++AUD':'5AYQ2vWZgg7pKsE8RjB73g';
  try {
    // Get AdWordsUser from credentials in "../auth.ini"
    // relative to the AdWordsUser.php file's directory.
    $user = new AdWordsUser(
      null,
      variable_get('adwords_email_address', ''),
      variable_get('adwords_password', ''),
      $developer_token,
      null,
      null,
      $client_login,
      $ini_file,
      null,
      null,
      null
    );

    // Log SOAP XML request and response.
    $user->LogDefaults();

  }
  catch (Exception $e) {
    watchdog('adwords', 'Get user exception %faultstring', array('%faultstring' => $e->faultstring), WATCHDOG_ERROR);
    return null;
  }

  return $user;
}
function adwords_process_emails() {
  watchdog('adwords', t('Processing emails'), null, WATCHDOG_INFO);
  // Open mailbox
  $mbox = imap_open(
  	variable_get('adwords_email_server', '{codeontheroad.com.au:110/pop3}INBOX'),
  	variable_get('adwords_email_address', ''),
  	variable_get('adwords_email_password', ''));
  $count = imap_num_msg($mbox);
  for($i = 1; $i <= $count; $i++) {
    $structure = imap_fetchstructure($mbox, $i);
    foreach($structure->parts as $section => $part) {
      if ($part->disposition == 'ATTACHMENT') {
        $partbody = imap_fetchbody($mbox, $i, $section + 1);
        // $filename = 'sites/default/files/' . $part->parameters[0]->value;
        file_put_contents('sites/default/files/report-xml.zip', base64_decode($partbody));
        // Open the attachment (zip file)
        $zip = zip_open('sites/default/files/report-xml.zip');
        if (is_resource($zip)) {
          while ($zip_entry = zip_read($zip)) {
            $zip_entry_name = zip_entry_name($zip_entry);
            if ($zip_entry_name == "report.xml") {
              $zip_file_content = zip_entry_read($zip_entry, zip_entry_filesize($zip_entry));
              zip_entry_close($zip_entry); 
              break;
            }
          }
        }
        
        // Have the content of the zipped file (xml) here
        // $xml = simplexml_load_string($zip_file_content);
        $xml = new SimpleXMLElement(utf8_decode($zip_file_content));
        // Write to database
        foreach ($xml->table->rows->row as $row) {
          $columns = null;
          $values = null;
          // insert each row
          foreach ($row->attributes() as $attribute => $value) {
            $columns[] = $attribute;
            $values[$attribute] = "'" . (string)$value . "'";
          }
          $column_statement = implode(',', $columns);
          $value_statement = implode(',', $values); 
          // Data comes with single quotes surrounding the values and db_query tries to escape them
          // so build up queries with concatenation rather than best practice of passing parameters.
          // Key for the data is the acctname and date so delete this row first
          db_query("DELETE FROM {adwords_data} WHERE date = " . $values['date'] . " AND acctname = " . $values['acctname']);
          // Now insert
          $insert = "INSERT INTO {adwords_data} (" . $column_statement . ") VALUES (" . $value_statement . ")";
          db_query($insert);
        }
        // var_dump($xml);
        drupal_set_message(t("File @file processed", array('@file' => $part->parameters[0]->value)));
      }
    }
  }
  imap_close($mbox);
  
  /*
   * @AM 6/1/2011 Get the unique list of clients and generate taxonomy terms if needed
   */
  db_query("
    INSERT INTO {term_data}
    SELECT DISTINCT null as tid, 1 as vid, acctname as name, '' as description, 0 as weight
    FROM adwords_data  ad
    LEFT OUTER JOIN {term_data} td
    ON ad.acctname = td.name
    WHERE td.name IS NULL");
  
  db_query("
    INSERT INTO {term_hierarchy}
    SELECT DISTINCT td.tid, 0 as parent
    FROM {term_data}  td
    LEFT OUTER JOIN {term_hierarchy} th
    ON td.tid = th.tid
    WHERE th.tid IS NULL");
}