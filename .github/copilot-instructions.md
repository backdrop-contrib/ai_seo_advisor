## Goal
Help contributors and automated agents be productive in the OpenAI SEO Advisor module by documenting the big-picture architecture, key integration points, developer workflows, and project-specific conventions.

## Quick context (big picture)
- This is a Backdrop CMS module that adds a Yoast-like SEO analysis panel to node edit forms.
- Rules-based checks (title, description, word count, headings, internal links, image alt) run locally; AI-driven suggestions and full audits require the contrib `openai` module and a configured API key.
- Analysis prefers rendered preview HTML (view mode `full`) for heuristics and AI inputs — see `openai_seo_advisor_build_preview_html()`.

## Where to look first (high signal files)
- `openai_seo_advisor.module` — entire implementation; read top-to-bottom for flows: settings form, `hook_form_alter`, analyze submit, AI helpers, and renderers.
- `openai_seo_advisor.info` — module metadata and admin path: `admin/config/openai/seo`.
- `README.md` — user-oriented install/requirements notes (AI integration is required).

## Contract (what code expects)
- Inputs: node edit `form`/`form_state` (preview HTML or submitted values), optional focus keyword.
- Outputs: analysis array with `score`, `status`, `checks[]`, optional `ai` suggestions or `ai_markdown_audit` HTML.
- Error modes: preview rendering may fail (module returns an error message in result), AI calls return NULL on failure and are logged to watchdog.

## Key integration points & patterns to preserve
- Settings stored via Backdrop config `openai_seo_advisor.settings` (settings form bound with `#config`).
- AI calls are gated: always check `module_exists('openai')` and presence of API key (see `openai_seo_advisor_generate_ai_suggestions()` / `openai_seo_advisor_generate_ai_audit()`).
- The module never auto-applies remote suggestions to storage — Apply handlers only update `form_state` or form values.

## Developer workflows & debugging tips
- To reproduce Analyze flow: open a node edit form for an enabled content type, click the Analyze button (AJAX path triggers `openai_seo_advisor_analyze_submit`).
- Watchdog logs: the module logs AI failures and (optionally) a truncated sanitized HTML when `options.log_ai_input` is enabled. Check system logs for `openai_seo_advisor` entries.
- If `log_ai_input` is enabled, full HTML may be saved via `file_save_data()` to the public/private files path — inspect `public://openai_seo_advisor_ai_input_*.html`.

## Tests & safe development (how to simulate AI)
- The module calls `OpenAIApi::chat()` from contrib `openai`. To run offline or unit tests, mock or stub `OpenAIApi` methods or set `module_exists('openai')` to false to exercise rules-only paths.
- Important: preview HTML length is truncated before being sent to AI (`openai_seo_advisor_generate_ai_audit` trims to ~12k chars). When testing, use small pages or truncate inputs to avoid token issues.

## Project-specific conventions & gotchas
- Prefer preview HTML as source. Many heuristics rely on rendered HTML (headings, images, links). If preview rendering fails, analysis will fall back to collected text and may return an error message.
- Parsing strategy: use `DOMDocument` when available; regex fallbacks exist — be cautious when changing parsing logic.
- Model selection: `openai_seo_advisor_get_model_options()` tries contrib OpenAI first then falls back to a static list. Do not assume a single model is always available.

## Small checklist for PRs affecting behavior
- Update `README.md` if public behavior (AI required, logging, file output) changes.
- Ensure AI calls remain gated by `module_exists('openai')` and API key checks.
- Preserve that the module doesn't auto-apply AI suggestions to DB; UI Apply buttons must remain explicit.

If anything here is unclear or you want more detail (example inputs, sample `form_state`, or a short unit test), tell me which area to expand. 
