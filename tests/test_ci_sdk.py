"""The CI checkout follows SDK pin edits instead of duplicating their values."""
from pathlib import Path
import sys
import tempfile
import unittest

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
from ci_sdk import checkout_pin


class CiSdkTests(unittest.TestCase):
    def test_checkout_tracks_repository_and_revision_changes(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            for owner, revision in [('first', 'a' * 40), ('second', 'b' * 40)]:
                (root / 'SDK_REPOSITORY').write_text(f'https://github.com/{owner}/sdk.git\n')
                (root / 'SDK_REVISION').write_text(revision + '\n')
                self.assertEqual(checkout_pin(root), {'repository': f'{owner}/sdk', 'revision': revision})

    def test_invalid_checkout_values_fail_before_actions_outputs(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            (root / 'SDK_REPOSITORY').write_text('https://github.com/example/sdk')
            (root / 'SDK_REVISION').write_text('staging')
            with self.assertRaisesRegex(ValueError, 'full commit SHA'):
                checkout_pin(root)
            (root / 'SDK_REVISION').write_text('a' * 40)
            (root / 'SDK_REPOSITORY').write_text('https://github.com/example/sdk\nrevision=injected')
            with self.assertRaisesRegex(ValueError, 'GitHub repository URL'):
                checkout_pin(root)
