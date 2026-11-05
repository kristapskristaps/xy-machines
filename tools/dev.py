"""One entry point for source iteration, audition, and hardware-build preparation."""
import argparse
import hashlib
import json
import math
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile
import threading
import time
import uuid

from workspace import (ROOT, CONFIG, checked_sdk, compiler, inventory, outside_checkout,
                       private_root, require, sdk_path, settings)

def run(command, cwd=ROOT):
    subprocess.run([str(x) for x in command], cwd=cwd, check=True)

def sdk_command(sdk, command, *args):
    run([sys.executable, sdk / 'sdk.py', command, *args], cwd=sdk)

def fresh(parent, label):
    return parent / (label + '-' + time.strftime('%Y%m%dT%H%M%S') + '-' + uuid.uuid4().hex[:6])

def save_config(config):
    CONFIG.parent.mkdir(exist_ok=True)
    temporary = CONFIG.with_suffix('.tmp')
    temporary.write_text(json.dumps(config, indent=2) + '\n')
    temporary.replace(CONFIG)

def setup(args):
    sdk = sdk_path(args.sdk)
    pin = (ROOT / 'SDK_REVISION').read_text().strip()
    if not sdk.exists():
        run(['git', 'clone', '--no-checkout', (ROOT / 'SDK_REPOSITORY').read_text().strip(), sdk])
        run(['git', '-C', sdk, 'fetch', 'origin', pin])
        run(['git', '-C', sdk, 'checkout', '--detach', pin])
    checked_sdk(sdk)  # Never reset an existing SDK or another chat's changes.
    config = settings()
    config.update(sdk=str(sdk), cxx=args.cxx or compiler())
    for key in ('stock_os', 'layout', 'private_root', 'toolchain'):
        value = getattr(args, key, None)
        if value:
            path = Path(value).expanduser().resolve()
            if key in ('stock_os', 'private_root'):
                path = outside_checkout(path)
            config[key] = str(path)
    save_config(config)
    venv = ROOT / '.venv'
    python = venv / ('Scripts/python.exe' if os.name == 'nt' else 'bin/python')
    if not python.exists():
        run([sys.executable, '-m', 'venv', venv])
    run([python, '-m', 'pip', 'install', 'numpy==2.3.5'])
    if args.native and not config.get('toolchain'):
        run([python, sdk / 'sdk.py', 'toolchain', 'fetch'], cwd=sdk)
    print('Configured. Use ./xy list, ./xy test vader, or ./xy preview vader --watch.')

def fingerprint(folder):
    excluded = {'.git', 'local', '__pycache__', '.venv', 'node_modules'}
    result = []
    for path in sorted(folder.rglob('*')):
        if (not path.is_file() or excluded.intersection(path.relative_to(folder).parts)
                or path.name == '.DS_Store' or path.suffix == '.pyc'):
            continue
        try:
            stat = path.stat()  # Editors can atomically replace a file mid-scan.
        except FileNotFoundError:
            continue
        result.append((str(path.relative_to(folder)), stat.st_mtime_ns, stat.st_size))
    return tuple(result)

