<?php 
/**
 * @file adwords-results.tpl.php
 */
?>
<div class="data">
<table class="data">
  <thead>
    <tr>
      <?php foreach ($headers as $header): ?>
        <th><?php print $header; ?></th>
      <?php endforeach;?>
    </tr>
  </thead>
  <tbody>
    <?php foreach ($results as $result): ?>
      <tr>
        <?php foreach ($result as $value): ?>
          <td><?php print $value; ?></td>
        <?php endforeach; ?>
      </tr>
    <?php endforeach; ?>
  </tbody>
  <tfoot>
    <tr>
        <?php foreach ($totals as $total): ?>
            <td><?php print $total; ?></td>
        <?php endforeach; ?>
    </tr>
  </tfoot>
</table>
</div>