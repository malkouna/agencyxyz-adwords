<?php
function _adwords_process($form_state) {
  $form['adwords']['description'] = array(
    '#value' => t('<div>Use the "Process" button below to retrieve email reports and populate the database.</div>'),
  );
  
  $form['adwords']['login'] = array(
    '#type' => 'submit',
    '#value' => t('Process'),
    '#submit' => array(_adwords_process_submit),
  );
  return $form;
}

function _adwords_process_submit(&$form, &$form_state) {
    adwords_process_emails();
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
        drupal_set_message(t("File @file processed", array('@file', $part->parameters[0]->value)));
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