def preview(args, sdk):
    machine = inventory().get(args.id)
    require(machine is not None, 'Unknown machine ID. Use ./xy list.')
    require(machine['metadata']['kind'] in ('synth', 'drum'),
            'The SDK browser workbench supports synths and drums. See docs/development.md for effect/sample auditions.')
    folder = machine['path']
    def build():
        out = fresh(ROOT / 'local/previews', args.id)
        sdk_command(sdk, 'preview', '--machine', folder, '--cxx', args.cxx or compiler(), '--out', out)
        return out
    out = build()
    if args.no_serve:
        return
    sys.path.insert(0, str(sdk))
    from tools.serve_harness import PreviewServer, Handler
    from tooling.preview import preview_bundle
    class PreviewHandler(Handler):
        def log_message(self, format, *values):
            if not self.path.startswith('/__dev_revision'):
                super().log_message(format, *values)
    server = PreviewServer(('127.0.0.1', args.port), PreviewHandler)
    stop = threading.Event()
    def publish(path):
        bundle = preview_bundle(path)  # SDK authenticates the complete snapshot.
        if args.watch:
            revision = uuid.uuid4().hex
            html = (sdk / 'harness/index.html').read_bytes().replace(
                b'</head>', ('<meta name="xy-dev-revision" content="' + revision + '">'
                '<script type="module" src="/__dev_reload.mjs"></script></head>').encode())
            for route in ('/', '/harness/', '/harness/index.html'):
                bundle[route] = ('text/html; charset=utf-8', html)
            bundle['/__dev_revision'] = ('application/json', json.dumps(revision).encode())
            bundle['/__dev_reload.mjs'] = ('text/javascript', b'''const revision=document.querySelector('meta[name="xy-dev-revision"]').content;
setInterval(async()=>{try{const next=await(await fetch('/__dev_revision',{cache:'no-store'})).json();if(next!==revision)location.reload();}catch{}},750);''')
        server.preview = bundle  # Swap only after a successful authenticated build.
    publish(out)
    def watch():
        previous = fingerprint(folder)
        while not stop.wait(.5):
            current = fingerprint(folder)
            if current == previous:
                continue
            if stop.wait(.35):
                return
            previous = fingerprint(folder)
            try:
                publish(build())
                print('Updated preview. Re-enable browser audio after reload.', flush=True)
            except (OSError, ValueError, subprocess.CalledProcessError) as error:
                print('Build failed; the previous audition stays available:', error, flush=True)
    if args.watch:
        threading.Thread(target=watch, daemon=True).start()
    print(f'Workbench: http://127.0.0.1:{server.server_port}/', flush=True)
    try:
        server.serve_forever()
    finally:
        stop.set()
        server.server_close()

def new_machine(args, sdk):
    require(re.fullmatch(r'[a-z][a-z0-9_]{0,14}', args.id), 'Use a machine ID of 1–15 lowercase letters/digits/underscores.')
    require(args.id not in inventory(), 'Machine ID already exists.')
    out = ROOT / 'machines' / args.id
    require(not out.exists(), 'Destination already exists.')
    sdk_command(sdk, 'new', '--repository', '--id', args.id, '--name', args.name,
                '--kind', args.kind, '--controls', args.controls, '--out', out)
    # --repository adds documentation and CI, not a nested Git repository.
    shutil.rmtree(out / '.github', ignore_errors=True)
    (out / 'AGENTS.md').write_text(f'# {args.name}\n\nThis machine is `machines/{args.id}`. Follow the repository root AGENTS.md\nand docs/development.md. Use `./xy test {args.id}` and `./xy preview {args.id} --watch`\nfrom the repository root. Preserve the public opxy API and machine-local tests.\n')
    readme = out / 'README.md'
    if readme.exists():
        readme.write_text(re.sub(r'(?:\.\./)+open-xy', '../../../xy-machines-sdk', readme.read_text()))
    path = ROOT / 'collection.json'
    collection = json.loads(path.read_text())
    collection['machines'].append({'path': out.relative_to(ROOT).as_posix()})
    collection['machines'].sort(key=lambda item: item['path'])
    path.write_text(json.dumps(collection, indent=2) + '\n')
    readme = ROOT / 'README.md'
    if readme.exists():
        items = inventory()
        counts = {kind: sum(item['metadata']['kind'] == kind for item in items.values())
                  for kind in ('synth', 'effect', 'sample')}
        text = re.sub(r'\d+ Open-XY machines: \*\*.*?\*\*\.',
                      f"{len(items)} Open-XY machines: **{counts['synth']} synths, {counts['effect']} send effects and {counts['sample']} sample engines**.",
                      readme.read_text(), count=1)
        if '## Inventory\n' in text:
            rows = ['| ID | Name | Kind | Folder |', '| --- | --- | --- | --- |']
            for ident, item in items.items():
                relative = item['path'].relative_to(ROOT).as_posix()
                rows.append(f"| {ident} | {item['definition']['name']} | {item['metadata']['kind']} | [{relative}]({relative}/README.md) |")
            text = text.split('## Inventory\n', 1)[0] + '## Inventory\n\n' + '\n'.join(rows) + '\n'
        readme.write_text(text)
    print('Registered', args.id, 'in collection.json. Implement its sound and tests before adding it to a hardware kit.')

