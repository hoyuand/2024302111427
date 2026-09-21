#!/usr/bin/env python3
from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[1]
REPO = ROOT.parent
LAB0_IMAGES = REPO / "doc" / "lab0" / "images"


class Lab0ArtifactsTest(unittest.TestCase):
    def test_required_diagrams_cover_acceptance_terms(self):
        expected = {
            "01-echo-control-flow.mmd": [
                "read(0)", "fork", "exec", "write", "exit", "用户栈",
                "内核栈", "调度器栈", "U态", "S态", "lock",
            ],
            "02-exec-state-snapshot.mmd": [
                "state=", "parent=", "pagetable=", "sz=", "ofile",
                "TRAPFRAME", "TRAMPOLINE", "V R X U", "V R W U", "inode",
            ],
            "03-timer-interrupt-journey.mmd": [
                "0x8000000000000005", "uservec", "usertrap", "tickslock",
                "yield", "swtch", "scheduler", "prepare_return", "sret",
            ],
        }
        for name, terms in expected.items():
            text = (LAB0_IMAGES / name).read_text(encoding="utf-8")
            for term in terms:
                self.assertIn(term, text, f"{name} 缺少验收词: {term}")

    def test_each_diagram_has_three_personal_notes(self):
        for path in sorted(LAB0_IMAGES.glob("*.mmd")):
            text = path.read_text(encoding="utf-8")
            self.assertGreaterEqual(
                text.count("个人思考"), 3, f"{path.name} 自主批注不足3处"
            )

    def test_personal_baseline_is_present_and_unchanged_shape(self):
        sid_header = (ROOT / "kernel" / "course_sid.h").read_text(encoding="utf-8")
        params = (REPO / "course-config" / "student-parameters.txt").read_text(encoding="utf-8")
        self.assertIn("#define COURSE_SID 2024302111427", sid_header)
        self.assertIn("LAB1_BANNER_PROTOCOL       = 2", params)
        self.assertIn("LAB1_STACK_KB              = 12", params)

    def test_doc_structure_is_complete(self):
        for lab in ("lab0", "lab1"):
            base = REPO / "doc" / lab
            self.assertTrue((base / "README.md").is_file())
            self.assertTrue((base / "images").is_dir())
            self.assertTrue((base / "docs").is_dir())

        lab0_images = REPO / "doc" / "lab0" / "images"
        lab1_images = REPO / "doc" / "lab1" / "images"
        lab1_docs = REPO / "doc" / "lab1" / "docs"
        self.assertTrue((lab0_images / "lab0-terminal-run.png").is_file())
        for name in (
            "lab1-terminal-run.png",
            "lab1-startup-sequence.png",
        ):
            self.assertTrue((lab1_images / name).is_file(), name)
        for name in (
            "lab1-result.md",
            "lab1-git-proof.txt",
            "lab1-requirement-matrix.md",
        ):
            self.assertTrue((lab1_docs / name).is_file(), name)
        self.assertTrue(
            (REPO / "doc" / "lab0" / "docs" / "lab0-artifact-check.txt").is_file()
        )

        self.assertFalse(list(lab0_images.glob("*.svg")))
        self.assertFalse(list(lab0_images.glob("*.md")))
        self.assertFalse(list(lab1_images.glob("*.svg")))
        self.assertFalse(list(lab1_images.glob("*.md")))
        for images in (lab0_images, lab1_images):
            unexpected = [
                p for p in images.iterdir()
                if p.is_file() and p.suffix.lower() not in {".png", ".mmd"}
            ]
            self.assertEqual([], unexpected, f"图片目录存在非 PNG/Mermaid 文件: {unexpected}")

    def test_lab1_mermaid_source_is_monochrome(self):
        source = (
            REPO / "doc" / "lab1" / "images" / "lab1-startup-sequence.mmd"
        ).read_text(encoding="utf-8")
        colors = set(re.findall(r"#[0-9a-fA-F]{6}", source.lower()))
        self.assertTrue(colors <= {"#000000", "#ffffff"}, sorted(colors))

    def test_lab0_diagrams_are_monochrome(self):
        for path in LAB0_IMAGES.glob("*.mmd"):
            text = path.read_text(encoding="utf-8")
            colors = set(re.findall(r"#[0-9a-fA-F]{6}", text.lower()))
            self.assertTrue(
                colors <= {"#000000", "#ffffff"},
                f"{path.name} 含非黑白颜色: {sorted(colors - {'#000000', '#ffffff'})}",
            )


if __name__ == "__main__":
    unittest.main(verbosity=2)
