param(
    [Parameter(Mandatory = $true)]
    [string] $AppBuildBin,

    [Parameter(Mandatory = $true)]
    [string] $PayloadDir
)

$ErrorActionPreference = 'Stop'

$resolvedBin = Resolve-Path -LiteralPath $AppBuildBin

if (Test-Path -LiteralPath $PayloadDir) {
    Remove-Item -LiteralPath $PayloadDir -Recurse -Force
}

New-Item -ItemType Directory -Force -Path $PayloadDir | Out-Null

$requiredFiles = @(
    'TrayApp.exe',
    'TrayService.exe'
)

foreach ($file in $requiredFiles) {
    $source = Join-Path $resolvedBin $file
    if (-not (Test-Path -LiteralPath $source)) {
        throw "Required application artifact was not found: $source"
    }
}

Get-ChildItem -LiteralPath $resolvedBin -Force | ForEach-Object {
    Copy-Item -LiteralPath $_.FullName -Destination $PayloadDir -Recurse -Force
}

$payloadFiles = Get-ChildItem -LiteralPath $PayloadDir -File -Recurse
if (-not $payloadFiles) {
    throw "Payload directory is empty: $PayloadDir"
}

Write-Host "Prepared TrayApp installer payload:"
$payloadFiles | ForEach-Object {
    $relative = Resolve-Path -LiteralPath $_.FullName -Relative
    Write-Host "  $relative"
}