def native_flags():
    toolchain = settings().get('toolchain')
    return ['--toolchain', str(Path(toolchain) / 'bin'), '--compiler', toolchain] if toolchain else []

def build(args, sdk):
    dirty = subprocess.check_output(['git', '-C', str(ROOT), 'status', '--porcelain'], text=True)
    require(not dirty.strip(), 'Commit source changes before creating a profiling-enabled hardware build.')
    kit = Path(args.kit).resolve()
    require(kit.is_file(), 'Choose an existing collection with --kit. The all-machine inventory is not a default kit.')
    layout = Path(args.layout or settings().get('layout', '')).expanduser().resolve()
    require(layout.is_file(), 'Pass --layout with the last installed layout.json; machine numbers must be preserved.')
    stock = args.stock_os or settings().get('stock_os')
    require(stock, 'Configure --stock-os using ./xy setup, or pass it to build.')
    stock = outside_checkout(stock)
    out = outside_checkout(args.out or fresh(private_root() / 'builds', kit.stem))
    sdk_command(sdk, 'build', '--collection', kit, '--layout', layout, '--stock-os', stock,
                '--profiling', 'on', '--sweep', args.sweep, '--out', out, *native_flags())
    print('Package:', out, '\nNext: ./xy stage "' + str(out) + '"')

def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()

