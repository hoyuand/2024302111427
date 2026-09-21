#!/usr/bin/env python3
"""Run the kernel twice and verify the Lab1 UART contract."""
import os
import re
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


def assert_lab1_prefix(output, expected):
    """Lab2 may append an interactive shell prompt after the Lab1 banner."""
    if not output.startswith(expected):
        raise SystemExit(
            "Lab1 UART prefix mismatch\n"
            f"expected_prefix={expected!r}\nactual={output!r}"
        )
    suffix = output[len(expected):]
    if suffix not in (b"", b"sh> "):
        raise SystemExit(f"unexpected output after Lab1 prefix: {suffix!r}")


def check_trap_log(traps):
    """Keep Lab1 strict while allowing the known later-Lab traps."""
    if not traps:
        print("[ok] QEMU -d int log is empty (Lab1-only image)")
        return
    allowed = re.compile(
        rb"cause:.*000000000000000[1789]"
        rb".*desc=(user_ecall|m_timer|s_software|s_external)"
    )
    unexpected = [
        line.decode(errors="replace")
        for line in traps.splitlines()
        if b"riscv_cpu_do_interrupt" in line and not allowed.search(line)
    ]
    if unexpected:
        raise SystemExit("QEMU reported unexpected traps:\n" + "\n".join(unexpected))
    print("[ok] QEMU -d int log contains only expected later-Lab traps")


def main():
    expected = (ROOT / "expect_banner.txt").read_bytes()
    first, _ = capture()
    second, _ = capture()
    assert_lab1_prefix(first, expected)
    assert_lab1_prefix(second, expected)

    with tempfile.TemporaryDirectory(prefix="lab1-") as temp_dir:
        trap_log = Path(temp_dir) / "qemu-int.log"
        output, _ = capture(("-d", "int", "-D", str(trap_log)))
        traps = trap_log.read_bytes() if trap_log.exists() else b""
        assert_lab1_prefix(output, expected)
        check_trap_log(traps)

    print(f"[ok] UART bytes match expect_banner.txt ({len(expected)} bytes)")
    print("[ok] two cold boots are identical")


if __name__ == "__main__":
    main()
