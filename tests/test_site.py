import hashlib
import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from serve_site import snapshot
from build_site import thumbnail


class GalleryExport(unittest.TestCase):
    def test_server_rejects_changed_export(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp).resolve()
            (root / 'index.html').write_text('original')
            (root / 'site-files.json').write_text(json.dumps({'index.html':hashlib.sha256(b'original').hexdigest()}))
            self.assertEqual(snapshot(root)['index.html'][1], b'original')
            (root / 'index.html').write_text('changed')
            with self.assertRaisesRegex(ValueError, 'Changed exported file'):
                snapshot(root)

    def test_server_rejects_export_escape(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp).resolve() / 'public'
            root.mkdir()
            (root.parent / 'private.txt').write_text('private')
            (root / 'site-files.json').write_text(json.dumps({'../private.txt':hashlib.sha256(b'private').hexdigest()}))
            with self.assertRaisesRegex(ValueError, 'Redirected or missing'):
                snapshot(root)

    def test_thumbnail_handles_zero_paths_frame_padding(self):
        metadata = {'graphics':{'paths':0,'points':0,'nodes':['node'],'texts':[],'palette':['gray']}}
        art = '<g><circle id="node" r="2" fill="#000000"/><circle id="gray" fill="#888888" stroke="#999999" stroke-width="1"/></g>'
        # Empty path storage still occupies one word before the first node.
        words = [0,10240,20480,1024,1024,0,0,2] + [0] * 17
        result = thumbnail(art, metadata, words)
        self.assertIn('matrix(1.0 0.0 -0.0 1.0 10.0 20.0)', result)
        self.assertIn('fill="#888888"', result)
