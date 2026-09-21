#!/usr/bin/env python3
"""Run the Lab1 kernel twice and verify UART bytes plus a zero trap log."""
import os
import signal
import subprocess
import tempfile
import time
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
QEMU = "qemu-system-riscv64"
BASE_ARGS = [
    QEMU, "-machine", "virt", "-bios", "none",
    "-kernel", "kernel/kernel", "-nographic", "-smp", "1",
]


def capture(extra_args=()):
    proc = subprocess.Popen(
        BASE_ARGS + list(extra_args),
        stdin=subprocess.DEVNULL,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        start_new_session=True,
        cwd=ROOT,
    )
    time.sleep(0.8)
    if hasattr(os, "killpg"):
        os.killpg(proc.pid, signal.SIGKILL)
    else:
        proc.kill()
    stdout, stderr = proc.communicate()
    return stdout, stderr


def main():
    expected = (ROOT / "expect_banner.txt").read_bytes()
    first, _ = capture()
    second, _ = capture()
    if first != expected or second != expected:
        raise SystemExit(
            "UART output mismatch\n"
            f"expected={expected!r}\nfirst={first!r}\nsecond={second!r}"
        )

    with tempfile.TemporaryDirectory(prefix="lab1-") as temp_dir:
        trap_log = Path(temp_dir) / "qemu-int.log"
        output, _ = capture(("-d", "int", "-D", str(trap_log)))
        traps = trap_log.read_bytes() if trap_log.exists() else b""
        if output != expected:
            raise SystemExit("UART output changed while QEMU trap logging was enabled")
        if traps:
            raise SystemExit(f"QEMU reported unexpected traps:\n{traps.decode(errors='replace')}")

    print(f"[ok] UART bytes match expect_banner.txt ({len(expected)} bytes)")
    print("[ok] two cold boots are identical")
    print("[ok] QEMU -d int log is empty")


if __name__ == "__main__":
    main()
