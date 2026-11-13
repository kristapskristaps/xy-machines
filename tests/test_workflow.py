"""Workflow contracts: isolation, selection, staging integrity and failed builds."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys
import tempfile
import types
import unittest
from unittest.mock import patch

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'tools'))
import dev
import workspace

class WorkflowTests(unittest.TestCase):
    def test_private_outputs_reject_ignored_checkout_directories(self):
        with self.assertRaisesRegex(ValueError, 'outside every Git checkout'):
            workspace.outside_checkout(workspace.ROOT / 'local/firmware-test')

    def test_private_outputs_reject_another_repository(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            subprocess.run(['git', 'init', '-q', str(root)], check=True)
            with self.assertRaises(ValueError):
                workspace.outside_checkout(root / 'ignored/new/os.ldr')

    def test_changed_selection_includes_untracked_machine_files(self):
        root = workspace.ROOT
        machines = {'a': {'path': root / 'machines/a'}, 'b': {'path': root / 'machines/b'}}
        with patch.object(workspace, 'inventory', return_value=machines), patch.object(
                workspace.subprocess, 'check_output', side_effect=['machines/a/core.h\n', 'machines/b/new.hpp\n']):
            self.assertEqual(set(workspace.selected(changed=True)), {'a', 'b'})

    def test_sdk_pin_change_selects_all_machines(self):
        machines = {'a': {}, 'b': {}}
        with patch.object(workspace, 'inventory', return_value=machines), patch.object(
                workspace.subprocess, 'check_output', side_effect=['SDK_REVISION\n', '']):
            self.assertEqual(workspace.selected(changed=True), machines)

    def test_watch_ignores_generated_files_but_sees_new_headers(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            (root / 'core.h').write_text('source')
            first = dev.fingerprint(root)
            (root / 'local').mkdir()
            (root / 'local/generated.cpp').write_text('output')
            self.assertEqual(dev.fingerprint(root), first)
            (root / 'new.hpp').write_text('source')
            self.assertNotEqual(dev.fingerprint(root), first)

    def test_stage_diff_and_status_do_not_claim_device_installation(self):
        self.check_stage()

    def test_stage_rejects_tampered_firmware(self):
        self.check_stage(tamper=True)

    def test_stage_rejects_unexpected_remote_path(self):
        self.check_stage(unexpected=True)

    def test_sdk_packaged_presets_are_verified_and_excluded_by_default(self):
        self.check_stage(preset=True)
        self.check_stage(preset=True, tamper_preset=True)

    def check_stage(self, tamper=False, unexpected=False, preset=False, tamper_preset=False):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            package = root / 'package'; package.mkdir()
            contents = {'os.ldr': b'non-firmware test text', 'content/op1_factory.json': b'{}'}
            if preset:
                contents['content/presets/probe/default.json'] = b'{}'
            if unexpected:
                contents['../other.txt'] = b'not an install path'
            manifest = {'profiling': True, 'files': {}}
            for name, data in contents.items():
                path = package / 'candidate' / name
                path.parent.mkdir(parents=True, exist_ok=True); path.write_bytes(data)
                manifest['files']['candidate/' + name] = hashlib.sha256(data).hexdigest()
            (package / 'layout.json').write_text('{}')
            if tamper:
                (package / 'candidate/os.ldr').write_text('changed')
            if tamper_preset:
                (package / 'candidate/content/presets/probe/default.json').write_text('changed')
            prior = root / 'prior.json'
            prior.write_text(json.dumps({'files': {'os.ldr': manifest['files']['candidate/os.ldr']}}))
            args = argparse.Namespace(package=str(package), out=str(root / 'stage'), against=str(prior), presets=False)
            module = types.ModuleType('tooling.profile_report')
            module.package_data = lambda _: (manifest, {}, {})
            with patch.dict(sys.modules, {'tooling.profile_report': module}), patch.object(sys, 'path', sys.path.copy()):
                if tamper or unexpected or tamper_preset:
                    with self.assertRaises(ValueError):
                        dev.stage(args, root / 'sdk')
                    self.assertFalse((root / 'stage/install-manifest.json').exists())
                else:
                    dev.stage(args, root / 'sdk')
                    record = json.loads((root / 'stage/install-manifest.json').read_text())
                    self.assertEqual(record['changed'], ['content/op1_factory.json'])
                    self.assertFalse(record['installed'])
                    self.assertFalse(record['boot_confirmed'])
                    self.assertFalse(record['needs_firmware_flash'])
                    self.assertIn('no boot-partition write', (root / 'stage/INSTALL.md').read_text())
                    self.assertEqual((root / 'stage/files/os.ldr').read_bytes(), contents['os.ldr'])
                    self.assertFalse((root / 'stage/files/content/presets').exists())

    def test_build_requires_clean_source_before_starting_sdk(self):
        with patch.object(dev.subprocess, 'check_output', return_value=' M machines/a/engine.cpp\n'), patch.object(dev, 'sdk_command') as sdk:
            with self.assertRaisesRegex(ValueError, 'Commit source changes'):
                dev.build(argparse.Namespace(), Path('/unused-sdk'))
            sdk.assert_not_called()

    def test_batch_profiling_reuses_three_captures_for_all_reports(self):
        with tempfile.TemporaryDirectory() as folder:
            root = Path(folder)
            module = types.ModuleType('tooling.profile_report')
            module.package_data = lambda _: ({}, {'benchmark_cases': [
                {'name': 'stock', 'source': 'stock'}, {'name': 'a', 'source': 'sdk'},
                {'name': 'b', 'source': 'sdk'}]}, {})
            args = argparse.Namespace(package=str(root / 'package'), machine=None, all_machines=True, runs=3)
            with (patch.dict(sys.modules, {'tooling.profile_report': module}),
                  patch.object(dev, 'private_root', return_value=root),
                  patch.object(dev, 'sdk_command') as command,
                  patch.object(sys, 'path', sys.path.copy())):
                dev.profile_package(args, root / 'sdk')
            captures = [call.args for call in command.call_args_list if call.args[2] == 'benchmark']
            reports = [call.args for call in command.call_args_list if call.args[2] == 'report']
            self.assertEqual(len(captures), 3)
            self.assertEqual(len(reports), 2)
            self.assertEqual({call[call.index('--machine') + 1] for call in reports}, {'a', 'b'})
            first = reports[0][reports[0].index('--capture'):reports[0].index('--out')]
            self.assertEqual(first, reports[1][reports[1].index('--capture'):reports[1].index('--out')])
            summary = json.loads(next(root.rglob('batch-summary.json')).read_text())
            self.assertEqual(summary['independent_runs'], 3)

    def test_unknown_profile_target_never_opens_the_device(self):
        module = types.ModuleType('tooling.profile_report')
        module.package_data = lambda _: ({}, {'benchmark_cases': [{'name': 'a', 'source': 'sdk'}]}, {})
        with tempfile.TemporaryDirectory() as folder, patch.dict(sys.modules, {'tooling.profile_report': module}), patch.object(
                dev, 'sdk_command') as command, patch.object(sys, 'path', sys.path.copy()):
            args = argparse.Namespace(package=folder, machine=['missing'], all_machines=False)
            with self.assertRaises(ValueError):
                dev.profile_package(args, Path(folder))
            command.assert_not_called()

if __name__ == '__main__':
    unittest.main()
