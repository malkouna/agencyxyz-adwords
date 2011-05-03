<?php
function adwords_views_data() {
  $data['adwords_data']['table']['group'] = t('AdWords');

  $data['adwords_data']['table']['base'] = array(
    'field' => 'rid',
    'title' => t('AdWords Data'), 
    'help' => t('Represents imported data from Google AdWords'),
  );

  $data['adwords_data']['date'] = array(
    'title' => t('Date'),
    'help' => t('Date recorded for this data'),
    'field' => array(
      'click sortable' => TRUE,
    ),
  );

  $data['adwords_data']['acctname'] = array(
    'title' => t('Account Name'),
    'help' => t('Full name of Account in AdWords'),
    'field' => array(
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    )
  );  
//      'customerid' => array(
//      ),
//      'currCode' => array(
//      ),
  $data['adwords_data']['imps'] = array(
    'title' => t('Impressions'),
    'help' => t('Number of times an ad is displayed on Google'),
    'field' => array(
      'click sortable' => TRUE,
    ),
  );
//      'clicks' => array(
//        'title' => t('Clicks'),
//      ),
//      'ctr' => array(
//      ),
//      'cpc' => array(
//      ),
//      'cost' => array(
//      ),
//      'pos' => array(
//      ),
//      'conv' => array(
//      ),
//      'convRate' => array(
//      ),
//      'costPerConv' => array(
//      ),
//      'transactions' => array(
//      ),
//      'cpt' => array(
//      ),
//      'purchases' => array(
//      ),
//      'leadValue' => array(
//      ),
//      'pageviews' => array(
//      ),
//      'defaultConvValue'' => array(
//      ),
  
  return $data;
}