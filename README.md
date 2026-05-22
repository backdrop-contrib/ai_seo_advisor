# AI SEO Advisor
===================

## What does it do?
-----------------
AI SEO Advisor adds an SEO Reports tab to node pages in Backdrop CMS. The module performs local, rules-based checks (title length, meta description, word count, headings, internal links, image alt text, etc.) and depends on the contrib `ai` module to produce AI suggestions. A configured AI provider is required for AI-driven features.

For site administrators, AI SEO Advisor provides on-page recommendations that include AI-powered suggestions (titles, descriptions, keywords, prioritized tips) tailored to configurable Report Types. The module never auto-applies changes — all AI output is advisory only.

## Key features
------------
- Rules-based SEO checks with tunable thresholds and per-check weights (config stored in `ai_seo_advisor.settings`).
- AI suggestions via the contrib `ai` module using `ai_chat()` — any configured AI provider works.
- Report Types: named analysis profiles with custom prompts, managed at `admin/config/ai/ai-seo/report-types`.
- Saved reports: each analysis run is stored as an `ai_seo_report` entity, viewable and deletable from the SEO tab.
- AJAX Analyze button on the SEO tab (`node/%node/ai-seo`).
- Optional custom system prompt and `log_ai_input` debug toggle in settings.

## Getting started
---------------
1. Place `ai_seo_advisor` in your Backdrop `modules` directory.
2. Enable the module via Admin -> Modules or with Drush.
3. Configure the module at Admin -> Configuration -> AI -> AI SEO Advisor (`admin/config/ai/ai-seo`).
4. Install and configure the contrib `ai` module with at least one AI provider and API key.
5. Enable the module on the desired content types using the "Enable on these content types" setting.
6. Optionally create Report Types at `admin/config/ai/ai-seo/report-types` to customize the AI analysis prompt.

## Requirements
------------
- Backdrop CMS 1.x
- The contrib `ai` module with at least one configured provider and API key.
- DOMDocument (PHP ext-dom) is used when available; regex fallbacks handle HTML parsing in constrained environments.

## Important config keys and places to look
--------------------------------------
- Settings form: `ai_seo_advisor_settings_form()` in `ai_seo_advisor.admin.inc` — controls enabled content types, AI model, custom system prompt, and `log_ai_input`.
- Settings storage: `ai_seo_advisor.settings` (Backdrop config API).
- Admin path: `admin/config/ai/ai-seo` (settings) and `admin/config/ai/ai-seo/report-types` (report types).
- Main logic functions:
  - `ai_seo_advisor_analyze()` in `ai_seo_advisor.reports.inc` — rules engine, returns score, status, and per-check results.
  - `ai_seo_advisor_generate_ai_suggestions()` in `ai_seo_advisor.reports.inc` — sends content to AI via `ai_chat()`, returns structured suggestions.
  - `ai_seo_advisor_analyze_form()` in `ai_seo_advisor.pages.inc` — AJAX form on the SEO tab.
  - `ai_seo_advisor_reports_page()` in `ai_seo_advisor.pages.inc` — renders saved reports list on the SEO tab.

## Developer notes and conventions
-------------------------------
- AI calls are gated by `module_exists('ai')`. Rules-only checks run regardless.
- AI integration uses `ai_chat()` from the contrib `ai` module — no provider-specific class is called directly.
- Report Types drive the AI prompt per analysis run. A fallback prompt is used when no report type is selected.
- Max tokens for AI calls are hardcoded at 3000 (matches Drupal ai_seo module behavior).
- Parsing strategy: prefer `DOMDocument` where available, with regex fallbacks.

## Security and privacy
--------------------
- Do not enable `log_ai_input` on production sites — it may log sanitized content containing private information. Enable only on development when troubleshooting.
- AI features send (sanitized) content to the configured AI provider endpoint. Ensure you have an appropriate privacy policy and user consent before enabling on production.

## Installation

- Install this module using the official [Backdrop CMS instructions](https://backdropcms.org/user-guide/modules).

## Issues

Bugs and feature requests should be reported in the [Issue Queue](https://github.com/backdrop-contrib/ai_seo_advisor/issues).


## Current Maintainer

[Justin Keiser](https://github.com/keiserjb)

## Credits
-------

Inspired by the [AI SEO Analyzer](https://www.drupal.org/project/ai_seo) module in Drupal.

- Developed with AI assistance.

## License

This project is GPL v2 software. See the LICENSE.txt file in this directory for complete text.
