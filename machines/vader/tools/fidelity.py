"""Measure Vader's deliberate shaped-wave lookup approximation against a frozen Git revision."""
import argparse
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
        vader_frozen::Voice before={};vader_frozen::Shared bs={};
        vader::Voice after={};vader::Shared as={};
        vader_frozen::initialize(before,bs,1);vader::initialize(after,as,1);
        unsigned random=7139;int p[8],a[64],b[64];unsigned pitch[64];float block[64];
        for(unsigned n=0;n<256;++n) {
            for(unsigned i=0;i<8;++i)p[i]=scenario.p[i];
            if(scenario.mode==1)for(unsigned i=0;i<8;++i) {
                const unsigned step=(n*(i+1)*3)%256;
                p[i]=static_cast<int>((step<128?step:255-step)*16777215u);
            }
            if(scenario.mode==2)for(unsigned i=0;i<8;++i)p[i]=static_cast<int>((random=random*1664525u+1013904223u)>>1);
            for(unsigned i=0;i<64;++i) {
                pitch[i]=scenario.increment;
                if(scenario.mode==3)pitch[i]=scenario.increment/2+static_cast<unsigned>((static_cast<unsigned long long>(scenario.increment)*(n*64+i))/8192);
            }
            if(n==127 && scenario.mode){vader_frozen::start(before,bs,1);vader::start(after,as,1);}
            vader_frozen::render(before,bs,1,a,p,pitch);vader::render(after,as,1,b,p,pitch);
            if(before.phase!=after.phase || before.pending!=after.pending || before.sweep!=after.sweep ||
               before.fade!=after.fade || before.drive!=after.drive || before.sub!=after.sub ||
               before.color!=after.color || before.boost!=after.boost || before.noise!=after.noise ||
               before.click!=after.click || before.click_low!=after.click_low)return 4;
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
    source=(MACHINE/'tables.hpp').read_text()
    old={name:np.asarray(list(map(int,values.split(','))),dtype=np.int64) for name,values in re.findall(r'const opxy::i16 (wave_\d+)\[513\] = \{([^}]+)\};',source)}
    grid=re.search(r'waves\[4\]\[9\]\[6\] = \{(.*?)\n\};',source,re.S).group(1)
    names=re.findall(r'wave_\d+',grid)
    if len(names)!=216:raise ValueError('Expected216 slots')
    frames=65536;bins=[10463,5231,2617,1309,655,327];limits=[1,3,7,15,31,63]
    rows=[]
    for slot,name in enumerate(names):
        band=slot%6;bin_index=bins[band]
        phase=(np.arange(frames,dtype=np.int64)*(bin_index<<16))&0xffffffff
        at=phase>>23;fraction=(phase>>8)&32767
        before=old[name][at]+((old[name][at+1]-old[name][at])*fraction>>15)
        after=old[name][((phase+0x400000)&0xffffffff)>>23]
        mask=np.zeros(frames//2+1,dtype=bool);mask[0]=True
        for harmonic in range(1,limits[band]+1):
            harmonic_bin=(bin_index*harmonic)%frames;mask[min(harmonic_bin,frames-harmonic_bin)]=True
        powers=[]
        for signal in (before,after):
            power=np.abs(np.fft.rfft(signal.astype(np.float64)))**2
            powers.append(float(10*np.log10(max(float(np.sum(power[~mask]))/max(float(np.sum(power[mask])),1e-30),1e-30))))
        rows.append(dict(mode=slot//54,level=(slot//6)%9,band=band,frequency=bin_index*44100/frames,reference_image_energy_db=powers[0],candidate_image_energy_db=powers[1]))
    return dict(frames=frames,method='Coherent single-bank render in all216 original slots, without drive/mip blend, filter, click or pure-sine foundation. Energy outside the bank harmonic limits1/3/7/15/31/63 includes reconstruction and folded images.',results=rows,worst_candidate=max(rows,key=lambda row:row['candidate_image_energy_db']))

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline',default='97b6a9cc011169159625186d57b5e040e31dfee7')
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
    defaults=[x['default'] for x in definition['parameters']]
    settings=[('defaults',defaults)]
    slots=[x['id'] for x in definition['parameters']]
    settings.extend((name,[preset[slot] for slot in slots]) for name,preset in definition['presets'].items())
    for mode in range(4):
        for drive in range(9):
            for tone in (0.25,1):
                values=defaults.copy();values[2]=drive/8;values[3]=(mode+0.5)/4;values[6]=tone
                settings.append((f'shape{mode} drive{drive}/8 tone{tone}',values))
    frequencies=[27.5,55,110,440,1760,7000]
    cases=[dict(label=label,values=values,frequency=f,mode=0) for label,values in settings for f in frequencies]
    cases.extend(dict(label=f'corner{mask}',values=[1 if mask&(1<<i) else 0 for i in range(8)],frequency=f,mode=0) for mask in range(256) for f in (27.5,110,880,7000))
    cases.extend(dict(label=label,values=defaults,frequency=f,mode=mode) for mode,label in [(1,'control sweeps'),(2,'random automation'),(3,'pitch glide')] for f in frequencies)
    cpp=[]
    for c in cases:
        params=[round(x*2147483647) for x in c['values']]
        increment=round(c['frequency']*(2**32)/44100)
        cpp.append('{{'+','.join(map(str,params))+'},'+str(increment)+'u,'+str(c['mode'])+'}')
    with tempfile.TemporaryDirectory(prefix='vader-fidelity-') as directory:
        scratch=Path(directory);before=scratch/'before';before.mkdir()
        paths=subprocess.check_output(['git','ls-tree','-r','--name-only',commit,'--','machines/vader'],cwd=REPO,text=True).splitlines()
        for path in paths:
            relative=Path(path).relative_to('machines/vader')
            if relative.parent!=Path('.') or relative.suffix not in ('.h','.hpp','.cpp'):continue
            content=subprocess.check_output(['git','show',commit+':'+path],cwd=REPO,text=True)
            (before/relative).write_text(content.replace('namespace vader {','namespace vader_frozen {'))
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
    result=dict(baseline=commit,approximation='Nearest512 original shaped-wave points; pure foundation/filter/phase/control updates unchanged',sample_rate=44100,frames_per_case=16384,cases=len(cases),phase_and_nonfilter_voice_state_exact=True,oscillator_images=oscillator_images(),output_peak=max(r['candidate_peak'] for r in rows),summary=dict(audible=group(audible),driven=group([r for r in audible if r['values'][2]>0 and r['values'][5]<0.95]),static=group(static),defaults_and_presets=group(musical)),results=rows,note='FFT uses last8192samples with Hann; novel spectral error is difference energy outside reference components above-70dB of its peak, broadened by3bins. This is a spectral/image proxy, not an independently alias-free reference. Loudness/error summaries exclude reference RMS below-60dBFS; all cases remain in results. The amplitude-aware3%error budget has a-78dBFS absolute floor to avoid meaningless relative penalties on nearly muted signals.')
    result['quality_budgets']=dict(maximum_absolute_rms_error_fs=0.01,maximum_relative_rms_error=0.03,absolute_error_floor_fs=0.000125,maximum_audible_rms_change_db=0.06,maximum_oscillator_image_energy_db=-32.0)
    result['quality_budget_passed']=bool(all(10**(r['error_rms_dbfs']/20)<=max(0.03*10**(r['reference_rms_dbfs']/20),0.000125) for r in rows) and max(10**(r['error_rms_dbfs']/20) for r in rows)<=0.01 and max(abs(r['rms_change_db']) for r in audible)<=0.06 and result['oscillator_images']['worst_candidate']['candidate_image_energy_db']<=-32.0)
    (args.out/'quality.json').write_text(json.dumps(result,indent=2)+'\n')
    for label,f in [('defaults',110),('shape3 drive8/8 tone1',55),('random automation',440)]:
        index=next(i for i,c in enumerate(cases) if c['label']==label and c['frequency']==f)
        for channel,name in enumerate(['reference','candidate']):
            path=args.out/(label.replace(' ','-').replace('/','-')+'-'+str(f)+'-'+name+'.wav')
            pcm=np.clip(np.rint(audio[index,channel]*32767),-32768,32767).astype('<i2')
            with wave.open(str(path),'wb') as out:out.setnchannels(1);out.setsampwidth(2);out.setframerate(44100);out.writeframes(pcm.tobytes())
    print(json.dumps({k:v for k,v in result.items() if k not in ('results','summary')},indent=2))
    print(json.dumps(result['summary'],indent=2))
    if not result['quality_budget_passed']:raise SystemExit('Quality budget failed; inspect quality.json')

if __name__=='__main__':main()
