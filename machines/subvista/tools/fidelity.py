"""Measure Subvista's block-coefficient filter approximation against a frozen Git revision."""
import argparse
import itertools
import json
import re
from pathlib import Path
import subprocess
import tempfile
import wave
import numpy as np

MACHINE=Path(__file__).resolve().parents[1]
REPO=MACHINE.parents[1]
SOURCE=r'''
#include @BEFORE@
#include @AFTER@
#include <cstdio>
#include <cstdlib>
struct Scenario { int p[8];unsigned increment,mode; };
const Scenario cases[]=@CASES@;
int main(int argc,char**argv) {
    if(argc!=2)return 2;
    FILE* file=std::fopen(argv[1],"wb");if(!file)return 3;
    for(const auto& scenario:cases) {
        subvista_reference::Voice before={};subvista_reference::Shared bs={};
        subvista::Voice after={};subvista::Shared as={};
        subvista_reference::initialize(before,bs,1);subvista::initialize(after,as,1);
        unsigned random=7139;int p[8],a[64],b[64];unsigned pitch[64];float block[64];
        for(unsigned n=0;n<256;++n) {
            for(unsigned i=0;i<8;++i)p[i]=scenario.p[i];
            if(scenario.mode==1)for(unsigned i=0;i<4;++i) {
                const unsigned step=(n*(i+1)*3)%256;
                p[i]=static_cast<int>((step<128?step:255-step)*16777215u);
            }
            if(scenario.mode==2)for(unsigned i=0;i<4;++i)p[i]=static_cast<int>((random=random*1664525u+1013904223u)>>1);
            for(unsigned i=0;i<64;++i) {
                pitch[i]=scenario.increment;
                if(scenario.mode==3)pitch[i]=scenario.increment/2+static_cast<unsigned>((static_cast<unsigned long long>(scenario.increment)*(n*64+i))/8192);
            }
            if(n==127 && scenario.mode){subvista_reference::start(before,bs,1);subvista::start(after,as,1);}
            subvista_reference::render(before,bs,1,a,p,pitch);subvista::render(after,as,1,b,p,pitch);
            if(before.phase!=after.phase || before.detuned!=after.detuned || before.sub!=after.sub ||
               before.envelope!=after.envelope || before.a1!=after.a1 || before.a2!=after.a2 || before.a3!=after.a3)return 4;
            for(auto sample:b)if(sample<-1073741824 || sample>1073741824)return 5;
            for(unsigned i=0;i<64;++i)block[i]=a[i]/2147483648.0f;
            if(std::fwrite(block,sizeof(block),1,file)!=1)return 6;
            for(unsigned i=0;i<64;++i)block[i]=b[i]/2147483648.0f;
            if(std::fwrite(block,sizeof(block),1,file)!=1)return 6;
        }
    }
    return std::fclose(file)!=0;
}
'''

def decibels(ratio):
    return float(20*np.log10(max(float(ratio),1e-15)))

