# OpenAI SEO Advisor
===================

## What does it do?
-----------------
OpenAI SEO Advisor provides a SEO analysis panel on node edit forms for Backdrop CMS. Checks are defined by config-driven report types (see `config/openai_seo_advisor.report_type.*.json`); many checks and full audits are generated via AI-driven report templates and therefore require the contrib `openai` module and a configured OpenAI API key.

For site administrators, OpenAI SEO Advisor provides on-page recommendations that include AI-powered title/description/keyword suggestions and a prioritized audit when the OpenAI integration is configured. The module never auto-applies changes — Apply buttons only set form values client-side or via `form_state`.

## Key features
------------
- Config-driven report types and checks with tunable thresholds and per-check weights (config stored in `openai_seo_advisor.settings`). Many checks and full audits are implemented via report-type templates and require the contrib `openai` module; the module uses the `OpenAIApi` class for AI-driven checks.
- Note: this module no longer provides a separate rules-only analysis path — report types drive the checks, and several report types rely on AI to produce results.
- Preview-based analysis: the module prefers generated preview HTML (view mode `full`) for most heuristics and AI inputs.
- AJAX Analyze button and collapsible HTML audit output when AI full-audit is enabled.

Config files
------------
- Report types are defined in JSON files under the `config/` directory as `openai_seo_advisor.report_type.*.json`. Recent changes add or modify several report-type definitions — edit or add these JSONs to change the available report/audit types.
- The current repository includes the following report-type files in `config/`:
  - `openai_seo_advisor.report_type.full.json`
  - `openai_seo_advisor.report_type.headings_and_structure.json`
  - `openai_seo_advisor.report_type.link_analysis.json`
  - `openai_seo_advisor.report_type.natural_language.json`
  - `openai_seo_advisor.report_type.schema_org_markup.json`
  - `openai_seo_advisor.report_type.topic_authority.json`
- Module-wide settings (schema and defaults) are stored in `config/openai_seo_advisor.settings.json`. When you add or change report types, ensure the settings schema and any UI lists that enumerate reports are updated accordingly.

Getting started
---------------
1. Place `openai_seo_advisor` in your Backdrop `modules` directory.
2. Enable the module via Admin -> Modules or with Drush.
3. Configure the module at Admin -> Configuration -> Open AI -> OpenAI SEO Advisor (`admin/config/openai/seo`).
4. Install and configure the contrib `openai` module and set a valid API key; AI integration is required for the module's main features.
5. Enable the module on the desired content types using the "Enable on these content types" setting.

## Requirements
------------
- Backdrop CMS 1.x
- This module requires the contrib `openai` module and a valid API key; AI features are an integral part of the module's functionality.
- DOMDocument (PHP ext-dom) is used when available; the code provides regex fallbacks for HTML parsing.

## Important config keys and places to look
--------------------------------------
- Settings form: `openai_seo_advisor_settings_form()` (defined in `openai_seo_advisor.module`) — see defaults for thresholds, weights and `options` like `ai_model`, `ai_max_tokens`, and `ai_full_audit_mode`.
- Settings storage: `openai_seo_advisor.settings` (Backdrop config API via `system_settings_form` binding).
- Admin path: `admin/config/openai/seo` (defined in `.info` and menu implementation).
- Main logic functions:
  - `openai_seo_advisor_analyze()` — orchestration function that selects the configured report type, runs the report's checks (which may include AI calls) and returns the report object (score, status, checks, etc.)
  - `openai_seo_advisor_generate_ai_suggestions()` — generate AI-driven suggestions and HTML report content for a selected report type (returns an array with an `html_report` key when available).
  - `openai_seo_advisor_build_preview_html()` — builds preview HTML used for analysis

## Developer notes and conventions
-------------------------------
- The module's checks are driven by report-type JSONs; many reports produce results via AI. AI calls are still gated: always check `module_exists('openai')` and presence of an API key before attempting AI-driven reports. The module treats AI output as suggestions only; Apply handlers only update `form_state` or form values.
- AI outputs are treated as suggestions only. The Apply AJAX handlers set form values or update `form_state` — nothing is persisted without author action.
- The module logs sanitized AI input only when `options.log_ai_input` is enabled; this is intended for development sites only.
- Parsing strategy: prefer `DOMDocument` where available, but use regex fallbacks so analysis still works in constrained environments.

## Security and privacy
--------------------
- Do not enable `log_ai_input` or save sanitized HTML on production sites — it may expose private content. Only enable on development when troubleshooting.
- AI features require sending (sanitized) content to the configured OpenAI endpoint. Ensure you have an acceptable privacy policy and appropriate consent before enabling on production.

## Installation

- Install this module using the official [Backdrop CMS instructions](https://backdropcms.org/user-guide/modules).

## Issues

Bugs and feature requests should be reported in the [Issue Queue](https://github.com/backdrop-contrib/openai_seo_advisor/issues).


## Current Maintainer

[Justin Keiser](https://github.com/keiserjb)

## Credits
-------

Inspired by the [AI SEO Analyzer](https://www.drupal.org/project/ai_seo) module in Drupal.

## License

This project is GPL v2 software. See the LICENSE.txt file in this directory for complete text.
