"""Compare Vapour's exact audio/state/scratch and paired host callback timings."""
import argparse
import json
from pathlib import Path
import statistics
import subprocess
import tempfile

MACHINE = Path(__file__).resolve().parents[1]
REPO = MACHINE.parents[1]
MAX = 2147483647
SOURCE = r'''
#include @BEFORE@
#include @AFTER@
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
struct Scenario { const char* label; int parameters[8]; unsigned mode; };
const Scenario scenarios[] = @SCENARIOS@;
const unsigned words=282624/4;
static int old_memory[words],new_memory[words];
volatile unsigned sink=0;
unsigned next(unsigned& random){return random=random*1664525u+1013904223u;}
template<class State,class Workspace,class Initialize,class Schedule>
void prepare(State& state,int* memory,const Scenario& s,Initialize initialize,Schedule schedule){
    initialize(state);unsigned random=7139;
    for(unsigned i=0;i<words;++i)memory[i]=int(next(random));
    if(!s.mode)return;
    state.written=vapour::ring_frames;state.write=vapour::ring_frames-16;state.initialized=1;
    state.diffusion_written=612;
    Workspace& workspace=*reinterpret_cast<Workspace*>(memory+vapour::ring_frames);workspace=Workspace();
    for(auto& sample:workspace.diffusion)sample=int(next(random)%262143u)-131071;
    for(auto& sample:workspace.feedback)sample=int(next(random)%65536u)-32768;
    for(auto& grain:workspace.grains){
        schedule(state,grain,vapour::granular::control(s.parameters[1]),
                 vapour::granular::lookup(vapour::tables::pitch_ratio,vapour::granular::control(s.parameters[2])),
                 vapour::granular::control(s.parameters[0]),vapour::granular::control(s.parameters[4]));
        grain.pre_delay=next(random)&63u;
    }
}
void input(const Scenario& s,unsigned block,int* p,int* l,int* r,unsigned& random){
    for(unsigned i=0;i<8;++i)p[i]=s.parameters[i];
    if(s.mode>=2){
        unsigned motion=7139+block/16;
        for(unsigned i=0;i<8;++i)p[i]=s.mode==3?int(next(motion)):int(next(motion)>>1);
    }
    for(unsigned i=0;i<64;++i){
        l[i]=int(next(random));r[i]=int(next(random));
        if(block%3==1){l[i]=i&1?(-2147483647-1):2147483647;r[i]=-2147483647;}
        if(block%3==2)l[i]=r[i]=0;
    }
}
bool equivalent(const Scenario& s){
    vapour_before::State old={};vapour::State now={};
    static_assert(sizeof(old)==sizeof(now),"same state contract");
    static_assert(sizeof(vapour_before::Workspace)==sizeof(vapour::Workspace),"same scratch contract");
    prepare<vapour_before::State,vapour_before::Workspace>(old,old_memory,s,vapour_before::initialize,vapour_before::granular::schedule);
    prepare<vapour::State,vapour::Workspace>(now,new_memory,s,vapour::initialize,vapour::granular::schedule);
    int p[8],bl[64],br[64],al[64],ar[64];unsigned random=9137;
    for(unsigned block=0;block<256;++block){
        input(s,block,p,bl,br,random);std::memcpy(al,bl,sizeof(bl));std::memcpy(ar,br,sizeof(br));
        if(block==191){vapour_before::initialize(old);vapour::initialize(now);}
        opxy::Buffer before={old_memory,words},after={new_memory,words};
        if(block==11){before.words=after.words=nullptr;before.count=after.count=0;}
        if(block==63)before.count=after.count=vapour::ring_frames+sizeof(vapour::Workspace)/4-1;
        vapour_before::process(old,before,bl,br,p);vapour::process(now,after,al,ar,p);
        if(std::memcmp(bl,al,sizeof(bl))||std::memcmp(br,ar,sizeof(br))||std::memcmp(&old,&now,sizeof(old))||std::memcmp(old_memory,new_memory,sizeof(old_memory))){
            std::fprintf(stderr,"FAIL: %s mode %u block %u audio/state/scratch differs\n",s.label,s.mode,block);return false;
        }
    }
    return true;
}
template<class State,class Workspace,class Initialize,class Schedule,class Process>
double timing(const Scenario& s,Initialize initialize,Schedule schedule,Process process,int* memory){
    State state={};Scenario primed=s;primed.mode=1;prepare<State,Workspace>(state,memory,primed,initialize,schedule);
    int p[8],l[64],r[64];unsigned random=9137;
    opxy::Buffer buffer={memory,words};
    std::chrono::steady_clock::time_point begin;
    for(unsigned block=0;block<512+@BLOCKS@;++block){
        if(block==512)begin=std::chrono::steady_clock::now();
        input(s,block,p,l,r,random);process(state,buffer,l,r,p);
        sink^=unsigned(l[block&63])^unsigned(r[block&63]);
    }
    return std::chrono::duration<double,std::nano>(std::chrono::steady_clock::now()-begin).count()/@BLOCKS@;
}
int main(){
    for(const auto& s:scenarios){
        if(!equivalent(s))return 1;
        double old_times[5]={},new_times[5]={},ratios[5]={};
        if(!@VERIFY_ONLY@)for(unsigned repeat=0;repeat<5;++repeat){
            if(repeat&1){
                new_times[repeat]=timing<vapour::State,vapour::Workspace>(s,vapour::initialize,vapour::granular::schedule,vapour::process,new_memory);
                old_times[repeat]=timing<vapour_before::State,vapour_before::Workspace>(s,vapour_before::initialize,vapour_before::granular::schedule,vapour_before::process,old_memory);
            }else{
                old_times[repeat]=timing<vapour_before::State,vapour_before::Workspace>(s,vapour_before::initialize,vapour_before::granular::schedule,vapour_before::process,old_memory);
                new_times[repeat]=timing<vapour::State,vapour::Workspace>(s,vapour::initialize,vapour::granular::schedule,vapour::process,new_memory);
            }
            ratios[repeat]=new_times[repeat]/old_times[repeat];
        }
        std::sort(old_times,old_times+5);std::sort(new_times,new_times+5);std::sort(ratios,ratios+5);
        std::printf("{\"label\":\"%s\",\"mode\":%u,\"before_ns\":%.1f,\"after_ns\":%.1f,\"paired_ratio\":%.6f}\n",s.label,s.mode,old_times[2],new_times[2],ratios[2]);
    }
}
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', default='75fcda3')
    parser.add_argument('--sdk', type=Path)
    parser.add_argument('--cxx', default='c++')
    parser.add_argument('--blocks', type=int, default=1000)
    parser.add_argument('--full-grid', action='store_true')
    parser.add_argument('--verify-only', action='store_true')
    args = parser.parse_args()
    if args.blocks < 400:
        parser.error('Use at least 400 blocks to reduce timer noise.')
    config_file = REPO/'local/dev.json'
    config = json.loads(config_file.read_text()) if config_file.exists() else {}
    sdk = (args.sdk or Path(config.get('sdk', REPO.parent/'xy-machines-sdk'))).resolve()
    if not (sdk/'sdk/include/opxy/effect.hpp').is_file():
        parser.error('Pass --sdk with an Open-XY SDK checkout.')
    baseline = subprocess.check_output(['git','rev-parse','--verify',args.baseline+'^{commit}'],cwd=REPO,text=True).strip()
    definition = json.loads((MACHINE/'definition.json').read_text())
    defaults = [round(p['default']*MAX) for p in definition['parameters']]
    settings = [('defaults',defaults)]
    for slot in range(8):
        for step in range(5):
            values = defaults.copy(); values[slot] = round(step*MAX/4)
            settings.append((f'control {slot} step {step}',values))
    for page in range(2):
        for mask in range(16):
            values = defaults.copy()
            for slot in range(4):values[page*4+slot]=MAX if mask&(1<<slot) else 0
            settings.append((f'page {page} corner {mask}',values))
    for slot, positions in ((3,(15400,15401,15402,16383,16384,17366,17367,17368)),
                            (4,(12287,12288,24575,24576,24577))):
        for value in positions:
            values=defaults.copy();values[slot]=value<<16
            settings.append((f'control {slot} boundary {value}',values))
    if args.full_grid:
        settings += [(f'full corner {mask}',[MAX if mask&(1<<slot) else 0 for slot in range(8)]) for mask in range(256)]
    scenarios = [(name,values,mode) for name,values in settings for mode in range(3)]
    scenarios.append(('signed control automation',defaults,3))
    rows_cpp=['{'+json.dumps(name)+',{'+','.join(map(str,values))+'},'+str(mode)+'}' for name,values,mode in scenarios]
    with tempfile.TemporaryDirectory(prefix='vapour-benchmark-') as directory:
        scratch=Path(directory);before=scratch/'before';before.mkdir()
        for name in ('core.h','vendor/tables.h','vendor/granular.h','fx.cpp'):
            content=subprocess.check_output(['git','show',baseline+':machines/vapour/'+name],cwd=REPO,text=True)
            (before/name).parent.mkdir(parents=True,exist_ok=True)
            (before/name).write_text(content.replace('namespace vapour {','namespace vapour_before {'))
        source=SOURCE.replace('@BEFORE@',json.dumps(str(before/'fx.cpp'))).replace('@AFTER@',json.dumps(str(MACHINE/'fx.cpp')))
        source=source.replace('@BLOCKS@',str(args.blocks)).replace('@VERIFY_ONLY@','true' if args.verify_only else 'false').replace('@SCENARIOS@','{'+',\n'.join(rows_cpp)+'}')
        (scratch/'benchmark.cpp').write_text(source);executable=scratch/'benchmark'
        subprocess.run([args.cxx,'-std=c++11','-O2','-I'+str(sdk/'sdk/include'),str(scratch/'benchmark.cpp'),'-o',str(executable)],check=True)
        rows=[json.loads(line) for line in subprocess.check_output([str(executable)],text=True).splitlines()]
    print(json.dumps(dict(baseline=baseline,host_timing_only=True,exact_audio_state_and_entire_scratch_each_block=True,
                         full_grid=args.full_grid,verify_only=args.verify_only,scenarios=len(rows),checked_blocks_per_scenario=256,
                         blocks_per_timing=0 if args.verify_only else args.blocks,repeats=0 if args.verify_only else 5,
                         median_reduction_percent=None if args.verify_only else round(100*(1-statistics.median(row['paired_ratio'] for row in rows)),1),results=rows),indent=2))


if __name__ == '__main__':
    main()
