"""The personal pool preserves catalog entries, local overrides and exclusions."""
import json
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from browser_gui import collection


class BrowserPoolTests(unittest.TestCase):
    def test_catalog_and_private_pool(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            def machine(folder, ident, kind):
                folder.mkdir()
                (folder / 'machine.json').write_text(json.dumps(dict(kind=kind)))
                (folder / 'definition.json').write_text(json.dumps(dict(id=ident)))
                return dict(path=folder.name)
            public_rows = [machine(root / 'published', 'shared', 'synth'),
                           machine(root / 'core', 'ringmod', 'effect')]
            (root / 'capture').mkdir()
            (root / 'capture/extension.json').write_text('{"id":"capture"}')
            custom_rows = [machine(root / 'private', 'shared', 'synth'),
                           machine(root / 'vader', 'vader', 'synth'),
                           machine(root / 'badfx', 'hkcrush', 'effect')]
            public, custom, output = [root / name for name in ('public.json', 'custom.json', 'pool.json')]
            public.write_text(json.dumps(dict(machines=public_rows, extensions=[dict(path='capture')])))
            custom.write_text(json.dumps(dict(machines=custom_rows)))
            summary = collection(public, custom, output)
            self.assertEqual([Path(e['path']).name for e in json.loads(output.read_text())['machines']],
                             ['private', 'core', 'vader'])
            self.assertEqual([Path(e['path']).name for e in json.loads(output.read_text())['extensions']], ['capture'])
            self.assertEqual(summary['local_overrides'], ['shared'])
            self.assertEqual(summary['excluded'], ['hkcrush'])
            self.assertTrue((root / 'badfx/machine.json').is_file())

    def test_duplicate_catalog_id_is_rejected(self):
        with tempfile.TemporaryDirectory() as temp:
            root = Path(temp)
            folder = root / 'machine'
            folder.mkdir()
            (folder / 'definition.json').write_text('{"id":"same"}')
            public, custom = root / 'public.json', root / 'custom.json'
            public.write_text('{"machines":[{"path":"machine"},{"path":"machine"}]}')
            custom.write_text('{"machines":[]}')
            with self.assertRaisesRegex(ValueError, 'Duplicate catalog'):
                collection(public, custom, root / 'pool.json')
