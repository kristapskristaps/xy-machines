"""Run the public SDK checks on the collection without contacting hardware."""
import argparse
import json
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sdk', type=Path, default=ROOT.parent / 'open-xy')
    parser.add_argument('--cxx', default='clang++')
    parser.add_argument('--host-cxx', default='c++')
    parser.add_argument('--native', action='store_true', help='also compile each machine with Blackfin')
    parser.add_argument('--native-only', action='store_true')
    args = parser.parse_args()
    sdk = args.sdk.resolve()
    entries = json.loads((ROOT / 'collection.json').read_text())['machines']
    machines = [(ROOT / entry['path']).resolve() for entry in entries]
    actual = set(ROOT.glob('machines/**/machine.json'))
    if not machines or len(set(machines)) != len(machines) or {p / 'machine.json' for p in machines} != actual:
        parser.error('Collection must list each machine folder exactly once')
    ids = [json.loads((p / 'definition.json').read_text())['id'] for p in machines]
    if len(set(ids)) != len(ids):
        parser.error('Duplicate machine IDs')
    if not (sdk / 'sdk.py').is_file():
        parser.error('Pass --sdk with the separate Open-XY SDK checkout')
    if not args.native_only:
        command = [sys.executable, str(sdk / 'sdk.py'), 'test', '--no-cpu',
                   '--cxx', args.cxx, '--host-cxx', args.host_cxx]
        for machine in machines:
            command += ['--machine', str(machine)]
        subprocess.run(command, cwd=sdk, check=True)
    if args.native or args.native_only:
        # Every machine is validated individually, independent of collection
        # memory limits. Even non-bootable modules stay outside the checkout.
        with tempfile.TemporaryDirectory(prefix='xy-machines-native-') as folder:
            for ident, machine in zip(ids, machines):
                print('Native compile:', ident, flush=True)
                command = [sys.executable, str(sdk / 'sdk.py'), 'compile',
                           '--machine', str(machine), '--out', str(Path(folder) / ident)]
                if json.loads((machine / 'machine.json').read_text())['kind'] == 'effect':
                    # SDK startup requires an engine. Pair an isolated effect
                    # with the collection's small Subvista synth for linking.
                    command += ['--machine', str(ROOT / 'machines/subvista')]
                subprocess.run(command, cwd=sdk, check=True)
    print('PASS: collection inventory and requested checks; no device operations.')

if __name__ == '__main__':
    main()
