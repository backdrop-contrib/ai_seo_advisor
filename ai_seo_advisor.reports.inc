<?php
/**
 * @file
 * Report generation and analysis functions for AI SEO Advisor module.
 */

function ai_seo_advisor_render_results(array $result) {
  $build = [];

  if (!empty($result['error'])) {
    $build['error'] = [
      '#theme' => 'ai_seo_advisor_report',
      '#report_html' => '',
      '#has_error' => TRUE,
      '#error_message' => check_plain($result['error']),
      '#attached' => [
        'css' => [
          backdrop_get_path('module', 'ai_seo_advisor') . '/css/ai_seo_advisor.css',
        ],
      ],
    ];
    return $build;
  }

  // Display AI-generated HTML report.
  if (!empty($result['ai']) && is_array($result['ai']) && !empty($result['ai']['html_report'])) {
    $report_html = ai_seo_advisor_normalize_ai_report_html($result['ai']['html_report']);

    // The AI returns fully formatted HTML. We need to wrap numbered H2 sections in accordions.
    // Strategy: Split by H2 tags that start with numbers (e.g., "1. Content Depth...")

    $formatted = '';

    // First, replace any H1 tags with H2 (H1 is reserved for page title)
    $report_html = preg_replace('/<h1([^>]*)>/i', '<h2$1>', $report_html);
    $report_html = preg_replace('/<\/h1>/i', '</h2>', $report_html);
    // Split content by H2 tags that start with numbers (the main sections)
    $parts = preg_split('/(<h2[^>]*>\s*\d+\..*?<\/h2>)/s', $report_html, -1, PREG_SPLIT_DELIM_CAPTURE);

    $in_section = FALSE;
    $section_content = '';

    foreach ($parts as $i => $part) {
      $part = trim($part);
      if (empty($part)) continue;

      // Check if this is a numbered H2 heading
      if (preg_match('/^<h2[^>]*>(\s*\d+\..*?)<\/h2>$/s', $part, $matches)) {
        // Close previous section if open
        if ($in_section && !empty($section_content)) {
          $formatted .= '</div></details>' . "\n";
        }

        // Start new accordion section (all closed by default)
        $formatted .= '<details class="seo-section-accordion">' . "\n";
        $formatted .= '<summary class="seo-section-summary">' . $matches[1] . '</summary>' . "\n";
        $formatted .= '<div class="seo-section-content">' . "\n";

        $in_section = TRUE;
        $section_content = '';
      }
      else {
        // Add to current section or to formatted output if no section
        if ($in_section) {
          $section_content .= $part . "\n";
          $formatted .= $part . "\n";
        }
        else {
          $formatted .= $part . "\n";
        }
      }
    }

    // Close final section if still open
    if ($in_section) {
      $formatted .= '</div></details>' . "\n";
    }

    // The report originates from an LLM (whose input includes site content),
    // and the template prints it unescaped — sanitize before display so a
    // prompt-injected <script> or event handler can never reach the admin.
    $formatted = filter_xss($formatted, [
      'a', 'abbr', 'b', 'blockquote', 'br', 'code', 'dd', 'details', 'div',
      'dl', 'dt', 'em', 'h2', 'h3', 'h4', 'h5', 'h6', 'hr', 'i', 'li', 'ol',
      'p', 'pre', 'section', 'span', 'strong', 'summary', 'table', 'tbody',
      'td', 'th', 'thead', 'tr', 'ul',
    ]);

    // Use theme function for output
    $build['ai_report'] = [
      '#theme' => 'ai_seo_advisor_report',
      '#report_html' => $formatted,
      '#has_error' => FALSE,
      '#error_message' => '',
      '#attached' => [
        'css' => [
          backdrop_get_path('module', 'ai_seo_advisor') . '/css/ai_seo_advisor.css',
        ],
      ],
    ];
  }
  else {
    $build['no_report'] = [
      '#theme' => 'ai_seo_advisor_report',
      '#report_html' => '',
      '#has_error' => TRUE,
      '#error_message' => t('No AI report was generated. Please try running the analysis again.'),
      '#attached' => [
        'css' => [
          backdrop_get_path('module', 'ai_seo_advisor') . '/css/ai_seo_advisor.css',
        ],
      ],
    ];
  }

  return $build;
}

