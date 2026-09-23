from pathlib import Path
import unittest


REPO = Path(__file__).resolve().parents[2]


class DocumentationStructureTest(unittest.TestCase):
    def test_each_lab_has_one_indexed_chinese_report(self):
        for lab in ("lab0", "lab1", "lab2"):
            base = REPO / "doc" / lab
            self.assertTrue((base / "README.md").is_file(), lab)
            self.assertTrue((base / "images").is_dir(), lab)
            self.assertTrue((base / "docs").is_dir(), lab)

            reports = sorted(
                path.name for path in (base / "docs").iterdir() if path.is_file()
            )
            self.assertEqual([f"{lab}-experiment-report.md"], reports, lab)
            report = (base / "docs" / reports[0]).read_text(encoding="utf-8")
            readme = (base / "README.md").read_text(encoding="utf-8")

            for path in sorted((base / "images").iterdir()):
                if not path.is_file():
                    continue
                self.assertIn(path.name, readme, f"{lab}/README.md 未索引 {path.name}")
                self.assertIn(path.suffix.lower(), {".png", ".mmd"})
                if path.suffix.lower() == ".png":
                    self.assertIn(
                        f"../images/{path.name}",
                        report,
                        f"{lab} 主报告未引用 {path.name}",
                    )

            pngs = list((base / "images").glob("*.png"))
            self.assertGreaterEqual(len(pngs), 1, lab)
            self.assertFalse(list((base / "images").glob("*.svg")), lab)


if __name__ == "__main__":
    unittest.main(verbosity=2)
