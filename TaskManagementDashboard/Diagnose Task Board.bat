@echo off
setlocal
cd /d "%~dp0"

echo Tower Shooter Task Board - Diagnostic Launcher
echo.
echo Current folder:
cd
echo.

echo PowerShell version:
powershell.exe -NoProfile -Command "$PSVersionTable.PSVersion.ToString()"
echo.

echo Testing task-board server script syntax...
powershell.exe -NoProfile -ExecutionPolicy Bypass -Command "& { [void][scriptblock]::Create((Get-Content -Raw '%~dp0server.ps1')); Write-Host 'server.ps1 parsed successfully.' }"
echo.

echo Starting server...
powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0server.ps1"

echo.
echo Server process ended.
pause
endlocal
