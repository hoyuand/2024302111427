#!/usr/bin/env python3
"""Run real single-core QEMU Lab5 tests and save their UART transcript.

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
    "zerocopy": ("T5-1 PASS", 40),
    "cowpair": ("T5-2 PASS", 40),
    "echo hi": ("T5-3 PASS: argc=2 argv[0]=echo aligned", 20),
    "echo one two three": ("T5-3 PASS: argc=4 argv[0]=echo aligned", 20),
    "cowtest": ("COWTEST PASS: exec reset and budget recovery", 120),
    "cowoom": ("COWOOM PASS", 180),
    "vmtest": ("VMTEST PASS", 60),
    "badaccess": ("TEST-3b PASS", 30),
    "memcap": ("TEST-4 PASS", 180),
}



def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--tree", type=Path, default=CODE)
    parser.add_argument("--log", type=Path, default=CODE / "lab5-qemu.log")
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
                checked = text
                if description == "running badaccess":
                    checked = re.sub(r"lab3: unexpected user trap cause=0xd epc=0x[0-9a-f]+", "", checked)
                if re.search(r"(?:panic|FAIL|unexpected user trap)", checked, re.I):
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
            if "KREF-SELFTEST PASS refs=1,3,2,1,0" not in boot:
                raise RuntimeError("allocator reference conservation selftest missing")
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
                if name == "zerocopy":
                    match = re.search(r"copies: .*\(delta (\d+)\)", text)
                    if not match or int(match[1]) > 2: raise RuntimeError("zero-copy counter failed")
                if name == "cowpair" and "copies delta = 2 (expect 2)" not in text:
                    raise RuntimeError("pair copy count must be exactly two")
                if name == "cowtest":
                    warnings = re.findall(r"pid=(\d+) reached LAB5_COW_BUDGET=195", text)
                    if len(warnings) != 2 or warnings[0] != warnings[1]:
                        raise RuntimeError(f"per-exec budget reset did not warn twice: {warnings}")
                    if text.count("lab5: rejected store") != 2:
                        raise RuntimeError("readonly and guard writes must both be rejected")
                if name == "cowoom":
                    if "COWOOM partial fork rollback returned -1" not in text or "lab5: rejected store" not in text:
                        raise RuntimeError("real OOM branches were not observed")
                completed.append(name)
                print(f"[ok] {name}: PASS and shell returned")
                for line in text.splitlines():
                    if line.startswith(("T5-", "COWTEST", "COWOOM", "VMTEST", "[cow]", "TEST-4", "lab5:")):
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