def stage(args, sdk):
    package = outside_checkout(args.package)
    sys.path.insert(0, str(sdk))
    from tooling.profile_report import package_data
    manifest, report, _ = package_data(package)  # Validate firmware and compile-report hashes.
    out = outside_checkout(args.out or fresh(private_root() / 'staging', 'install'))
    require(not out.exists(), 'Choose a fresh staging directory.')
    files = {}
    for name, sha in manifest['files'].items():
        if not name.startswith('candidate/'):
            continue
        remote = name.removeprefix('candidate/')
        preset = re.fullmatch(r'content/presets/[a-z][a-z0-9_]{0,14}/[a-z0-9][a-z0-9_-]{0,23}\.json', remote)
        require(remote in ('os.ldr', 'content/op1_factory.json') or
                re.fullmatch(r'content/display/[a-z][a-z0-9_]*\.svg', remote) or preset,
                'Unexpected install path: ' + remote)
        source = package / name
        require(source.is_file() and digest(source) == sha, 'Package hash mismatch: ' + name)
        if preset:
            continue  # Presets are generated from verified source only when explicitly requested below.
        files[remote] = source
    require('os.ldr' in files and 'content/op1_factory.json' in files, 'Incomplete firmware package.')
    previous = json.loads(Path(args.against).read_text())['files'] if args.against else {}
    out.mkdir(parents=True)
    for name, source in files.items():
        destination = out / 'files' / name
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copyfile(source, destination)
    shutil.copyfile(package / 'layout.json', out / 'layout.json')
    if args.presets:
        sys.path.insert(0, str(sdk))
        from tooling import native
        _, machines, _ = native.configuration(native.package_collection(package), native.package_layout(package))
        descriptors = []
        for machine in machines:
            prefix = 'external/' + machine.id + '/'
            expected = {name: sha for name, sha in report['sources'].items() if name.startswith(prefix)}
            require({name: digest(path) for name, path in machine.files().items()} == expected,
                    'Source changed since this package was built; rebuild before staging presets: ' + machine.id)
            values = [math.floor(p['default'] * 32767 + .5) for p in machine.definition['parameters']]
            metadata = json.loads((machine.root / 'machine.json').read_text())
            descriptors.append(dict(id=machine.id, kind=machine.kind, presets=machine.presets or {},
                presetDefaults=metadata.get('presetDefaults', {}),
                catalog=json.dumps(dict(type=machine.id, default_params=values + [0] * (8 - len(values))))))
        descriptor = out / 'preset-descriptors.json'
        descriptor.write_text(json.dumps(descriptors))
        try:
            run(['node', ROOT / 'tools/stage_presets.mjs', sdk, descriptor, out / 'files'])
        finally:
            descriptor.unlink()
    hashes = {p.relative_to(out / 'files').as_posix(): digest(p) for p in (out / 'files').rglob('*') if p.is_file()}
    changed = sorted(name for name, sha in hashes.items() if previous.get(name) != sha)
    record = dict(schema=1, package=str(package), firmware_sha256=hashes['os.ldr'], files=hashes,
                  changed=changed, compared_to=args.against, installed=False, boot_confirmed=False,
                  needs_firmware_flash='os.ldr' in changed)
    (out / 'install-manifest.json').write_text(json.dumps(record, indent=2) + '\n')
    flash = ('Then exit MTP with track 2, write OS with track 5, wait for firmware written ok, and power-cycle.\n'
             'Confirm normal boot before profiling. DSP is statically linked; USB file transfer does not hot-load it.\n'
             if 'os.ldr' in changed else
             'Firmware is unchanged relative to the comparison bundle; no boot-partition write is needed.\n'
             'Reload changed presets or restart normal playback as appropriate after verified content transfer.\n'
             if changed else 'No bundle files changed. No transfer or flash is needed.\n')
    (out / 'INSTALL.md').write_text('# Install prepared development firmware\n\n'
        'This bundle has not been installed. Back up device files before replacing them.\n'
        'The changed list compares build bundles, not the live device. Do not delete other files.\n\n'
        + '\n'.join('- `files/' + name + '` → `/' + name + '`' for name in changed) + '\n\n'
        'Presets are opt-in; preserve edited device presets. Keep layout.json on the computer for future builds.\n'
        'Use dev-menu MTP, copy the listed files, and verify their full readback against install-manifest.json.\n'
        + flash)
    print(f'Staged {len(hashes)} files; {len(changed)} differ from the comparison bundle. No device operations.\n{out}')

def profile_package(args, sdk):
    package = outside_checkout(args.package)
    sys.path.insert(0, str(sdk))
    from tooling.profile_report import package_data
    _, report, _ = package_data(package)
    available = list(dict.fromkeys(case['name'] for case in report['benchmark_cases'] if case.get('source') == 'sdk'))
    names = available if args.all_machines else list(dict.fromkeys(args.machine or []))
    require(names and all(name in available for name in names), 'Machine is not in this diagnostic package.')
    output = fresh(private_root() / 'cpu', package.name + '-batch')
    captures = []
    print('Active device diagnostic: leave the saved/stopped XY untouched. Restart afterwards without saving test settings.')
    for number in range(1, args.runs + 1):
        out = output / f'run-{number}'
        sdk_command(sdk, 'profile', 'benchmark', '--package', package, '--installed', '--greet', '--out', out)
        captures += ['--capture', out / 'capture.json']
    reports = {}
    for name in names:
        out = output / 'reports' / name
        sdk_command(sdk, 'profile', 'report', '--package', package, '--machine', name, *captures, '--out', out)
        reports[name] = str(out)
    output.mkdir(parents=True, exist_ok=True)
    (output / 'batch-summary.json').write_text(json.dumps(dict(package=str(package), reports=reports,
        captures=[str(path) for path in captures[1::2]], independent_runs=args.runs), indent=2) + '\n')
    print(f'{len(names)} machine reports from {args.runs} complete capture(s): {output}')

