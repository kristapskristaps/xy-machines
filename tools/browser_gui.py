"""Launch the upstream browser builder with catalog and private machine sources."""
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import time
import uuid

from workspace import ROOT, compiler, outside_checkout, private_root, require, settings


def collection(public, custom, output):
    """Keep catalog order and extensions, with local versions taking precedence."""
    local = {}
    excluded = []
    for row in json.loads(custom.read_text())['machines']:
        folder = (custom.parent / row['path']).resolve()
        metadata = json.loads((folder / 'machine.json').read_text())
        ident = json.loads((folder / 'definition.json').read_text())['id']
        if ident == 'hkgranular' or (ident.startswith('hk') and metadata['kind'] == 'effect'):
            excluded.append(ident)
        else:
            require(ident not in local, 'Duplicate local machine: ' + ident)
            local[ident] = folder
    entries, seen, overrides = [], set(), []
    for row in json.loads(public.read_text())['machines']:
        folder = (public.parent / row['path']).resolve()
        definition = folder / 'definition.json'
        if definition.is_file():
            ident = json.loads(definition.read_text())['id']
            require(ident not in seen, 'Duplicate catalog machine: ' + ident)
            seen.add(ident)
            if ident in local:
                folder = local.pop(ident)
                overrides.append(ident)
        entries.append({'path': str(folder)})
    entries.extend({'path': str(folder)} for folder in local.values())
    extensions = {}
    for source in (public, custom):
        for row in json.loads(source.read_text()).get('extensions', []):
            folder = (source.parent / row['path']).resolve()
            ident = json.loads((folder / 'extension.json').read_text())['id']
            extensions[ident] = {'path': str(folder)}
    output.write_text(json.dumps({'schema': 1, 'machines': entries,
                                  'extensions': list(extensions.values())}, indent=2) + '\n')
    return dict(machines=len(entries), extensions=len(extensions), excluded=excluded, local_overrides=overrides)


def receipt_path():
    return private_root() / 'browser' / 'latest.json'


def runtime_signature(sdk):
    # Verification scripts and catalog listings do not change the browser or
    # firmware runtime. A verification-only SDK branch may export the same app.
    paths = subprocess.check_output(['git', '-C', str(sdk), 'ls-files', '-z',
                                     'sdk', 'builder', 'profiles', 'linker', 'tooling', 'VERSION'])
    digest = hashlib.sha256()
    for raw in sorted(paths.split(b'\0')):
        if raw:
            digest.update(raw + b'\0')
            digest.update((sdk / raw.decode()).read_bytes())
    return digest.hexdigest()


def remember_kit(sdk, folder):
    folder = outside_checkout(folder)
    html = folder / 'opxy-builder.html'
    require(html.is_file(), 'No verified builder: the SDK names its HTML only after all export checks pass.')
    require((folder / 'kit.json').is_file(), 'Missing exported machine collection.')
    kit = json.loads((folder / 'kit.json').read_text())
    receipt = dict(html=str(html), sha256=hashlib.sha256(html.read_bytes()).hexdigest(),
                   export_sdk_commit=subprocess.check_output(['git', '-C', str(sdk), 'rev-parse', 'HEAD'], text=True).strip(),
                   runtime_signature=runtime_signature(sdk),
                   exported_at=time.strftime('%Y-%m-%dT%H:%M:%S%z'),
                   machines=len(kit['machines']), extensions=len(kit.get('extensions', [])))
    path = receipt_path()
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix('.pending.json')
    temporary.write_text(json.dumps(receipt, indent=2) + '\n')
    temporary.replace(path)
    return html


def cached_kit(sdk):
    path = receipt_path()
    require(path.is_file(), 'Create the browser kit first: ./xy browser --refresh --stock-os /external/private/os.ldr')
    receipt = json.loads(path.read_text())
    html = outside_checkout(receipt['html'])
    require(html.is_file() and hashlib.sha256(html.read_bytes()).hexdigest() == receipt['sha256'],
            'The cached builder changed or is missing; run ./xy browser --refresh.')
    require(runtime_signature(sdk) == receipt['runtime_signature'],
            'SDK runtime changed; refresh the browser kit before launching it.')
    return html


def launch(args, sdk, native_flags):
    if args.refresh:
        stock = args.stock_os or settings().get('stock_os')
        require(stock, 'First export needs --stock-os pointing to the external original OS.')
        stock = outside_checkout(stock)
        subprocess.run([sys.executable, str(sdk / 'sdk.py'), 'catalog', 'fetch'], cwd=sdk, check=True)
        base = private_root() / 'browser'
        base.mkdir(parents=True, exist_ok=True)
        label = time.strftime('%Y%m%dT%H%M%S') + '-' + uuid.uuid4().hex[:6]
        selected = base / ('collection-' + label + '.json')
        summary = collection(sdk / 'local/catalog/collection.json', ROOT / 'collection.json', selected)
        print('Browser pool:', summary, flush=True)
        out = outside_checkout(base / ('kit-' + label))
        exporter = Path(settings().get('browser_export_sdk', str(sdk))).resolve()
        require(runtime_signature(exporter) == runtime_signature(sdk),
                'The export SDK must use the pinned browser and firmware runtime.')
        subprocess.run([sys.executable, str(exporter / 'sdk.py'), 'kit', '--personal', '--collection', str(selected),
                        '--stock-os', str(stock), '--preview-cxx', compiler(), *native_flags,
                        '--out', str(out)], cwd=exporter, check=True)
        html = remember_kit(exporter, out)
    else:
        html = cached_kit(sdk)
    print('Verified browser builder:', html, flush=True)
    if args.build_only:
        return
    require(1 <= args.port <= 65535, 'Choose a valid localhost port.')
    if not (sdk / 'node_modules/vite').is_dir():
        subprocess.run(['npm', 'ci'], cwd=sdk, check=True)
    env = dict(os.environ, OPEN_XY_KIT=str(html),
               OPEN_XY_OUT_DIR=str(private_root() / 'browser/site'))
    print(f'Open Chrome or Edge at http://127.0.0.1:{args.port}/', flush=True)
    print('The official browser handles OS selection, layout, machine selection, building and USB transfer. '
          'Writing the boot partition is a device step. Use --refresh after changing machine source or updating the catalog.', flush=True)
    os.chdir(sdk)
    os.execvpe('npm', ['npm', 'run', 'dev', '--', '--host', '127.0.0.1', '--port', str(args.port), '--strictPort'], env)
