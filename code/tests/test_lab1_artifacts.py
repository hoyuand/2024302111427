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
        self.assertRegex(text.rstrip(), r"\[chk=\d+\]$")

    def test_startup_invariants_are_present(self):
        entry = (ROOT / "kernel" / "entry.S").read_text(encoding="utf-8")
        self.assertIn("csrw    mie, zero", entry)
        self.assertIn("csrr    tp, mhartid", entry)
        self.assertIn("LAB1_STACK_KB * 1024", entry)
        self.assertIn("call    start", entry)
        self.assertIn(".balign 4", entry)
        start = (ROOT / "kernel" / "start.c").read_text(encoding="utf-8")
        self.assertNotIn("r_mhartid", start)
        self.assertIn("aligned(4096)", start)
        self.assertIn("bootstack[LAB1_STACK_KB * 1024]", start)
        for term in (
            "w_pmpaddr0", "w_pmpcfg0", "w_satp(0)", "w_medeleg",
            "w_mideleg", "w_mepc((uint64)main)", "MSTATUS_MPP_S", 'asm volatile("mret")',
        ):
            self.assertIn(term, start)
        console = (ROOT / "kernel" / "console.c").read_text(encoding="utf-8")
        self.assertIn("consoleinit(void)", console)
        self.assertIn("consputc(int c)", console)
        self.assertIsNotNone(
            re.search(r"consputc\(int c\)[\s\S]*?uartputc_sync\(c\)", console)
        )
        self.assertIn("COURSE_SID % 16", console)
        self.assertIn("UART_LSR_THRE", console)
        printf = (ROOT / "kernel" / "printf.c").read_text(encoding="utf-8")
        self.assertIn('console_raw_puts("0x")', printf)

    def test_boundary_cases_are_emitted_by_main(self):
        main = (ROOT / "kernel" / "main.c").read_text(encoding="utf-8")
        for term in ("min=%d", "hex=%x", "percent=%%", "2147483647"):
            self.assertIn(term, main)


if __name__ == "__main__":
    unittest.main(verbosity=2)
