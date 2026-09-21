param(
    [string]$Title = "Lab1-REAL-FINAL",
    [string]$Output = "doc/lab1/images/lab1-terminal-run.png"
)

$ErrorActionPreference = "Stop"
$window = Get-Process WindowsTerminal | Where-Object { $_.MainWindowTitle -eq $Title } | Select-Object -First 1
if (-not $window) {
    throw "Windows Terminal window not found: '$Title'."
}

$outputPath = [System.IO.Path]::GetFullPath($Output)
Add-Type -Path "$env:WINDIR\Microsoft.NET\Framework64\v4.0.30319\System.Drawing.dll"
Add-Type -Path (Join-Path $PSScriptRoot "TerminalWindowCapture.cs") -ReferencedAssemblies "$env:WINDIR\Microsoft.NET\Framework64\v4.0.30319\System.Drawing.dll"

[TerminalWindowCapture]::ShowWindow($window.MainWindowHandle, 3)
Start-Sleep -Milliseconds 300
[TerminalWindowCapture]::Save($window.MainWindowHandle, $outputPath)
Write-Host "Saved: $outputPath"
