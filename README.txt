TrayApp - Windows GUI tray application
=====================================

TrayApp is a single Windows GUI application.

Features:
- tray icon on startup
- left click opens the main window
- right click opens the context menu
- context menu commands: Open and Exit
- single running instance via named mutex
- tray icon recovery after taskbar recreation

Build on Windows:

cmake -S . -B build -G "Visual Studio 17 2022" -A ARM64
cmake --build build --config Release

Result:

build\bin\TrayApp.exe

GitHub Actions builds TrayApp.exe on every push to any branch.
