$ErrorActionPreference = "Stop"
$repo = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path
$wslRepo = "/mnt/c/" + $repo.Substring(3).Replace("\", "/")

Write-Host "Lab2 QEMU REAL RUN"
Write-Host "Path: $repo"
Write-Host ""
Write-Host '$ make -C code qemu   # virt, -bios none, -smp 1'

$command = @"
cd '$wslRepo/code'
(
  sleep 1
  printf 'hi\n'
  sleep 1
  printf 'badecall\n'
  sleep 1
  printf 'bufstorm\n'
  sleep 1
  printf 'A\nB\nC\nD\n'
  sleep 2
  printf '\001x'
) | timeout 10s qemu-system-riscv64 -machine virt -bios none -kernel kernel/kernel -m 128M -smp 1 -nographic
"@

wsl.exe --exec /bin/bash --noprofile --norc -c $command 2>&1
Write-Host ""
Write-Host "Direct QEMU virt single-core terminal output."
Read-Host "Press Enter to close"
