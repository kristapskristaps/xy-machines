"""Check exact X0X audio/state and compare paired host timings across all styles."""
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
void controls(unsigned model,unsigned family,unsigned variant,int* parameters) {
    const unsigned defaults[]={x0x::family_raw(model,family),64,100,64,0,64,64,0};
    for(unsigned slot=0;slot<8;++slot)
        parameters[slot]=static_cast<int>((slot==0 || variant==0?defaults[slot]:variant==1?0u:127u)<<24);
}
void automate(unsigned mode,unsigned block,int* parameters,unsigned& random) {
    if(mode==1)parameters[x0x::tone]=static_cast<int>((block%128)<<24);
    if(mode==2 && block%17==0)for(unsigned slot=1;slot<8;++slot)
        parameters[slot]=static_cast<int>(((random=random*1664525u+1013904223u)>>25)<<24);
}
bool equivalent(unsigned model,unsigned family,unsigned variant,unsigned mode) {
    x0x_before::Voice old;x0x_before::Shared bs;
    x0x::Voice now;x0x::Shared as;
    x0x_before::initialize(old,bs,1);x0x::initialize(now,as,1);
    int parameters[8],before[64],after[64];unsigned pitch[64]={},random=7139;
    controls(model,family,variant,parameters);
    for(unsigned block=0;block<256;++block) {
        automate(mode,block,parameters,random);
        if(block%127==0){
            x0x_before::start(old,bs,1);x0x_before::drum_select(old,bs,1,model);
            x0x::start(now,as,1);x0x::drum_select(now,as,1,model);
        }
        x0x_before::render(old,bs,1,before,parameters,pitch);x0x::render(now,as,1,after,parameters,pitch);
        if(sizeof(old)!=sizeof(now) || std::memcmp(before,after,sizeof(before)) || std::memcmp(&old,&now,sizeof(old)))return false;
    }
    return true;
}
template <class Voice,class Shared,class Initialize,class Start,class Select,class Render>
double timing(Voice& voice,Shared& shared,unsigned model,unsigned family,unsigned variant,unsigned mode,
              Initialize initialize,Start start,Select select,Render render) {
    initialize(voice,shared,1);
    int parameters[8],output[64];unsigned pitch[64]={},random=7139;
    controls(model,family,variant,parameters);
    std::chrono::steady_clock::time_point begin;
    for(unsigned block=0;block<32+@BLOCKS@;++block) {
        if(block==32)begin=std::chrono::steady_clock::now();
        automate(mode,block,parameters,random);
        if(block%127==0 || (!voice.active && !voice.pending)){start(voice,shared,1);select(voice,shared,1,model);}
        render(voice,shared,1,output,parameters,pitch);sink^=static_cast<unsigned>(output[block&63]);
    }
    return std::chrono::duration<double,std::nano>(std::chrono::steady_clock::now()-begin).count()/@BLOCKS@;
}
int main() {
    x0x_before::Voice before;x0x_before::Shared bs;
    x0x::Voice after;x0x::Shared as;
    const char* modes[]={"steady","tone automation","all control automation"};
    for(unsigned model=0;model<x0x::model_count;++model)for(unsigned family=0;family<4;++family){
        if(!(x0x::family_masks[model]&(1u<<family)))continue;
        for(unsigned variant=0;variant<3;++variant)for(unsigned mode=0;mode<3;++mode){
            if(!equivalent(model,family,variant,mode)){
                std::fprintf(stderr,"FAIL: audio/state differs at model %u family %u variant %u mode %u\n",model,family,variant,mode);return 1;
            }
            double old_times[5],new_times[5],ratios[5];
            for(unsigned repeat=0;repeat<5;++repeat){
                if(repeat&1){
                    new_times[repeat]=timing(after,as,model,family,variant,mode,x0x::initialize,x0x::start,x0x::drum_select,x0x::render);
                    old_times[repeat]=timing(before,bs,model,family,variant,mode,x0x_before::initialize,x0x_before::start,x0x_before::drum_select,x0x_before::render);
                }else{
                    old_times[repeat]=timing(before,bs,model,family,variant,mode,x0x_before::initialize,x0x_before::start,x0x_before::drum_select,x0x_before::render);
                    new_times[repeat]=timing(after,as,model,family,variant,mode,x0x::initialize,x0x::start,x0x::drum_select,x0x::render);
                }
                ratios[repeat]=new_times[repeat]/old_times[repeat];
            }
            std::sort(old_times,old_times+5);std::sort(new_times,new_times+5);std::sort(ratios,ratios+5);
            std::printf("{\"model\":%u,\"name\":\"%s\",\"family\":%u,\"variant\":%u,\"mode\":\"%s\",\"before_ns\":%.1f,\"after_ns\":%.1f,\"paired_ratio\":%.6f}\n",
                        model,x0x::short_name(model),family,variant,modes[mode],old_times[2],new_times[2],ratios[2]);
        }
    }
}
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', default='3f8aa0f')
    parser.add_argument('--sdk', type=Path)
    parser.add_argument('--cxx', default='c++')
    parser.add_argument('--blocks', type=int, default=1000)
    args = parser.parse_args()
    if args.blocks < 400:
        parser.error('Use at least 400 blocks to reduce timer noise.')
    config_file = REPO/'local/dev.json'
    config = json.loads(config_file.read_text()) if config_file.exists() else {}
    sdk = (args.sdk or Path(config.get('sdk', REPO.parent/'xy-machines-sdk'))).resolve()
    if not (sdk/'sdk/include/opxy/audio.hpp').is_file():
        parser.error('Pass --sdk with an Open-XY SDK checkout.')
    baseline = subprocess.check_output(['git', 'rev-parse', '--verify', args.baseline+'^{commit}'], cwd=REPO, text=True).strip()
    with tempfile.TemporaryDirectory(prefix='x0x-benchmark-') as directory:
        scratch = Path(directory); before = scratch/'before'; before.mkdir()
        paths = subprocess.check_output(['git', 'ls-tree', '-r', '--name-only', baseline, '--', 'machines/x0x'], cwd=REPO, text=True).splitlines()
        for path in paths:
            relative = Path(path).relative_to('machines/x0x')
            if relative.as_posix() not in ('core.h', 'models.h', 'engine.cpp') and not (relative.parts[0] == 'dsp' and relative.suffix in ('.cpp', '.h', '.hpp')):
                continue
            target = before/relative; target.parent.mkdir(parents=True, exist_ok=True)
            content = subprocess.check_output(['git', 'show', baseline+':'+path], cwd=REPO, text=True)
            target.write_text(content.replace('namespace x0x {', 'namespace x0x_before {').replace('x0x::', 'x0x_before::'))
        source = SOURCE.replace('@BEFORE@', json.dumps(str(before/'engine.cpp'))).replace('@AFTER@', json.dumps(str(MACHINE/'engine.cpp')))
        (scratch/'benchmark.cpp').write_text(source.replace('@BLOCKS@', str(args.blocks)))
        executable = scratch/'benchmark'
        subprocess.run([args.cxx, '-std=c++11', '-O2', '-I'+str(sdk/'sdk/include'), str(scratch/'benchmark.cpp'),
                        str(before/'dsp/tables.cpp'), str(before/'dsp/sample_data.cpp'),
                        str(MACHINE/'dsp/tables.cpp'), str(MACHINE/'dsp/sample_data.cpp'), '-o', str(executable)], check=True)
        rows = [json.loads(line) for line in subprocess.check_output([str(executable)], text=True).splitlines()]
    print(json.dumps(dict(baseline=baseline, host_timing_only=True, exact_audio_and_state_each_block=True,
                         timing_keeps_voices_active=True,
                         scenarios=len(rows), checked_blocks_per_scenario=256, blocks_per_timing=args.blocks, repeats=5,
                         median_reduction_percent=round(100*(1-statistics.median(row['paired_ratio'] for row in rows)), 1),
                         worst_before=max(rows, key=lambda row: row['before_ns']),
                         worst_after=max(rows, key=lambda row: row['after_ns']), results=rows), indent=2))


if __name__ == '__main__':
    main()
