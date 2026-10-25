$ErrorActionPreference = 'Stop'
$libraryPath = Split-Path -Parent $PSScriptRoot
$libraryParent = Split-Path -Parent $libraryPath
$cli = Get-Command arduino-cli -ErrorAction SilentlyContinue
if (-not $cli) { throw 'arduino-cli is not installed; no compilation was run.' }
$core = & $cli.Source core list | Out-String
if ($core -notmatch 'esp32:esp32\s+3\.3\.12') { throw 'Install Espressif esp32:esp32 core 3.3.12 before running this compile matrix.' }
$sketches = Get-ChildItem -LiteralPath (Join-Path $libraryPath 'examples') -Recurse -Filter '*.ino'
foreach ($sketch in $sketches) {
  Write-Host "Compiling $($sketch.FullName)"
  & $cli.Source compile --fqbn esp32:esp32:esp32 --libraries $libraryParent --build-property build.partitions=huge_app $sketch.DirectoryName
  if ($LASTEXITCODE -ne 0) { throw "Compile failed: $($sketch.FullName)" }
}
Write-Host "All $($sketches.Count) sketches compiled with esp32:esp32 3.3.12."

