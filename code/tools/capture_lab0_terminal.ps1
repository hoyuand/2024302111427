$ErrorActionPreference = "Stop"
$repo = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
Set-Location -LiteralPath $repo
$wslRepo = "/mnt/c/" + $repo.Substring(3).Replace("\", "/")

Write-Host "Lab0 ENVIRONMENT CHECK"
Write-Host "Path: $repo"
Write-Host ""
Write-Host '$ python3 code/tools/preflight.py'
wsl.exe bash -lc "cd '$wslRepo' && python3 code/tools/preflight.py" 2>&1
Write-Host ""
Write-Host "Direct WSL2 environment check output."
Read-Host "Press Enter to close"
