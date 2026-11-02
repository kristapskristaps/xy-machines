"""Compare HK Mbira's audio/state and paired host timings against a Git revision."""
import argparse
import json
from pathlib import Path
import statistics
import subprocess
import tempfile

MACHINE = Path(__file__).resolve().parents[1]
REPO = MACHINE.parents[2]
SOURCE = r'''
#include @BEFORE@
#include @AFTER@
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <cstddef>
struct Scenario { const char* label; int parameters[8]; unsigned pitch,mode; };
const Scenario scenarios[] = @SCENARIOS@;
volatile unsigned sink=0;
void controls(const Scenario& s,unsigned block,int* p,unsigned* pitch,unsigned& random){
    for(unsigned i=0;i<8;++i)p[i]=s.parameters[i];
    if(s.mode==1)for(unsigned i=0;i<8;++i)p[i]=int((random=random*1664525u+1013904223u)>>1);
    if(s.mode==3)for(unsigned i=0;i<8;++i)p[i]=int(random=random*1664525u+1013904223u);
    if(s.mode==4){p[2]=int((random=random*1664525u+1013904223u)>>1);p[3]=int((random=random*1664525u+1013904223u)>>1);}
    if(s.mode==5)p[1]=int((random=random*1664525u+1013904223u)>>1);
    if(s.mode==6)p[0]=int((random=random*1664525u+1013904223u)>>1);
    for(unsigned i=0;i<64;++i)pitch[i]=s.mode==3?(random=random*1664525u+1013904223u):s.mode==2?s.pitch+((i+block)%64)*17613u:s.pitch;
}
bool equivalent(const Scenario& s){
    hkmbira_before::Voice old={};hkmbira_before::Shared bs={};
    hkmbira::Voice now={};hkmbira::Shared as={};
    hkmbira_before::initialize(old,bs,1);hkmbira::initialize(now,as,1);
    int p[8],before[64],after[64];unsigned pitch[64],random=7139;
    for(unsigned block=0;block<256;++block){
        controls(s,block,p,pitch,random);
        if(block%127==0){hkmbira_before::start(old,bs,1);hkmbira::start(now,as,1);}
        if(block==191){hkmbira_before::destroy(old,bs,1);hkmbira::destroy(now,as,1);hkmbira_before::initialize(old,bs,1);hkmbira::initialize(now,as,1);}
        hkmbira_before::render(old,bs,1,before,p,pitch);hkmbira::render(now,as,1,after,p,pitch);
        if(sizeof(old)!=sizeof(now) || std::memcmp(before,after,sizeof(before)) || std::memcmp(&old,&now,sizeof(old))){
            std::fprintf(stderr,"difference at %s pitch %u mode %u block %u\n",s.label,s.pitch,s.mode,block);return false;
        }
    }
    return true;
}
template<class Voice,class Shared,class Initialize,class Start,class Render>
double timing(const Scenario& s,Initialize initialize,Start start,Render render){
    Voice voice={};Shared shared={};initialize(voice,shared,1);
    int p[8],out[64];unsigned pitch[64],random=7139;
    std::chrono::steady_clock::time_point begin;
    for(unsigned block=0;block<32+@BLOCKS@;++block){
        if(block==32)begin=std::chrono::steady_clock::now();
        controls(s,block,p,pitch,random);
        if(block%127==0)start(voice,shared,1);
        render(voice,shared,1,out,p,pitch);sink^=unsigned(out[block&63]);
    }
    return std::chrono::duration<double,std::nano>(std::chrono::steady_clock::now()-begin).count()/@BLOCKS@;
}
int main(){
    for(const auto& s:scenarios){
        if(!equivalent(s))return 1;
        double old_times[5]={},new_times[5]={},ratios[5]={};
        if(!@VERIFY_ONLY@)for(unsigned repeat=0;repeat<5;++repeat){
            if(repeat&1){
                new_times[repeat]=timing<hkmbira::Voice,hkmbira::Shared>(s,hkmbira::initialize,hkmbira::start,hkmbira::render);
                old_times[repeat]=timing<hkmbira_before::Voice,hkmbira_before::Shared>(s,hkmbira_before::initialize,hkmbira_before::start,hkmbira_before::render);
            }else{
                old_times[repeat]=timing<hkmbira_before::Voice,hkmbira_before::Shared>(s,hkmbira_before::initialize,hkmbira_before::start,hkmbira_before::render);
                new_times[repeat]=timing<hkmbira::Voice,hkmbira::Shared>(s,hkmbira::initialize,hkmbira::start,hkmbira::render);
            }
            ratios[repeat]=new_times[repeat]/old_times[repeat];
        }
        std::sort(old_times,old_times+5);std::sort(new_times,new_times+5);std::sort(ratios,ratios+5);
        std::printf("{\"label\":\"%s\",\"pitch\":%u,\"mode\":%u,\"before_ns\":%.1f,\"after_ns\":%.1f,\"paired_ratio\":%.6f}\n",s.label,s.pitch,s.mode,old_times[2],new_times[2],ratios[2]);
    }
}
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', default='64b4846')
    parser.add_argument('--sdk', type=Path)
    parser.add_argument('--cxx', default='c++')
    parser.add_argument('--blocks', type=int, default=1000)
    parser.add_argument('--verify-only', action='store_true')
    parser.add_argument('--sanitize', action='store_true', help='Check memory safety with AddressSanitizer.')
    args = parser.parse_args()
    if args.sanitize and not args.verify_only:
        parser.error('Sanitizer runs require --verify-only; they are not host timings.')
    if args.blocks < 400:
        parser.error('Use at least 400 blocks to reduce timer noise.')
    config_file = REPO/'local/dev.json'
    config = json.loads(config_file.read_text()) if config_file.exists() else {}
    sdk = (args.sdk or Path(config.get('sdk', REPO.parent/'xy-machines-sdk'))).resolve()
    if not (sdk/'sdk/include/opxy/audio.hpp').is_file():
        parser.error('Pass --sdk with an Open-XY SDK checkout.')
    baseline = subprocess.check_output(['git','rev-parse','--verify',args.baseline+'^{commit}'],cwd=REPO,text=True).strip()
    definition = json.loads((MACHINE/'definition.json').read_text())
    defaults = [round(p['default']*2147483647) for p in definition['parameters']]
    slots = [p['id'] for p in definition['parameters']]
    defaults += [0]*(8-len(defaults))
    settings = [('defaults',defaults)]
    for slot in range(7):
        for step in range(9):
            values=defaults.copy();values[slot]=round(step*2147483647/8)
            settings.append((f'control {slot} step {step}',values))
    import itertools
    settings += [('corner '+''.join(map(str,corner)),[value*2147483647 for value in corner]+[0])
                 for corner in itertools.product(range(2),repeat=7)]
    pitches=[0,5356535,10713070,42852281,171409123,685636493]
    scenarios=[(name,values,pitch,0) for name,values in settings for pitch in pitches]
    scenarios += [('automation',defaults,pitch,mode) for mode in range(1,7) for pitch in pitches]
    rows_cpp = ['{'+json.dumps(name)+',{'+','.join(map(str,values))+'},'+str(pitch)+'u,'+str(mode)+'}' for name,values,pitch,mode in scenarios]
    with tempfile.TemporaryDirectory(prefix='hkmbira-benchmark-') as directory:
        scratch = Path(directory); before = scratch/'before'; before.mkdir()
        for path in ['core.h','engine.cpp','dsp.hpp','wide.hpp','sine.hpp']:
            content = subprocess.check_output(['git','show',baseline+':machines/hundekraft/hkmbira/'+path],cwd=REPO,text=True)
            (before/path).write_text(content.replace('namespace hkmbira {','namespace hkmbira_before {'))
        source = SOURCE.replace('@BEFORE@',json.dumps(str(before/'engine.cpp'))).replace('@AFTER@',json.dumps(str(MACHINE/'engine.cpp')))
        (scratch/'benchmark.cpp').write_text(source.replace('@BLOCKS@',str(args.blocks)).replace('@VERIFY_ONLY@','true' if args.verify_only else 'false').replace('@SCENARIOS@','{'+',\n'.join(rows_cpp)+'}'))
        executable = scratch/'benchmark'
        flags = ['-fsanitize=address',
                 '-fno-omit-frame-pointer', '-g'] if args.sanitize else []
        subprocess.run([args.cxx,'-std=c++11','-O2',*flags,'-I'+str(sdk/'sdk/include'),str(scratch/'benchmark.cpp'),'-o',str(executable)],check=True)
        rows = [json.loads(line) for line in subprocess.check_output([str(executable)],text=True).splitlines()]
    print(json.dumps(dict(baseline=baseline,host_timing_only=True,exact_audio_and_complete_voice_state_each_block=True,
                         verify_only=args.verify_only,sanitized=args.sanitize,
                         sanitizer='address' if args.sanitize else None,
                         scenarios=len(rows),checked_blocks_per_scenario=256,
                         blocks_per_timing=0 if args.verify_only else args.blocks,repeats=0 if args.verify_only else 5,
                         median_reduction_percent=None if args.verify_only else round(100*(1-statistics.median(row['paired_ratio'] for row in rows)),1),results=rows),indent=2))


if __name__ == '__main__':
    main()