/**
 * Normalize AI report output to safe, readable HTML.
 *
 * Providers/models sometimes return Markdown-like text even when asked for HTML.
 * This converts common Markdown patterns into simple HTML so reports render well.
 *
 * @param string $report
 *   Raw AI report.
 *
 * @return string
 *   HTML-safe report.
 */
function ai_seo_advisor_normalize_ai_report_html($report) {
  $report = trim((string) $report);
  if ($report === '') {
    return '';
  }

  // Remove empty code fences and normalize line endings.
  $report = str_replace(["\r\n", "\r"], "\n", $report);
  $report = preg_replace('/```[a-zA-Z0-9_-]*\s*```/s', '', $report);
  $report = preg_replace('/^```[a-zA-Z0-9_-]*\s*$/m', '', $report);
  $report = preg_replace('/^\s*```\s*$/m', '', $report);

  $has_html = (bool) preg_match('/<(h[1-6]|p|ul|ol|li|details|summary|pre|table|div|section|article|br)\b/i', $report);
  $has_markdown = (strpos($report, '**') !== FALSE)
    || (bool) preg_match('/(?:^|\n)#{1,6}\s+\S/', $report)
    || (bool) preg_match('/(?:^|\n)[*+-]\s+\S/', $report)
    || (bool) preg_match('/(?:^|\n)\d+[.)]\s+\S/', $report);
  // If it's already HTML and doesn't look markdown-ish, keep it.
  if ($has_html && !$has_markdown) {
    return trim($report);
  }

  // Mixed HTML + markdown: convert to plain text first, then parse markdown-ish
  // structures into clean HTML below.
  if ($has_html && $has_markdown) {
    $report = preg_replace('#<br\s*/?>#i', "\n", $report);
    $report = strip_tags($report);
    $report = decode_entities($report);
    $report = preg_replace('/\s+/', ' ', $report);
    $report = trim($report);
  }

  // Heuristic: break dense markdown-ish single-line output into lines.
  if (substr_count($report, "\n") < 3 && substr_count($report, '**') > 4) {
    $report = preg_replace('/\s+(?=\*\*[^*]{2,80}\*\*)/', "\n", $report);
    $report = preg_replace('/\s+(?=\d+[.)]\s+)/', "\n", $report);
    $report = preg_replace('/\s+(?=\*\s+)/', "\n", $report);
    $report = preg_replace('/(?<=[.!?:])\s+(?=[-+]\s+)/', "\n", $report);
  }

  // Additional splitting for compact outputs that mix headings/lists inline.
  $report = preg_replace('/\.\s+(?=\*\*[^*]{2,120}\*\*:)/', ".\n", $report);
  $report = preg_replace('/\s+(?=\*\*[^*]{2,120}\*\*:)/', "\n", $report);
  $report = preg_replace('/\s+(?=\d+[.)]\s+)/', "\n", $report);
  $report = preg_replace('/\s+(?=[*+-]\s+)/', "\n", $report);

  // Normalize "1)" list markers to "1." for simpler parsing.
  $report = preg_replace('/^(\d+)\)\s+/m', '$1. ', $report);
  $report = preg_replace('/\n(\d+)\)\s+/', "\n$1. ", $report);

  // Ensure obvious section separators become newlines.
  if (substr_count($report, "\n") < 2) {
    $report = str_replace(' **', "\n**", $report);
    $report = str_replace(' 1. ', "\n1. ", $report);
    $report = str_replace(' 2. ', "\n2. ", $report);
    $report = str_replace(' 3. ', "\n3. ", $report);
    $report = str_replace(' 4. ', "\n4. ", $report);
    $report = str_replace(' 5. ', "\n5. ", $report);
  }

  $lines = preg_split('/\n+/', $report);
  $out = '';
  $para = [];
  $list_type = '';
  $in_code = FALSE;
  $code_lines = [];

  $inline = function($text) {
    $text = check_plain(trim((string) $text));
    $text = preg_replace('/\*\*(.+?)\*\*/', '<strong>$1</strong>', $text);
    $text = preg_replace('/\*(.+?)\*/', '<em>$1</em>', $text);
    $text = preg_replace('/`(.+?)`/', '<code>$1</code>', $text);
    return $text;
  };

  $flush_para = function() use (&$out, &$para, $inline) {
    if (!empty($para)) {
      $out .= '<p>' . $inline(implode(' ', $para)) . '</p>' . "\n";
      $para = [];
    }
  };

  $flush_list = function() use (&$out, &$list_type) {
    if ($list_type !== '') {
      $out .= '</' . $list_type . '>' . "\n";
      $list_type = '';
    }
  };

  foreach ($lines as $line_raw) {
    $line = trim($line_raw);
    if ($line === '') {
      $flush_para();
      $flush_list();
      continue;
    }

    if (preg_match('/^```/', $line)) {
      $flush_para();
      $flush_list();
      $in_code = !$in_code;
      continue;
    }
    if ($in_code) {
      $code_lines[] = $line_raw;
      continue;
    }

    if (preg_match('/^###\s+(.+)$/', $line, $m)) {
      $flush_para();
      $flush_list();
      $out .= '<h4>' . $inline($m[1]) . '</h4>' . "\n";
      continue;
    }
    if (preg_match('/^##\s+(.+)$/', $line, $m)) {
      $flush_para();
      $flush_list();
      $out .= '<h3>' . $inline($m[1]) . '</h3>' . "\n";
      continue;
    }
    if (preg_match('/^#\s+(.+)$/', $line, $m)) {
      $flush_para();
      $flush_list();
      $out .= '<h2>' . $inline($m[1]) . '</h2>' . "\n";
      continue;
    }

    if (preg_match('/^[-*+]\s+(.+)$/', $line, $m)) {
      $flush_para();
      if ($list_type !== 'ul') {
        $flush_list();
        $list_type = 'ul';
        $out .= '<ul>' . "\n";
      }
      $out .= '<li>' . $inline($m[1]) . '</li>' . "\n";
      continue;
    }

    if (preg_match('/^\d+\.\s+(.+)$/', $line, $m)) {
      $flush_para();
      if ($list_type !== 'ol') {
        $flush_list();
        $list_type = 'ol';
        $out .= '<ol>' . "\n";
      }
      $out .= '<li>' . $inline($m[1]) . '</li>' . "\n";
      continue;
    }

    // Treat "**Heading:** trailing text" as heading + paragraph.
    if (preg_match('/^\*\*([^*]{2,120})\*\*:\s*(.+)$/', $line, $m)) {
      $flush_para();
      $flush_list();
      $out .= '<h3>' . $inline($m[1]) . '</h3>' . "\n";
      $out .= '<p>' . $inline($m[2]) . '</p>' . "\n";
      continue;
    }

    // Treat bold labels as subheadings when they look like section headings.
    if (preg_match('/^\*\*([^*]{2,120})\*\*:?\s*$/', $line, $m)) {
      $flush_para();
      $flush_list();
      $out .= '<h3>' . $inline($m[1]) . '</h3>' . "\n";
      continue;
    }

    $para[] = $line;
  }

  $flush_para();
  $flush_list();

  if (!empty($code_lines)) {
    $out .= '<pre><code>' . check_plain(implode("\n", $code_lines)) . '</code></pre>' . "\n";
  }

  return trim($out);
}

