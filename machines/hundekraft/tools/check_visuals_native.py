#!/usr/bin/env python3
"""Cross-compile display callbacks, including unregistered effect/sample studies."""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor,as_completed
import argparse,json,sys
ROOT=Path(__file__).resolve().parents[1]
SDK=ROOT.parents[2] / 'open-xy'
sys.path.insert(0,str(SDK))
from tooling.native import compile_cxx,compiler_paths
from tooling.pipeline import tool_paths
p=argparse.ArgumentParser();p.add_argument('--out',required=True,type=Path);args=p.parse_args()
out=args.out.resolve()
if out.is_relative_to(ROOT) or out.is_relative_to(SDK):raise SystemExit('Keep native outputs outside checkouts')
out.mkdir(parents=True,exist_ok=False)
compiler=compiler_paths(SDK/'local/toolchain')['cxx']
assembler=tool_paths(SDK/'local/toolchain/bin')['as']
def check(folder):
    record=json.loads((folder/'machine.json').read_text());ident=folder.name
    source=folder if record['kind']=='synth' else folder/'visual'
    cpp=out/(ident+'.cpp');obj=out/(ident+'.o')
    cpp.write_text(f'''#include "{source/'display.cpp'}"
extern "C" void render_{ident}({ident}::DisplayState* s,const opxy::draw::Input* in,{ident}::Frame* frame){{{ident}::display(*s,*in,*frame);}}
''')
    compile_cxx(compiler,assembler,cpp,obj,[SDK/'sdk/include',SDK/'sdk/platform/blackfin/freestanding'])
    return ident
with ThreadPoolExecutor(max_workers=4) as pool:
    for f in as_completed([pool.submit(check,p.parent) for p in sorted(ROOT.glob('hk*/machine.json'))]):print('PASS: Blackfin display callback',f.result(),flush=True)
print('Display objects only; effect/sample studies are not registered in firmware.',flush=True)
