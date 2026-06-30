<?php

/**
 * @file
 * Tool definitions and executors for ai_seo_advisor agent tools.
 */

/**
 * Returns tool definitions for hook_ai_tools().
 */
function ai_seo_advisor_tools_definitions() {
  $tools = [];

  $tools['get_seo_report'] = [
    'definition' => [
      'type' => 'function',
      'function' => [
        'name' => 'get_seo_report',
        'description' => 'Return the SEO analysis report for a node. Returns a cached report if available. Pass force_refresh=true to re-run the analysis. Returns title length, description status, readability, heading structure, and AI recommendations.',
        'parameters' => [
          'type' => 'object',
          'properties' => [
            'nid' => ['type' => 'integer', 'description' => 'The node ID.'],
            'report_type' => ['type' => 'string', 'description' => 'Report type: full (default), aeo, natural_language, headings_and_structure, link_analysis, schema_org_markup, topic_authority.'],
            'force_refresh' => ['type' => 'boolean', 'description' => 'Re-run the analysis even if a recent report exists (default false).'],
          ],
          'required' => ['nid'],
          'additionalProperties' => FALSE,
        ],
      ],
    ],
    'execute' => 'ai_seo_advisor_tool_get_seo_report',
    'operation' => 'read',
    'permission' => 'analyze content seo with ai',
    'module' => 'ai_seo_advisor',
    'category' => 'seo',
  ];

  $tools['get_bulk_seo_report'] = [
    'definition' => [
      'type' => 'function',
      'function' => [
        'name' => 'get_bulk_seo_report',
        'description' => 'Return an SEO health summary across multiple nodes using saved reports. Use unanalyzed=true to find nodes that have never been analyzed.',
        'parameters' => [
          'type' => 'object',
          'properties' => [
            'bundle' => ['type' => 'string', 'description' => 'Limit to a content type machine name (optional).'],
            'limit' => ['type' => 'integer', 'description' => 'Max nodes to return (default 20, max 50).'],
            'unanalyzed' => ['type' => 'boolean', 'description' => 'Return only nodes with no saved report (default false).'],
          ],
          'required' => [],
          'additionalProperties' => FALSE,
        ],
      ],
    ],
    'execute' => 'ai_seo_advisor_tool_get_bulk_seo_report',
    'operation' => 'read',
    'permission' => 'analyze content seo with ai',
    'module' => 'ai_seo_advisor',
    'category' => 'seo',
  ];

  $tools['get_nodes_missing_description'] = [
    'definition' => [
      'type' => 'function',
      'function' => [
        'name' => 'get_nodes_missing_description',
        'description' => 'Find published nodes with no meta description set. Missing descriptions are the highest-priority SEO gap.',
        'parameters' => [
          'type' => 'object',
          'properties' => [
            'bundle' => ['type' => 'string', 'description' => 'Limit to a content type (optional).'],
            'limit' => ['type' => 'integer', 'description' => 'Max results (default 25, max 100).'],
          ],
          'required' => [],
          'additionalProperties' => FALSE,
        ],
      ],
    ],
    'execute' => 'ai_seo_advisor_tool_get_nodes_missing_description',
    'operation' => 'read',
    'permission' => 'access content',
    'module' => 'ai_seo_advisor',
    'category' => 'seo',
  ];

  $tools['get_nodes_short_title'] = [
    'definition' => [
      'type' => 'function',
      'function' => [
        'name' => 'get_nodes_short_title',
        'description' => 'Find published nodes with a title shorter than the recommended minimum. SEO titles under 30 characters are typically too thin.',
        'parameters' => [
          'type' => 'object',
          'properties' => [
            'min_length' => ['type' => 'integer', 'description' => 'Minimum acceptable title length in characters (default 30).'],
            'bundle' => ['type' => 'string', 'description' => 'Limit to a content type (optional).'],
            'limit' => ['type' => 'integer', 'description' => 'Max results (default 25).'],
          ],
          'required' => [],
          'additionalProperties' => FALSE,
        ],
      ],
    ],
    'execute' => 'ai_seo_advisor_tool_get_nodes_short_title',
    'operation' => 'read',
    'permission' => 'access content',
    'module' => 'ai_seo_advisor',
    'category' => 'seo',
  ];

  $tools['get_keyword_density'] = [
    'definition' => [
      'type' => 'function',
      'function' => [
        'name' => 'get_keyword_density',
        'description' => 'Analyze how frequently a keyword or phrase appears in a node body field. Returns occurrence count, word count, and density percentage. Healthy range is 1-3% for a primary keyword.',
        'parameters' => [
          'type' => 'object',
          'properties' => [
            'nid' => ['type' => 'integer', 'description' => 'The node ID.'],
            'keyword' => ['type' => 'string', 'description' => 'The keyword or phrase to check (case-insensitive).'],
          ],
          'required' => ['nid', 'keyword'],
          'additionalProperties' => FALSE,
        ],
      ],
    ],
    'execute' => 'ai_seo_advisor_tool_get_keyword_density',
    'operation' => 'read',
    'permission' => 'access content',
    'module' => 'ai_seo_advisor',
    'category' => 'seo',
  ];

  $tools['get_readability_score'] = [
    'definition' => [
      'type' => 'function',
      'function' => [
        'name' => 'get_readability_score',
        'description' => 'Estimate the reading level of a node body using the Flesch-Kincaid formula. Returns grade level and ease score. Grade 8-10 is accessible to most general audiences.',
        'parameters' => [
          'type' => 'object',
          'properties' => [
            'nid' => ['type' => 'integer', 'description' => 'The node ID.'],
          ],
          'required' => ['nid'],
          'additionalProperties' => FALSE,
        ],
      ],
    ],
    'execute' => 'ai_seo_advisor_tool_get_readability_score',
    'operation' => 'read',
    'permission' => 'access content',
    'module' => 'ai_seo_advisor',
    'category' => 'seo',
  ];

  return $tools;
}

