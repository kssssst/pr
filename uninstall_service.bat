@echo off
setlocal

set "SERVICE_NAME=TrayService"

sc query "%SERVICE_NAME%" >nul 2>&1
if %errorlevel% neq 0 (
    echo Service is not installed.
    exit /b 0
)

echo Stopping service...
sc stop "%SERVICE_NAME%" >nul 2>&1

echo Deleting service...
sc delete "%SERVICE_NAME%"
exit /b %errorlevel%
