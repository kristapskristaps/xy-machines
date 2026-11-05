"""Startup artwork contracts shared by the five generated Weave scenes."""
import importlib.util
from pathlib import Path
import re
import tempfile
import unittest
import xml.etree.ElementTree as ET


SOURCE = Path(__file__).resolve().parents[1] / 'tools/generate_split_art.py'
SPEC = importlib.util.spec_from_file_location('split_art', SOURCE)
ART = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(ART)


class StartupArtwork(unittest.TestCase):
    def test_integer_polylines_have_visible_strokes_and_no_open_fills(self):
        for short in ART.NAMES:
            root = ET.fromstring((ART.ROOT / ART.NAMES[short].lower() / 'panel.svg.fragment').read_text())
            for node in root:
                if node.tag != 'path':
                    continue
                with self.subTest(machine=short, node=node.get('id')):
                    self.assertGreaterEqual(float(node.get('stroke-width')), 1)
                    self.assertRegex(node.get('d'), r'^[MLZ0-9\s-]+$')
                    if not node.get('d').endswith('Z'):
                        self.assertEqual(node.get('fill'), 'none')
            for index in range(3):
                node = root.find(f"path[@id='{ART.NAMES[short].lower()}_node_{index}']")
                points = list(zip(*[iter(map(int, re.findall(r'-?\d+', node.get('d'))))]*2))
                self.assertEqual(len(points), 32)
                self.assertEqual(len(set(points)), 32)
                self.assertNotEqual(sum(x*points[(i+1)%32][1]-points[(i+1)%32][0]*y
                                        for i,(x,y) in enumerate(points)), 0)

    def test_artwork_only_regeneration_preserves_runtime_display(self):
        original = ART.ROOT
        try:
            with tempfile.TemporaryDirectory() as folder:
                ART.ROOT = Path(folder)
                for short, name in ART.NAMES.items():
                    root = ART.ROOT / ART.NAMES[short].lower()
                    root.mkdir()
                    display = root / 'display.cpp'
                    display.write_text('preserved display callback')
                    ART.generate(short, name, artwork_only=True)
                    self.assertEqual(display.read_text(), 'preserved display callback')
                    self.assertEqual((root / 'panel.svg.fragment').read_bytes(),
                                     (original / ART.NAMES[short].lower() / 'panel.svg.fragment').read_bytes())
        finally:
            ART.ROOT = original


if __name__ == '__main__':
    unittest.main()