def oscillator_images():
    return dict(unchanged=True,method='Oscillator sources and tables are unchanged from the exact baseline; this candidate changes only the recursive filter.')

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline',default='87ef0be94632955bf72c6560ff91ea588f7111d3')
    parser.add_argument('--sdk',type=Path)
    parser.add_argument('--cxx')
    parser.add_argument('--out',type=Path,required=True)
    args=parser.parse_args()
    config_path=REPO/'local/dev.json'
    config=json.loads(config_path.read_text()) if config_path.exists() else {}
    args.sdk=(args.sdk or Path(config.get('sdk',REPO.parent/'xy-machines-sdk'))).resolve()
    args.cxx=args.cxx or config.get('cxx','c++')
    if not (args.sdk/'sdk/include/opxy/audio.hpp').is_file():parser.error('Pass --sdk with the pinned SDK checkout')
    args.out.mkdir(parents=True,exist_ok=False)
    commit=subprocess.check_output(['git','rev-parse',args.baseline+'^{commit}'],cwd=REPO,text=True).strip()
    definition=json.loads((MACHINE/'definition.json').read_text())
    settings=[('grid '+''.join(map(str,c)),[x/2 for x in c]) for c in itertools.product(range(3),repeat=4)]
    settings.append(('defaults',[x['default'] for x in definition['parameters']]))
    slots=[x['id'] for x in definition['parameters']]
    settings.extend((name,[preset[slot] for slot in slots]) for name,preset in definition['presets'].items())
    frequencies=[27.5,55,110,440,1760,7000]
    cases=[dict(label=label,values=values,frequency=f,mode=0) for label,values in settings for f in frequencies]
    defaults=settings[81][1]
    cases.extend(dict(label=label,values=defaults,frequency=f,mode=mode) for mode,label in [(1,'control sweeps'),(2,'random automation'),(3,'pitch glide')] for f in frequencies)
    cpp=[]
    for c in cases:
        params=[round(x*2147483647) for x in c['values']]+[0]*4
        increment=round(c['frequency']*(2**32)/44100)
        cpp.append('{{'+','.join(map(str,params))+'},'+str(increment)+'u,'+str(c['mode'])+'}')
    with tempfile.TemporaryDirectory(prefix='subvista-fidelity-') as directory:
        scratch=Path(directory);before=scratch/'before';before.mkdir()
        paths=subprocess.check_output(['git','ls-tree','-r','--name-only',commit,'--','machines/subvista'],cwd=REPO,text=True).splitlines()
        for path in paths:
            relative=Path(path).relative_to('machines/subvista')
            if relative.parent!=Path('.') or relative.suffix not in ('.h','.hpp','.cpp'):continue
            content=subprocess.check_output(['git','show',commit+':'+path],cwd=REPO,text=True)
            (before/relative).write_text(content.replace('namespace subvista {','namespace subvista_reference {'))
        source=SOURCE.replace('@BEFORE@',json.dumps(str(before/'engine.cpp'))).replace('@AFTER@',json.dumps(str(MACHINE/'engine.cpp'))).replace('@CASES@','{'+',\n'.join(cpp)+'}')
        (scratch/'render.cpp').write_text(source);exe=scratch/'render'
        subprocess.run([args.cxx,'-std=c++11','-O2','-I'+str(args.sdk/'sdk/include'),str(scratch/'render.cpp'),'-o',str(exe)],check=True)
        raw=scratch/'audio.f32';subprocess.run([str(exe),str(raw)],check=True)
        audio=np.fromfile(raw,dtype='<f4').reshape(len(cases),256,2,64).transpose(0,2,1,3).reshape(len(cases),2,16384).astype(np.float64)
    window=np.hanning(8192)
    rows=[]
    for c,signals in zip(cases,audio):
        before,after=signals;error=after-before
        rms=float(np.sqrt(np.mean(before*before)));new_rms=float(np.sqrt(np.mean(after*after)));error_rms=float(np.sqrt(np.mean(error*error)))
        a=np.fft.rfft(before[-8192:]*window);b=np.fft.rfft(after[-8192:]*window);power=float(np.sum(np.abs(a)**2))
        active=np.abs(a)>max(np.max(np.abs(a))*10**(-70/20),1e-14)
        mask=active.copy()
        for shift in range(1,4):mask[shift:]|=active[:-shift];mask[:-shift]|=active[shift:]
        novel=float(np.sum(np.abs(b[~mask]-a[~mask])**2))
        centroid=lambda spectrum: float(np.sum(np.arange(len(spectrum))*np.abs(spectrum)**2)/max(float(np.sum(np.abs(spectrum)**2)),1e-30)*44100/8192)
        row={**c,'reference_rms_dbfs':decibels(rms),'candidate_rms_dbfs':decibels(new_rms),'rms_change_db':decibels(new_rms/max(rms,1e-30)),'error_rms_dbfs':decibels(error_rms),'error_relative_db':decibels(error_rms/max(rms,1e-30)),'peak_error':float(np.max(np.abs(error))),'candidate_peak':float(np.max(np.abs(after))),'novel_spectral_error_db':float(10*np.log10(max(novel/max(power,1e-30),1e-30))),'novel_spectral_error_dbfs':decibels(np.sqrt(2*novel/(8192*np.sum(window*window)))),'spectral_centroid_change_hz':centroid(b)-centroid(a)}
        rows.append(row)
    audible=[r for r in rows if r['reference_rms_dbfs']>-60]
    static=[r for r in audible if not r['mode']]
    musical=[r for r in audible if r['label']=='defaults' or r['label'] in definition['presets']]
    group=lambda rs:dict(cases=len(rs),median_error_relative_db=float(np.median([r['error_relative_db'] for r in rs])),worst_error=max(rs,key=lambda r:r['error_relative_db']),worst_loudness=max(rs,key=lambda r:abs(r['rms_change_db'])),worst_novel_spectral_error=max(rs,key=lambda r:r['novel_spectral_error_db']))
    result=dict(baseline=commit,approximation='Three-multiply symmetric filter matrix with block-midpoint coefficients; oscillator, phase, envelope, coefficient endpoints and original state clamps unchanged',sample_rate=44100,frames_per_case=16384,cases=len(cases),phase_envelope_and_coefficient_state_exact=True,oscillator_images=oscillator_images(),output_peak=max(r['candidate_peak'] for r in rows),summary=dict(audible=group(audible),static=group(static),defaults_and_presets=group(musical)),results=rows,note='FFT uses last8192samples with Hann; novel spectral error is difference energy outside reference components above-70dB of its peak, broadened by3bins. This is a spectral/image proxy, not an independently alias-free reference. Loudness/error summaries exclude reference RMS below-60dBFS; all cases remain in results.')
    result['quality_budgets']=dict(maximum_absolute_rms_error_fs=0.0015,maximum_audible_relative_rms_error=0.025,maximum_audible_rms_change_db=0.05)
    result['quality_budget_passed']=bool(max(10**(r['error_rms_dbfs']/20) for r in rows)<=0.0015 and max(r['error_relative_db'] for r in audible)<=20*np.log10(0.025) and max(abs(r['rms_change_db']) for r in audible)<=0.05)
    (args.out/'quality.json').write_text(json.dumps(result,indent=2)+'\n')
    for label,f in [('defaults',110),('grid 2022',55),('random automation',440)]:
        index=next(i for i,c in enumerate(cases) if c['label']==label and c['frequency']==f)
        for channel,name in enumerate(['reference','candidate']):
            path=args.out/(label.replace(' ','-')+'-'+str(f)+'-'+name+'.wav')
            pcm=np.clip(np.rint(audio[index,channel]*32767),-32768,32767).astype('<i2')
            with wave.open(str(path),'wb') as out:out.setnchannels(1);out.setsampwidth(2);out.setframerate(44100);out.writeframes(pcm.tobytes())
    print(json.dumps({k:v for k,v in result.items() if k not in ('results','summary')},indent=2))
    print(json.dumps(result['summary'],indent=2))
    if not result['quality_budget_passed']:raise SystemExit('Quality budget failed; inspect quality.json')

if __name__=='__main__':main()
