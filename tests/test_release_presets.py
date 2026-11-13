"""The release presets must be enabled and retain the auditioned patch settings."""
import json
import math
from pathlib import Path
import re
import unittest

ROOT = Path(__file__).resolve().parents[1] / 'machines'
SYNTHS = ('tandem', 'atlas', 'choir', 'sift', 'crease', 'bitflip')


class ReleasePresets(unittest.TestCase):
    def test_all_seven_ship_supported_native_presets(self):
        total = 0
        for mid in (*SYNTHS, 'x0x'):
            with self.subTest(machine=mid):
                root = ROOT / mid
                metadata = json.loads((root / 'machine.json').read_text())
                self.assertEqual(metadata.get('presets'), 'presets')
                files = list((root / metadata['presets']).glob('*.json'))
                self.assertTrue(1 <= len(files) <= 16)
                self.assertTrue((root / 'presets/default.json').is_file())
                for file in files:
                    preset = json.loads(file.read_text())
                    self.assertEqual((preset['version'], preset['platform'], preset['type']), (4, 'OP-XY', mid))
                    self.assertEqual(len(preset['engine']['params']), 8)
                    self.assertEqual(preset['regions'], [])
                total += len(files)
        self.assertEqual(total, 37)

    def test_synth_native_engine_settings_match_browser_patches(self):
        for mid in SYNTHS:
            root = ROOT / mid
            definition = json.loads((root / 'definition.json').read_text())
            for title, patch in definition['presets'].items():
                with self.subTest(machine=mid, patch=title):
                    name = 'default' if title in ('Init', 'Init Pulse') else re.sub('[^a-z0-9]+', '-', title.lower()).strip('-')
                    preset = json.loads((root / 'presets' / (name + '.json')).read_text())
                    words = [math.floor(patch.get(p['id'], p['default']) * 32767 + .5)
                             for p in definition['parameters']]
                    self.assertEqual(preset['engine']['params'], words + [0] * (8-len(words)))

    def test_x0x_authored_kits_are_preserved_separately(self):
        folder=ROOT.parent/'preset-packs/x0x/content/presets/x0x'
        files=list(folder.glob('*.json'))
        self.assertEqual(len(files),7)
        for path in files:
            with self.subTest(kit=path.stem):
                kit=json.loads(path.read_text())
                self.assertEqual((kit['type'],kit['version']),('x0x',4))
                self.assertEqual(kit['engine']['params'],[16384]*8)
                self.assertEqual(len(kit['regions']),24)
                self.assertEqual([p['hikey'] for p in kit['regions']],list(range(53,77)))
                self.assertTrue(all('sample' not in p and 'file' not in p for p in kit['regions']))
