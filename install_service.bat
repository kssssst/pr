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
    powershell -NoProfile -ExecutionPolicy Bypass -Command "try { $e = [System.Threading.EventWaitHandle]::OpenExisting('Global\TrayServiceStopEvent'); $e.Set() | Out-Null; $e.Dispose() } catch { }"
    sc stop "%SERVICE_NAME%" >nul 2>&1
    timeout /t 5 /nobreak >nul

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

echo Updating service permissions...
sc sdset "%SERVICE_NAME%" D:(A;;CCDCLCSWRPWPDTLOCRSDRCWDWO;;;SY)(A;;CCDCLCSWRPWPDTLOCRSDRCWDWO;;;BA)(A;;LCRP;;;IU)(A;;LCRP;;;AU) >nul

echo Starting service...
sc start "%SERVICE_NAME%"
exit /b %errorlevel%
