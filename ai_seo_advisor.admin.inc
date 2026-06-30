<?php
/**
 * @file
 * Administrative forms for AI SEO Advisor module.
 */


function ai_seo_advisor_settings_form($form, &$form_state) {
  // Bundle enablement.
  $types = node_type_get_types();
  $options = [];
  foreach ($types as $type) { $options[$type->type] = check_plain($type->name); }

  $enabled_bundles = config_get('ai_seo_advisor.settings', 'enabled_bundles');
  $enabled_bundles = is_array($enabled_bundles) ? $enabled_bundles : [];

  // Normalize to array of selected keys for FAPI checkboxes.
  $default_selected = [];
  foreach ($enabled_bundles as $bundle_key => $is_enabled) {
    if (!empty($is_enabled)) { $default_selected[$bundle_key] = $bundle_key; }
  }

  $form['enabled_bundles'] = [
    '#type' => 'checkboxes',
    '#title' => t('Enable on these content types'),
    '#options' => $options,
    '#default_value' => $default_selected,
    '#description' => t('The SEO Analysis panel will appear on node edit forms for these content types.'),
    '#element_validate' => ['ai_seo_advisor_enabled_bundles_validate'],
  ];

  // AI Options.
  $opts_cfg = is_array(config_get('ai_seo_advisor.settings', 'options')) ? config_get('ai_seo_advisor.settings', 'options') : [];
  $form['options'] = [
    '#type' => 'fieldset',
    '#title' => t('AI settings'),
    '#tree' => TRUE,
  ];

  $form['options']['system_prompt_fieldset'] = [
    '#type' => 'fieldset',
    '#title' => t('System prompt'),
    '#collapsible' => TRUE,
    '#collapsed' => TRUE,
  ];

  $default_system_prompt = "You are an SEO analysis expert specialized in evaluating HTML content from an SEO perspective. Your role is to provide a comprehensive audit, including clear suggestions and improvements for each aspect of SEO best practices. You should aim to be thorough, precise, and provide examples wherever possible to illustrate your points. Provide your report with HTML markup for headings, paragraphs, lists, etc.";

  $form['options']['system_prompt_fieldset']['default_system_prompt'] = [
    '#type' => 'textarea',
    '#title' => t('Default system prompt'),
    '#value' => $default_system_prompt,
    '#disabled' => TRUE,
    '#description' => t('This is the default system prompt that defines the AI\'s role and behavior. It cannot be edited here.'),
    '#rows' => 4,
  ];

  $form['options']['system_prompt'] = [
    '#type' => 'textarea',
    '#title' => t('Custom system prompt (optional)'),
    '#default_value' => isset($opts_cfg['system_prompt']) ? $opts_cfg['system_prompt'] : '',
    '#description' => t('Override the default system prompt. Leave empty to use the default. This defines how the AI should behave and respond.'),
    '#rows' => 6,
  ];

  // Model selector: use ai module helper for all chat models from all providers.
  $model_options = ai_get_all_chat_models();

  $model_select_disabled = empty($model_options);
  $default_model_value = ((is_array(config_get('ai_seo_advisor.settings', 'options')) && isset(config_get('ai_seo_advisor.settings', 'options')['ai_model']) && config_get('ai_seo_advisor.settings', 'options')['ai_model'] !== '') ? config_get('ai_seo_advisor.settings', 'options')['ai_model'] : (config_get('ai.settings', 'default_text_model')));
  $model_description = t('Select which text model to use for AI suggestions.');
  if ($model_select_disabled) {
    $model_description .= ' ' . t('No models were discovered. Configure an AI provider at admin/config/ai/settings to enable model discovery.');
  }
  $form['options']['ai_model'] = [
    '#type' => 'select',
    '#title' => t('AI model'),
    '#options' => $model_options,
    '#default_value' => $default_model_value,
    '#description' => $model_description,
    '#attributes' => ['class' => ['ai-seo-model-select']],
    '#disabled' => $model_select_disabled,
  ];
  // ai_max_tokens is hardcoded at 3000 (same as Drupal ai_seo module)
  // No user setting needed.

  $form['options']['report_type_info'] = [
    '#type' => 'markup',
    '#markup' => '<div class="messages info">' . t('Report type prompts are configured at <a href="@link">Report Types</a>. Each report type has its own detailed analysis prompt.', ['@link' => url('admin/config/ai/ai-seo/report-types')]) . '</div>',
  ];

  $form['options']['log_ai_input'] = [
    '#type' => 'checkbox',
    '#title' => t('Log AI input (debug)'),
    '#default_value' => (is_array(config_get('ai_seo_advisor.settings', 'options')) && !empty(config_get('ai_seo_advisor.settings', 'options')['log_ai_input'])) ? 1 : 0,
    '#description' => t('Log sanitized content sent to AI for debugging. Use only on development sites.'),
  ];

  $form['#submit'][] = 'ai_seo_advisor_settings_form_submit';
  // Bind this settings form to Backdrop config so values persist in
  // ai_seo_advisor.settings instead of legacy variables.
  $form['#config'] = 'ai_seo_advisor.settings';
  return system_settings_form($form);
}

