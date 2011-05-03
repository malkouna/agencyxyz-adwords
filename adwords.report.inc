<?php
function _adwords_list_reports($form_state) {
  require_once 'src/Google/Api/Ads/AdWords/Lib/AdWordsUser.php';
  ini_set('arg_separator.output', '&');
  // If I don't have a google login then redirect to login
  if (empty($_SESSION['adworduser'])) {
    $form['#redirect'] = 'adwords/login';
    return $form;
  }
  
  // TODO work out if I can save the user in the session $adworduser = $_SESSION['adworduser'];
  try {
    // TODO can this be saved or remove the need for it?
    $adworduser = new AdWordsUser();

    // Log SOAP XML request and response.
    $adworduser->LogDefaults();
    
    // Get the GetReportDefinitionService.
    $reportDefinitionService = $adworduser->GetReportDefinitionService('v201003');
  
    // Create selector.
    $selector = new ReportDefinitionSelector();
  
    // Get all report definitions.
    $page = $reportDefinitionService->get($selector);
    $form['reports'] = array(
      '#type' => 'fieldset',
      '#title' => t('Reports'),
    );  
    // Display report definitions.
    if (isset($page->entries)) {
//      $from['reports']['list'] = array(
//        '#prefix' => '<table>',
//        '#suffix' => '</table>',
//      );
      foreach ($page->entries as $key => $reportDefinition) {
        $form['reports'][$key]['message'] = array(
          '#value' => t('ReportDefinition with name @name and id @id was found',
            array('@name' => $reportDefinition->reportName, '@id' => $reportDefinition->id)),
        );
        $form['reports'][$key]['view'] = array(
          '#value' => l(t('View'), 'adwords/downloadreport'),
        );
      }
    } 
    else {
      $form['reports']['message'] = array(
        '#value' => t('No report definitions were found.'),
      ); 
    }
    $form['add'] = array(
      '#value' => t('Add Report'),
      '#type' => 'submit',
    );
  } 
  catch (Exception $e) {
    drupal_set_message($e);
  }
  return $form;
}

function _adwords_download_report($form_state) {
  require_once 'src/Google/Api/Ads/AdWords/Lib/AdWordsUser.php';
  require_once 'src/Google/Api/Ads/AdWords/Util/ReportUtils.php';
  ini_set('arg_separator.output', '&');
  // If I don't have a google login then redirect to login
  if (empty($_SESSION['adworduser'])) {
    $form['#redirect'] = 'adwords/login';
    return $form;
  }
  try {
    // Get AdWordsUser from credentials in "../auth.ini"
    // relative to the AdWordsUser.php file's directory.
    $adworduser = new AdWordsUser();
  
    // Log SOAP XML request and response.
    $adworduser->LogDefaults();
  
    $reportDefintionId = (float) '1035';
    $fileName = 'report.xml';
  
    $path = dirname(__FILE__) . '/' . $fileName;
  
    // Download report.
    ReportUtils::DownloadReport($reportDefintionId, $path, $adworduser);
  
//    printf("Report with definition id '%s' was downloaded to '%s'.\n",
//        $reportDefintionId, $fileName);
  } catch (Exception $e) {
    drupal_set_message($e);
}    
}

function _adwords_list_reports_submit(&$form, &$form_state) {
  require_once 'src/Google/Api/Ads/AdWords/Lib/AdWordsUser.php';
  ini_set('arg_separator.output', '&');
  // If I don't have a google login then redirect to login
  if (empty($_SESSION['adworduser'])) {
    $form['#redirect'] = 'adwords/login';
    return $form;
  }
  try {
    // Get AdWordsUser from credentials in "../auth.ini"
    // relative to the AdWordsUser.php file's directory.
    $user = new AdWordsUser();
  
    // Log SOAP XML request and response.
    $user->LogDefaults();
  
    // Get the GetReportDefinitionService.
    $reportDefinitionService = $user->GetReportDefinitionService('v201003');
  
    $adGroupId = (float) '3000012850';
    $startDate = 'INSERT_START_DATE_HERE';
    $endDate = 'INSERT_END_DATE_HERE';
  
    // Create ad group predicate.
    $adGroupPredicate = new Predicate();
    $adGroupPredicate->field = 'AdGroupId';
    $adGroupPredicate->operator = 'EQUALS';
    $adGroupPredicate->values = array($adGroupId);
  
    // Create selector.
    $selector = new Selector();
    $selector->fields = array('AdGroupId', 'Id', 'KeywordText',
        'KeywordMatchType', 'Impressions', 'Clicks', 'Cost');
    $selector->predicates = array($adGroupPredicate);
    // $selector->dateRange = new DateRange($startDate, $endDate);
  
    // Create report definition.
    $reportDefinition = new ReportDefinition();
    $reportDefinition->reportName = 'Keywords performance report #' . time();
    $reportDefinition->dateRangeType = 'CUSTOM_DATE';
    $reportDefinition->reportType = 'KEYWORDS_PERFORMANCE_REPORT';
    $reportDefinition->downloadFormat = 'XML';
    $reportDefinition->selector = $selector;
  
    // Create operations.
    $operation = new ReportDefinitionOperation();
    $operation->operand = $reportDefinition;
    $operation->operator = 'ADD';
  
    $operations = array($operation);
  
    // Add report definition.
    $result = $reportDefinitionService->mutate($operations);
  } 
  catch (Exception $e) {
    print_r($e);
  }
}

function _adwords_report_form(&$form_state) {
  $form['filter'] = array(
    '#type' => 'fieldset',
    '#title' => t('Report Filters'),
  );
  
  $query = db_query("
    SELECT td.tid, td.name
    FROM {term_data} td
    JOIN {vocabulary} v
    ON td.vid = v.vid
    WHERE v.name = 'Clients'");
  while($client = db_fetch_object($query)) {
    $options[$client->tid] = $client->name;
  }
  $form['filter']['client'] = array(
    '#type' => 'select',
    '#title' => t('Client'),
    '#options' => $options,
  );
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Apply Filters'),
  );
  
  // If I have a client selected then show results for that else show all
  $form['results'] = array(
    '#type' => 'fieldset'
  );
  
  if (!empty($form_state['storage']['client'])) {
    $results = array();
    
    $query = db_query("
      SELECT ad.*
      FROM adwords_data ad
      JOIN {term_data} td
      ON ad.acctname = td.name
      WHERE td.tid = %d
    ", $form_state['storage']['client']);
    
    while($row = db_fetch_object($query)) {
      foreach($row as $column => $value) {
        $result[$column] = $value;
      }
      $results[] = $result;  
    }
    // Now theme the results
    $results = theme('adwords_results', $results);
    // $results = $form_state['storage']['client'];
  }
  else {
    $results = t('Select a client to filter AdWords data.');
  }
  
  $form['results']['data'] = array(
    '#value' => $results,
  );
  return $form;
}

function adwords_report_form_submit($form, &$form_state) {
  // print_r($form);
  $form_state['storage']['client'] = $form['filter']['client']['#value'];
}
