#!/usr/bin/env python3
"""Write behavioral checks and run the SDK's machine contracts for the ports."""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor, as_completed
import json
import subprocess
import sys

ROOT=Path(__file__).resolve().parents[1]
SDK=ROOT.parents[2] / 'open-xy'
PYTHON='/Users/tapsy/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/bin/python3'
CXX='/opt/homebrew/opt/llvm/bin/clang++'

def tests(port):
    p=Path(port['path']);ident=port['id'];d=json.loads((p/'definition.json').read_text())
    values=[round(x['default']*2147483647) for x in d['parameters']];values+=[0]*(8-len(values))
    q=','.join(map(str,values))
    if port['kind']=='synth':
        text=f'''// Sound, buffer ownership and interleaved voice isolation checks.
#include "../engine.cpp"
#include <cstdio>
using namespace {ident};
static Voice target, other;
static Shared shared;
static opxy::i32 reference[256][64];
struct Guarded {{ opxy::u32 before; opxy::i32 samples[64]; opxy::u32 after; }};
int main() {{
    opxy::i32 parameters[8]={{{q}}};
    const opxy::i32 unchanged[8]={{{q}}};
    opxy::u32 pitch[64]; for(unsigned i=0;i<64;++i) pitch[i]=25480391u;
    bool audible=false;
    for(unsigned pass=0;pass<2;++pass) {{
        initialize(target,shared,1);start(target,shared,1);
        initialize(other,shared,2);start(other,shared,2);
        for(unsigned block=0;block<256;++block) {{
            Guarded out={{0xdeadbeefu,{{}},0xabcdef01u}};
            render(target,shared,1,out.samples,parameters,pitch);
            if(out.before!=0xdeadbeefu || out.after!=0xabcdef01u) return 10;
            for(unsigned i=0;i<64;++i) {{
                if(out.samples[i]<-1073741824 || out.samples[i]>1073741824) return 11;
                audible=audible || out.samples[i]!=0;
                if(pass==0) reference[block][i]=out.samples[i];
                else if(reference[block][i]!=out.samples[i]) {{std::puts("voice state leaked between instances");return 12;}}
                if(pitch[i]!=25480391u) return 13;
            }}
            if(pass) {{
                opxy::i32 alternate[8];for(unsigned i=0;i<8;++i) alternate[i]=i&1?0:0x7fffffff;
                render(other,shared,2,out.samples,alternate,pitch);
            }}
        }}
        destroy(target,shared,1);destroy(other,shared,2);
    }}
    for(unsigned i=0;i<8;++i) if(parameters[i]!=unchanged[i]) return 14;
    if(!audible) {{std::puts("default patch is silent");return 15;}}
    return 0;
}}
'''
    else:
        dry=port['labels'].index('Dry')
        transport=port['name'] in ['stutter','in_a_room']
        tr='clock,' if transport else ''
        extra='opxy::Transport clock={1200,0,true};' if transport else ''
        tick='clock.ticks96=block*64*192/44100;' if transport else ''
        capture='if(block==200) p[7]=other[7]=0x7fffffff;' if port['name']=='in_a_room' else ''
        size=d['resources']['bufferBytes']//4
        text=f'''// Dirty-buffer recovery, send isolation, dry routing and output guards.
#include "../fx.cpp"
#include <cstdio>
using namespace {ident};
struct Memory {{ opxy::u32 before; alignas(8) opxy::i32 words[{size}]; opxy::u32 after; }};
static Memory a,b;
int main() {{
    a.before=b.before=0x12345678u;a.after=b.after=0x87654321u;
    for(unsigned i=0;i<{size};++i) {{a.words[i]=0x42424242;b.words[i]=-12424242;}}
    State first={{}},second={{}};initialize(first);initialize(second);
    opxy::i32 p[8]={{{q}}}, other[8]={{{q}}};other[{dry}]=0;
    {extra}
    bool audible=false;
    for(unsigned block=0;block<4000;++block) {{
        {tick}
        {capture}
        opxy::i32 l[64],r[64],l2[64],r2[64];
        for(unsigned i=0;i<64;++i) {{
            const unsigned t=block*64+i;
            l[i]=l2[i]=static_cast<opxy::i32>((t*100003u)&0x3fffffffu)-0x20000000;
            r[i]=r2[i]=static_cast<opxy::i32>((t*73333u)&0x3fffffffu)-0x20000000;
        }}
        process(first,opxy::Buffer{{a.words,{size}}},{tr}l,r,p);
        process(second,opxy::Buffer{{b.words,{size}}},{tr}l2,r2,other);
        if(a.before!=0x12345678u||b.before!=0x12345678u||a.after!=0x87654321u||b.after!=0x87654321u) return 20;
        for(unsigned i=0;i<64;++i) {{
            if(l[i]!=l2[i]||r[i]!=r2[i]) {{std::puts("dirty memory or Dry leaked into wet output");return 21;}}
            audible=audible||l[i]!=0||r[i]!=0;
        }}
    }}
    if(!audible) {{std::puts("default effect never produced output");return 22;}}
    initialize(first);
    for(unsigned block=0;block<200;++block) {{
        opxy::i32 l[64]={{}},r[64]={{}};
        process(first,opxy::Buffer{{a.words,{size}}},{tr}l,r,p);
        for(unsigned i=0;i<64;++i) if(l[i]||r[i]) {{std::puts("old effect tail survived reset");return 23;}}
    }}
    return 0;
}}
'''
    (p/'tests/sound.cpp').write_text(text)
    if (p/'wide.hpp').exists():
        (p/'tests/wide.cpp').write_text(f'''// Check DSP wide arithmetic against the host's built-in 64-bit arithmetic.
#include "../wide.hpp"
#include <cstdint>
using {ident}::port::Wide;
int main() {{
    uint32_t random=0x4856837u;
    for(unsigned i=0;i<10000;++i) {{
        random=random*1664525u+1013904223u;const uint64_t high=random;
        random=random*1664525u+1013904223u;const int64_t a=static_cast<int64_t>((high<<32)|random);
        random=random*1664525u+1013904223u;const int64_t b=static_cast<int32_t>(random)|1;
        const uint64_t product=static_cast<uint64_t>(a)*static_cast<uint64_t>(b);
        if(static_cast<uint64_t>(Wide(a)*Wide(b))!=product)return 1;
        if(static_cast<int64_t>(Wide(a)/Wide(b))!=a/b)return 2;
        if(static_cast<int64_t>(Wide(a)%Wide(b))!=a%b)return 3;
        if(static_cast<int64_t>(Wide(a)>>unsigned(i%63))!=(a>>unsigned(i%63)))return 4;
    }}
    return 0;
}}
''')

def run(port):
    ident=port['id'];log=ROOT/'local'/('test-'+ident+'.log')
    command=[PYTHON,str(SDK/'sdk.py'),'test','--machine',port['path'],'--cxx',CXX,'--host-cxx',CXX,'--no-cpu']
    completed=subprocess.run(command,cwd=SDK,capture_output=True,text=True,timeout=300)
    log.write_text(completed.stdout+completed.stderr)
    return ident,completed.returncode,log.read_text()[-2200:]

def main():
    ports=json.loads((ROOT/'local/ports.json').read_text());selected=set(sys.argv[1:])
    ports=[p for p in ports if not selected or p['name'] in selected or p['id'] in selected]
    for p in ports:tests(p)
    with ThreadPoolExecutor(max_workers=4) as pool:
        for future in as_completed([pool.submit(run,p) for p in ports]):
            ident,code,out=future.result();print(('PASS' if code==0 else 'FAIL')+': '+ident,flush=True)
            if code:print(out,flush=True)

if __name__=='__main__':main()
