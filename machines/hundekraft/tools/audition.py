#!/usr/bin/env python3
"""Build SDK synth previews and render firmware-free WAV auditions for the collection."""
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor,as_completed
import argparse
import json
import subprocess
import sys
import wave
import numpy as np

ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT.parents[1]/'tools'))
from workspace import sdk_path
SDK=sdk_path()
CXX='/opt/homebrew/opt/llvm/bin/clang++'

def ports():
    result=[]
    for entry in json.loads((ROOT/'collection.json').read_text())['machines']:
        folder=ROOT/entry['path'];record=json.loads((folder/'machine.json').read_text());definition=json.loads((folder/'definition.json').read_text())
        result.append(dict(id=definition['id'],name=definition['name'],kind=record['kind'],folder=folder,definition=definition))
    return result

def render_source(items,out):
    includes=[];functions=[];dispatch=[]
    for p in items:
        ident=p['id'];kind=p['kind'];folder=p['folder'];d=p['definition']
        includes.append('#include "'+str(folder/('engine.cpp' if kind=='synth' else 'fx.cpp'))+'"')
        q=[round(c['default']*2147483647) for c in d['parameters']];q+=[0]*(8-len(q))
        head='void audition_'+ident+'(){\nconst opxy::i32 initial[8]={'+','.join(map(str,q))+'};\nopxy::i32 parameters[8];for(unsigned i=0;i<8;++i)parameters[i]=initial[i];\n'
        if kind=='synth':
            body=f'''
    static {ident}::Voice voice[4];{ident}::Shared shared;
    unsigned notes[4]={{48,55,60,63}};long started[4]={{-100000,-100000,-100000,-100000}};
    for(unsigned i=0;i<4;++i){ident}::initialize(voice[i],shared,i+1);
    const unsigned arp[8]={{48,55,60,63,67,70,67,63}};
    for(unsigned block=0;block<blocks;++block){{
        const unsigned frame=block*64;
        if(frame<176400 && (frame/22050)!=((frame>=64?frame-64:~0u)/22050)){{
            const unsigned step=frame/22050,slot=step%4;notes[slot]=arp[step%8];started[slot]=frame;
            {ident}::start(voice[slot],shared,slot+1);
        }}
        if(frame>=176400 && frame-64<176400)for(unsigned i=0;i<4;++i){{notes[i]=arp[i];started[i]=frame;{ident}::start(voice[i],shared,i+1);}}
        float mixed[64]={{}};
        for(unsigned v=0;v<4;++v){{
            const double seconds=(static_cast<long>(frame)-started[v])/44100.;
            const double hold=frame<176400?.35:3.;
            if(seconds<0 || seconds>hold+.5)continue;
            opxy::u32 pitch[64];opxy::i32 audio[64];
            const opxy::u32 increment=static_cast<opxy::u32>(440.*std::pow(2.,(int(notes[v])-69)/12.)*4294967296./44100.);
            for(unsigned i=0;i<64;++i)pitch[i]=increment;
            {ident}::render(voice[v],shared,v+1,audio,parameters,pitch);
            for(unsigned i=0;i<64;++i){{
                const double age=seconds+i/44100.;
                const double amp=std::fmin(1.,age/.008)*std::exp(-.12*age)*(age<hold?1.:std::exp(-(age-hold)/.10));
                mixed[i]+=static_cast<float>(audio[i]/2147483648.*amp*.45);
            }}
        }}
        for(unsigned i=0;i<64;++i)write(mixed[i],mixed[i]);
    }}
'''
        elif kind=='effect':
            words=d['resources']['bufferBytes']//4;tempo='transport,' if d['resources'].get('transport') else ''
            latch=next((i for i,c in enumerate(d['parameters']) if c['label']=='Capture'),None)
            switch=f'if(block==350)parameters[{latch}]=0x7fffffff;' if latch is not None else ''
            body=f'''
    static opxy::i32 memory[{words}];{ident}::State state={{}};{ident}::initialize(state);
    for(unsigned block=0;block<blocks;++block){{
        opxy::i32 left[64],right[64];const opxy::Transport transport={{1200,block*64*192/44100,true}};(void)transport;
        {switch}
        for(unsigned i=0;i<64;++i){{const double t=(block*64+i)/44100.;left[i]=static_cast<opxy::i32>(input(t,0)*2147483647.);right[i]=static_cast<opxy::i32>(input(t,1)*2147483647.);}}
        {ident}::process(state,opxy::Buffer{{memory,{words}}},{tempo}left,right,parameters);
        for(unsigned i=0;i<64;++i)write(left[i]/2147483648.f*.6f,right[i]/2147483648.f*.6f);
    }}
'''
        functions.append(head+body+'}\n');dispatch.append('if(!std::strcmp(argv[1],"'+ident+'")){audition_'+ident+'();return 0;}')
    source='''#include <cstdio>
#include <cmath>
#include <cstring>
'''+ '\n'.join(includes)+'''
const unsigned blocks=6891;
void write(float l,float r){const float audio[2]={l,r};std::fwrite(audio,sizeof(float),2,stdout);}
double input(double t,unsigned channel){
    if(t<.5 || t>6.)return 0.;
    t-=.5;const double beat=std::fmod(t,.5),attack=std::fmin(1.,beat/.003),decay=std::exp(-7*beat);
    const unsigned chord=static_cast<unsigned>(t/2.)%2;const double root=chord?146.8324:130.8128;
    const double spread=channel?1.002:1.;
    const double pad=(std::sin(6.28318530718*root*spread*t)+.6*std::sin(6.28318530718*root*1.5*spread*t)+.35*std::sin(6.28318530718*root*2.378414*spread*t))*.065;
    const double kick=std::sin(6.28318530718*(55*beat+2*(1-std::exp(-30*beat))))*std::exp(-30*beat)*.13;
    return (pad*decay+kick)*attack;
}
'''+ '\n'.join(functions)+'''int main(int argc,char**argv){if(argc!=2)return 2;
'''+ '\n'.join(dispatch)+'''if(!std::strcmp(argv[1],"dry")){for(unsigned i=0;i<blocks*64;++i)write(input(i/44100.,0)*.6,input(i/44100.,1)*.6);return 0;}return 3;}
'''
    out.write_text(source)

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--out',required=True,type=Path);parser.add_argument('--previews',action='store_true');args=parser.parse_args()
    out=args.out.resolve();out.mkdir(parents=True,exist_ok=False);items=ports()
    source=out/'render.cpp';render_source(items,source);binary=out/'render'
    subprocess.run([CXX,'-std=c++11','-O2','-I'+str(SDK/'sdk/include'),str(source),'-o',str(binary)],check=True)
    audio=out/'audio';audio.mkdir();metrics={}
    for ident in ['dry']+[p['id'] for p in items]:
        raw=subprocess.check_output([str(binary),ident]);samples=np.frombuffer(raw,dtype=np.float32)
        peak=float(np.max(np.abs(samples)));rms=float(np.sqrt(np.mean(samples*samples)))
        if not np.isfinite(samples).all() or rms<1e-6:raise RuntimeError(f'{ident}: invalid or silent audition ({rms})')
        with wave.open(str(audio/(ident+'.wav')),'wb') as wav:
            wav.setnchannels(2);wav.setsampwidth(2);wav.setframerate(44100);wav.writeframes((np.clip(samples,-.95,.95)*32767).astype('<i2').tobytes())
        metrics[ident]={'peak':peak,'rms':rms};print('Rendered',ident,'RMS',round(rms,5),flush=True)
    (out/'metrics.json').write_text(json.dumps(metrics,indent=2)+'\n')
    if args.previews:
        def build(p):
            dest=out/'previews'/p['id'];command=[sys.executable,str(SDK/'sdk.py'),'preview','--machine',str(p['folder']),'--out',str(dest),'--cxx',CXX]
            result=subprocess.run(command,cwd=SDK,capture_output=True,text=True)
            (out/(p['id']+'-preview.log')).write_text(result.stdout+result.stderr)
            if result.returncode:raise RuntimeError(result.stderr)
            return p['id']
        with ThreadPoolExecutor(max_workers=4) as pool:
            for future in as_completed([pool.submit(build,p) for p in items if p['kind']=='synth']):print('Preview',future.result(),flush=True)
    (out/'machines.json').write_text(json.dumps([{'id':p['id'],'name':p['name'],'kind':p['kind']} for p in items],indent=2)+'\n')
    print(out,flush=True)

if __name__=='__main__':main()
