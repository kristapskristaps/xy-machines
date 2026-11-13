"""Render dry factory patches and an original game loop with the actual engine."""
from pathlib import Path
import argparse
import json
import subprocess
import tempfile
import wave
import numpy as np

ROOT = Path(__file__).resolve().parents[1]


def renderer(folder, sdk, cxx):
    source = folder/'render.cpp'
    source.write_text('#include '+json.dumps(str(ROOT/'engine.cpp'))+'\n'+r'''
#include <cstdio>
#include <cstdlib>
int main(int argc,char** argv){
    if(argc!=11)return 1;
    bitflip::Voice voice={};bitflip::Shared shared={};
    opxy::i32 p[8],out[64];opxy::u32 pitch[64];
    for(unsigned i=0;i<8;++i)p[i]=static_cast<opxy::i32>(std::atof(argv[i+1])*2147483647.0);
    for(auto& value:pitch)value=static_cast<opxy::u32>(std::atof(argv[9])*4294967296.0/44100);
    bitflip::start(voice,shared,1);
    const int blocks=std::atoi(argv[10]);
    for(int b=0;b<blocks;++b){bitflip::render(voice,shared,1,out,p,pitch);std::fwrite(out,4,64,stdout);}
}
''')
    binary=folder/'render'
    subprocess.run([cxx,'-std=c++11','-O2','-I'+str(sdk/'sdk/include'),str(source),'-o',str(binary)],check=True)
    return binary


def raw(binary,params,hz,seconds):
    count=int(np.ceil(seconds*44100/64))
    result=subprocess.run([str(binary),*map(str,params),str(hz),str(count)],capture_output=True,check=True)
    return np.frombuffer(result.stdout,dtype='<i4').astype(np.float64)/2147483648


def save(path,data):
    with wave.open(str(path),'wb') as handle:
        handle.setnchannels(1);handle.setsampwidth(2);handle.setframerate(44100)
        handle.writeframes(np.rint(np.clip(data,-1,1)*32767).astype('<i2').tobytes())


def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--sdk',type=Path,required=True)
    parser.add_argument('--out',type=Path,required=True)
    parser.add_argument('--cxx',default='c++')
    args=parser.parse_args();out=args.out.resolve()
    if out.is_relative_to(ROOT.parents[1]):parser.error('Keep generated audio and binaries outside the source checkout.')
    out.mkdir(parents=True,exist_ok=False)
    definition=json.loads((ROOT/'definition.json').read_text())
    keys=[p['id'] for p in definition['parameters']]
    presets={name:[values[key] for key in keys] for name,values in definition['presets'].items()}
    report={'audio':'Dry host render with a demonstration amplitude gate; not the stock AMP envelope.','presets':{},'static_alias':{}}
    with tempfile.TemporaryDirectory(prefix='bitflip-render-') as temp:
        binary=renderer(Path(temp),args.sdk.resolve(),args.cxx)
        def note(params,midi,duration,release=.10,decay=None):
            data=raw(binary,params,440*2**((midi-69)/12),duration+release*6)
            t=np.arange(len(data))/44100
            gate=np.minimum(1,t/.001)
            if decay:gate*=np.exp(-t/decay)
            gate*=np.exp(-np.maximum(0,t-duration)/release)
            return data*gate
        tour=[]
        for name,params in presets.items():
            data=np.zeros(44100*3)
            for at,midi,length in [(0,60,.20),(.35,67,.20),(.70,72,.20),(1.1,75,.20),(1.5,72,.85)]:
                decay=.09 if name in ['Metal Coin','Pixel Dust','Short Circuit','Laser Down'] else None
                sound=note(params,midi,length,decay=decay)*2.5
                start=int(at*44100);count=min(len(sound),len(data)-start)
                data[start:start+count]+=sound[:count]
            data[-2205:]*=np.linspace(1,0,2205)
            peak=float(np.max(np.abs(data)));rms=float(np.sqrt(np.mean(data**2)))
            if peak>.98 or rms<.0001:raise RuntimeError('Inaudible or clipped factory patch: '+name)
            save(out/(name.lower().replace(' ','-')+'.wav'),data)
            report['presets'][name]={'peak':peak,'rms':rms}
            tour.extend([data,np.zeros(6615)])
        save(out/'bitflip-preset-tour.wav',np.concatenate(tour))
        # The periodic models should not produce substantial nonharmonic foldback
        # with intentional DAC reduction and free chord stepping switched off.
        for model in [0,1,3,4,5,7]:
            for hz in [110,440,2000,6000]:
                params=[(model+.5)/8,.55,.60,.5,0,.4,1,1]
                data=raw(binary,params,hz,1.4)[-44100:]
                power=np.abs(np.fft.rfft(data*np.hanning(len(data))))**2
                allowed=np.zeros(len(power),dtype=bool);allowed[:8]=True
                for h in range(1,int(22050/hz)+1):
                    center=round(h*hz);allowed[max(0,center-4):center+5]=True
                db=float(10*np.log10(max(1e-20,power[~allowed].sum()/max(1e-20,power.sum()))))
                report['static_alias'][f'model_{model}_{hz}Hz']=db
                if db>-42:raise RuntimeError(f'Static nonharmonic energy too high: model {model}, {hz} Hz, {db:.1f} dB')
        beat=60/132;bar=4*beat
        song=np.zeros(int((8*bar+1.5)*44100))
        def add(params,midi,at,length,volume,decay=None):
            data=note(params,midi,length,decay=decay)*volume
            start=int(at*44100);count=min(len(data),len(song)-start)
            song[start:start+count]+=data[:count]
        melody=[
            [72,75,79,82,79,75,74,70], [72,75,79,84,82,79,75,74],
            [68,72,75,79,75,72,70,67], [68,72,75,80,79,75,72,70],
            [65,68,72,75,72,68,67,63], [65,68,72,77,75,72,68,67],
            [67,71,74,77,74,71,69,65], [67,71,74,79,77,74,71,72],
        ]
        roots=[48,48,44,44,41,41,43,48]
        for b,notes in enumerate(melody):
            for n,midi in enumerate(notes):add(presets['Pocket Hero'],midi,b*bar+n*beat/2,beat*.37,2.0)
            for n,offset in enumerate([0,0,7,12]):add(presets['Triangle Bass'],roots[b]-12+offset,b*bar+n*beat,beat*.72,1.8)
            arp=presets['Minor Boss'].copy();arp[1]=.18;arp[2]=.08
            if b==6:arp[4]=2.5/8
            add(arp,roots[b],b*bar,bar*.94,.45)
            for n in range(8):add(presets['Pixel Dust'],76,b*bar+n*beat/2,.035,.27,decay=.018)
            for n in [1,3]:add(presets['Short Circuit'],57,b*bar+n*beat,.075,.55,decay=.035)
            kick=presets['Triangle Bass'].copy();kick[3]=1
            for n in [0,2]:add(kick,33,b*bar+n*beat,.10,1.6,decay=.045)
        add(presets['Power Up'],84,8*bar,.45,1.4)
        gain=min(1,.9/max(.0001,float(np.max(np.abs(song)))))
        song*=gain;song[-4410:]*=np.linspace(1,0,4410)
        save(out/'bitflip-game-loop.wav',song)
        report['game_loop']={'tempo_bpm':132,'bars':8,'master_gain':gain,'peak':float(np.max(np.abs(song))),'rms':float(np.sqrt(np.mean(song**2)))}
    (out/'audition-report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))


if __name__=='__main__':main()
