#!/usr/bin/env python3
import re
import subprocess
import sys
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]


class Lab1ArtifactsTest(unittest.TestCase):
    def test_expected_banner_shape_and_checksum(self):
        data = (ROOT / "expect_banner.txt").read_bytes()
        text = data.decode("utf-8")
        self.assertTrue(data.endswith(b"\n"), "期望输出以行结束符收尾")
        result = subprocess.run(
            [sys.executable, str(ROOT / "check_expect.py"), "2024302111427",
             str(ROOT / "expect_banner.txt")],
            capture_output=True, text=True, check=False,
        )
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertRegex(text, r"\[chk=\d+\]$")

    def test_startup_invariants_are_present(self):
        entry = (ROOT / "kernel" / "entry.S").read_text(encoding="utf-8")
        self.assertIn("csrw    pmpaddr0", entry)
        self.assertIn("csrw    pmpcfg0", entry)
        self.assertIn("csrw    satp, zero", entry)
        self.assertIn(".balign 4", entry)
        start = (ROOT / "kernel" / "start.c").read_text(encoding="utf-8")
        self.assertNotIn("r_mhartid", start)
        console = (ROOT / "kernel" / "console.c").read_text(encoding="utf-8")
        self.assertIn("COURSE_SID % 16", console)
        self.assertIn("UART_LSR_THRE", console)


if __name__ == "__main__":
    unittest.main(verbosity=2)