// ---------------------------------------------------------------------------
// Executors
// ---------------------------------------------------------------------------

/**
 * Executor: get_seo_report.
 */
function ai_seo_advisor_tool_get_seo_report(array $args) {
  $nid = isset($args['nid']) ? (int) $args['nid'] : 0;
  if ($nid <= 0) {
    return ['error' => 'Invalid nid.'];
  }
  $node = node_load($nid);
  if (!$node) {
    return ['error' => 'Node not found.'];
  }

  $report_type = isset($args['report_type']) ? trim((string) $args['report_type']) : 'full';
  $force_refresh = !empty($args['force_refresh']);

  // Try cached report first.
  if (!$force_refresh) {
    $existing = db_select('ai_seo_report', 'r')
      ->fields('r')
      ->condition('r.entity_type', 'node')
      ->condition('r.entity_id', $nid)
      ->condition('r.report_type', $report_type)
      ->orderBy('r.changed', 'DESC')
      ->range(0, 1)
      ->execute()
      ->fetchObject();

    if ($existing) {
      $data = !empty($existing->report_data) ? unserialize($existing->report_data, ['allowed_classes' => FALSE]) : [];
      $data = is_array($data) ? $data : [];
      return _ai_seo_advisor_tool_format_report($node, $existing, $data, TRUE);
    }
  }

  // Run fresh analysis by loading pages.inc functions.
  module_load_include('inc', 'ai_seo_advisor', 'ai_seo_advisor.pages');
  if (!function_exists('ai_seo_advisor_analyze_form_submit')) {
    return ['error' => 'SEO Advisor analysis function not available.'];
  }

  // Build a minimal form + form_state to trigger analysis. The submit handler
  // reads the node from $form['#node'], so it must be set here.
  $form_state = [
    'values' => [
      'report_type' => $report_type,
      'focus_keyword' => '',
    ],
    'build_info' => ['args' => [$node]],
  ];
  $form = ['#node' => $node];
  ai_seo_advisor_analyze_form_submit($form, $form_state);

  // Re-fetch the report that was just saved.
  $saved = db_select('ai_seo_report', 'r')
    ->fields('r')
    ->condition('r.entity_type', 'node')
    ->condition('r.entity_id', $nid)
    ->condition('r.report_type', $report_type)
    ->orderBy('r.changed', 'DESC')
    ->range(0, 1)
    ->execute()
    ->fetchObject();

  if ($saved) {
    $data = !empty($saved->report_data) ? unserialize($saved->report_data, ['allowed_classes' => FALSE]) : [];
    $data = is_array($data) ? $data : [];
    return _ai_seo_advisor_tool_format_report($node, $saved, $data, FALSE);
  }

  return ['error' => 'Analysis ran but report could not be retrieved.'];
}

