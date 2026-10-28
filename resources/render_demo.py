"""Render dry host auditions of the actual machine. Outputs stay outside the repository."""
from pathlib import Path
import argparse
import json
import subprocess
import tempfile
import wave
import numpy as np

ROOT=Path(__file__).resolve().parents[1]

def build_renderer(folder, sdk, cxx):
    source=folder/'render.cpp'
    source.write_text('#include '+json.dumps(str(ROOT/'engine.cpp'))+'\n'+r'''
#include <cstdio>
#include <cstdlib>
int main(int argc,char**argv){
    if(argc!=11)return 1;
    westfold::Voice voice={};westfold::Shared shared;
    opxy::i32 p[8],out[64];opxy::u32 pitch[64];
    for(int i=0;i<8;++i)p[i]=static_cast<opxy::i32>(std::atof(argv[i+1])*2147483647.0);
    const double hz=std::atof(argv[9]);const int blocks=std::atoi(argv[10]);
    for(auto& x:pitch)x=static_cast<opxy::u32>(hz/44100.0*4294967296.0);
    westfold::initialize(voice,shared,1);westfold::start(voice,shared,1);
    for(int b=0;b<blocks;++b){westfold::render(voice,shared,1,out,p,pitch);std::fwrite(out,4,64,stdout);}
    return 0;
}
''')
    binary=folder/'render'
    subprocess.run([cxx,'-std=c++11','-O2','-I'+str(sdk/'sdk/include'),str(source),'-o',str(binary)],check=True)
    return binary

def render(binary, values, hz, seconds):
    blocks=int(np.ceil(seconds*44100/64))
    result=subprocess.run([str(binary),*map(str,values),str(hz),str(blocks)],capture_output=True,check=True)
    return np.frombuffer(result.stdout,dtype='<i4').astype(np.float64)/2147483648

def save(path, data):
    with wave.open(str(path),'wb') as output:
        output.setnchannels(1);output.setsampwidth(2);output.setframerate(44100)
        output.writeframes(np.rint(np.clip(data,-1,1)*32767).astype('<i2').tobytes())

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--out',type=Path,required=True)
    parser.add_argument('--sdk',type=Path,default=ROOT.parent/'open-xy')
    parser.add_argument('--cxx',default='c++')
    args=parser.parse_args()
    out=args.out.resolve()
    if out.is_relative_to(ROOT):parser.error('Keep auditions outside the machine repository')
    out.mkdir(parents=True,exist_ok=False)
    definition=json.loads((ROOT/'definition.json').read_text())
    keys=[p['id'] for p in definition['parameters']]
    tour=[];report={}
    with tempfile.TemporaryDirectory(prefix='westfold-render-') as folder:
        binary=build_renderer(Path(folder),args.sdk.resolve(),args.cxx)
        for name,preset in definition['presets'].items():
            values=[preset[k] for k in keys]
            audio=np.zeros(44100*5)
            for time,note,length in [(0,48,.3),(.5,55,.3),(1,60,.35),(1.5,63,.4),(2.15,67,.5),(3,60,1.0)]:
                rendered=render(binary,values,440*2**((note-69)/12),5-time)
                env=np.ones(len(rendered))
                release=int(length*44100)
                env[release:]=np.exp(-np.arange(len(env)-release)/(44100*.35))
                at=int(time*44100);count=min(len(audio)-at,len(rendered))
                audio[at:at+count]+=rendered[:count]*env[:count]*2.5
            audio[-4410:]*=np.linspace(1,0,4410)
            save(out/(name.lower().replace(' ','-')+'.wav'),audio)
            tour.extend([audio,np.zeros(11025)])
            report[name]={'peak':float(np.max(np.abs(audio))),'rms':float(np.sqrt(np.mean(audio**2)))}
        save(out/'westfold-preset-tour.wav',np.concatenate(tour))
        # Coherent tones expose non-harmonic foldback, separately from intentional FM.
        values=[.35,.9,.85,1,1/7,0,.3,0]
        for hz in [110,440,2000,6000,12000]:
            data=render(binary,values,hz,1.3)[-44100:]
            spectrum=np.abs(np.fft.rfft(data*np.hanning(len(data))))**2
            allowed=np.zeros(len(spectrum),dtype=bool);allowed[:8]=True
            for harmonic in range(1,int(22050/hz)+1):
                center=int(round(harmonic*hz));allowed[max(0,center-4):center+5]=True
            unwanted=float(spectrum[~allowed].sum())
            total=float(spectrum.sum())
            alias_db=float(10*np.log10(max(1e-20,unwanted/max(total,1e-20))))
            report[f'static_alias_{hz}Hz']={'non_harmonic_energy_db':alias_db}
            if alias_db>-42:raise RuntimeError(f'Non-harmonic energy too high at {hz} Hz: {alias_db:.1f} dB')
    (out/'audition-report.json').write_text(json.dumps(report,indent=2)+'\n')
    print(json.dumps(report,indent=2))

if __name__=='__main__':main()
