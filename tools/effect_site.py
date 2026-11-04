"""Compile collection-owned effect auditions without changing the pinned SDK."""
import hashlib
import html
import json
from pathlib import Path
import shutil
import subprocess
from workspace import ROOT, require


def export_effect(sdk, item, target, scratch, cxx, description, guide):
    definition, folder = item['definition'], item['path']
    ident = definition['id']
    words = definition['resources']['bufferBytes'] // 4
    require(words > 0, 'Effect requires a nonempty audio buffer')
    scratch.mkdir(parents=True, exist_ok=True)
    bridge = scratch / (ident + '.cpp')
    transport = 'transport,' if definition['resources'].get('transport') else ''
    bridge.write_text(f'''#include "{folder / 'fx.cpp'}"
static {ident}::State state;
static opxy::i32 buffer[{words}],left[64],right[64],parameters[8];
extern "C" {{
opxy::i32* xy_left(){{return left;}}
opxy::i32* xy_right(){{return right;}}
opxy::i32* xy_parameters(){{return parameters;}}
void xy_reset(){{{ident}::initialize(state);}}
void xy_process(unsigned bpm,unsigned ticks,unsigned playing){{
    const opxy::Transport transport={{bpm,ticks,playing!=0}};(void)transport;
    {ident}::process(state,opxy::Buffer{{buffer,{words}}},{transport}left,right,parameters);
}}
}}
''')
    target.mkdir(parents=True)
    wasm = target / 'effect.wasm'
    command = [cxx, '--target=wasm32', '-std=c++11', '-O2', '-fno-exceptions', '-fno-rtti',
               '-ffreestanding', '-fno-builtin', '-nostdlib', '-I' + str(sdk / 'sdk/include'),
               str(bridge), str(sdk / 'sdk/platform/blackfin/memory.cpp'),
               '-Wl,--no-entry', '-Wl,--export-memory', '-Wl,--initial-memory=2097152',
               '-Wl,--max-memory=2097152', '-Wl,-z,stack-size=65536']
    command += ['-Wl,--export=' + name for name in ('xy_left', 'xy_right', 'xy_parameters', 'xy_reset', 'xy_process')]
    subprocess.run(command + ['-o', str(wasm)], check=True)
    dry = next(i for i, parameter in enumerate(definition['parameters']) if parameter['id'] == item['metadata']['dryParameter'])
    manifest = dict(kind='effect', definition=definition, description=description, dryIndex=dry,
                    sample_rate=44100, wasm_sha256=hashlib.sha256(wasm.read_bytes()).hexdigest())
    (target / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
    for name in ('app.mjs', 'worklet.mjs', 'demo.mjs', 'style.css'):
        shutil.copyfile(ROOT / 'site/effect' / name, target / name)
    shutil.copyfile(sdk / 'sdk/browser/controls.mjs', target / 'controls.mjs')
    page = (ROOT / 'site/effect/index.html').read_text()
    for key, value in {'NAME':definition['name'], 'DESCRIPTION':description, 'GUIDE':guide}.items():
        page = page.replace('{{' + key + '}}', html.escape(value, quote=True))
    if ident.startswith('hk'):
        page = page.replace('{{STATUS}}', 'Experimental browser audition. These HK effects have not passed OP-XY listening tests; check the guide before building for hardware.')
    else:
        page = page.replace('{{STATUS}}', 'Browser audition of the machine’s sound engine. Hardware sound and performance still need device testing; see the guide for details.')
    (target / 'index.html').write_text(page)
