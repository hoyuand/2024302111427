$ErrorActionPreference = "Stop"
$repo = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
Set-Location -LiteralPath $repo
$wslRepo = "/mnt/c/" + $repo.Substring(3).Replace("\", "/")

Write-Host "Lab1 QEMU REAL RUN"
Write-Host "Path: $repo"
Write-Host ""
Write-Host '$ qemu-system-riscv64 -machine virt -bios none -kernel code/kernel/kernel -nographic -smp 1'
wsl.exe bash -lc "cd '$wslRepo' && timeout -s KILL 1.2s qemu-system-riscv64 -machine virt -bios none -kernel code/kernel/kernel -nographic -smp 1" 2>&1
Write-Host ""
Write-Host "Direct QEMU virt single-core terminal output."
Read-Host "Press Enter to close"
