"""Compare Rebound's exact audio/state/scratch and paired host callback timings."""
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
static int old_memory[rebound_before::buffer_words],new_memory[rebound::buffer_words];
volatile unsigned sink=0;
unsigned next(unsigned& random){return random=random*1664525u+1013904223u;}
template<class State,class Initialize>
void prepare(State& state,int* memory,unsigned mode,Initialize initialize){
    initialize(state);unsigned random=7139;
    for(unsigned i=0;i<rebound::buffer_words;++i)
        memory[i]=mode?int(next(random)%2147483647u)-1073741823:0x60000000;
    if(mode){state.written=rebound::line_frames;state.write=rebound::line_frames-16;}
}
void input(const Scenario& s,unsigned block,int* p,opxy::Transport& transport,int* l,int* r,unsigned& random){
    for(unsigned i=0;i<8;++i)p[i]=s.parameters[i];
    if(s.mode>=2){
        for(unsigned i=0;i<8;++i)p[i]=s.mode==3?int(next(random)):int(next(random)>>1);
        const unsigned tempos[]={0,1200,3000,1,4294967295u};transport.bpm_x10=tempos[(block/8)%5];
    }
    for(unsigned i=0;i<64;++i){
        l[i]=int(next(random));r[i]=int(next(random));
        if(block%3==1){l[i]=i&1?(-2147483647-1):2147483647;r[i]=-2147483647;}
        if(block%3==2)l[i]=r[i]=0;
    }
}
bool equivalent(const Scenario& s){
    rebound_before::State old={};rebound::State now={};
    static_assert(sizeof(old)==sizeof(now),"same state contract");
    static_assert(rebound_before::buffer_words==rebound::buffer_words,"same scratch contract");
    prepare(old,old_memory,s.mode,rebound_before::initialize);prepare(now,new_memory,s.mode,rebound::initialize);
    int p[8],bl[64],br[64],al[64],ar[64];unsigned random=9137;
    opxy::Transport transport={1200,0,true};
    for(unsigned block=0;block<256;++block){
        input(s,block,p,transport,bl,br,random);std::memcpy(al,bl,sizeof(bl));std::memcpy(ar,br,sizeof(br));
        if(block==191){rebound_before::initialize(old);rebound::initialize(now);}
        opxy::Buffer before={old_memory,rebound::buffer_words},after={new_memory,rebound::buffer_words};
        if(block==11){before.words=after.words=nullptr;before.count=after.count=0;}
        if(block==63)before.count=after.count=rebound::buffer_words-1;
        rebound_before::process(old,before,transport,bl,br,p);rebound::process(now,after,transport,al,ar,p);
        if(std::memcmp(bl,al,sizeof(bl))||std::memcmp(br,ar,sizeof(br))||std::memcmp(&old,&now,sizeof(old))||std::memcmp(old_memory,new_memory,sizeof(old_memory))){
            std::fprintf(stderr,"FAIL: %s mode %u block %u audio/state/scratch differs\n",s.label,s.mode,block);return false;
        }
    }
    return true;
}
template<class State,class Initialize,class Process>
double timing(const Scenario& s,Initialize initialize,Process process,int* memory){
    State state={};prepare(state,memory,1,initialize);
    int p[8],l[64],r[64];unsigned random=9137;opxy::Transport transport={1200,0,true};
    opxy::Buffer buffer={memory,rebound::buffer_words};
    std::chrono::steady_clock::time_point begin;
    for(unsigned block=0;block<32+@BLOCKS@;++block){
        if(block==32)begin=std::chrono::steady_clock::now();
        input(s,block,p,transport,l,r,random);process(state,buffer,transport,l,r,p);
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
                new_times[repeat]=timing<rebound::State>(s,rebound::initialize,rebound::process,new_memory);
                old_times[repeat]=timing<rebound_before::State>(s,rebound_before::initialize,rebound_before::process,old_memory);
            }else{
                old_times[repeat]=timing<rebound_before::State>(s,rebound_before::initialize,rebound_before::process,old_memory);
                new_times[repeat]=timing<rebound::State>(s,rebound::initialize,rebound::process,new_memory);
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
    parser.add_argument('--baseline', default='ad0136c')
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
    for index in (80,119,120,121,160,190):
        values=defaults.copy();values[1]=((2*index+1)*2147483648)//(2*191)
        settings.append((f'feedback index {index}',values))
    if args.full_grid:
        settings += [(f'full corner {mask}',[MAX if mask&(1<<slot) else 0 for slot in range(8)]) for mask in range(256)]
    scenarios = [(name,values,mode) for name,values in settings for mode in range(3)]
    scenarios.append(('signed control automation',defaults,3))
    rows_cpp=['{'+json.dumps(name)+',{'+','.join(map(str,values))+'},'+str(mode)+'}' for name,values,mode in scenarios]
    with tempfile.TemporaryDirectory(prefix='rebound-benchmark-') as directory:
        scratch=Path(directory);before=scratch/'before';before.mkdir()
        for name in ('core.h','tables.h','fx.cpp'):
            content=subprocess.check_output(['git','show',baseline+':machines/rebound/'+name],cwd=REPO,text=True)
            (before/name).write_text(content.replace('namespace rebound {','namespace rebound_before {'))
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
