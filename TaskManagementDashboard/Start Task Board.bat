@echo off
setlocal
cd /d "%~dp0"

echo ==========================================
echo   Tower Shooter Task Board
echo ==========================================
echo.

where powershell >nul 2>nul
if not %errorlevel%==0 (
    echo ERROR: Windows PowerShell could not be found.
    echo.
    pause
    exit /b 1
)

powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0server.ps1"
set EXITCODE=%errorlevel%

if not "%EXITCODE%"=="0" (
    echo.
    echo The task board server stopped with exit code %EXITCODE%.
    echo See the error above.
    echo.
    pause
)

endlocal