/**
 * Element validator for enabled_bundles checkboxes.
 */
function ai_seo_advisor_enabled_bundles_validate(&$element, &$form_state, $form) {
  $values = isset($form_state['values']['enabled_bundles']) ? $form_state['values']['enabled_bundles'] : [];
  $normalized = [];
  foreach ($values as $bundle => $val) {
    $normalized[$bundle] = !empty($val) ? 1 : 0;
  }
  $form_state['values']['enabled_bundles'] = $normalized;
}

/**
 * Settings submit: normalize nested arrays and clamp ranges.
 */
function ai_seo_advisor_settings_form_submit($form, &$form_state) {
  // Ensure options exist and set defaults.
  if (empty($form_state['values']['options']) || !is_array($form_state['values']['options'])) {
    $form_state['values']['options'] = [];
  }
  $opts = & $form_state['values']['options'];
  $opts += [
    'ai_model' => (string) (config_get('ai.settings', 'default_text_model')),
  ];
  $opts['ai_model'] = isset($opts['ai_model']) && is_string($opts['ai_model']) && trim($opts['ai_model']) !== '' ? trim($opts['ai_model']) : (config_get('ai.settings', 'default_text_model'));

  // Normalize system_prompt.
  if (isset($opts['system_prompt']) && is_string($opts['system_prompt'])) {
    $sp = trim($opts['system_prompt']);
    if (backdrop_strlen($sp) > 2000) { $sp = backdrop_substr($sp, 0, 2000) . '…'; }
    $opts['system_prompt'] = $sp;
  }
  else {
    $opts['system_prompt'] = '';
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
function ai_seo_advisor_report_types_list() {
  $output = [];

  // Load all report type configs
  $report_types = ai_seo_advisor_get_report_types();

  if (empty($report_types)) {
    $output['empty'] = [
      '#type' => 'markup',
      '#markup' => '<p>' . t('No report types defined yet.') . '</p>',
    ];
    return $output;
  }

  // Build table.
  $header = [
    t('Label'),
    t('Machine Name'),
    t('Description'),
    t('Status'),
    t('Operations'),
  ];

  $rows = [];
  foreach ($report_types as $name => $type) {
    $label = isset($type['label']) ? $type['label'] : $name;
    $description = isset($type['description']) ? $type['description'] : '';
    $status = !empty($type['status']) ? t('Enabled') : t('Disabled');

    $operations = [];
    $operations[] = l(t('Edit'), 'admin/config/ai/ai-seo/report-types/' . $name . '/edit');
    if (!in_array($name, ['full', 'aeo'])) {
      $operations[] = l(t('Delete'), 'admin/config/ai/ai-seo/report-types/' . $name . '/delete');
    }

    $rows[] = [
      check_plain($label),
      check_plain($name),
      check_plain($description),
      $status,
      implode(' | ', $operations),
    ];
  }

  $output['table'] = [
    '#theme' => 'table',
    '#header' => $header,
    '#rows' => $rows,
  ];

  return $output;
}

/**
 * Form constructor for add/edit report type.
 */
function ai_seo_advisor_report_type_form($form, &$form_state, $name = NULL) {
  $is_new = empty($name);

  if (!$is_new) {
    // Load from individual config file
    $config = config('ai_seo_advisor.report_type.' . $name);
    $report_type = $config->get();

    if (empty($report_type) || empty($report_type['name'])) {
      backdrop_set_message(t('Report type not found.'), 'error');
      backdrop_goto('admin/config/ai/ai-seo/report-types');
    }
  }
  else {
    $report_type = [
      'name' => '',
      'label' => '',
      'description' => '',
      'prompt' => '',
      'status' => 1,
    ];
  }

  $form['name'] = [
    '#type' => 'machine_name',
    '#title' => t('Machine name'),
    '#default_value' => isset($report_type['name']) ? $report_type['name'] : '',
    '#maxlength' => 64,
    '#required' => TRUE,
    '#machine_name' => [
      'exists' => 'ai_seo_advisor_report_type_exists',
    ],
    '#disabled' => !$is_new,
  ];

  $form['label'] = [
    '#type' => 'textfield',
    '#title' => t('Label'),
    '#default_value' => isset($report_type['label']) ? $report_type['label'] : '',
    '#required' => TRUE,
    '#maxlength' => 255,
  ];

  $form['description'] = [
    '#type' => 'textarea',
    '#title' => t('Description'),
    '#default_value' => isset($report_type['description']) ? $report_type['description'] : '',
    '#rows' => 3,
  ];

  $form['prompt'] = [
    '#type' => 'textarea',
    '#title' => t('AI Prompt'),
    '#description' => t('The prompt sent to the AI for this report type. Use detailed instructions.'),
    '#default_value' => isset($report_type['prompt']) ? $report_type['prompt'] : '',
    '#rows' => 20,
  ];

  $form['status'] = [
    '#type' => 'checkbox',
    '#title' => t('Enabled'),
    '#default_value' => !empty($report_type['status']),
  ];

  $form['weight'] = [
    '#type' => 'weight',
    '#title' => t('Weight'),
    '#default_value' => isset($report_type['weight']) ? $report_type['weight'] : 0,
  ];

  $form['actions'] = ['#type' => 'actions'];
  $form['actions']['submit'] = [
    '#type' => 'submit',
    '#value' => $is_new ? t('Create report type') : t('Save report type'),
  ];
  $form['actions']['cancel'] = [
    '#type' => 'link',
    '#title' => t('Cancel'),
    '#href' => 'admin/config/ai/ai-seo/report-types',
  ];

  return $form;
}

/**
 * Submit handler for report type form.
 */
function ai_seo_advisor_report_type_form_submit($form, &$form_state) {
  $name = $form_state['values']['name'];

  // Save to individual config file
  $config = config('ai_seo_advisor.report_type.' . $name);

  $config->set('_config_name', 'ai_seo_advisor.report_type.' . $name);
  $config->set('name', $name);
  $config->set('label', $form_state['values']['label']);
  $config->set('description', $form_state['values']['description']);
  $config->set('prompt', $form_state['values']['prompt']);
  $config->set('status', !empty($form_state['values']['status']) ? 1 : 0);
  $config->set('weight', isset($form_state['values']['weight']) ? (int) $form_state['values']['weight'] : 0);
  $config->save();

  backdrop_set_message(t('Report type saved.'));
  $form_state['redirect'] = 'admin/config/ai/ai-seo/report-types';
}

/**
 * Form constructor for delete report type.
 */
function ai_seo_advisor_report_type_delete_form($form, &$form_state, $name) {
  // Don't allow deleting default types.
  if (in_array($name, ['full', 'aeo'])) {
    backdrop_set_message(t('Cannot delete default report types.'), 'error');
    backdrop_goto('admin/config/ai/ai-seo/report-types');
  }

  // Load from individual config file
  $config = config('ai_seo_advisor.report_type.' . $name);
  $report_type = $config->get();

  if (empty($report_type) || empty($report_type['name'])) {
    backdrop_set_message(t('Report type not found.'), 'error');
    backdrop_goto('admin/config/ai/ai-seo/report-types');
  }

  $form['name'] = [
    '#type' => 'value',
    '#value' => $name,
  ];

  return confirm_form(
    $form,
    t('Are you sure you want to delete the report type %label?', ['%label' => $report_type['label']]),
    'admin/config/ai/ai-seo/report-types',
    t('This action cannot be undone.'),
    t('Delete'),
    t('Cancel')
  );
}

/**
 * Submit handler for delete report type form.
 */
function ai_seo_advisor_report_type_delete_form_submit($form, &$form_state) {
  $name = $form_state['values']['name'];

  // Delete individual config file
  $config = config('ai_seo_advisor.report_type.' . $name);
  $config->delete();

  backdrop_set_message(t('Report type deleted.'));
  $form_state['redirect'] = 'admin/config/ai/ai-seo/report-types';
}
