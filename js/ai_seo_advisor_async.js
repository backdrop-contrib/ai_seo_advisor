/**
 * @file
 * Polls ai_async job status for AI SEO Advisor reports and renders the report
 * when the background worker completes.
 */

(function ($) {
  'use strict';

  Backdrop.behaviors.aiSeoAdvisorAsync = {
    attach: function (context, settings) {
      $('[data-ai-async-job-id]', context).once('ai-seo-async', function () {
        var $container = $(this);
        var jobId = $container.attr('data-ai-async-job-id');
        var reportId = $container.attr('data-ai-async-report-id');
        var nid = $container.attr('data-ai-async-nid');

        if (!jobId || typeof AIAsync === 'undefined' || typeof AIAsync.poll !== 'function') {
          return;
        }

        AIAsync.poll(jobId, {
          interval: 2000,
          maxWait: 180000,

          onComplete: function (result) {
            if (result && result.rendered_html) {
              $container.html(result.rendered_html);
              Backdrop.attachBehaviors($container);
            }
            else if (result && result.view_url) {
              window.location.href = result.view_url;
            }
            else {
              window.location.reload();
            }
          },

          onError: function (message) {
            var $msg = $('<div class="messages error">' + Backdrop.checkPlain(message) + '</div>');
            $container.html($msg);
          },

          onTimeout: function () {
            var $msg = $('<div class="messages warning">' +
              Backdrop.t('AI report generation is taking longer than usual. The report will finish in the background; you can check the Report History below shortly.') +
              '</div>');
            $container.html($msg);
          }
        });
      });
    }
  };

})(jQuery);
