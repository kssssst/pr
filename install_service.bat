@echo off
REM Install TrayApp Service
REM Требуются права администратора

setlocal enabledelayedexpansion

REM Проверяем права администратора
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo Error: This script requires administrator privileges!
    echo Please run as administrator.
    pause
    exit /b 1
)

REM Получаем полный путь к сервису
set "SERVICE_PATH=%~dp0TrayService.exe"

if not exist "!SERVICE_PATH!" (
    echo Error: TrayService.exe not found at !SERVICE_PATH!
    pause
    exit /b 1
)

REM Проверяем, установлен ли уже сервис
sc query TrayAppService >nul 2>&1
if !errorLevel! equ 0 (
    echo Service is already installed. Removing old version...
    sc delete TrayAppService
    timeout /t 2 /nobreak
)

REM Устанавливаем сервис
echo Installing TrayApp Service...
sc create TrayAppService binPath= "!SERVICE_PATH!" start= auto DisplayName= "TrayApp Service"

if !errorLevel! equ 0 (
    echo Service installed successfully!
    echo Starting service...
    net start TrayAppService
    if !errorLevel! equ 0 (
        echo Service started successfully!
    ) else (
        echo Warning: Service installation succeeded but startup failed.
        echo You may need to start it manually.
    )
) else (
    echo Error: Failed to install service. Error code: !errorLevel!
    pause
    exit /b 1
)

echo.
echo Installation complete. Press any key to exit.
pause
exit /b 0
