<?php 
/**
 * @file adwords-notes.tpl.php
 */
drupal_add_js('misc/collapse.js');
?>
<fieldset class="collapsible collapsed">
    <legend>Notes</legend>
	<table>
	  <thead>
	    <tr>
	        <th>Date</th>
          <th>Created</th>
          <th>Modified</th>
	        <th>Note</th>
          <th>Edit</th>
	    </tr>
	  </thead>
	  <?php foreach ($notes as $note): ?>
	    <tr>
	        <td><?php print $note['date']; ?></td>
          <td><?php print $note['created']; ?></td>
          <td><?php print $note['changed']; ?></td>
	        <td><?php print $note['note']; ?></td>
          <td><?php print l?></td>
	    </tr>
	  <?php endforeach; ?>
	</table>
</fieldset>