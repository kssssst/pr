# TrayApp Installer

This folder is a standalone WiX-based installer project for TrayApp.

The MSI installs every file from `src/payload`, registers `TrayService` as an automatic Windows service, starts it after install, stops it before uninstall by signaling `Global\TrayServiceStopEvent`, removes the service from the Service Control Manager, and removes installed application files.

## Dependencies

The current TrayApp build links the MSVC runtime statically and uses Windows platform APIs only, so there are no third-party runtime installers to chain at the moment. If future builds add runtime dependencies such as Qt, Windows App SDK, .NET, or VC++ redistributables, add them as WiX Burn packages or MSI merge modules so Windows Installer reference counting can avoid removing shared dependencies still used by other applications.

## Local build on Windows ARM64

From the repository root containing the application source:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A ARM64 -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release

.\TrayAppInstaller\scripts\Prepare-Payload.ps1 -AppBuildBin .\build\bin -PayloadDir .\TrayAppInstaller\src\payload
.\TrayAppInstaller\scripts\Generate-WixPayload.ps1 -PayloadDir .\TrayAppInstaller\src\payload -OutputPath .\TrayAppInstaller\src\Payload.generated.wxs

dotnet build .\TrayAppInstaller\src\TrayAppInstaller.wixproj -c Release -p:ProductVersion=1.0.0
```

The MSI is produced under `TrayAppInstaller\src\bin\Release`.

## CI

`.github/workflows/build.yml` is intentionally inside this standalone folder. When this folder is moved to a repository or branch that contains only the installer, keep it at the repository root.

The workflow builds TrayApp from the current repository by default, generates the WiX payload file, builds the MSI, and uploads it as `TrayAppInstaller-ARM64`.

When this installer lives in a separate repository, configure one of these and the workflow will perform an extra checkout into `app-source`:

1. Set repository variables `APP_SOURCE_REPOSITORY` and optionally `APP_SOURCE_REF`.
2. Or start the workflow manually with `app_repository` and `app_ref`.

Default behavior:

- If no external source repository is configured, CMake builds from the current repository root.
- If an external source repository is configured, the workflow checks it out into `app-source` and builds from there.