/**
 * Convert mixed value to plain text for heuristics.
 */
function ai_seo_advisor_to_plain($mixed) {
  if (is_string($mixed)) {
    return trim(strip_tags(decode_entities($mixed)));
  }
  if (is_array($mixed)) {
    $out = '';
    foreach ($mixed as $v) {
      $out .= ' ' . ai_seo_advisor_to_plain($v);
    }
    return trim(preg_replace('/\s+/', ' ', $out));
  }
  if (is_object($mixed)) {
    return ai_seo_advisor_to_plain((array) $mixed);
  }
  return '';
}

/**
 * Analyze content and compute score.
 */
function ai_seo_advisor_analyze($title, $html_or_text, $meta_description, $focus_keyword) {
  $th = config_get('ai_seo_advisor.settings', 'thresholds'); $th = is_array($th) ? $th : [];
  $ck = config_get('ai_seo_advisor.settings', 'checks_enabled'); $ck = is_array($ck) ? array_filter($ck) : [];
  $weights = config_get('ai_seo_advisor.settings', 'weights'); $weights = is_array($weights) ? $weights : [];

  $checks = []; $score = 0; $total_weight = 0;
  $add_check = function($key, $pass, $message) use (&$checks, &$weights, &$score, &$total_weight) {
    $w = isset($weights[$key]) ? (int) $weights[$key] : 0;
    $checks[$key] = ['pass' => (bool) $pass, 'message' => $message, 'weight' => $w];
    $total_weight += $w; if ($pass) { $score += $w; }
  };

  $title = trim((string) $title);
  $desc = trim((string) $meta_description);
  $text_plain = ai_seo_advisor_to_plain($html_or_text);

  // Title length.
  if (!empty($ck['title_length'])) {
    $len = backdrop_strlen($title);
    $min = isset($th['title_min']) ? (int) $th['title_min'] : 10;
    $max = isset($th['title_max']) ? (int) $th['title_max'] : 60;
    $pass = ($len >= $min && $len <= $max);
    $add_check('title_length', $pass, t('Title length: @len chars (recommended @min–@max).', ['@len' => $len, '@min' => $min, '@max' => $max]));
  }

  // Meta description.
  if (!empty($ck['description_length'])) {
    $dmin = isset($th['description_min']) ? (int) $th['description_min'] : 140;
    $dmax = isset($th['description_max']) ? (int) $th['description_max'] : 160;
    $dlen = backdrop_strlen($desc);
    $pass = ($dlen >= $dmin && $dlen <= $dmax);
    $add_check('description_length', $pass, t('Meta description length: @len chars (recommended @min–@max).', ['@len' => $dlen, '@min' => $dmin, '@max' => $dmax]));
  }

  // Word count.
  if (!empty($ck['word_count'])) {
    $words = preg_split('/\s+/u', $text_plain, -1, PREG_SPLIT_NO_EMPTY);
    $count = is_array($words) ? count($words) : 0;
    $min_wc = isset($th['min_word_count']) ? (int) $th['min_word_count'] : 300;
    $pass = $count >= $min_wc;
    $add_check('word_count', $pass, t('Word count: @count (minimum @min).', ['@count' => $count, '@min' => $min_wc]));
  }

  // Headings (H1/H2) counts based on HTML.
  if (!empty($ck['headings'])) {
    $text = $html_or_text; // allow regex on HTML.
    $h1s = preg_match_all('/<h1\b[^>]*>(.*?)<\/h1>/i', $text, $m1);
    $h2s = preg_match_all('/<h2\b[^>]*>(.*?)<\/h2>/i', $text, $m2);
    $pass = ($h1s === 1 && $h2s >= 1);
    $add_check('headings', $pass, t('Headings: @h1 H1 and @h2 H2 found (recommend exactly one H1 and at least one H2).', ['@h1' => (int) $h1s, '@h2' => (int) $h2s]));
  }

  // Internal links (count anchors that are relative OR absolute to this host).
  if (!empty($ck['internal_links'])) {
    $html = (string) $html_or_text;
    $internal = 0;

    // Determine base host of the current site.
    $base_host = '';
    if (!empty($GLOBALS['base_url'])) {
      $base_host = parse_url($GLOBALS['base_url'], PHP_URL_HOST);
    }

    $counted = FALSE;

    if (class_exists('DOMDocument')) {
      $dom = new DOMDocument('1.0', 'UTF-8');
      libxml_use_internal_errors(TRUE);
      $htmlUtf8 = mb_convert_encoding($html, 'HTML-ENTITIES', 'UTF-8');
      $loaded = $dom->loadHTML(
        '<!DOCTYPE html><html lang="en"><body>' . $htmlUtf8 . '</body></html>',
        LIBXML_HTML_NOIMPLIED | LIBXML_HTML_NODEFDTD
      );
      libxml_clear_errors();

      if ($loaded) {
        foreach ($dom->getElementsByTagName('a') as $a) {
          $href = trim($a->getAttribute('href'));
          if ($href === '') { continue; }

          // Ignore mailto:, tel:, javascript:, etc.
          if (preg_match('#^[a-z]+:#i', $href) && !preg_match('#^https?://#i', $href)) {
            continue;
          }

          if (preg_match('#^https?://#i', $href)) {
            // Absolute URL: count if host matches site host.
            if ($base_host) {
              $h = parse_url($href, PHP_URL_HOST);
              if ($h !== NULL && strcasecmp($h, $base_host) === 0) {
                $internal++;
              }
            }
          } else {
            // Relative URL (starts with /, ?, or a relative path): count as internal.
            $internal++;
          }
        }
        $counted = TRUE;
      }
    }

    // Regex fallback if DOM fails.
    if (!$counted) {
      if (preg_match_all('/<a\b[^>]*href\s*=\s*["\'"]?([^"\'"\s>]+)["\'"]?/i', $html, $m)) {
        foreach ($m[1] as $href) {
          $href = trim($href);
          if ($href === '') { continue; }
          if (preg_match('#^[a-z]+:#i', $href) && !preg_match('#^https?://#i', $href)) {
            continue;
          }
          if (preg_match('#^https?://#i', $href)) {
            if ($base_host) {
              $h = parse_url($href, PHP_URL_HOST);
              if ($h !== NULL && strcasecmp($h, $base_host) === 0) {
                $internal++;
              }
            }
          } else {
            $internal++;
          }
        }
      }
    }

    $pass = ($internal >= 1);
    $add_check('internal_links', $pass, t('Internal links: @n found (at least 1 recommended).', ['@n' => (int) $internal]));
  }

  // Image alt attributes (preview HTML parsing).
  if (!empty($ck['images_alt'])) {
    $html = (string) $html_or_text;
    $img_count_dom = 0;
    $missing_alt_dom = 0;

    // DOM parse first.
    if (class_exists('DOMDocument')) {
      $dom = new DOMDocument('1.0', 'UTF-8');
      libxml_use_internal_errors(TRUE);
      $htmlUtf8 = mb_convert_encoding($html, 'HTML-ENTITIES', 'UTF-8');
      $loaded = $dom->loadHTML('<!DOCTYPE html><html lang="en"><body>' . $htmlUtf8 . '</body></html>', LIBXML_HTML_NOIMPLIED | LIBXML_HTML_NODEFDTD);
      libxml_clear_errors();
      if ($loaded) {
        $imgs = $dom->getElementsByTagName('img');
        $img_count_dom = $imgs->length;
        foreach ($imgs as $img) {
          $alt = $img->getAttribute('alt');
          if ($alt === NULL || trim($alt) === '') { $missing_alt_dom++; }
        }
      }
    }

    // Regex parsing (always run; used to augment DOM in case DOM misses some tags in certain fragments).
    $img_count_rx = 0;
    $missing_alt_rx = 0;
    if (preg_match_all('/<img\b[^>]*>/i', $html, $m)) {
      $img_count_rx = count($m[0]);
      foreach ($m[0] as $tag) {
        if (!preg_match('/\balt\s*=\s*("[^"]*"|\'[^\']*\')/i', $tag)) {
          $missing_alt_rx++;
        }
        else {
          // Treat alt="" or alt='   ' as missing too.
          if (preg_match('/\balt\s*=\s*"\s*"|\balt\s*=\s*\'\s*\'/i', $tag)) {
            $missing_alt_rx++;
          }
        }
      }
    }

    // Prefer the richer count: if regex finds more <img> than DOM, use regex stats; otherwise use DOM.
    $use_regex = ($img_count_rx > $img_count_dom);
    $img_count = $use_regex ? $img_count_rx : $img_count_dom;
    $missing_alt = $use_regex ? $missing_alt_rx : $missing_alt_dom;

    if ($img_count > 0) {
      $pass = ($missing_alt === 0);
      $msg = $pass
        ? t('Images: @n found; all have alt text.', ['@n' => $img_count])
        : t('Images: @n found; @m missing alt text.', ['@n' => $img_count, '@m' => $missing_alt]);
      $add_check('images_alt', $pass, $msg);
    }
    else {
      $add_check('images_alt', TRUE, t('No images found in preview HTML (alt text not required).'));
    }
  }

  // Focus keyword checks.
  if (!empty($ck['keyword'])) {
    $focus = trim(mb_strtolower($focus_keyword));
    if ($focus !== '') {
      $in_title = (mb_stripos(mb_strtolower($title), $focus) !== FALSE);
      $first_words = implode(' ', array_slice(preg_split('/\s+/u', $text_plain, -1, PREG_SPLIT_NO_EMPTY), 0, 100));
      $in_lead = (mb_stripos(mb_strtolower($first_words), $focus) !== FALSE);
      $add_check('keyword_in_title', $in_title, $in_title ? t('Focus keyword appears in the title.') : t('Focus keyword not found in the title.'));
      $add_check('keyword_in_body', $in_lead, $in_lead ? t('Focus keyword appears early in the content.') : t('Focus keyword not found in the beginning of the content.'));
    }
  }

  $normalized = ($total_weight > 0) ? round(($score / $total_weight) * 100) : 0;
  $status = ($normalized >= 80) ? 'GREEN' : (($normalized >= 50) ? 'AMBER' : 'RED');

  $result = [
    'score' => $normalized,
    'status' => $status,
    'checks' => $checks,
    'focus' => $focus_keyword,
  ];

  return $result;
}

