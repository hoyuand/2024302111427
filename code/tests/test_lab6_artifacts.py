import hashlib,re,unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[2]
class Lab6ArtifactsTest(unittest.TestCase):
    def test_course_files(self):
        for row in (ROOT/'course-config/lab6-course-files.sha256').read_text(encoding='utf-8').splitlines():
            digest,name=row.split('  ',1)
            self.assertEqual(hashlib.sha256((ROOT/name).read_bytes()).hexdigest(),digest,name)
    def test_parameters(self):
        s=(ROOT/'code/kernel/course_sid.h').read_text()
        v={k:int(n) for k,n in re.findall(r'^#define (\w+) (\d+)',s,re.M)}
        self.assertEqual((v['LAB6_REPLACE_POLICY'],v['LAB6_BUF_SLOTS'],v['LAB6_LOG_BLOCKS']),(3,11,27))
    def test_delivery_links(self):
        for p in list((ROOT/'doc/lab6').rglob('*.md'))+list((ROOT/'course-config').glob('lab6*.md')):
            for target in re.findall(r'!?\[[^\]]*\]\(([^)]+)\)',p.read_text(encoding='utf-8')):
                if not re.match(r'[a-z]+://|#',target):self.assertTrue((p.parent/target.split('#')[0]).exists(),str(p)+': '+target)
