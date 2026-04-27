@echo off
setlocal

set "SERVICE_NAME=TrayService"
set "SERVICE_EXE=%~dp0TrayService.exe"

if not exist "%SERVICE_EXE%" (
    echo TrayService.exe not found: "%SERVICE_EXE%"
    exit /b 1
)

sc query "%SERVICE_NAME%" >nul 2>&1
if %errorlevel% equ 0 (
    echo Stopping existing service...
    sc stop "%SERVICE_NAME%" >nul 2>&1
    timeout /t 3 /nobreak >nul

    echo Service already exists. Updating binary path...
    sc config "%SERVICE_NAME%" binPath= "\"%SERVICE_EXE%\"" start= auto
) else (
    echo Creating service...
    sc create "%SERVICE_NAME%" binPath= "\"%SERVICE_EXE%\"" start= auto DisplayName= "Tray Service"
)

if errorlevel 1 (
    echo Failed to create or configure service.
    exit /b 1
)

echo Starting service...
sc start "%SERVICE_NAME%"
exit /b %errorlevel%
