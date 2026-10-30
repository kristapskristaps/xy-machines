#!/usr/bin/env python3
"""Compile the actual effect callbacks into local WebAssembly auditions."""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor,as_completed
import hashlib
import json
import shutil
import subprocess
import sys
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT.parents[1]/'tools'))
from workspace import sdk_path
SDK=sdk_path()
CXX='/opt/homebrew/opt/llvm/bin/clang++'

def main():
    out=Path(sys.argv[1]).resolve();live=out/'live';live.mkdir()
    items=[];sources={}
    for entry in json.loads((ROOT/'collection.json').read_text())['machines']:
        p=ROOT/entry['path'];record=json.loads((p/'machine.json').read_text());d=json.loads((p/'definition.json').read_text())
        item={'id':d['id'],'name':d['name'],'kind':record['kind'],'definition':d}
        items.append(item)
        for path in p.rglob('*'):
            if path.is_file() and path.suffix in ['.cpp','.h','.hpp','.json','.fragment']:
                sources[str(path)]=hashlib.sha256(path.read_bytes()).hexdigest()
    def build(item):
        ident=item['id'];d=item['definition'];words=d['resources']['bufferBytes']//4
        transport='transport,' if d['resources'].get('transport') else ''
        bridge=live/(ident+'.cpp')
        bridge.write_text(f'''#include "{ROOT/ident/'fx.cpp'}"
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
        command=[CXX,'--target=wasm32','-std=c++11','-O2','-fno-exceptions','-fno-rtti','-ffreestanding','-fno-builtin','-nostdlib','-I'+str(SDK/'sdk/include'),str(bridge),str(SDK/'sdk/platform/blackfin/memory.cpp'),'-Wl,--no-entry','-Wl,--export-memory','-Wl,--initial-memory=2097152','-Wl,--max-memory=2097152','-Wl,-z,stack-size=65536']
        command+=['-Wl,--export='+n for n in ['xy_left','xy_right','xy_parameters','xy_reset','xy_process']]
        subprocess.run(command+['-o',str(live/(ident+'.wasm'))],check=True,capture_output=True,text=True)
        return ident
    with ThreadPoolExecutor(max_workers=4) as pool:
        for f in as_completed([pool.submit(build,item) for item in items if item['kind']=='effect']):print('Live effect:',f.result(),flush=True)
    (out/'machines.json').write_text(json.dumps(items,indent=2)+'\n')
    for name in ['index.html','app.mjs','worklet.mjs','style.css']:shutil.copyfile(ROOT/'audition'/name,out/name)
    names=['index.html','app.mjs','worklet.mjs','style.css','machines.json']
    names += ['audio/'+p.name for p in sorted((out/'audio').glob('*.wav'))]
    names += ['live/'+p.name for p in sorted(live.glob('*.wasm'))]
    files={name:hashlib.sha256((out/name).read_bytes()).hexdigest() for name in names}
    (out/'audition.json').write_text(json.dumps({'files':files,'source':sources},indent=2)+'\n')
if __name__=='__main__':main()
