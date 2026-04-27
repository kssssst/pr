@echo off
setlocal

set "SERVICE_NAME=TrayService"

sc query "%SERVICE_NAME%" >nul 2>&1
if %errorlevel% neq 0 (
    echo Service is not installed.
    exit /b 0
)

echo Stopping service...
powershell -NoProfile -ExecutionPolicy Bypass -Command "try { $e = [System.Threading.EventWaitHandle]::OpenExisting('Global\TrayServiceStopEvent'); $e.Set() | Out-Null; $e.Dispose() } catch { }"
sc stop "%SERVICE_NAME%" >nul 2>&1
timeout /t 5 /nobreak >nul

echo Deleting service...
sc delete "%SERVICE_NAME%"
exit /b %errorlevel%
