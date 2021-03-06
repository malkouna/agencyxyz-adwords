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
	        <th>Note</th>
	        <?php if (user_access('administer adwords')): ?>
          	<th></th>
        	<?php endif; ?>
	    </tr>
	  </thead>
	  <?php if(empty($notes)): ?>
	  	<tr><td colspan="3">There are no notes for this month.</td></tr>
	  <?php else: ?>
		  <?php foreach ($notes as $note): ?>
		    <tr class="<?php print $note['zebra']; ?>">
	  	      <td class="daily-narrow"><?php print $note['date']; ?></td>
	    	    <td><?php print $note['note']; ?></td>
	    	    <?php if (user_access('administer adwords')): ?>
          	  <td class="daily-narrow">
            	  <?php print l('Edit', 'adwords/daily/note/edit/' . $note['nid']) . ' | ' .
                   l('Delete', 'adwords/daily/note/delete/' . $note['nid']); ?>
            	</td>
          	<?php endif; ?>
	    	</tr>
	    <?php endforeach; ?>
    <?php endif; ?>
	</table>
</fieldset>