/**
 * Generate AI suggestions based on content.
 */
function ai_seo_advisor_generate_ai_suggestions($title, $html_text, $raw_text, $meta_description, $focus, $model, $max_tokens, $report_type = 'full') {
  if (!module_exists('ai')) { return NULL; }

  $opts_cfg = is_array(config_get('ai_seo_advisor.settings', 'options')) ? config_get('ai_seo_advisor.settings', 'options') : [];

  // Build system prompt: default or custom.
  $default_system_prompt = "You are an SEO analysis expert specialized in evaluating HTML content from an SEO perspective. Your role is to provide a comprehensive audit, including clear suggestions and improvements for each aspect of SEO best practices. You should aim to be thorough, precise, and provide examples wherever possible to illustrate your points. Provide your report with HTML markup for headings, paragraphs, lists, etc.";
  $system_prompt = !empty($opts_cfg['system_prompt']) ? trim($opts_cfg['system_prompt']) : $default_system_prompt;

  // Get site context (global).
  $site_context = '';
  if (isset($opts_cfg['site_context']) && is_string($opts_cfg['site_context']) && trim($opts_cfg['site_context']) !== '') {
    $site_context = trim($opts_cfg['site_context']);
  }

  // Get report type prompt.
  $report_types = ai_seo_advisor_get_report_types();
  $report_type_prompt = '';
  if (!empty($report_types[$report_type]['prompt'])) {
    $report_type_prompt = trim($report_types[$report_type]['prompt']);
  }
  else {
    // Fallback if report type not found or has no prompt.
    $report_type_prompt = 'Analyze this page for SEO and write a concise HTML report. Include: improvement tips, up to 5 prioritized recommendations, 3 title ideas (under 60 characters), 3 meta description ideas (140-160 characters), and 8 keyword suggestions. Use HTML headings and lists for structure.';
  }

  // Build full system prompt: system_prompt + site_context.
  $sys_full = $system_prompt;
  if ($site_context !== '') {
    $sys_full .= "\n\n## Site Context\n" . $site_context;
  }

  // Build messages exactly like Drupal ai_seo: separate prompt and HTML messages
  // First user message: just the prompt
  $prompt_message = $report_type_prompt;

  // Second user message: HTML content with title and meta
  $html_message = "**Page Title:** " . $title . "\n\n";
  if (!empty($meta_description)) {
    $html_message .= "**Meta Description:** " . $meta_description . "\n\n";
  }
  $html_message .= "**HTML Content:**\n\n" . $html_text;

  try {
    // Send as separate messages like Drupal ai_seo does
    $messages = [
      ['role' => 'system', 'content' => $sys_full],
      ['role' => 'user', 'content' => $prompt_message],
      ['role' => 'user', 'content' => $html_message],
    ];

    // Use the centralized helper function for chat.
    $response = ai_chat($model, $messages, 0.3, (int) $max_tokens, FALSE, ['operation' => 'seo_analysis']);

    // Normalize response across providers. Some adapters return strings,
    // others return arrays with output_text/choices/output structures.
    $content = '';
    if (is_string($response)) {
      $content = trim($response);
    }
    elseif (is_array($response)) {
      if (!empty($response['output_text']) && is_string($response['output_text'])) {
        $content = trim($response['output_text']);
      }
      if ($content === '' && !empty($response['choices'][0]['message']['content']) && is_string($response['choices'][0]['message']['content'])) {
        $content = trim($response['choices'][0]['message']['content']);
      }
      if ($content === '' && !empty($response['output']) && is_array($response['output'])) {
        foreach ($response['output'] as $output_item) {
          if (!empty($output_item['content']) && is_array($output_item['content'])) {
            foreach ($output_item['content'] as $part) {
              if (!empty($part['text']) && is_string($part['text'])) {
                $content .= $part['text'];
              }
            }
          }
        }
        $content = trim($content);
      }
    }

    if ($content === '') {
      watchdog('ai_seo_advisor', 'AI report generation returned empty content for model @model.', ['@model' => $model], WATCHDOG_WARNING);
      return NULL;
    }

    // Return raw HTML report for display.
    return [
      'html_report' => $content,
      'report_type' => $report_type,
    ];
  }
  catch (\Exception $e) {
    watchdog('ai_seo_advisor', 'AI suggestions failed: @m', ['@m' => $e->getMessage()], WATCHDOG_WARNING);
    return NULL;
  }
}

/**
 * Page callback: Display list of SEO reports for a node.
 */
