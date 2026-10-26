#!/usr/bin/env python3
"""Run real single-core QEMU Lab4 tests and save their UART transcript.

This script does not build the kernel and does not infer a runtime result from
source.  Run it under WSL after ``make -C code``.  Every command must emit its
own PASS marker and return to the shell before the next command is sent.
"""
import argparse
import os
from pathlib import Path
import re
import selectors
import subprocess
import time


CODE = Path(__file__).resolve().parents[1]
CASES = {
    "lab4check": ("PASS: lab4check", 40),
    "alt": ("PASS: alt", 90),
    "bench": ("PASS: bench", 120),
    "stride_test": ("PASS: stride", 180),
    "policychange": ("PASS: policychange", 120),
}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--tree", type=Path, default=CODE)
    parser.add_argument("--log", type=Path, default=CODE / "lab4-qemu.log")
    parser.add_argument("--tests", nargs="+", choices=CASES, default=list(CASES))
    parser.add_argument("--qemu", default="qemu-system-riscv64")
    args = parser.parse_args()
    tree = args.tree.resolve()
    args.log.parent.mkdir(parents=True, exist_ok=True)
    proc = subprocess.Popen(
        [args.qemu, "-machine", "virt", "-bios", "none", "-kernel",
         "kernel/kernel", "-m", "128M", "-smp", "1", "-nographic"],
        cwd=tree, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT, bufsize=0,
    )
    selector = selectors.DefaultSelector()
    selector.register(proc.stdout, selectors.EVENT_READ)
    output = bytearray()
    completed = []

    with args.log.open("wb") as transcript:
        def pump(timeout=0.1):
            for key, _ in selector.select(timeout):
                data = os.read(key.fd, 8192)
                if data:
                    output.extend(data)
                    transcript.write(data)
                    transcript.flush()
                elif proc.poll() is not None:
                    raise RuntimeError("QEMU exited before tests completed")

        def wait_for(predicate, timeout, description, start=0):
            limit = time.monotonic() + timeout
            while time.monotonic() < limit:
                pump()
                text = output[start:].decode("utf-8", errors="replace")
                if re.search(r"(?:panic|FAIL:|unexpected user trap)", text, re.I):
                    raise RuntimeError(f"kernel/test failure while {description}:\n{text}")
                if predicate(text):
                    return text
            raise RuntimeError(f"timeout while {description}:\n" + output[start:].decode(errors="replace"))

        try:
            boot = wait_for(lambda text: text.endswith("sh> "), 15, "waiting for boot")
            expected = (tree / "expect_banner.txt").read_bytes()
            if not output.startswith(expected):
                raise RuntimeError("Lab1 exact UART banner prefix regressed")
            if "DUMP-PAGETABLE begin" not in boot or "DUMP-PAGETABLE end" not in boot:
                raise RuntimeError("Lab3 boot page-table diagnostic missing")
            print("[ok] boot: exact Lab1 banner, Lab3 page-table dump, shell")
            for name in args.tests:
                marker, timeout = CASES[name]
                start = len(output)
                proc.stdin.write((name + "\n").encode())
                proc.stdin.flush()
                text = wait_for(
                    lambda chunk: marker in chunk and chunk.endswith("sh> "),
                    timeout, f"running {name}", start,
                )
                if name == "alt":
                    # Output may be interrupted by diagnostics, so join only
                    # complete lines containing child letters A/B.
                    letters = "".join(re.findall(r"^[AB]+\r?$", text, re.M)).replace("\r", "")
                    if letters.count("A") != 16 or letters.count("B") != 16:
                        raise RuntimeError(f"alt output missing child work: {letters!r}")
                    switches = sum(a != b for a, b in zip(letters, letters[1:]))
                    if switches < 2:
                        raise RuntimeError(f"alt has no observable preemption: {letters!r}")
                if name == "bench" and "RESULT concurrent_children = 8" not in text:
                    raise RuntimeError("bench did not reclaim eight children")
                completed.append(name)
                print(f"[ok] {name}: PASS and shell returned")
                for line in text.splitlines():
                    if line.startswith("RESULT "):
                        print(line)
            print("[ok] real QEMU tests:", ", ".join(completed))
            print("UART transcript:", args.log.resolve())
        finally:
            selector.close()
            if proc.poll() is None:
                try:
                    proc.stdin.write(b"\x01x")
                    proc.stdin.flush()
                    proc.wait(timeout=3)
                except (BrokenPipeError, subprocess.TimeoutExpired):
                    proc.kill()
            proc.wait()


if __name__ == "__main__":
    main()
