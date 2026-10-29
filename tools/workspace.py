"""Shared configuration for collection development commands."""
import json
import os
from pathlib import Path
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
CONFIG = ROOT / 'local/dev.json'

def require(condition, message):
    if not condition:
        raise ValueError(message)

def settings():
    return json.loads(CONFIG.read_text()) if CONFIG.exists() else {}

def compiler():
    configured = settings().get('cxx')
    brew = Path('/opt/homebrew/opt/llvm/bin/clang++')
    return configured or (str(brew) if brew.is_file() else 'clang++')

def sdk_path(explicit=None):
    return Path(explicit or os.environ.get('OPEN_XY_SDK') or settings().get('sdk')
                or ROOT.parent / 'xy-machines-sdk').expanduser().resolve()

def checked_sdk(explicit=None):
    sdk = sdk_path(explicit)
    require((sdk / 'sdk.py').is_file(), 'SDK missing. Run ./xy setup, or pass --sdk PATH.')
    sha = subprocess.check_output(['git', '-C', str(sdk), 'rev-parse', 'HEAD'], text=True).strip()
    require(sha == (ROOT / 'SDK_REVISION').read_text().strip(), 'SDK revision differs from SDK_REVISION; use a separate pinned checkout.')
    dirty = subprocess.check_output(['git', '-C', str(sdk), 'status', '--porcelain', '--untracked-files=no'], text=True)
    require(not dirty.strip(), 'SDK has tracked changes; use a separate clean SDK checkout.')
    return sdk

def inventory():
    entries = json.loads((ROOT / 'collection.json').read_text())['machines']
    result = {}
    for entry in entries:
        folder = (ROOT / entry['path']).resolve()
        require(folder.is_relative_to(ROOT / 'machines'), 'Machine path must be under machines/.')
        definition = json.loads((folder / 'definition.json').read_text())
        metadata = json.loads((folder / 'machine.json').read_text())
        ident = definition['id']
        require(ident not in result, 'Duplicate machine ID: ' + ident)
        result[ident] = dict(path=folder, definition=definition, metadata=metadata)
    return result

def selected(ids=None, changed=False, base='HEAD'):
    machines = inventory()
    if changed:
        names = subprocess.check_output(['git', '-C', str(ROOT), 'diff', '--name-only', base, '--'], text=True).splitlines()
        names += subprocess.check_output(['git', '-C', str(ROOT), 'ls-files', '--others', '--exclude-standard'], text=True).splitlines()
        if any(n in ('SDK_REVISION', 'SDK_REPOSITORY', 'collection.json') for n in names):
            return machines
        ids = [ident for ident, item in machines.items()
               if any((ROOT / name).is_relative_to(item['path']) for name in names)]
    if ids is None:
        return machines
    require(all(ident in machines for ident in ids), 'Unknown machine ID. Use ./xy list.')
    return {ident: machines[ident] for ident in dict.fromkeys(ids)}

def private_root():
    if os.name == 'nt':
        base = Path(os.environ['LOCALAPPDATA']) / 'OPXY-SDK/private'
    elif sys.platform == 'darwin':
        base = Path.home() / 'Library/Application Support/OPXY-SDK/private'
    else:
        base = Path.home() / '.local/share/OPXY-SDK/private'
    return outside_checkout(Path(settings().get('private_root', base / 'xy-machines')))

def outside_checkout(path):
    path = Path(path).expanduser().resolve()
    parent = path if path.is_dir() else path.parent
    while not parent.exists():
        parent = parent.parent
    probe = subprocess.run(['git', '-C', str(parent), 'rev-parse', '--git-dir'],
                           capture_output=True, text=True)
    require(probe.returncode != 0,
            'Firmware inputs and outputs must stay outside every Git checkout: ' + str(path))
    return path
