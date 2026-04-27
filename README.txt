TrayApp + TrayService
=====================

Build:

cmake -S . -B build -G "Visual Studio 17 2022" -A ARM64
cmake --build build --config Release

Artifacts:

build\bin\TrayApp.exe
build\bin\TrayService.exe

Install the service as administrator:

install_service.bat

The GUI communicates with the service through Windows RPC over local ncalrpc
transport (ALPC). The service owns GUI lifetime and starts TrayApp.exe in user
terminal sessions.
