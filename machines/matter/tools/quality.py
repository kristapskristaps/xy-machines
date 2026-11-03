"""Compare rendered Matter sound against a Git revision, without requiring equal PCM."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

import numpy as np

MACHINE = Path(__file__).resolve().parents[1]
REPO = MACHINE.parents[1]
RATE = 44100
SOURCE = r'''
#include @BEFORE@
#include @AFTER@
#include <cstdio>
int main(int argc,char** argv) {
    if(argc!=2)return 1;
    const unsigned increments[]={10713070u,25479940u,101919760u};
    const int maximum=2147483647;
    const unsigned moving_cases=@AUTOMATION@?12:0;
    for(unsigned test=0;test<90+moving_cases;++test) {
        const bool moving=test>=90;
        const unsigned extra=moving?test-90:0;
        const unsigned model=moving?extra%6:test/15;
        const unsigned variant=moving?0:test/3%5,note=moving?1:test%3;
        matter_before::Voice before={};matter_before::Shared bs={};
        matter::Voice after={};matter::Shared as={};
        matter_before::initialize(before,bs,1);matter::initialize(after,as,1);
        int p[8]={maximum/5,maximum/2,maximum*3LL/10,maximum*3LL/10,0,maximum/2,maximum*6LL/10,maximum};
        int old_samples[64],new_samples[64];unsigned pitch[64];
        if(variant==1){p[0]=0;p[1]=maximum/4;p[2]=maximum/8;}
        if(variant==2)for(unsigned i=0;i<8;++i)p[i]=maximum;
        if(variant==3)p[7]=0;
        if(variant==4){for(unsigned i=0;i<8;++i)p[i]=0;p[7]=maximum;}
        p[4]=int((2*model+1)*2147483647LL/12);
        for(auto& x:pitch)x=increments[note];
        char name[4096];
        if(moving)std::snprintf(name,sizeof(name),"%s/moving-model-%u-type-%u.pcm",argv[1],model,extra/6);
        else std::snprintf(name,sizeof(name),"%s/model-%u-variant-%u-note-%u.pcm",argv[1],model,variant,note);
        std::FILE* file=std::fopen(name,"wb");if(!file)return 2;
        for(unsigned block=0;block<1024;++block) {
            if(moving && !(block%16)) {
                unsigned random=7139+block/16;
                for(unsigned i=0;i<8;++i)if(i!=4){random=random*1664525u+1013904223u;p[i]=extra<6?int(random>>1):((block/16+i)&1)?maximum:0;}
            }
            if(block==512){matter_before::start(before,bs,1);matter::start(after,as,1);}
            matter_before::render(before,bs,1,old_samples,p,pitch);
            matter::render(after,as,1,new_samples,p,pitch);
            int interleaved[128];for(unsigned i=0;i<64;++i){interleaved[2*i]=old_samples[i];interleaved[2*i+1]=new_samples[i];}
            if(std::fwrite(interleaved,sizeof(int),128,file)!=128)return 3;
        }
        std::fclose(file);
    }
}
'''


def features(samples):
    rms = float(np.sqrt(np.mean(samples * samples)))
    frames = np.lib.stride_tricks.sliding_window_view(samples, 4096)[::2048]
    spectrum = np.mean(abs(np.fft.rfft(frames * np.hanning(4096), axis=1)) ** 2, axis=0)
    frequencies = np.fft.rfftfreq(4096, 1 / RATE)
    edges = np.geomspace(30, RATE / 2, 33)
    bands = np.array([np.sum(spectrum[(frequencies >= a) & (frequencies < b)])
                      for a, b in zip(edges[:-1], edges[1:])])
    energy = float(np.sum(spectrum))
    centroid = float(np.sum(spectrum * frequencies) / max(energy, 1e-30))
    dominant = float(frequencies[1 + np.argmax(spectrum[1:])])
    envelope = np.sqrt(np.mean(samples[:32768].reshape(-1, 1024) ** 2, axis=1))
    times = (np.arange(len(envelope)) + .5) * 1024 / RATE
    selected = (times > .02) & (envelope > max(float(np.max(envelope)) * .01, 1e-12))
    decay = None
    decay_fit = None
    if sum(selected)>=6 and float(np.ptp(times[selected]))>.12:
        y=20*np.log10(envelope[selected]);fit=np.polyfit(times[selected],y,1)
        residual=float(np.sum((y-np.polyval(fit,times[selected]))**2))
        variance=float(np.sum((y-np.mean(y))**2))
        decay_fit=1-residual/max(variance,1e-30)
        if decay_fit>.7 and fit[0]<-.1:
            decay=float(fit[0])
    return dict(rms=rms, peak=float(np.max(abs(samples))), bands=bands,
                centroid_hz=centroid, dominant_hz=dominant, decay_db_second=decay,decay_fit=decay_fit)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', required=True)
    parser.add_argument('--sdk', type=Path, required=True)
    parser.add_argument('--cxx', default='c++')
    parser.add_argument('--out', type=Path, required=True)
    parser.add_argument('--automation',action='store_true')
    args = parser.parse_args()
    out = args.out.resolve()
    if out.exists():
        parser.error('Use a fresh output directory.')
    if REPO == out or REPO in out.parents:
        parser.error('Keep renders outside this checkout.')
    out.mkdir(parents=True)
    baseline = subprocess.check_output(['git','rev-parse',args.baseline+'^{commit}'],cwd=REPO,text=True).strip()
    with tempfile.TemporaryDirectory(prefix='matter-quality-') as temp:
        scratch = Path(temp);before=scratch/'before';before.mkdir()
        paths=subprocess.check_output(['git','ls-tree','-r','--name-only',baseline,'--','machines/matter'],cwd=REPO,text=True).splitlines()
        for path in paths:
            relative=Path(path).relative_to('machines/matter')
            if relative.suffix not in ('.h','.cpp') or relative.parts[0] in ('tests','tools'):
                continue
            target=before/relative;target.parent.mkdir(parents=True,exist_ok=True)
            content=subprocess.check_output(['git','show',baseline+':'+path],cwd=REPO,text=True)
            target.write_text(content.replace('matter_dsp','matter_before_dsp').replace('namespace matter {','namespace matter_before {'))
        cpp=SOURCE.replace('@BEFORE@',json.dumps(str(before/'engine.cpp'))).replace('@AFTER@',json.dumps(str(MACHINE/'engine.cpp')))
        cpp=cpp.replace('@AUTOMATION@','true' if args.automation else 'false')
        (scratch/'quality.cpp').write_text(cpp)
        executable=scratch/'render'
        subprocess.run([args.cxx,'-std=c++11','-O2','-I'+str(args.sdk.resolve()/'sdk/include'),str(scratch/'quality.cpp'),'-o',str(executable)],check=True)
        subprocess.run([str(executable),str(out)],check=True)
    results=[]
    for path in sorted(out.glob('*.pcm')):
        raw=np.fromfile(path,dtype=np.int32).reshape(-1,2).astype(np.float64)/2147483648
        old,new=features(raw[:,0]),features(raw[:,1])
        selected=old['bands']>max(float(np.max(old['bands']))*1e-4,1e-24)
        band_db=10*np.log10(np.maximum(new['bands'][selected],1e-30)/old['bands'][selected])
        a=old['bands']/max(float(sum(old['bands'])),1e-30);b=new['bands']/max(float(sum(new['bands'])),1e-30)
        denominator=max(old['rms']*new['rms'],1e-30)
        row=dict(case=path.stem,rms_db_change=float(20*np.log10(max(new['rms'],1e-15)/max(old['rms'],1e-15))),
                 coarse_spectral_distance=float(sum(abs(a-b))/2),maximum_active_band_db_change=float(max(abs(band_db),default=0)),
                 centroid_hz_before=old['centroid_hz'],centroid_hz_after=new['centroid_hz'],
                 dominant_hz_before=old['dominant_hz'],dominant_hz_after=new['dominant_hz'],
                 decay_db_second_before=old['decay_db_second'],decay_db_second_after=new['decay_db_second'],
                 decay_fit_before=old['decay_fit'],decay_fit_after=new['decay_fit'],
                 pcm_correlation=float(np.mean(raw[:,0]*raw[:,1])/denominator),peak_before=old['peak'],peak_after=new['peak'])
        results.append(row)
    head=subprocess.check_output(['git','rev-parse','HEAD'],cwd=REPO,text=True).strip()
    dirty=bool(subprocess.check_output(['git','status','--porcelain','--untracked-files=all','--','machines/matter'],cwd=REPO,text=True).strip())
    sources={str(path.relative_to(MACHINE)):hashlib.sha256(path.read_bytes()).hexdigest()
             for path in sorted(MACHINE.rglob('*')) if path.suffix in ('.cpp','.h') and path.relative_to(MACHINE).parts[0] not in ('tests','tools')}
    summary=dict(baseline=baseline,candidate_commit=None if dirty else head,candidate_head=head,candidate_dirty=dirty,
                 sources=sources,scenarios=len(results),frames_each=65536,rate=RATE,
                 decay_method='First40dB of amplitude envelope after20ms; at least6windows over120ms; negative fit withR2>0.7. Dominant spectralpeak is not a fundamental-pitch estimate.',
                 maximum_abs_rms_db_change=max(abs(r['rms_db_change']) for r in results),
                 maximum_coarse_spectral_distance=max(r['coarse_spectral_distance'] for r in results),
                 maximum_peak=max(r['peak_after'] for r in results),results=results)
    (out/'quality.json').write_text(json.dumps(summary,indent=2)+'\n')
    print(json.dumps({k:v for k,v in summary.items() if k!='results'},indent=2))


if __name__=='__main__':
    main()
