# Persistent SEO Reports - Implementation Guide

## Overview

The AI SEO Advisor module now saves SEO analysis reports to the database, allowing users to:
- View historical SEO reports on a dedicated tab
- Track SEO improvements over time
- Keep analysis on the edit form for immediate feedback

## Architecture

### Database Schema

**Table: `ai_seo_report`**
- Stores all SEO analysis reports
- Links to entities (typically nodes)
- Includes serialized report data, score, status, and metadata

### Entity System

**Entity Type: `ai_seo_report`**
- Uses Entity Plus for enhanced entity functionality
- Custom entity class: `AiSEOReport`
- Custom controller: `AiSEOReportController`

## Features Implemented

### 1. Persistent Storage
- Reports are automatically saved when "Analyze SEO" is clicked on node edit forms
- Each report includes:
  - SEO score and status (GREEN/AMBER/RED)
  - All check results
  - AI suggestions (if enabled)
  - Focus keyword used
  - HTML snapshot for reference
  - Timestamp and user who generated it

### 2. Report History Tab
- New "SEO" tab on node pages (`node/%node/seo`)
- Lists all historical reports for that content
- Shows score, status, date, and focus keyword
- Accessible to users with "view ai seo reports" permission

### 3. Individual Report View
- Each report can be viewed at `node/%node/seo/%report_id`
- Shows full report details with metadata
- Breadcrumb navigation back to report list
- Uses the same rendering function as the edit form

### 4. Analysis Form on SEO Tab
- Analysis form appears on the SEO tab
- Generate reports directly from saved content
- Immediate display of results
- Report history shown on same page

## Report Types Configuration

The module now supports multiple report types through configuration:

### Default Report Types

**SEO Analysis** (`seo_analysis`)
- Comprehensive on-page SEO checks
- Title, description, headings, keywords
- Links, images, word count
- **Status: Enabled by default**

**AEO Analysis** (`aeo_analysis`)  
- Answer Engine Optimization checks
- Featured snippet candidates
- Question-style headings
- FAQ and HowTo schema
- **Status: Disabled by default** (toggle in settings)

## Permissions

Three permissions control access:

1. **Analyze content SEO with AI**
   - Run SEO analysis on edit forms
   - Create new reports

2. **View AI SEO reports**
   - View saved reports on content pages
   - Access report history tab

3. **Administer AI SEO Advisor settings**
   - Configure module settings
   - Full access to all features

## Enabling AEO Reports

To enable AEO (Answer Engine Optimization) analysis:

1. Go to **Administration > Configuration > AI > SEO Advisor**
2. Expand **Analysis options** fieldset
3. Check **Enable AEO checks**
4. Optionally check **Include AEO deliverables in AI suggestions**
5. Save configuration

AEO checks include:
- Featured snippet paragraph detection
- Question-style headings (Who/What/When/Where/Why/How)
- Immediate answer paragraphs
- Procedural content step lists
- Schema.org FAQPage and HowTo markup
- Indexability checks

## Usage Workflow

### For Content Editors

1. **Create/Edit and Save Content**
   - Edit a node (on an enabled content type)
   - Fill in content, metatags, etc.
   - **Save the node** (must be saved before analysis)

2. **Navigate to SEO Tab**
   - View the node page
   - Click the **"SEO"** tab in local tasks

3. **Run Analysis**
   - Enter an optional focus keyword
   - Click "Analyze SEO"
   - Report displays immediately below the form

4. **Review Report History**
   - All reports shown in table on the same page
   - Click "View" on any report for full details
   - Track SEO improvements over time

### For SEO Managers

1. **Enable on Content Types**
   - Admin > Config > AI > SEO Advisor
   - Check content types to enable analysis

2. **Configure Checks**
   - Enable/disable specific SEO checks
   - Enable AEO analysis if desired
   - Set thresholds and weights

3. **Review Reports**
   - View reports on any enabled content
   - Track SEO scores over time
   - Identify patterns and improvement areas

## Database Queries

### Get all reports for a node
```php
$reports = db_select('ai_seo_report', 'r')
  ->fields('r')
  ->condition('entity_type', 'node')
  ->condition('entity_id', $nid)
  ->orderBy('created', 'DESC')
  ->execute();
```

### Get latest report for a node
```php
$latest = db_select('ai_seo_report', 'r')
  ->fields('r')
  ->condition('entity_type', 'node')
  ->condition('entity_id', $nid)
  ->orderBy('created', 'DESC')
  ->range(0, 1)
  ->execute()
  ->fetchObject();
```

### Load report entity
```php
$report = entity_load('ai_seo_report', $report_id);
```

## Future Enhancements

Potential additions:

1. **Report Comparison**
   - Compare two reports side-by-side
   - Show score changes over time
   - Highlight improved/declined checks

2. **Scheduled Analysis**
   - Cron job to re-analyze content
   - Email notifications for score changes
   - Automated monitoring

3. **Bulk Operations**
   - Analyze multiple nodes at once
   - Export reports to CSV/PDF
   - Batch delete old reports

4. **Custom Report Types**
   - UI for creating custom report types
   - Custom check definitions
   - Custom AI prompts per report type

5. **Views Integration**
   - Views handlers for report fields
   - Filter content by SEO score
   - Display recent reports in dashboards

## Technical Notes

### Entity Plus Dependency
- Uses Entity Plus (`entity_plus`) module for enhanced entity support
- Provides EntityPlusController for better CRUD operations
- Supports entity metadata wrappers (future enhancement)

### Serialized Data
- `report_data` field stores serialized PHP array
- Contains checks, recommendations, AI suggestions, AEO data
- Can be unserialized for display or processing

### HTML Snapshots
- `html_snapshot` field stores rendered HTML at analysis time
- Useful for debugging or re-analysis
- Can be large - consider cleanup for old reports

## Files Changed/Added

### New Files
- `ai_seo_advisor.install` - Schema and installation
- `includes/ai_seo_report.entity.inc` - Entity class
- `PERSISTENT_REPORTS.md` - This documentation

### Modified Files
- `ai_seo_advisor.info` - Added entity_plus dependency
- `ai_seo_advisor.module` - Added entity hooks, menu items, page callbacks

## Installation

If the module was already enabled before this update:

1. **Disable the module**
   ```bash
   drush dis ai_seo_advisor -y
   ```

2. **Re-enable to run install hooks**
   ```bash
   drush en ai_seo_advisor -y
   ```

3. **Verify schema was created**
   ```bash
   drush sqlq "DESCRIBE ai_seo_report"
   ```

4. **Clear all caches**
   ```bash
   drush cc all
   ```

## Testing

After installation, test the following:

1. ✓ Edit a node on an enabled content type
2. ✓ See SEO Analysis panel
3. ✓ Click "Analyze SEO" button
4. ✓ See report results on edit form
5. ✓ See "Report saved" message with link
6. ✓ View node page - see "SEO" tab
7. ✓ Click "SEO" tab - see report list
8. ✓ Click "View" on a report - see full report
9. ✓ Generate multiple reports - see history
10. ✓ Test permissions with different user roles

## Troubleshooting

### Reports not saving
- Check if Entity Plus is enabled
- Verify database table exists
- Check watchdog for errors

### SEO tab not appearing
- Verify content type is enabled in settings
- Check user permissions
- Clear menu cache

### Entity class not found
- Clear registry: `drush rr`
- Verify autoload hook is present
- Check file path in autoload_info

## Support

For issues or questions:
- Check watchdog logs: `drush wd-show`
- Review error logs
- Check permission settings
