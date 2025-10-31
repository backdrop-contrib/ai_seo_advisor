<?php
/**
 * @file
 * Administrative forms for OpenAI SEO Advisor module.
 */

/**
 * Returns available text/chat models from the OpenAI module or a fallback.
 */
function openai_seo_advisor_get_model_options() {
  $options = array();

  // Ensure the OpenAI class is available.
  if (!class_exists('OpenAIApi')) {
    $api_file = backdrop_get_path('module', 'openai') . '/includes/OpenAIApi.php';
    if (file_exists($api_file)) {
      require_once $api_file;
    }
  }

  // Attempt to fetch available models via the contrib OpenAI API.
  if (class_exists('OpenAIApi')) {
    try {
      $api_key_name = config_get('openai.settings', 'api_key');
      $api_key = $api_key_name ? key_get_key_value($api_key_name) : '';
      if (!empty($api_key)) {
        $api = new OpenAIApi($api_key);
        if (method_exists($api, 'filterModels')) {
          $models = $api->filterModels(array('gpt', 'text'));
          if (is_array($models)) {
            foreach ($models as $m) {
              // Filter out image models just in case.
              if (preg_match('/^gpt-image/i', $m)) { continue; }
              $options[$m] = $m;
            }
          }
        }
      }
      else {
        watchdog('openai_seo_advisor', 'OpenAI API key not configured; no models available.', array(), WATCHDOG_NOTICE);
      }
    }
    catch (Exception $e) {
      watchdog('openai_seo_advisor', 'Failed to fetch OpenAI models: @m', array('@m' => $e->getMessage()), WATCHDOG_WARNING);
    }
  }

  // Return any models discovered via the OpenAI API. If none were found,
  // return an empty array — the caller will fall back to configured defaults.
  return $options;
}

