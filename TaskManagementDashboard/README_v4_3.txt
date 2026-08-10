Tower Shooter Task Board v4.3 — application update

IMPORTANT:
This update intentionally DOES NOT contain TowerShooter_Tasks.json.
Your existing JSON file is the canonical task database and should not be overwritten.

To update:
1. Stop the task-board server if it is running.
2. Copy these files into your existing TaskManagementDashboard folder.
3. Allow Windows to replace the existing application files.
4. Leave your existing TowerShooter_Tasks.json untouched.
5. Double-click Start Task Board.bat.

Changes:
- Task icons are derived from task Type using Unicode escapes, preventing corrupted/mojibake icons.
- "Download Snapshot" is now "Show JSON Snapshot".
- Snapshot JSON appears in a copyable panel at the bottom of the page.
- Added Copy JSON button.
- Fixed automatic project-data loading on page startup.
