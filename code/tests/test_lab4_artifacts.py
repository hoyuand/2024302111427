import hashlib
import unittest
from pathlib import Path


CODE = Path(__file__).resolve().parents[1]
REPO = CODE.parent


class Lab4ArtifactsTest(unittest.TestCase):
    """Static checks for the immutable lab4 v2 course material and local tests."""

    def test_course_gifts_are_byte_exact(self):
        # The archive is the authority for these files.  Keeping this check
        # here catches accidental edits to the ABI stubs and swtch assembly.
        expected = {
            "kernel/swtch.S": "baff65e42b8f80e3b2ec5e1f67e2033b0b9bd7b35564db998573f576bc816088",
            "kernel/syscall.h": "45492f1822d10f7caa1c3318f551c91d161b841f8a11fe6b8505f9bf35a47cb3",
            "user/user.h": "cf54c6c68d60556a330625011b7e4e3239be440a28c46ffe063b1c358a193b3f",
            "user/usys.pl": "78dd2d981c9e5194bbe7b9082d6e74980bcb286f0fb53574d1b57f1347b45b39",
        }
        for relative, digest in expected.items():
            path = CODE / relative
            self.assertTrue(path.is_file(), path)
            self.assertEqual(hashlib.sha256(path.read_bytes()).hexdigest(), digest, relative)

    def test_lab4_user_tests_and_interfaces_are_present(self):
        for relative in (
            "user/alt.c",
            "user/lab4check.c",
            "user/policychange.c",
            "user/common.h",
            "user/common.c",
            "user/bench.c",
            "user/lottery_test.c",
            "user/stride_test.c",
        ):
            path = CODE / relative
            self.assertTrue(path.is_file() and path.stat().st_size > 0, path)
        syscall = (CODE / "kernel/syscall.h").read_text(encoding="utf-8")
        self.assertIn("SYS_schedstat  23", syscall)
        self.assertIn("SYS_time_rdtime 24", syscall)
        self.assertIn("SYS_setickets  25", syscall)
        sid = (CODE / "kernel/course_sid.h").read_text(encoding="utf-8")
        self.assertIn("#define LAB4_POLICY_ID 7", sid)
        lab4check = (CODE / "user/lab4check.c").read_text(encoding="utf-8")
        for term in ("setickets(0)", "setickets(-1)", "schedstat(2147483647",
                     "0x3ffffff000UL", "pause(3)", "wait(&st)"):
            self.assertIn(term, lab4check)
        policy = (CODE / "user/policychange.c").read_text(encoding="utf-8")
        for term in ("child(start, per_tick, 10, 30)",
                     "child(start, per_tick, 30, 10)",
                     "RESULT policy_before", "RESULT policy_after"):
            self.assertIn(term, policy)

    def test_lab4_report_contract_is_explicit(self):
        lab = REPO / "doc/lab4"
        if not lab.exists():
            self.skipTest("lab4 documentation is maintained by the documentation agent")
        report = lab / "docs/lab4-experiment-report.md"
        self.assertTrue(report.is_file() and report.stat().st_size > 0)
        text = report.read_text(encoding="utf-8", errors="replace")
        for term in ("schedstat", "time_rdtime", "setickets", "pause", "stride"):
            self.assertIn(term, text)
        self.assertTrue("lost wakeup" in text or "丢失唤醒" in text or "�������ѡ" in text)


if __name__ == "__main__":
    unittest.main()
