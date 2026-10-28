#!/usr/bin/env python3
"""Compile every authored C++ display for the local audition and visual gallery."""
from concurrent.futures import ThreadPoolExecutor,as_completed
from pathlib import Path
import hashlib,json,shutil,subprocess,sys
ROOT=Path(__file__).resolve().parents[1]
SDK=ROOT.parents[2] / 'open-xy'
CXX='/opt/homebrew/opt/llvm/bin/clang++'
EXPORTS=['abi','state_bytes','input_bytes','frame_bytes','state','input','frame','reset','render','valid']
def main():
    out=Path(sys.argv[1]).resolve();dest=out/'visuals';dest.mkdir()
    machines=json.loads((out/'machines.json').read_text())
    def build(m):
        folder=ROOT/m['id'];source=folder if m['kind']=='synth' else folder/'visual';ident=m['id']
        meta=json.loads((source/'visual.json').read_text())
        unit=dest/(ident+'.cpp')
        unit.write_text(f'''#include "{source/'display.cpp'}"
static {ident}::DisplayState state;
static opxy::draw::Input input;
static {ident}::Frame frame;
extern "C" {{
unsigned opxy_display_abi(){{return 2;}}
unsigned opxy_display_state_bytes(){{return sizeof(state);}}
unsigned opxy_display_input_bytes(){{return sizeof(input);}}
unsigned opxy_display_frame_bytes(){{return sizeof(frame);}}
void* opxy_display_state(){{return &state;}}
void* opxy_display_input(){{return &input;}}
void* opxy_display_frame(){{return &frame;}}
void opxy_display_reset(){{{ident}::display_reset(state);}}
void opxy_display_render(){{{ident}::display(state,input,frame);}}
unsigned opxy_display_valid(){{return opxy::draw::valid(frame);}}
}}
''')
        command=[CXX,'--target=wasm32','-std=c++11','-O2','-Wall','-Werror','-Wno-unused','-fno-exceptions','-fno-rtti',
            '-ffreestanding','-fno-builtin','-nostdlib','-I'+str(SDK/'sdk/include'),str(unit),str(SDK/'sdk/platform/blackfin/memory.cpp'),
            '-Wl,--no-entry','-Wl,--export-memory','-Wl,--initial-memory=2097152','-Wl,--max-memory=2097152','-Wl,-z,stack-size=16384']
        command += ['-Wl,--export=opxy_display_'+e for e in EXPORTS]
        r=subprocess.run(command+['-o',str(dest/(ident+'.wasm'))],capture_output=True,text=True)
        if r.returncode:raise RuntimeError(ident+': '+r.stderr)
        shutil.copyfile(source/'panel.svg.fragment',dest/(ident+'.svg'))
        (dest/(ident+'.json')).write_text(json.dumps(meta,indent=2)+'\n')
        return ident
    with ThreadPoolExecutor(max_workers=4) as pool:
        for f in as_completed([pool.submit(build,m) for m in machines]):print('Visual:',f.result(),flush=True)
    for name in ['gallery.html','visuals.mjs','visual-host.mjs','index.html','app.mjs','style.css','worklet.mjs']:
        shutil.copyfile(ROOT/'audition'/name,out/name)
    for name in ['display.mjs','controls.mjs']:shutil.copyfile(SDK/'sdk/browser'/name,out/name)
    shutil.copyfile(SDK/'LICENSE',out/'SDK-LICENSE.txt')
    manifest=json.loads((out/'audition.json').read_text())
    names=['gallery.html','visuals.mjs','visual-host.mjs','display.mjs','controls.mjs','index.html','app.mjs','style.css','worklet.mjs']
    names += [p.relative_to(out).as_posix() for p in dest.iterdir() if p.suffix in ('.wasm','.svg','.json')]
    for name in names:manifest['files'][name]=hashlib.sha256((out/name).read_bytes()).hexdigest()
    for machine in machines:
        for p in (ROOT/machine['id']).rglob('*'):
            if p.is_file() and p.suffix in ('.cpp','.hpp','.h','.json','.fragment'):manifest['source'][str(p)]=hashlib.sha256(p.read_bytes()).hexdigest()
    for name in ['display.mjs','controls.mjs']:
        p=SDK/'sdk/browser'/name;manifest.setdefault('sdkSource',{})[str(p)]=hashlib.sha256(p.read_bytes()).hexdigest()
    (out/'audition.json').write_text(json.dumps(manifest,indent=2)+'\n')
if __name__=='__main__':main()