/**
 * Format a report row into a structured tool response.
 */
function _ai_seo_advisor_tool_format_report($node, $report_row, array $data, $cached) {
  $title = $node->title;
  $title_length = mb_strlen($title);

  // Extract body text for quick stats.
  $body_text = '';
  $langcode = LANGUAGE_NONE;
  if (!empty($node->body[$langcode][0]['value'])) {
    $body_text = strip_tags($node->body[$langcode][0]['value']);
  }
  $word_count = $body_text ? str_word_count($body_text) : 0;

  // Check meta description.
  $has_desc = FALSE;
  $desc_length = 0;
  if (!empty($node->metatags)) {
    $lang = LANGUAGE_NONE;
    if (function_exists('metatag_entity_get_language')) {
      $lang = metatag_entity_get_language('node', $node);
    }
    $meta = !empty($node->metatags[$lang]) ? $node->metatags[$lang] : (!empty($node->metatags[LANGUAGE_NONE]) ? $node->metatags[LANGUAGE_NONE] : []);
    $desc_val = isset($meta['description']['value']) ? trim($meta['description']['value']) : '';
    $has_desc = $desc_val !== '';
    $desc_length = mb_strlen($desc_val);
  }

  $ai_summary = '';
  if (!empty($data['ai_suggestions'])) {
    $ai_summary = is_string($data['ai_suggestions']) ? $data['ai_suggestions'] : '';
  }
  if (empty($ai_summary) && !empty($data['ai']['html_report'])) {
    $ai_summary = substr(strip_tags($data['ai']['html_report']), 0, 500) . '...';
  }

  return [
    'nid' => (int) $node->nid,
    'title' => $title,
    'bundle' => $node->type,
    'report_type' => $report_row->report_type,
    'title_length' => $title_length,
    'title_ok' => $title_length >= 30 && $title_length <= 60,
    'has_description' => $has_desc,
    'desc_length' => $desc_length,
    'body_word_count' => $word_count,
    'ai_summary' => $ai_summary,
    'analyzed_at' => (int) $report_row->changed,
    'cached' => $cached,
  ];
}

/**
 * Executor: get_bulk_seo_report.
 */
function ai_seo_advisor_tool_get_bulk_seo_report(array $args) {
  $bundle = isset($args['bundle']) ? trim((string) $args['bundle']) : '';
  $limit = isset($args['limit']) ? min(50, max(1, (int) $args['limit'])) : 20;
  $unanalyzed = !empty($args['unanalyzed']);

  $query = db_select('node', 'n')
    ->fields('n', ['nid', 'title', 'type'])
    ->condition('n.status', 1);
  if ($bundle !== '') {
    $query->condition('n.type', $bundle);
  }

  if ($unanalyzed) {
    $subq = db_select('ai_seo_report', 'r')
      ->fields('r', ['entity_id'])
      ->condition('r.entity_type', 'node');
    $query->condition('n.nid', $subq, 'NOT IN');
    $query->range(0, $limit);
    $rows = $query->execute()->fetchAll();
    $nodes = [];
    foreach ($rows as $row) {
      $nodes[] = ['nid' => (int) $row->nid, 'title' => $row->title, 'bundle' => $row->type, 'analyzed' => FALSE];
    }
    return ['unanalyzed_count' => count($nodes), 'nodes' => $nodes];
  }

  // Join with report table for analyzed nodes.
  $query->join('ai_seo_report', 'r', 'r.entity_id = n.nid AND r.entity_type = :et', [':et' => 'node']);
  $query->fields('r', ['report_type', 'changed']);
  $query->orderBy('r.changed', 'DESC');
  $query->range(0, $limit);
  $rows = $query->execute()->fetchAll();

  $nodes = [];
  foreach ($rows as $row) {
    $nodes[] = [
      'nid' => (int) $row->nid,
      'title' => $row->title,
      'bundle' => $row->type,
      'report_type' => $row->report_type,
      'analyzed_at' => (int) $row->changed,
      'analyzed' => TRUE,
    ];
  }

  return ['returned' => count($nodes), 'nodes' => $nodes];
}

