"""Compare rendered Weave sound against a Git revision, without requiring equal PCM."""
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
#include <cstdlib>
#include <cstring>
int main(int argc,char** argv) {
    if(argc!=2)return 1;
    const unsigned increments[]={10713070u,25479940u,101919760u};
    const int maximum=2147483647;
    const unsigned fm_cases=@FM_PATCHES@?192:0;
    const unsigned moving_cases=@AUTOMATION@?30:0;
    for(unsigned test=0;test<180+fm_cases+moving_cases;++test) {
        const bool fm=test>=180 && test<180+fm_cases;
        const bool moving=test>=180+fm_cases;
        const unsigned extra=moving?test-180-fm_cases:fm?test-180:0;
        const unsigned model=moving?extra%15:fm?14:test/12;
        const unsigned variant=(fm||moving)?0:test/3%4,note=(fm||moving)?1:test%3;
        weave_before::Voice before={};weave_before::Shared bs={};
        weave::Voice after={};weave::Shared as={};
        weave_before::initialize(before,bs,1);weave::initialize(after,as,1);
        int parameters[8]={},old_samples[64],new_samples[64];unsigned pitch[64];
        parameters[0]=int((2*model+1)*2147483647LL/30);
        parameters[1]=variant==1?0:variant==2?maximum:maximum/2;
        parameters[2]=variant==1?maximum/4:variant==2?maximum:maximum/2;
        parameters[3]=variant==1?maximum/8:variant==2?maximum:maximum/2;
        parameters[4]=variant==3?maximum:0;
        if(fm){const unsigned raw=(4*(extra/2)+2)/3;parameters[1]=int((2*raw+1)*2147483647LL/256);parameters[6]=(extra&1)?maximum:0;}
        for(auto& p:pitch)p=increments[note];
        char name[4096];
        if(fm)std::snprintf(name,sizeof(name),"%s/fm-patch-%02u-held-%u.pcm",argv[1],extra/2,extra&1);
        else if(moving)std::snprintf(name,sizeof(name),"%s/moving-model-%02u-type-%u.pcm",argv[1],model,extra/15);
        else std::snprintf(name,sizeof(name),"%s/model-%02u-variant-%u-note-%u.pcm",argv[1],model,variant,note);
        std::FILE* file=std::fopen(name,"wb");if(!file)return 2;
        for(unsigned block=0;block<1024;++block) {
            if(moving && !(block%16)) {
                unsigned random=7139+block/16;
                for(unsigned slot=1;slot<5;++slot){random=random*1664525u+1013904223u;parameters[slot]=extra<15?int(random>>1):((block/16+slot)&1)?maximum:0;}
            }
            if(block==512){weave_before::start(before,bs,1);weave::start(after,as,1);}
            weave_before::render(before,bs,1,old_samples,parameters,pitch);
            weave::render(after,as,1,new_samples,parameters,pitch);
            if(@EXACT_STATE@ && (sizeof(before)!=sizeof(after) || std::memcmp(&before,&after,sizeof(after)))) {
                std::fprintf(stderr,"State mismatch case%u block%u\n",test,block);return 5;
            }
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
    parser.add_argument('--all-fm-patches',action='store_true')
    parser.add_argument('--automation',action='store_true')
    parser.add_argument('--exact-state',action='store_true',help='Require identical complete voice state after every block.')
    args = parser.parse_args()
    out = args.out.resolve()
    if out.exists():
        parser.error('Use a fresh output directory.')
    if REPO == out or REPO in out.parents:
        parser.error('Keep renders outside this checkout.')
    out.mkdir(parents=True)
    baseline = subprocess.check_output(['git','rev-parse',args.baseline+'^{commit}'],cwd=REPO,text=True).strip()
    with tempfile.TemporaryDirectory(prefix='weave-quality-') as temp:
        scratch = Path(temp);before=scratch/'before';before.mkdir()
        paths=subprocess.check_output(['git','ls-tree','-r','--name-only',baseline,'--','machines/weave'],cwd=REPO,text=True).splitlines()
        for path in paths:
            relative=Path(path).relative_to('machines/weave')
            if relative.suffix not in ('.h','.cpp') or relative.parts[0] in ('tests','tools'):
                continue
            target=before/relative;target.parent.mkdir(parents=True,exist_ok=True)
            content=subprocess.check_output(['git','show',baseline+':'+path],cwd=REPO,text=True)
            target.write_text(content.replace('weave_dsp','weave_before_dsp').replace('namespace weave {','namespace weave_before {'))
        cpp=SOURCE.replace('@BEFORE@',json.dumps(str(before/'engine.cpp'))).replace('@AFTER@',json.dumps(str(MACHINE/'engine.cpp')))
        cpp=cpp.replace('@FM_PATCHES@','true' if args.all_fm_patches else 'false').replace('@AUTOMATION@','true' if args.automation else 'false').replace('@EXACT_STATE@','true' if args.exact_state else 'false')
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
    dirty=bool(subprocess.check_output(['git','status','--porcelain','--untracked-files=all','--','machines/weave'],cwd=REPO,text=True).strip())
    sources={str(path.relative_to(MACHINE)):hashlib.sha256(path.read_bytes()).hexdigest()
             for path in sorted(MACHINE.rglob('*')) if path.suffix in ('.cpp','.h') and path.relative_to(MACHINE).parts[0] not in ('tests','tools')}
    summary=dict(baseline=baseline,candidate_commit=None if dirty else head,candidate_head=head,candidate_dirty=dirty,
                 sources=sources,require_exact_state=args.exact_state,scenarios=len(results),frames_each=65536,rate=RATE,
                 decay_method='First40dB of amplitude envelope after20ms; at least6windows over120ms; negative fit withR2>0.7. Dominant spectralpeak is not a fundamental-pitch estimate.',
                 maximum_abs_rms_db_change=max(abs(r['rms_db_change']) for r in results),
                 maximum_coarse_spectral_distance=max(r['coarse_spectral_distance'] for r in results),
                 maximum_peak=max(r['peak_after'] for r in results),results=results)
    (out/'quality.json').write_text(json.dumps(summary,indent=2)+'\n')
    print(json.dumps({k:v for k,v in summary.items() if k!='results'},indent=2))


if __name__=='__main__':
    main()