function openai_seo_advisor_settings_form($form, &$form_state) {
  // Bundle enablement.
  $types = node_type_get_types();
  $options = array();
  foreach ($types as $type) { $options[$type->type] = check_plain($type->name); }

  $enabled_bundles = config_get('openai_seo_advisor.settings', 'enabled_bundles');
  $enabled_bundles = is_array($enabled_bundles) ? $enabled_bundles : array();

  // Normalize to array of selected keys for FAPI checkboxes.
  $default_selected = array();
  foreach ($enabled_bundles as $bundle_key => $is_enabled) {
    if (!empty($is_enabled)) { $default_selected[$bundle_key] = $bundle_key; }
  }

  $form['enabled_bundles'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Enable on these content types'),
    '#options' => $options,
    '#default_value' => $default_selected,
    '#description' => t('The SEO Analysis panel will appear on node edit forms for these content types.'),
    '#element_validate' => array('openai_seo_advisor_enabled_bundles_validate'),
  );

  // AI Options.
  $opts_cfg = is_array(config_get('openai_seo_advisor.settings', 'options')) ? config_get('openai_seo_advisor.settings', 'options') : array();
  $form['options'] = array(
    '#type' => 'fieldset',
    '#title' => t('OpenAI settings'),
    '#tree' => TRUE,
  );

  $form['options']['system_prompt_fieldset'] = array(
    '#type' => 'fieldset',
    '#title' => t('System prompt'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  );

  $default_system_prompt = "You are an SEO analysis expert specialized in evaluating HTML content from an SEO perspective. Your role is to provide a comprehensive audit, including clear suggestions and improvements for each aspect of SEO best practices. You should aim to be thorough, precise, and provide examples wherever possible to illustrate your points. Provide your report with HTML markup for headings, paragraphs, lists, etc.";

  $form['options']['system_prompt_fieldset']['default_system_prompt'] = array(
    '#type' => 'textarea',
    '#title' => t('Default system prompt'),
    '#value' => $default_system_prompt,
    '#disabled' => TRUE,
    '#description' => t('This is the default system prompt that defines the AI\'s role and behavior. It cannot be edited here.'),
    '#rows' => 4,
  );

  $form['options']['system_prompt'] = array(
    '#type' => 'textarea',
    '#title' => t('Custom system prompt (optional)'),
    '#default_value' => isset($opts_cfg['system_prompt']) ? $opts_cfg['system_prompt'] : '',
    '#description' => t('Override the default system prompt. Leave empty to use the default. This defines how the AI should behave and respond.'),
    '#rows' => 6,
  );

  $model_options = openai_seo_advisor_get_model_options();
  $model_select_disabled = empty($model_options);
  $default_model_value = ((is_array(config_get('openai_seo_advisor.settings', 'options')) && isset(config_get('openai_seo_advisor.settings', 'options')['ai_model']) && config_get('openai_seo_advisor.settings', 'options')['ai_model'] !== '') ? config_get('openai_seo_advisor.settings', 'options')['ai_model'] : (config_get('openai.settings', 'default_text_model') ?: 'gpt-4o-mini'));
  $model_description = t('Select which text model to use for AI suggestions.');
  if ($model_select_disabled) {
    $model_description .= ' ' . t('No models were discovered. Configure the OpenAI API key at admin/config/openai/settings to enable model discovery.');
  }
  $form['options']['ai_model'] = array(
    '#type' => 'select',
    '#title' => t('AI model'),
    '#options' => $model_options,
    '#default_value' => $default_model_value,
    '#description' => $model_description,
    '#attributes' => array('class' => array('openai-seo-model-select')),
    '#disabled' => $model_select_disabled,
  );
  // ai_max_tokens is hardcoded at 3000 (same as Drupal ai_seo module)
  // No user setting needed.

  $form['options']['site_context'] = array(
    '#type' => 'textarea',
    '#title' => t('Site context'),
    '#default_value' => (is_array(config_get('openai_seo_advisor.settings', 'options')) && isset(config_get('openai_seo_advisor.settings', 'options')['site_context'])) ? config_get('openai_seo_advisor.settings', 'options')['site_context'] : '',
    '#description' => t('Provide domain-specific context to help the AI understand your industry, terminology, and acronyms. Example: "AMA refers to Academy of Model Aeronautics, not American Medical Association. Content is about model aviation, RC aircraft, and aeromodelling." This context is included with every analysis to ensure accurate, relevant recommendations.'),
    '#rows' => 6,
  );

  $form['options']['report_type_info'] = array(
    '#type' => 'markup',
    '#markup' => '<div class="messages info">' . t('Report type prompts are configured at <a href="@link">Report Types</a>. Each report type has its own detailed analysis prompt.', array('@link' => url('admin/config/openai/seo/report-types'))) . '</div>',
  );

  $form['options']['log_ai_input'] = array(
    '#type' => 'checkbox',
    '#title' => t('Log AI input (debug)'),
    '#default_value' => (is_array(config_get('openai_seo_advisor.settings', 'options')) && !empty(config_get('openai_seo_advisor.settings', 'options')['log_ai_input'])) ? 1 : 0,
    '#description' => t('Log sanitized content sent to AI for debugging. Use only on development sites.'),
  );

  $form['#submit'][] = 'openai_seo_advisor_settings_form_submit';
  // Bind this settings form to Backdrop config so values persist in
  // openai_seo_advisor.settings instead of legacy variables.
  $form['#config'] = 'openai_seo_advisor.settings';
  return system_settings_form($form);
}

/**
 * Element validator for enabled_bundles checkboxes.
 */
function openai_seo_advisor_enabled_bundles_validate(&$element, &$form_state, $form) {
  $values = isset($form_state['values']['enabled_bundles']) ? $form_state['values']['enabled_bundles'] : array();
  $normalized = array();
  foreach ($values as $bundle => $val) {
    $normalized[$bundle] = !empty($val) ? 1 : 0;
  }
  $form_state['values']['enabled_bundles'] = $normalized;
}

/**
 * Settings submit: normalize nested arrays and clamp ranges.
 */
function openai_seo_advisor_settings_form_submit($form, &$form_state) {
  // Ensure options exist and set defaults.
  if (empty($form_state['values']['options']) || !is_array($form_state['values']['options'])) {
    $form_state['values']['options'] = array();
  }
  $opts = & $form_state['values']['options'];
  $opts += array(
    'ai_model' => (string) (config_get('openai.settings', 'default_text_model') ?: 'gpt-4o-mini'),
  );
  $opts['ai_model'] = isset($opts['ai_model']) && is_string($opts['ai_model']) && trim($opts['ai_model']) !== '' ? trim($opts['ai_model']) : (config_get('openai.settings', 'default_text_model') ?: 'gpt-4o-mini');

  // Normalize system_prompt.
  if (isset($opts['system_prompt']) && is_string($opts['system_prompt'])) {
    $sp = trim($opts['system_prompt']);
    if (backdrop_strlen($sp) > 2000) { $sp = backdrop_substr($sp, 0, 2000) . '…'; }
    $opts['system_prompt'] = $sp;
  }
  else {
    $opts['system_prompt'] = '';
  }

  if (isset($opts['site_context']) && is_string($opts['site_context'])) {
    $sc = trim($opts['site_context']);
    if (backdrop_strlen($sc) > 4000) { $sc = backdrop_substr($sc, 0, 4000) . '…'; }
    $opts['site_context'] = $sc;
  }
  else {
    $opts['site_context'] = '';
  }

  // Remove the fieldset wrapper that doesn't need to be saved.
  if (isset($opts['system_prompt_fieldset'])) {
    unset($opts['system_prompt_fieldset']);
  }
  if (isset($opts['report_type_info'])) {
    unset($opts['report_type_info']);
  }
}

/**
 * Implements hook_form_alter().
 *
 * Note: SEO analysis has been moved to the SEO tab on node pages.
 * This hook is kept for potential future use.
 */
function openai_seo_advisor_report_types_list() {
  $output = array();

  // Load all report type configs
  $report_types = openai_seo_advisor_get_report_types();

  if (empty($report_types)) {
    $output['empty'] = array(
      '#type' => 'markup',
      '#markup' => '<p>' . t('No report types defined yet.') . '</p>',
    );
    return $output;
  }

  // Build table.
  $header = array(
    t('Label'),
    t('Machine Name'),
    t('Description'),
    t('Status'),
    t('Operations'),
  );

  $rows = array();
  foreach ($report_types as $name => $type) {
    $label = isset($type['label']) ? $type['label'] : $name;
    $description = isset($type['description']) ? $type['description'] : '';
    $status = !empty($type['status']) ? t('Enabled') : t('Disabled');

    $operations = array();
    $operations[] = l(t('Edit'), 'admin/config/openai/seo/report-types/' . $name . '/edit');
    if (!in_array($name, array('seo_analysis', 'aeo_analysis'))) {
      $operations[] = l(t('Delete'), 'admin/config/openai/seo/report-types/' . $name . '/delete');
    }

    $rows[] = array(
      check_plain($label),
      check_plain($name),
      check_plain($description),
      $status,
      implode(' | ', $operations),
    );
  }

  $output['table'] = array(
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
  );

  return $output;
}

/**
 * Form constructor for add/edit report type.
 */
function openai_seo_advisor_report_type_form($form, &$form_state, $name = NULL) {
  $is_new = empty($name);

  if (!$is_new) {
    // Load from individual config file
    $config = config('openai_seo_advisor.report_type.' . $name);
    $report_type = $config->get();

    if (empty($report_type) || empty($report_type['name'])) {
      backdrop_set_message(t('Report type not found.'), 'error');
      backdrop_goto('admin/config/openai/seo/report-types');
    }
  }
  else {
    $report_type = array(
      'name' => '',
      'label' => '',
      'description' => '',
      'prompt' => '',
      'status' => 1,
    );
  }

  $form['name'] = array(
    '#type' => 'machine_name',
    '#title' => t('Machine name'),
    '#default_value' => isset($report_type['name']) ? $report_type['name'] : '',
    '#maxlength' => 64,
    '#required' => TRUE,
    '#machine_name' => array(
      'exists' => 'openai_seo_advisor_report_type_exists',
    ),
    '#disabled' => !$is_new,
  );

  $form['label'] = array(
    '#type' => 'textfield',
    '#title' => t('Label'),
    '#default_value' => isset($report_type['label']) ? $report_type['label'] : '',
    '#required' => TRUE,
    '#maxlength' => 255,
  );

  $form['description'] = array(
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#default_value' => isset($report_type['description']) ? $report_type['description'] : '',
    '#rows' => 3,
  );

  $form['prompt'] = array(
    '#type' => 'textarea',
    '#title' => t('AI Prompt'),
    '#description' => t('The prompt sent to the AI for this report type. Use detailed instructions.'),
    '#default_value' => isset($report_type['prompt']) ? $report_type['prompt'] : '',
    '#rows' => 20,
  );

  $form['status'] = array(
    '#type' => 'checkbox',
    '#title' => t('Enabled'),
    '#default_value' => !empty($report_type['status']),
  );

  $form['weight'] = array(
    '#type' => 'weight',
    '#title' => t('Weight'),
    '#default_value' => isset($report_type['weight']) ? $report_type['weight'] : 0,
  );

  $form['actions'] = array('#type' => 'actions');
  $form['actions']['submit'] = array(
    '#type' => 'submit',
    '#value' => $is_new ? t('Create report type') : t('Save report type'),
  );
  $form['actions']['cancel'] = array(
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => 'admin/config/openai/seo/report-types',
  );

  return $form;
}

/**
 * Submit handler for report type form.
 */
function openai_seo_advisor_report_type_form_submit($form, &$form_state) {
  $name = $form_state['values']['name'];

  // Save to individual config file
  $config = config('openai_seo_advisor.report_type.' . $name);

  $config->set('_config_name', 'openai_seo_advisor.report_type.' . $name);
  $config->set('name', $name);
  $config->set('label', $form_state['values']['label']);
  $config->set('description', $form_state['values']['description']);
  $config->set('prompt', $form_state['values']['prompt']);
  $config->set('status', !empty($form_state['values']['status']) ? 1 : 0);
  $config->save();

  backdrop_set_message(t('Report type saved.'));
  $form_state['redirect'] = 'admin/config/openai/seo/report-types';
}

/**
 * Form constructor for delete report type.
 */
function openai_seo_advisor_report_type_delete_form($form, &$form_state, $name) {
  // Don't allow deleting default types.
  if (in_array($name, array('seo_analysis', 'aeo_analysis'))) {
    backdrop_set_message(t('Cannot delete default report types.'), 'error');
    backdrop_goto('admin/config/openai/seo/report-types');
  }

  // Load from individual config file
  $config = config('openai_seo_advisor.report_type.' . $name);
  $report_type = $config->get();

  if (empty($report_type) || empty($report_type['name'])) {
    backdrop_set_message(t('Report type not found.'), 'error');
    backdrop_goto('admin/config/openai/seo/report-types');
  }

  $form['name'] = array(
    '#type' => 'value',
    '#value' => $name,
  );

  return confirm_form(
    $form,
    t('Are you sure you want to delete the report type %label?', array('%label' => $report_type['label'])),
    'admin/config/openai/seo/report-types',
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Submit handler for delete report type form.
 */
function openai_seo_advisor_report_type_delete_form_submit($form, &$form_state) {
  $name = $form_state['values']['name'];

  // Delete individual config file
  $config = config('openai_seo_advisor.report_type.' . $name);
  $config->delete();

  backdrop_set_message(t('Report type deleted.'));
  $form_state['redirect'] = 'admin/config/openai/seo/report-types';
}