/**
 * Executor: get_nodes_missing_description.
 */
function ai_seo_advisor_tool_get_nodes_missing_description(array $args) {
  $bundle = isset($args['bundle']) ? trim((string) $args['bundle']) : '';
  $limit = isset($args['limit']) ? min(100, max(1, (int) $args['limit'])) : 25;

  $query = db_select('node', 'n')
    ->fields('n', ['nid', 'title', 'type'])
    ->condition('n.status', 1);
  if ($bundle !== '') {
    $query->condition('n.type', $bundle);
  }
  $query->range(0, 200);
  $rows = $query->execute()->fetchAll();

  $missing = [];
  foreach ($rows as $row) {
    $node = node_load($row->nid);
    if (!$node) {
      continue;
    }
    $langcode = LANGUAGE_NONE;
    if (function_exists('metatag_entity_get_language')) {
      $langcode = metatag_entity_get_language('node', $node);
    }
    $meta = !empty($node->metatags[$langcode]) ? $node->metatags[$langcode] : (!empty($node->metatags[LANGUAGE_NONE]) ? $node->metatags[LANGUAGE_NONE] : []);
    $desc = isset($meta['description']['value']) ? trim($meta['description']['value']) : '';
    if ($desc === '') {
      $body = '';
      if (!empty($node->body[LANGUAGE_NONE][0]['value'])) {
        $body = strip_tags($node->body[LANGUAGE_NONE][0]['value']);
      }
      $missing[] = [
        'nid' => (int) $row->nid,
        'title' => $row->title,
        'bundle' => $row->type,
        'body_word_count' => $body ? str_word_count($body) : 0,
      ];
    }
    if (count($missing) >= $limit) {
      break;
    }
  }

  return ['total_returned' => count($missing), 'nodes' => $missing];
}

/**
 * Executor: get_nodes_short_title.
 */
function ai_seo_advisor_tool_get_nodes_short_title(array $args) {
  $min_length = isset($args['min_length']) ? max(1, (int) $args['min_length']) : 30;
  $bundle = isset($args['bundle']) ? trim((string) $args['bundle']) : '';
  $limit = isset($args['limit']) ? min(100, max(1, (int) $args['limit'])) : 25;

  $query = db_select('node', 'n')
    ->fields('n', ['nid', 'title', 'type'])
    ->condition('n.status', 1)
    ->where('CHAR_LENGTH(n.title) < :min', [':min' => $min_length]);
  if ($bundle !== '') {
    $query->condition('n.type', $bundle);
  }
  $query->range(0, $limit);
  $rows = $query->execute()->fetchAll();

  $nodes = [];
  foreach ($rows as $row) {
    $nodes[] = [
      'nid' => (int) $row->nid,
      'title' => $row->title,
      'title_length' => mb_strlen($row->title),
      'bundle' => $row->type,
    ];
  }

  return ['min_length' => $min_length, 'returned' => count($nodes), 'nodes' => $nodes];
}

/**
 * Executor: get_keyword_density.
 */
