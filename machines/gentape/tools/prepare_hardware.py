"""Freeze an installed collection and add Gen Tape plus four full-control probes.

No device operations. The base package identifies the installed source set and
layout. Existing source bytes must still match its compile report. Diagnostic
copies change only the namespace/identity and parameter defaults, never DSP.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess

MACHINE = Path(__file__).resolve().parents[1]


def require(ok, message):
    if not ok:
        raise ValueError(message)


def digest(path):
    return hashlib.sha256(path.read_bytes()).hexdigest()


def write_json(path, value):
    path.write_text(json.dumps(value, indent=2) + '\n')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--base-package', type=Path, required=True)
    parser.add_argument('--sdk', type=Path, required=True)
    parser.add_argument('--out', type=Path, required=True)
    args = parser.parse_args()
    package, sdk, out = args.base_package.resolve(), args.sdk.resolve(), args.out.resolve()
    require(not out.exists(), 'Choose a fresh preparation directory.')
    ancestor = out.parent
    while not ancestor.exists():
        ancestor = ancestor.parent
    require(subprocess.run(['git', '-C', str(ancestor), 'rev-parse', '--git-dir'], capture_output=True).returncode != 0,
            'Hardware preparation and builds must stay outside every checkout.')
    manifest = json.loads((package / 'manifest.json').read_text())
    report = json.loads((package / 'analysis/compile-report.json').read_text())
    for name, expected in manifest['files'].items():
        require(digest(package / name) == expected, 'Base package changed: ' + name)
    require(not subprocess.check_output(['git', '-C', str(sdk), 'status', '--porcelain', '--untracked-files=no'], text=True).strip(),
            'SDK must have clean tracked source.')
    roots, extensions = {}, {}
    for entry in manifest['collection']:
        if 'path' in entry:
            root = Path(entry['path'])
            roots[json.loads((root / 'definition.json').read_text())['id']] = root
        else:
            root = Path(entry['extension'])
            extensions[json.loads((root / 'extension.json').read_text())['id']] = root
    # Read and verify every byte before writing the frozen source tree.
    files = []
    for name, expected in report['sources'].items():
        if name.startswith('external/'):
            _, ident, relative = name.split('/', 2)
            source, destination = roots[ident] / relative, out / 'sources' / ident / relative
        elif name.startswith('extension:'):
            ident, relative = name[len('extension:'):].split('/', 1)
            source, destination = extensions[ident] / relative, out / 'extensions' / ident / relative
        elif name != 'collection':
            source, destination = sdk / name, None
        else:
            continue
        require(source.is_file() and digest(source) == expected, 'Installed-source mismatch: ' + name)
        if destination:
            files.append((destination, source.read_bytes()))
    require(not subprocess.check_output(['git', '-C', str(MACHINE), 'status', '--porcelain'], text=True).strip(),
            'Commit Gen Tape source before hardware preparation.')
    commit = subprocess.check_output(['git', '-C', str(MACHINE), 'rev-parse', 'HEAD'], text=True).strip()
    out.mkdir(parents=True)
    for destination, data in files:
        destination.parent.mkdir(parents=True, exist_ok=True)
        destination.write_bytes(data)
    collection = [{'path': str(out / 'sources' / ident)} for ident in roots]
    collection.append({'path': str(MACHINE)})
    probes = []
    for index, suffix in enumerate(('clean', 'vhs', 'cass', 'dict')):
        ident = 'gttest' + suffix
        folder = out / 'diagnostics' / ident
        folder.mkdir(parents=True)
        for name in ('core.h', 'fx.cpp', 'tables.h', 'LICENSE'):
            content = (MACHINE / name).read_text()
            renamed = content.replace('namespace gentape {', 'namespace ' + ident + ' {')
            require(renamed.replace('namespace ' + ident + ' {', 'namespace gentape {') == content,
                    'Diagnostic DSP differs: ' + name)
            (folder / name).write_text(renamed)
        definition = json.loads((MACHINE / 'definition.json').read_text())
        definition.update(id=ident, name='GT Test ' + suffix.title())
        settings = [1, 1, 1, 1, (index + .5) / 4, 1, 1, 1]
        for parameter, value in zip(definition['parameters'], settings):
            parameter['default'] = value
        write_json(folder / 'definition.json', definition)
        metadata = json.loads((MACHINE / 'machine.json').read_text())
        metadata['description'] = 'Gen Tape CPU diagnostic: all controls full, model ' + suffix + '.'
        write_json(folder / 'machine.json', metadata)
        collection.append({'path': str(folder)})
        probes.append(dict(id=ident, model=suffix, parameters=settings, purpose='Full-control CPU callback workload'))
    extension_entries = [{'path': str(out / 'extensions' / ident)} for ident in extensions]
    write_json(out / 'collection.json', dict(schema=1, machines=collection, extensions=extension_entries))
    (out / 'base-layout.json').write_bytes((package / 'layout.json').read_bytes())
    write_json(out / 'preparation.json', dict(base_package=str(package), base_firmware_sha256=digest(package / 'candidate/os.ldr'),
        sdk=str(sdk), sdk_commit=subprocess.check_output(['git', '-C', str(sdk), 'rev-parse', 'HEAD'], text=True).strip(),
        gentape_source=str(MACHINE), gentape_commit=commit,
        gentape_hashes={name:digest(MACHINE / name) for name in ('core.h', 'fx.cpp', 'tables.h', 'definition.json', 'machine.json')},
        preserved_machines=list(roots), preserved_extensions=list(extensions), source_files_frozen=len(files),
        preserved_layout_reservations=len(json.loads((package / 'layout.json').read_text())['machines']), probes=probes,
        device_operations=False, hardware_cpu_measured=False))
    print(out)


if __name__ == '__main__':
    main()
