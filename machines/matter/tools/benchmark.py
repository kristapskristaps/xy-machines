"""Compare Matter audio, voice/pool state and paired host render timings."""
import argparse
import json
from pathlib import Path
import statistics
import subprocess
import tempfile

MACHINE = Path(__file__).resolve().parents[1]
REPO = MACHINE.parents[1]
SOURCE = r'''
#include @BEFORE@
#include @AFTER@
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <cstddef>
volatile unsigned sink=0;
template<class Delay> short delay_sample(const Delay& delay,unsigned mode,unsigned frame) {
    // Compare logical samples for both the preceding planar and new interleaved layout.
    if(sizeof(delay.samples)/sizeof(delay.samples[0])==8)return delay.samples[mode][frame];
    return delay.samples[frame][mode];
}
bool shared_equal(const matter_before::Shared& before,const matter::Shared& after) {
    if(sizeof(before)!=sizeof(after) || before.bows.clock!=after.bows.clock ||
       std::memcmp(before.bows.owners,after.bows.owners,sizeof(before.bows.owners)) ||
       std::memcmp(before.bows.stamps,after.bows.stamps,sizeof(before.bows.stamps)))return false;
    for(unsigned resource=0;resource<2;++resource)for(unsigned mode=0;mode<8;++mode)for(unsigned frame=0;frame<1024;++frame)
        if(delay_sample(before.bows.resources[resource],mode,frame)!=delay_sample(after.bows.resources[resource],mode,frame))return false;
    return true;
}
void controls(unsigned model,unsigned variant,unsigned block,int* p,unsigned& random) {
    const unsigned defaults[]={6553,16384,9830,9830,0,16384,19661,32767};
    for(unsigned slot=0;slot<8;++slot) {
        unsigned value=variant==1?0:variant==2?32767:defaults[slot];
        if(variant==3 && slot==3)value=block*257%32768;
        if(variant==4)value=(random=random*1664525u+1013904223u)>>17;
        p[slot]=int(value<<16);
    }
    p[4]=int(((model*32768u+1200u)/6u)<<16);
}
bool equivalent(unsigned model,unsigned variant,unsigned increment) {
    matter_before::Voice old[3]={};matter_before::Shared bs={};
    matter::Voice now[3]={};matter::Shared as={};
    for(unsigned i=0;i<3;++i){matter_before::initialize(old[i],bs,i+1);matter::initialize(now[i],as,i+1);}
    int parameters[8],before[64],after[64];unsigned pitch[64],random=7139;
    for(unsigned block=0;block<256;++block) {
        controls(model,variant,block,parameters,random);
        for(auto& p:pitch)p=variant==4?increment+(block%17)*10371:increment;
        // Include live model edits, retriggers, two bow leases and resource stealing.
        if(block>=192)parameters[4]=int(((((model+block/8)%6)*32768u+1200u)/6u)<<16);
        for(unsigned i=0;i<3;++i){
            if(block%31==0){matter_before::start(old[i],bs,i+1);matter::start(now[i],as,i+1);}
            if(block==127){matter_before::destroy(old[i],bs,i+1);matter::destroy(now[i],as,i+1);}
            if(block==128){matter_before::initialize(old[i],bs,i+1);matter::initialize(now[i],as,i+1);}
            matter_before::render(old[i],bs,i+1,before,parameters,pitch);
            matter::render(now[i],as,i+1,after,parameters,pitch);
            static_assert(sizeof(matter_before::Voice)==sizeof(matter::Voice) ||
                          sizeof(matter_before::Voice)==offsetof(matter::Voice,exciter_timbre),"Compared voice fields stay intact");
            bool voice_difference=false;
            for(unsigned j=0;j<3;++j)voice_difference=voice_difference||std::memcmp(&old[j],&now[j],sizeof(old[j]));
            if(std::memcmp(before,after,sizeof(before))||voice_difference||!shared_equal(bs,as)) {
                std::fprintf(stderr,"difference at model %u variant %u pitch %u block %u voice %u\n",model,variant,increment,block,i);return false;
            }
        }
    }
    return true;
}
template<class Voice,class Shared,class Initialize,class Start,class Render>
double timing(unsigned model,unsigned variant,unsigned increment,Initialize initialize,Start start,Render render) {
    Voice voice={};Shared shared={};initialize(voice,shared,1);
    int parameters[8],output[64];unsigned pitch[64],random=7139;
    for(auto& p:pitch)p=increment;
    std::chrono::steady_clock::time_point begin;
    for(unsigned block=0;block<32+@BLOCKS@;++block) {
        if(block==32)begin=std::chrono::steady_clock::now();
        controls(model,variant,block,parameters,random);
        if(variant==4)for(auto& p:pitch)p=increment+(block%17)*10371;
        if(block%127==0)start(voice,shared,1);
        render(voice,shared,1,output,parameters,pitch);sink^=unsigned(output[block&63]);
    }
    return std::chrono::duration<double,std::nano>(std::chrono::steady_clock::now()-begin).count()/@BLOCKS@;
}
int main(){
    const unsigned pitches[]={2678268,5356535,10713070,42852281,171409123,685636493};
    for(unsigned model=0;model<6;++model)for(unsigned variant=0;variant<5;++variant)for(unsigned pitch:pitches){
        if(!equivalent(model,variant,pitch))return 1;
        if(@VERIFY_ONLY@) {
            std::printf("{\"model\":%u,\"variant\":%u,\"pitch\":%u,\"verified\":true}\n",model,variant,pitch);continue;
        }
        double old_times[5],new_times[5],ratios[5];
        for(unsigned repeat=0;repeat<5;++repeat){
            if(repeat&1){
                new_times[repeat]=timing<matter::Voice,matter::Shared>(model,variant,pitch,matter::initialize,matter::start,matter::render);
                old_times[repeat]=timing<matter_before::Voice,matter_before::Shared>(model,variant,pitch,matter_before::initialize,matter_before::start,matter_before::render);
            }else{
                old_times[repeat]=timing<matter_before::Voice,matter_before::Shared>(model,variant,pitch,matter_before::initialize,matter_before::start,matter_before::render);
                new_times[repeat]=timing<matter::Voice,matter::Shared>(model,variant,pitch,matter::initialize,matter::start,matter::render);
            }
            ratios[repeat]=new_times[repeat]/old_times[repeat];
        }
        std::sort(old_times,old_times+5);std::sort(new_times,new_times+5);std::sort(ratios,ratios+5);
        std::printf("{\"model\":%u,\"variant\":%u,\"pitch\":%u,\"before_ns\":%.1f,\"after_ns\":%.1f,\"paired_ratio\":%.6f}\n",model,variant,pitch,old_times[2],new_times[2],ratios[2]);
    }
}
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', default='81e77d8')
    parser.add_argument('--sdk', type=Path)
    parser.add_argument('--cxx', default='c++')
    parser.add_argument('--blocks', type=int, default=1000)
    parser.add_argument('--verify-only', action='store_true', help='compare audio, voices and all logical delay samples without timing')
    args = parser.parse_args()
    if args.blocks < 400:
        parser.error('Use at least 400 blocks to reduce timer noise.')
    config_file = REPO/'local/dev.json'
    config = json.loads(config_file.read_text()) if config_file.exists() else {}
    sdk = (args.sdk or Path(config.get('sdk', REPO.parent/'xy-machines-sdk'))).resolve()
    if not (sdk/'sdk/include/opxy/audio.hpp').is_file():
        parser.error('Pass --sdk with an Open-XY SDK checkout.')
    baseline = subprocess.check_output(['git','rev-parse','--verify',args.baseline+'^{commit}'],cwd=REPO,text=True).strip()
    with tempfile.TemporaryDirectory(prefix='matter-benchmark-') as directory:
        scratch = Path(directory); before = scratch/'before'; before.mkdir()
        paths = subprocess.check_output(['git','ls-tree','-r','--name-only',baseline,'--','machines/matter'],cwd=REPO,text=True).splitlines()
        for path in paths:
            relative = Path(path).relative_to('machines/matter')
            if relative.as_posix() not in ('core.h','engine.cpp','vendor/numeric.h','vendor/resources.h'):
                continue
            target = before/relative; target.parent.mkdir(parents=True,exist_ok=True)
            content = subprocess.check_output(['git','show',baseline+':'+path],cwd=REPO,text=True)
            target.write_text(content.replace('matter_dsp','matter_before_dsp').replace('namespace matter {','namespace matter_before {'))
        source = SOURCE.replace('@BEFORE@',json.dumps(str(before/'engine.cpp'))).replace('@AFTER@',json.dumps(str(MACHINE/'engine.cpp')))
        (scratch/'benchmark.cpp').write_text(source.replace('@BLOCKS@',str(args.blocks)).replace('@VERIFY_ONLY@','true' if args.verify_only else 'false'))
        executable = scratch/'benchmark'
        subprocess.run([args.cxx,'-std=c++11','-O2','-I'+str(sdk/'sdk/include'),str(scratch/'benchmark.cpp'),'-o',str(executable)],check=True)
        rows = [json.loads(line) for line in subprocess.check_output([str(executable)],text=True).splitlines()]
    if args.verify_only:
        print(json.dumps(dict(baseline=baseline,exact_audio_original_voice_and_shared_state_each_block=True,
                             scenarios=len(rows),voices_per_equivalence_scenario=3,checked_blocks_per_scenario=256,
                             timing_executed=False),indent=2))
        return
    print(json.dumps(dict(baseline=baseline,host_timing_only=True,exact_audio_original_voice_and_shared_state_each_block=True,
                         scenarios=len(rows),voices_per_equivalence_scenario=3,checked_blocks_per_scenario=256,
                         blocks_per_timing=args.blocks,repeats=5,
                         median_reduction_percent=round(100*(1-statistics.median(row['paired_ratio'] for row in rows)),1),results=rows),indent=2))


if __name__ == '__main__':
    main()