function ai_seo_advisor_tool_get_keyword_density(array $args) {
  $nid = isset($args['nid']) ? (int) $args['nid'] : 0;
  $keyword = isset($args['keyword']) ? trim((string) $args['keyword']) : '';
  if ($nid <= 0 || $keyword === '') {
    return ['error' => 'nid and keyword are required.'];
  }

  $node = node_load($nid);
  if (!$node) {
    return ['error' => 'Node not found.'];
  }

  $body = '';
  if (!empty($node->body[LANGUAGE_NONE][0]['value'])) {
    $body = strip_tags($node->body[LANGUAGE_NONE][0]['value']);
  }
  if ($body === '') {
    return ['nid' => $nid, 'keyword' => $keyword, 'occurrences' => 0, 'word_count' => 0, 'density' => '0%'];
  }

  $body_lower = mb_strtolower($body);
  $keyword_lower = mb_strtolower($keyword);
  $occurrences = substr_count($body_lower, $keyword_lower);
  $word_count = str_word_count($body);
  $density = $word_count > 0 ? round(($occurrences / $word_count) * 100, 2) : 0;

  // Check title and meta description.
  $in_title = stripos($node->title, $keyword) !== FALSE;
  $langcode = LANGUAGE_NONE;
  if (function_exists('metatag_entity_get_language')) {
    $langcode = metatag_entity_get_language('node', $node);
  }
  $meta = !empty($node->metatags[$langcode]) ? $node->metatags[$langcode] : (!empty($node->metatags[LANGUAGE_NONE]) ? $node->metatags[LANGUAGE_NONE] : []);
  $desc_val = isset($meta['description']['value']) ? $meta['description']['value'] : '';
  $in_description = stripos($desc_val, $keyword) !== FALSE;

  return [
    'nid' => $nid,
    'keyword' => $keyword,
    'occurrences' => $occurrences,
    'word_count' => $word_count,
    'density' => $density . '%',
    'in_title' => $in_title,
    'in_description' => $in_description,
    'assessment' => $density < 0.5 ? 'Too low' : ($density > 3 ? 'Too high — risk of keyword stuffing' : 'Healthy range'),
  ];
}

/**
 * Executor: get_readability_score.
 */
function ai_seo_advisor_tool_get_readability_score(array $args) {
  $nid = isset($args['nid']) ? (int) $args['nid'] : 0;
  if ($nid <= 0) {
    return ['error' => 'Invalid nid.'];
  }
  $node = node_load($nid);
  if (!$node) {
    return ['error' => 'Node not found.'];
  }

  $body = '';
  if (!empty($node->body[LANGUAGE_NONE][0]['value'])) {
    $body = strip_tags($node->body[LANGUAGE_NONE][0]['value']);
  }
  if ($body === '') {
    return ['nid' => $nid, 'error' => 'No body text found.'];
  }

  $word_count = str_word_count($body);
  // Count sentences (split on . ! ? followed by space or end).
  $sentence_count = preg_match_all('/[.!?](\s|$)/', $body, $m);
  $sentence_count = max(1, $sentence_count);

  // Syllable count via vowel-group heuristic.
  $syllable_count = _ai_seo_advisor_count_syllables($body);

  // Flesch Reading Ease: 206.835 - 1.015*(words/sentences) - 84.6*(syllables/words)
  $ease = 206.835 - 1.015 * ($word_count / $sentence_count) - 84.6 * ($syllable_count / max(1, $word_count));
  $ease = round($ease, 1);

  // Flesch-Kincaid Grade: 0.39*(words/sentences) + 11.8*(syllables/words) - 15.59
  $grade = 0.39 * ($word_count / $sentence_count) + 11.8 * ($syllable_count / max(1, $word_count)) - 15.59;
  $grade = round($grade, 1);

  if ($ease >= 70) {
    $assessment = 'Easy — accessible to most readers';
  }
  elseif ($ease >= 50) {
    $assessment = 'Fairly Difficult — consider simplifying for broader audiences';
  }
  else {
    $assessment = 'Difficult — best suited for specialized audiences';
  }

  return [
    'nid' => $nid,
    'word_count' => $word_count,
    'sentence_count' => $sentence_count,
    'syllable_count' => $syllable_count,
    'flesch_kincaid_grade' => $grade,
    'flesch_ease' => $ease,
    'assessment' => $assessment,
  ];
}

/**
 * Estimate syllable count using vowel-group heuristic.
 */
function _ai_seo_advisor_count_syllables($text) {
  $words = preg_split('/\s+/', strtolower(strip_tags($text)), -1, PREG_SPLIT_NO_EMPTY);
  $count = 0;
  foreach ($words as $word) {
    $word = preg_replace('/[^a-z]/', '', $word);
    if ($word === '') {
      continue;
    }
    // Count vowel groups.
    $syllables = preg_match_all('/[aeiouy]+/', $word, $m);
    // Subtract silent e at end.
    if (substr($word, -1) === 'e' && strlen($word) > 2) {
      $syllables--;
    }
    $count += max(1, $syllables);
  }
  return $count;
}
