# OpenAI SEO Advisor
===================

## What does it do?
-----------------
OpenAI SEO Advisor provides a SEO analysis panel on node edit forms for Backdrop CMS. The module performs local, rules-based checks (title length, meta description, word count, headings, internal links, image alt text, etc.) and depends on the contrib `openai` module to produce AI suggestions and full HTML SEO/AEO audits. A configured OpenAI API key is required for normal operation.

For site administrators, OpenAI SEO Advisor provides on-page recommendations that include AI-powered title/description/keyword suggestions and a prioritized audit when the OpenAI integration is configured. The module never auto-applies changes — Apply buttons only set form values client-side or via `form_state`.

## Key features
------------
- Rules-only SEO checks with tunable thresholds and per-check weights (config stored in `openai_seo_advisor.settings`).
- AI suggestions and full audits via the contrib `openai` module are required for the module's AI-driven features (the module uses the `OpenAIApi` class).
- Preview-based analysis: the module prefers generated preview HTML (view mode `full`) for most heuristics and AI inputs.
- AJAX Analyze button and collapsible HTML audit output when AI full-audit is enabled.

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
- Settings storage: `openai_seo_advisor.settings` (Backrop config API via `system_settings_form` binding).
- Admin path: `admin/config/openai/seo` (defined in `.info` and menu implementation).
- Main logic functions:
  - `openai_seo_advisor_analyze()` — rules engine that returns score, status and checks
  - `openai_seo_advisor_generate_ai_suggestions()` — compact JSON suggestions via contrib OpenAI
  - `openai_seo_advisor_generate_ai_audit()` — long-form HTML audit prompt to the AI
  - `openai_seo_advisor_build_preview_html()` — builds preview HTML used for analysis

## Developer notes and conventions
-------------------------------
- The module intentionally performs rule-only checks locally and marks AI as optional; AI calls always check `module_exists('openai')` and the presence of the configured API key.
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
