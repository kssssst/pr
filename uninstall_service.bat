@echo off
REM Uninstall TrayApp Service

setlocal enabledelayedexpansion

REM Проверяем права администратора
net session >nul 2>&1
if %errorLevel% neq 0 (
    echo Error: This script requires administrator privileges!
    echo Please run as administrator.
    pause
    exit /b 1
)

REM Проверяем, установлен ли сервис
sc query TrayAppService >nul 2>&1
if !errorLevel! neq 0 (
    echo Error: Service TrayAppService not found.
    echo Nothing to uninstall.
    pause
    exit /b 1
)

REM Останавливаем сервис
echo Stopping TrayApp Service...
net stop TrayAppService
timeout /t 2 /nobreak

REM Удаляем сервис
echo Removing TrayApp Service...
sc delete TrayAppService

if !errorLevel! equ 0 (
    echo Service removed successfully!
) else (
    echo Error: Failed to remove service. Error code: !errorLevel!
    pause
    exit /b 1
)

echo.
echo Uninstall complete. Press any key to exit.
pause
exit /b 0
