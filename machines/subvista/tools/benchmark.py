"""Compare exact Subvista audio/state and paired host render timings."""
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
volatile unsigned sink=0;
const int defaults[4]=@DEFAULTS@;
const int presets[][4]=@PRESETS@;
void controls(unsigned setting,unsigned mode,unsigned block,int* parameters,unsigned& random) {
    const int values[]={0,1073741824,2147483647};
    for(unsigned slot=0;slot<8;++slot)parameters[slot]=0;
    for(unsigned slot=0;slot<4;++slot){
        parameters[slot]=setting<81?values[(setting/(slot==0?1:slot==1?3:slot==2?9:27))%3]:setting==81?defaults[slot]:presets[setting-82][slot];
        if(mode==1)parameters[slot]=int((random=random*1664525u+1013904223u)>>1);
    }
    if(mode==1 && block%4==0)parameters[2]=0;
}
void pitches(unsigned increment,unsigned mode,unsigned block,unsigned* pitch) {
    for(unsigned i=0;i<64;++i)pitch[i]=mode==2?increment+((i+block)%64)*17613u:increment;
}
bool equivalent(unsigned setting,unsigned mode,unsigned increment) {
    subvista_before::Voice old={};subvista_before::Shared bs={};
    subvista::Voice now={};subvista::Shared as={};
    subvista_before::initialize(old,bs,1);subvista::initialize(now,as,1);
    int parameters[8],before[64],after[64];unsigned pitch[64],random=7139;
    for(unsigned block=0;block<256;++block) {
        controls(setting,mode,block,parameters,random);pitches(increment,mode,block,pitch);
        if(block%127==0){subvista_before::start(old,bs,1);subvista::start(now,as,1);}
        if(block==191){subvista_before::destroy(old,bs,1);subvista::destroy(now,as,1);subvista_before::initialize(old,bs,1);subvista::initialize(now,as,1);}
        subvista_before::render(old,bs,1,before,parameters,pitch);subvista::render(now,as,1,after,parameters,pitch);
        if(sizeof(old)!=sizeof(now)||std::memcmp(before,after,sizeof(before))||std::memcmp(&old,&now,sizeof(old))) {
            std::fprintf(stderr,"difference at setting %u mode %u pitch %u block %u\n",setting,mode,increment,block);return false;
        }
    }
    return true;
}
template<class Voice,class Shared,class Initialize,class Start,class Render>
double timing(unsigned setting,unsigned mode,unsigned increment,Initialize initialize,Start start,Render render) {
    Voice voice={};Shared shared={};initialize(voice,shared,1);
    int parameters[8],output[64];unsigned pitch[64],random=7139;
    std::chrono::steady_clock::time_point begin;
    for(unsigned block=0;block<32+@BLOCKS@;++block){
        if(block==32)begin=std::chrono::steady_clock::now();
        controls(setting,mode,block,parameters,random);pitches(increment,mode,block,pitch);
        if(block%127==0)start(voice,shared,1);
        render(voice,shared,1,output,parameters,pitch);sink^=unsigned(output[block&63]);
    }
    return std::chrono::duration<double,std::nano>(std::chrono::steady_clock::now()-begin).count()/@BLOCKS@;
}
bool scenario(unsigned setting,unsigned mode,unsigned pitch){
    if(!equivalent(setting,mode,pitch))return false;
    if(@VERIFY_ONLY@) {
        std::printf("{\"setting\":%u,\"mode\":%u,\"pitch\":%u,\"verified\":true}\n",setting,mode,pitch);return true;
    }
    double old_times[5],new_times[5],ratios[5];
    for(unsigned repeat=0;repeat<5;++repeat){
        if(repeat&1){
            new_times[repeat]=timing<subvista::Voice,subvista::Shared>(setting,mode,pitch,subvista::initialize,subvista::start,subvista::render);
            old_times[repeat]=timing<subvista_before::Voice,subvista_before::Shared>(setting,mode,pitch,subvista_before::initialize,subvista_before::start,subvista_before::render);
        }else{
            old_times[repeat]=timing<subvista_before::Voice,subvista_before::Shared>(setting,mode,pitch,subvista_before::initialize,subvista_before::start,subvista_before::render);
            new_times[repeat]=timing<subvista::Voice,subvista::Shared>(setting,mode,pitch,subvista::initialize,subvista::start,subvista::render);
        }
        ratios[repeat]=new_times[repeat]/old_times[repeat];
    }
    std::sort(old_times,old_times+5);std::sort(new_times,new_times+5);std::sort(ratios,ratios+5);
    std::printf("{\"setting\":%u,\"mode\":%u,\"pitch\":%u,\"before_ns\":%.1f,\"after_ns\":%.1f,\"paired_ratio\":%.6f}\n",setting,mode,pitch,old_times[2],new_times[2],ratios[2]);
    return true;
}
int main(){
    const unsigned pitches[]={0,796254,5356535,42852281,171798692,1790000000};
    for(unsigned setting=0;setting<82+sizeof(presets)/sizeof(presets[0]);++setting)for(unsigned pitch:pitches)
        if(!scenario(setting,0,pitch))return 1;
    for(unsigned mode=1;mode<3;++mode)for(unsigned pitch:pitches)if(!scenario(81,mode,pitch))return 1;
    for(unsigned band=0;band<7;++band){
        const unsigned boundary=0x80000000u>>(band+1);
        const unsigned pitch=(static_cast<unsigned long long>(boundary)*256)/257;
        for(int offset=-1;offset<=1;++offset)if(!scenario(81,0,pitch+offset))return 1;
    }
}
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', default='aedce43')
    parser.add_argument('--sdk', type=Path)
    parser.add_argument('--cxx', default='c++')
    parser.add_argument('--blocks', type=int, default=1000)
    parser.add_argument('--verify-only', action='store_true', help='compare all audio and voice-state blocks without timing')
    args = parser.parse_args()
    if args.blocks < 400:
        parser.error('Use at least 400 blocks to reduce timer noise.')
    config_file = REPO/'local/dev.json'
    config = json.loads(config_file.read_text()) if config_file.exists() else {}
    sdk = (args.sdk or Path(config.get('sdk', REPO.parent/'xy-machines-sdk'))).resolve()
    if not (sdk/'sdk/include/opxy/audio.hpp').is_file():
        parser.error('Pass --sdk with an Open-XY SDK checkout.')
    baseline = subprocess.check_output(['git','rev-parse','--verify',args.baseline+'^{commit}'],cwd=REPO,text=True).strip()
    definition = json.loads((MACHINE/'definition.json').read_text())
    slots = [p['id'] for p in definition['parameters']]
    cpp_values = lambda values:'{'+','.join(str(round(value*2147483647)) for value in values)+'}'
    defaults = cpp_values(p['default'] for p in definition['parameters'])
    presets = '{'+','.join(cpp_values(preset[slot] for slot in slots) for preset in definition['presets'].values())+'}'
    with tempfile.TemporaryDirectory(prefix='subvista-benchmark-') as directory:
        scratch = Path(directory); before = scratch/'before'; before.mkdir()
        paths = subprocess.check_output(['git','ls-tree','-r','--name-only',baseline,'--','machines/subvista'],cwd=REPO,text=True).splitlines()
        for path in paths:
            relative=Path(path).relative_to('machines/subvista')
            if relative.as_posix() not in ('core.h','engine.cpp','tables.h','wave_slopes.h','wave_points.h','wave_pairs.h'):
                continue
            content = subprocess.check_output(['git','show',baseline+':'+path],cwd=REPO,text=True)
            (before/relative).write_text(content.replace('namespace subvista {','namespace subvista_before {'))
        source = SOURCE.replace('@BEFORE@',json.dumps(str(before/'engine.cpp'))).replace('@AFTER@',json.dumps(str(MACHINE/'engine.cpp')))
        (scratch/'benchmark.cpp').write_text(source.replace('@BLOCKS@',str(args.blocks)).replace('@DEFAULTS@',defaults).replace('@PRESETS@',presets).replace('@VERIFY_ONLY@','true' if args.verify_only else 'false'))
        executable = scratch/'benchmark'
        subprocess.run([args.cxx,'-std=c++11','-O2','-I'+str(sdk/'sdk/include'),str(scratch/'benchmark.cpp'),'-o',str(executable)],check=True)
        rows = [json.loads(line) for line in subprocess.check_output([str(executable)],text=True).splitlines()]
    if args.verify_only:
        print(json.dumps(dict(baseline=baseline,exact_audio_and_voice_state_each_block=True,
                             scenarios=len(rows),checked_blocks_per_scenario=256,timing_executed=False),indent=2))
        return
    print(json.dumps(dict(baseline=baseline,host_timing_only=True,exact_audio_and_voice_state_each_block=True,
                         scenarios=len(rows),checked_blocks_per_scenario=256,blocks_per_timing=args.blocks,repeats=5,
                         median_reduction_percent=round(100*(1-statistics.median(row['paired_ratio'] for row in rows)),1),results=rows),indent=2))


if __name__ == '__main__':
    main()
