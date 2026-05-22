<?php
/**
 * @file
 * Template for AI SEO Advisor report display.
 *
 * Available variables:
 * - $report_html: The formatted HTML report content with accordions.
 * - $has_error: Boolean indicating if there was an error.
 * - $error_message: Error message if any.
 */
?>
<?php if ($has_error): ?>
  <div class="messages error"><?php print $error_message; ?></div>
<?php else: ?>
  <div class="ai-seo-ai-report">
    <?php print $report_html; ?>
  </div>
<?php endif; ?>
