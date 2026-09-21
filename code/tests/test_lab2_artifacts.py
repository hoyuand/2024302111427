import hashlib
import re
import unittest
from pathlib import Path


CODE = Path(__file__).resolve().parents[1]
REPO = CODE.parent


class Lab2ArtifactsTest(unittest.TestCase):
    def test_course_gifts_are_byte_exact(self):
        expected = {
            "kernel/fcntl.h": "c8c2a7fa8a9c432be55dcd4bd6db9429c88c3a95cec6f79a576968b36048e166",
            "kernel/param.h": "2e231d4c792da80c39668de92d56c28e9d24fd1bdc190435f7ce4025d24bbb59",
            "kernel/stat.h": "653da9a25a16140de6733b0fa87d2988d0dee962fe75ab5f17fcb50d19e3f8be",
            "kernel/syscall.h": "cd8b85b251ca89434ce0ce31d86d98521b8ecb3e2b224d95c74db6ee9948525f",
            "kernel/trampoline.S": "92c055df5557217dc6008714c01dc801836a86538a1ab67b617b66bcd873053f",
            "kernel/vm.h": "ebd5dfc1a2d28cb16a15dea12ac9467bded6ca4bf652f44b580e4c5bef3fb1a3",
            "user/hi.c": "10f3fbc0b6baf730a34cee74c2db87b654c13259f84681f144bb2093d061b8bd",
            "user/printf.c": "8c98780668f6418103de788fa8f2c2d16ed419de95a4484e20fe9531194ccbca",
            "user/sh.c": "112a764f79d690b0d1a013c25c184e93df075b2b8c6bdc8f582b61c273692661",
            "user/spin.c": "dcf1b6e196705428c18b1a7ad63e3d8f8ea2422974fc15bc5e1dbc65c380bbf5",
            "user/ulib.c": "0bb30cebbd735f730e335e807742e0d20ddf5779d534714f558a606aac2d7a4b",
            "user/user.h": "7bdb892e879d1ef3affc682e3ae9a6c13e965c87b35fb838a413cfa3a48f25d4",
            "user/user.ld": "d0285fe236059fcac87c547a8ea5fc3a304bf78e1e38ec932bf2dd95862c74a6",
            "user/usys.pl": "377fa27e4f7c07436fb487be27ef6c004ba1ddc4f018da0a19139e8946653e58",
        }
        for relative, digest in expected.items():
            actual = hashlib.sha256((CODE / relative).read_bytes()).hexdigest()
            self.assertEqual(actual, digest, relative)

    def test_personalized_trap_and_syscall_contracts(self):
        sid = (CODE / "kernel/course_sid.h").read_text(encoding="utf-8")
        self.assertIn("#define LAB2_TICK 3", sid)
        self.assertIn("#define LAB2_BUF_SEMANTICS 1", sid)
        self.assertIn("#define LAB2_BUF_SIZE 128", sid)

        trap = (CODE / "kernel/trap.c").read_text(encoding="utf-8")
        syscall = (CODE / "kernel/syscall.c").read_text(encoding="utf-8")
        self.assertIn("tf->epc = r_sepc() + 4", trap)
        self.assertRegex(syscall, r"default:\s*return \(uint64\)-1;")
        self.assertIn("argint", syscall)
        self.assertIn("argaddr", syscall)
        self.assertIn("argstr", syscall)

        makefile = (CODE / "Makefile").read_text(encoding="utf-8")
        self.assertIn("-mno-relax", makefile)
        self.assertIn("-T user/user.ld", makefile)
        self.assertRegex(makefile, r"UPROGS\s*=.*sh.*hi.*spin.*badecall.*bufstorm")
        self.assertRegex(makefile, r"qemu: kernel/kernel[\s\S]*-smp 1")

    def test_acceptance_assets_cover_real_results(self):
        lab = REPO / "doc/lab2"
        required = [
            lab / "README.md",
            lab / "docs/lab2-design-notes.md",
            lab / "docs/lab2-requirement-matrix.md",
            lab / "docs/lab2-build-and-smoke.txt",
            lab / "images/lab2-trap-flow.png",
            lab / "images/lab2-console-sequence.png",
            lab / "images/lab2-terminal-run.png",
        ]
        for path in required:
            self.assertTrue(path.is_file() and path.stat().st_size > 0, path)
        smoke = (lab / "docs/lab2-build-and-smoke.txt").read_text(
            encoding="utf-8", errors="replace"
        )
        self.assertIn("hi: user program running", smoke)
        self.assertIn("TEST-1 PASS", smoke)


if __name__ == "__main__":
    unittest.main()
