"""Compare Halo's exact output, scratch and state with paired host render timings."""
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
const int settings[][8]={@SETTINGS@};
const char* labels[]={@LABELS@};
static halo_before::State before;
static halo::State after;
static int old_memory[halo_before::buffer_words],new_memory[halo::buffer_words];
volatile unsigned sink=0;
bool equivalent(const int* initial,unsigned mode) {
    halo_before::initialize(before);halo::initialize(after);
    for(unsigned i=0;i<halo::buffer_words;++i)old_memory[i]=new_memory[i]=0x3f3f3f3f;
    int parameters[8],old_left[64],old_right[64],new_left[64],new_right[64];
    unsigned phase=0,seed=7139;
    for(unsigned j=0;j<8;++j)parameters[j]=initial[j];
    opxy::Buffer old_buffer={old_memory,halo::buffer_words},new_buffer={new_memory,halo::buffer_words};
    opxy::Transport transport={1200,0,true};
    for(unsigned block=0;block<4096;++block) {
        if(block==2048){halo_before::initialize(before);halo::initialize(after);}
        if(mode==2) {
            if(block%8==0)for(unsigned j=0;j<8;++j)parameters[j]=initial[j];
            parameters[block%8]=static_cast<int>((seed=seed*1664525u+1013904223u)>>1);
            transport.bpm_x10=block%3==0?0:block%3==1?400:3000;
        }
        for(unsigned i=0;i<64;++i) {
            phase+=23192823u;
            old_left[i]=mode==1 && block>=256?0:static_cast<int>(phase)/4;
            old_right[i]=-old_left[i]/2;
            if(block%113==0 && i==0)old_right[i]=536870912;
            new_left[i]=old_left[i];new_right[i]=old_right[i];
        }
        halo_before::process(before,old_buffer,transport,old_left,old_right,parameters);
        halo::process(after,new_buffer,transport,new_left,new_right,parameters);
        if(std::memcmp(old_left,new_left,sizeof(old_left)) || std::memcmp(old_right,new_right,sizeof(old_right)) ||
           std::memcmp(&before,&after,sizeof(before)) || std::memcmp(old_memory,new_memory,sizeof(old_memory))) {
            std::fprintf(stderr,"FAIL: complete audio/state/scratch differs at block %u\n",block);return false;
        }
    }
    return true;
}
template <class State,class Initialize,class Process>
double timing(State& state,int* memory,const int* initial,unsigned mode,
              Initialize initialize,Process process,unsigned& hash) {
    initialize(state);std::memset(memory,0,halo::buffer_words*sizeof(int));
    int parameters[8],left[64],right[64];unsigned phase=0,seed=7139;
    for(unsigned j=0;j<8;++j)parameters[j]=initial[j];
    opxy::Buffer buffer={memory,halo::buffer_words};opxy::Transport transport={1200,0,true};
    hash=2166136261u;
    std::chrono::steady_clock::time_point begin;
    for(unsigned block=0;block<2304+@BLOCKS@;++block) {
        if(block==2304)begin=std::chrono::steady_clock::now();
        if(mode==2 && block>=2304) {
            if(block%8==0)for(unsigned j=0;j<8;++j)parameters[j]=initial[j];
            parameters[block%8]=static_cast<int>((seed=seed*1664525u+1013904223u)>>1);
            transport.bpm_x10=block%3==0?0:block%3==1?400:3000;
        }
        for(unsigned i=0;i<64;++i) {
            phase+=23192823u;
            left[i]=mode==1 && block>=2304?0:static_cast<int>(phase)/4;
            right[i]=-left[i]/2;
            if(block%113==0 && i==0)right[i]=536870912;
        }
        process(state,buffer,transport,left,right,parameters);
        hash=(hash^static_cast<unsigned>(left[block&63]))*16777619u;
        hash=(hash^static_cast<unsigned>(right[block&63]))*16777619u;
    }
    sink^=hash;
    return std::chrono::duration<double,std::nano>(std::chrono::steady_clock::now()-begin).count()/@BLOCKS@;
}
int main() {
    const char* modes[]={"steady stereo","tail","control and tempo automation"};
    static_assert(sizeof(before)==sizeof(after),"same Halo state contract");
    static_assert(halo_before::buffer_words==halo::buffer_words,"same scratch contract");
    for(unsigned setting=0;setting<sizeof(settings)/sizeof(settings[0]);++setting)for(unsigned mode=0;mode<3;++mode) {
        if(@VERIFY_ONLY@) {
            if(!equivalent(settings[setting],mode))return 1;
            std::printf("{\"setting\":\"%s\",\"mode\":\"%s\",\"verified\":true}\n",labels[setting],modes[mode]);continue;
        }
        double old_times[5],new_times[5],ratios[5];
        for(unsigned repeat=0;repeat<5;++repeat) {
            unsigned old_hash,new_hash;
            if(repeat&1) {
                new_times[repeat]=timing(after,new_memory,settings[setting],mode,halo::initialize,halo::process,new_hash);
                old_times[repeat]=timing(before,old_memory,settings[setting],mode,halo_before::initialize,halo_before::process,old_hash);
            } else {
                old_times[repeat]=timing(before,old_memory,settings[setting],mode,halo_before::initialize,halo_before::process,old_hash);
                new_times[repeat]=timing(after,new_memory,settings[setting],mode,halo::initialize,halo::process,new_hash);
            }
            if(old_hash!=new_hash || std::memcmp(&before,&after,sizeof(before)) || std::memcmp(old_memory,new_memory,sizeof(old_memory))) {
                std::fprintf(stderr,"FAIL: audio/state/scratch differs at %s / %s\n",labels[setting],modes[mode]);return 1;
            }
            ratios[repeat]=new_times[repeat]/old_times[repeat];
        }
        std::sort(old_times,old_times+5);std::sort(new_times,new_times+5);std::sort(ratios,ratios+5);
        std::printf("{\"setting\":\"%s\",\"mode\":\"%s\",\"before_ns\":%.1f,\"after_ns\":%.1f,\"paired_ratio\":%.6f}\n",
                    labels[setting],modes[mode],old_times[2],new_times[2],ratios[2]);
    }
}
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', required=True)
    parser.add_argument('--sdk', type=Path)
    parser.add_argument('--cxx', default='c++')
    parser.add_argument('--blocks', type=int, default=2000)
    parser.add_argument('--verify-only', action='store_true', help='compare all audio samples, state and scratch after every block')
    args = parser.parse_args()
    if args.blocks < 400:
        parser.error('Use at least 400 blocks to reduce timer noise.')
    config_file = REPO / 'local/dev.json'
    config = json.loads(config_file.read_text()) if config_file.exists() else {}
    sdk = (args.sdk or Path(config.get('sdk', REPO.parent / 'xy-machines-sdk'))).resolve()
    if not (sdk / 'sdk/include/opxy/effect.hpp').is_file():
        parser.error('Pass --sdk with an Open-XY SDK checkout.')
    baseline = subprocess.check_output(['git', 'rev-parse', '--verify', args.baseline+'^{commit}'], cwd=REPO, text=True).strip()
    baseline_core = subprocess.check_output(['git','show',baseline+':machines/halo/core.h'],cwd=REPO,text=True)
    if ('struct Cic' in baseline_core) != ('struct Cic' in (MACHINE/'core.h').read_text()):
        parser.error('Different Halo topology: use tests/space.cpp for plate/FDN comparisons, not exact-state timing.')
    definition = json.loads((MACHINE / 'definition.json').read_text())
    defaults = [round(p['default']*MAX) for p in definition['parameters']]
    settings = [defaults]; labels = ['default']
    for name, preset in definition['presets'].items():
        settings.append([round(preset[p['id']]*MAX) for p in definition['parameters']]); labels.append(name)
    for index, parameter in enumerate(definition['parameters'][:7]):
        for value in (0, MAX):
            row = defaults.copy(); row[index] = value
            settings.append(row); labels.append(parameter['id']+':'+str(value))
    settings += [[0]*8, [MAX]*8]; labels += ['all zero', 'all maximum']
    with tempfile.TemporaryDirectory(prefix='halo-benchmark-') as directory:
        scratch = Path(directory); before = scratch/'before'; before.mkdir()
        names = subprocess.check_output(['git','ls-tree','--name-only',baseline+':machines/halo'],cwd=REPO,text=True).splitlines()
        for name in (n for n in names if n.endswith('.h') or n == 'fx.cpp'):
            content = subprocess.check_output(['git', 'show', baseline+':machines/halo/'+name], cwd=REPO, text=True)
            (before/name).write_text(content.replace('namespace halo {', 'namespace halo_before {'))
        source = SOURCE.replace('@BEFORE@', json.dumps(str(before/'fx.cpp'))).replace('@AFTER@', json.dumps(str(MACHINE/'fx.cpp')))
        source = source.replace('@SETTINGS@', ','.join('{'+','.join(map(str, row))+'}' for row in settings))
        source = source.replace('@LABELS@', ','.join(map(json.dumps, labels))).replace('@BLOCKS@', str(args.blocks))
        source = source.replace('@VERIFY_ONLY@', 'true' if args.verify_only else 'false')
        (scratch/'benchmark.cpp').write_text(source)
        executable = scratch/'benchmark'
        subprocess.run([args.cxx, '-std=c++11', '-O2', '-I'+str(sdk/'sdk/include'), str(scratch/'benchmark.cpp'), '-o', str(executable)], check=True)
        rows = [json.loads(line) for line in subprocess.check_output([str(executable)], text=True).splitlines()]
    if args.verify_only:
        print(json.dumps(dict(baseline=baseline, exact_audio_state_and_scratch_each_block=True,
                             scenarios=len(rows), checked_blocks_per_scenario=4096, timing_executed=False), indent=2))
        return
    print(json.dumps(dict(baseline=baseline, host_timing_only=True, matching_audio_hash_state_and_scratch=True,
                         scenarios=len(rows), blocks_per_timing=args.blocks, repeats=5,
                         median_reduction_percent=round(100*(1-statistics.median(row['paired_ratio'] for row in rows)), 1),
                         worst_before=max(rows, key=lambda row: row['before_ns']),
                         worst_after=max(rows, key=lambda row: row['after_ns']), results=rows), indent=2))


if __name__ == '__main__':
    main()
