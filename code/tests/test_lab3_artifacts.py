import unittest
from pathlib import Path


REPO = Path(__file__).resolve().parents[2]


class Lab3ArtifactsTest(unittest.TestCase):
    def test_lab3_acceptance_tree_and_requirements(self):
        lab = REPO / "doc/lab3"
        for relative in (
            "README.md",
            "docs/lab3-experiment-report.md",
            "images/lab3-address-space.png",
            "images/lab3-address-space.mmd",
            "images/lab3-syscall-copy.png",
            "images/lab3-syscall-copy.mmd",
            "images/lab3-rollback.png",
            "images/lab3-rollback.mmd",
            "images/lab3-terminal-run.png",
        ):
            path = lab / relative
            self.assertTrue(path.is_file() and path.stat().st_size > 0, path)
        report = (lab / "docs/lab3-experiment-report.md").read_text(encoding="utf-8")
        for term in ("SV39", "copyin/copyout", "sbrk", "fork", "exec", "TEST-3a PASS", "TEST-4 PASS"):
            self.assertIn(term, report)

    def test_lab3_interfaces_and_personal_parameters(self):
        vm = (REPO / "code/kernel/vm.c").read_text(encoding="utf-8")
        syscall = (REPO / "code/kernel/syscall.c").read_text(encoding="utf-8")
        makefile = (REPO / "code/Makefile").read_text(encoding="utf-8")
        self.assertIn("LAB3_ALLOC_ORDER", vm)
        self.assertIn("mappages", vm)
        self.assertIn("copyin", syscall)
        self.assertIn("copyout", syscall)
        self.assertIn("user/user-lab3.ld", makefile)
        self.assertIn("badaccess", makefile)
        self.assertIn("memcap", makefile)


if __name__ == "__main__":
    unittest.main()
