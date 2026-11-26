"""Static provenance/parameter/link checks, not substitutes for QEMU tests."""
import hashlib
from pathlib import Path
import re
import unittest
REPO = Path(__file__).resolve().parents[2]

class Lab5ArtifactsTest(unittest.TestCase):
    def test_latest_course_files_are_byte_exact(self):
        manifest = REPO / "course-config/lab5-course-files.sha256"
        for row in manifest.read_text(encoding="utf-8").splitlines():
            digest, relative = row.split("  ", 1)
            if relative in {"code/kernel/syscall.h", "code/user/user.h", "code/user/usys.pl"}: continue  # Lab6 verifies superseding gifts
            self.assertEqual(hashlib.sha256((REPO/relative).read_bytes()).hexdigest(), digest, relative)

    def test_personal_parameters_match_student_id(self):
        text = (REPO/"code/kernel/course_sid.h").read_text(encoding="utf-8")
        values = dict((k, int(v)) for k,v in re.findall(r"^#define (\w+) (\d+)", text, re.M))
        sid=values["COURSE_SID"]
        self.assertEqual(sid,2024302111427)
        for key,expected in {"LAB5_REFCNT_SCHEME":sid%3,"LAB5_COW_BUDGET":128+sid%128,
                             "LAB5_KILL_SEMANTICS":sid%2,"LAB1_BANNER_PROTOCOL":2,
                             "LAB1_STACK_KB":12,"LAB2_TICK":3,"LAB2_BUF_SEMANTICS":1,
                             "LAB2_BUF_SIZE":128}.items():
            self.assertEqual(values[key],expected,key)

    def test_delivery_markdown_local_links_resolve(self):
        files = [REPO/"README.md",REPO/"code/README.md",REPO/"doc/README.md"]
        files += list((REPO/"course-config").glob("lab5*.md"))
        files += list((REPO/"doc/lab5").rglob("*.md"))
        for path in files:
            text=path.read_text(encoding="utf-8")
            for target in re.findall(r"!?\[[^\]]*\]\(([^)]+)\)",text):
                if re.match(r"[a-z]+://|#",target): continue
                target=target.split("#",1)[0].strip("<>")
                self.assertTrue((path.parent/target).exists(),f"{path}: {target}")

if __name__ == "__main__":
    unittest.main()
