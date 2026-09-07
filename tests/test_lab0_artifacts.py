#!/usr/bin/env python3
from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]


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
            text = (ROOT / "figures" / name).read_text(encoding="utf-8")
            for term in terms:
                self.assertIn(term, text, f"{name} 缺少验收词: {term}")

    def test_each_diagram_has_three_personal_notes(self):
        for path in sorted((ROOT / "figures").glob("*.mmd")):
            text = path.read_text(encoding="utf-8")
            self.assertGreaterEqual(
                text.count("个人思考"), 3, f"{path.name} 自主批注不足3处"
            )

    def test_markdown_wrappers_embed_exact_mermaid_source(self):
        for source in sorted((ROOT / "figures").glob("*.mmd")):
            wrapper = source.with_suffix(".md").read_text(encoding="utf-8-sig")
            start = wrapper.index("```mermaid\n") + len("```mermaid\n")
            end = wrapper.rindex("```")
            embedded = wrapper[start:end]
            self.assertEqual(
                source.read_text(encoding="utf-8").rstrip() + "\n",
                embedded,
                f"{source.name} 与 Markdown 包装内容不一致",
            )

    def test_personal_baseline_is_present_and_unchanged_shape(self):
        sid_header = (ROOT / "kernel" / "course_sid.h").read_text(encoding="utf-8")
        params = (ROOT / "我的参数.txt").read_text(encoding="utf-8")
        self.assertIn("#define COURSE_SID 2024302111427", sid_header)
        self.assertIn("LAB1_BANNER_PROTOCOL       = 2", params)
        self.assertIn("LAB1_STACK_KB              = 12", params)


if __name__ == "__main__":
    unittest.main(verbosity=2)