def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sdk', type=Path)
    sub = parser.add_subparsers(dest='command', required=True)
    p = sub.add_parser('setup', help='Create a pinned SDK checkout and local Python environment')
    p.add_argument('--sdk', type=Path, default=argparse.SUPPRESS)
    for name in ('cxx', 'stock-os', 'layout', 'private-root', 'toolchain'):
        p.add_argument('--' + name)
    p.add_argument('--native', action='store_true')
    sub.add_parser('list')
    sub.add_parser('doctor')
    p = sub.add_parser('new')
    p.add_argument('id'); p.add_argument('--name', required=True)
    p.add_argument('--kind', choices=('synth', 'effect'), default='synth')
    p.add_argument('--controls', type=int, choices=range(4, 9), default=4)
    p = sub.add_parser('test')
    p.add_argument('ids', nargs='*'); p.add_argument('--changed', action='store_true')
    p.add_argument('--base', default='HEAD'); p.add_argument('--native', action='store_true')
    p.add_argument('--cpu', action='store_true')
    p = sub.add_parser('preview')
    p.add_argument('id'); p.add_argument('--watch', action='store_true'); p.add_argument('--no-serve', action='store_true')
    p.add_argument('--port', type=int, default=0); p.add_argument('--cxx')
    p = sub.add_parser('browser', help='Official browser builder with catalog plus local machines')
    p.add_argument('--refresh', action='store_true'); p.add_argument('--stock-os')
    p.add_argument('--build-only', action='store_true'); p.add_argument('--port', type=int, default=8928)
    p = sub.add_parser('build')
    p.add_argument('--kit', required=True); p.add_argument('--layout'); p.add_argument('--stock-os'); p.add_argument('--out')
    p.add_argument('--sweep', choices=('on', 'off'), default='on', help='on: SDK default full synth sweep; off: quick midpoint-only development')
    p = sub.add_parser('stage')
    p.add_argument('package'); p.add_argument('--against'); p.add_argument('--out'); p.add_argument('--presets', action='store_true')
    p = sub.add_parser('profile', help='Explicit active device diagnostic; save and stop the project first')
    p.add_argument('--package', required=True); p.add_argument('--installed', required=True, action='store_true')
    p.add_argument('--runs', type=int, choices=range(1, 101), default=1, help='SDK default: one capture; use 2+ to confirm silent settings, 3+ for variable results')
    group = p.add_mutually_exclusive_group(required=True)
    group.add_argument('--machine', action='append', help='machine ID; repeat to report a subset')
    group.add_argument('--all', dest='all_machines', action='store_true', help='report every SDK machine from the same capture batch')
    args = parser.parse_args()
    if args.command == 'setup':
        setup(args); return
    if args.command == 'list':
        for ident, item in inventory().items():
            print(f"{ident:16} {item['metadata']['kind']:7} {item['path'].relative_to(ROOT)}")
        return
    sdk = checked_sdk(args.sdk)
    if args.command == 'doctor':
        print('Pinned SDK:', sdk, '\nCompiler:', compiler(), '\nPrivate outputs:', private_root())
        sdk_command(sdk, 'doctor', '--preview', '--cxx', compiler())
    elif args.command == 'new':
        new_machine(args, sdk)
    elif args.command == 'test':
        command = [sys.executable, ROOT / 'tools/check.py', '--sdk', sdk, '--cxx', compiler(), '--base', args.base]
        for ident in args.ids:
            command += ['--machine', ident]
        if args.changed: command += ['--changed']
        if args.native: command += ['--native']
        if args.cpu: command += ['--cpu']
        run(command)
    elif args.command == 'preview':
        preview(args, sdk)
    elif args.command == 'browser':
        from browser_gui import launch
        launch(args, sdk, native_flags())
    elif args.command == 'build':
        build(args, sdk)
    elif args.command == 'stage':
        stage(args, sdk)
    elif args.command == 'profile':
        profile_package(args, sdk)

if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        pass
    except (OSError, ValueError, KeyError, subprocess.CalledProcessError) as error:
        print('ERROR:', error, file=sys.stderr)
        sys.exit(1)
