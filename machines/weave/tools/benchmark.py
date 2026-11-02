"""Compare all Weave models against a Git revision using paired host timings."""
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
volatile unsigned sink=0;
void inputs(const int* initial,unsigned mode,unsigned block,int* parameters,unsigned* pitch,unsigned& random) {
    if(mode==2) {
        if(block%8==0)for(unsigned j=0;j<8;++j)parameters[j]=initial[j];
        const unsigned slot=1+block%3;
        parameters[slot]=static_cast<int>((random=random*1664525u+1013904223u)>>1);
        parameters[5]=static_cast<int>((random=random*1664525u+1013904223u)>>1);
        parameters[6]=static_cast<int>((random=random*1664525u+1013904223u)>>1);
        parameters[7]=static_cast<int>((random=random*1664525u+1013904223u)>>1);
    }
    for(unsigned i=0;i<64;++i)
        pitch[i]=mode==0?5356535u:mode==1?342818246u:3000001u+(block%128)*1720031u+i*12345u;
}
bool equivalent(const int* initial,unsigned mode) {
    weave_before::Voice old={};weave_before::Shared bs;
    weave::Voice now={};weave::Shared as;
    weave_before::initialize(old,bs,1);weave::initialize(now,as,1);
    int parameters[8],before[64],after[64];unsigned pitch[64],random=7139;
    for(unsigned j=0;j<8;++j)parameters[j]=initial[j];
    for(unsigned block=0;block<256;++block) {
        inputs(initial,mode,block,parameters,pitch,random);
        if(block%127==0){weave_before::start(old,bs,1);weave::start(now,as,1);}
        weave_before::render(old,bs,1,before,parameters,pitch);weave::render(now,as,1,after,parameters,pitch);
        if(sizeof(old)!=sizeof(now) || std::memcmp(before,after,sizeof(before)) || std::memcmp(&old,&now,sizeof(old)))return false;
    }
    return true;
}
template <class Voice,class Shared,class Initialize,class Start,class Render>
double timing(Voice& voice,Shared& shared,const int* initial,unsigned mode,Initialize initialize,Start start,Render render) {
    voice=Voice();initialize(voice,shared,1);
    int parameters[8],output[64];unsigned pitch[64],random=7139;
    for(unsigned j=0;j<8;++j)parameters[j]=initial[j];
    std::chrono::steady_clock::time_point begin;
    for(unsigned block=0;block<64+@BLOCKS@;++block) {
        if(block==64)begin=std::chrono::steady_clock::now();
        inputs(initial,mode,block,parameters,pitch,random);
        if(block%31==0)start(voice,shared,1);
        render(voice,shared,1,output,parameters,pitch);sink^=static_cast<unsigned>(output[block&63]);
    }
    return std::chrono::duration<double,std::nano>(std::chrono::steady_clock::now()-begin).count()/@BLOCKS@;
}
int main() {
    weave_before::Voice before;weave_before::Shared bs;
    weave::Voice after;weave::Shared as;
    const char* modes[]={"bass","high note","control and pitch automation"};
    for(unsigned setting=0;setting<sizeof(settings)/sizeof(settings[0]);++setting)for(unsigned mode=0;mode<3;++mode) {
        if(!equivalent(settings[setting],mode)){
            std::fprintf(stderr,"FAIL: audio/state differs at %s / %s\n",labels[setting],modes[mode]);return 1;
        }
        if(@VERIFY_ONLY@){
            std::printf("{\"setting\":\"%s\",\"mode\":\"%s\",\"verified\":true}\n",labels[setting],modes[mode]);continue;
        }
        double old_times[5],new_times[5],ratios[5];
        for(unsigned repeat=0;repeat<5;++repeat) {
            if(repeat&1) {
                new_times[repeat]=timing(after,as,settings[setting],mode,weave::initialize,weave::start,weave::render);
                old_times[repeat]=timing(before,bs,settings[setting],mode,weave_before::initialize,weave_before::start,weave_before::render);
            } else {
                old_times[repeat]=timing(before,bs,settings[setting],mode,weave_before::initialize,weave_before::start,weave_before::render);
                new_times[repeat]=timing(after,as,settings[setting],mode,weave::initialize,weave::start,weave::render);
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
    parser.add_argument('--baseline', default='2db2808')
    parser.add_argument('--sdk', type=Path)
    parser.add_argument('--cxx', default='c++')
    parser.add_argument('--blocks', type=int, default=1000)
    parser.add_argument('--all-fm-patches', action='store_true', help='also cover every six-op patch in normal/held mode')
    parser.add_argument('--verify-only', action='store_true', help='compare every checked audio/state block without timing')
    args = parser.parse_args()
    if args.blocks < 400:
        parser.error('Use at least 400 blocks to reduce timer noise.')
    config_file = REPO/'local/dev.json'
    config = json.loads(config_file.read_text()) if config_file.exists() else {}
    sdk = (args.sdk or Path(config.get('sdk', REPO.parent/'xy-machines-sdk'))).resolve()
    if not (sdk/'sdk/include/opxy/audio.hpp').is_file():
        parser.error('Pass --sdk with an Open-XY SDK checkout.')
    baseline = subprocess.check_output(['git', 'rev-parse', '--verify', args.baseline+'^{commit}'], cwd=REPO, text=True).strip()
    definition = json.loads((MACHINE/'definition.json').read_text())
    defaults = [round(p['default']*MAX) for p in definition['parameters']]
    settings, labels = [], []
    for model, name in enumerate(definition['parameters'][0]['mapping']['labels']):
        for controls in ('default', 'minimum', 'maximum'):
            for output, value in (('main', 0), ('blend', MAX//2), ('aux', MAX)):
                row = defaults.copy(); row[0] = (2*model+1)*MAX//30; row[4] = value
                if controls != 'default':
                    for index in (1, 2, 3, 5, 6, 7):row[index] = 0 if controls == 'minimum' else MAX
                settings.append(row); labels.append(name+'/'+controls+'/'+output)
    if args.all_fm_patches:
        for patch in range(96):
            raw = (4*patch+2)//3
            for held in (False, True):
                for output, value in (('main', 0), ('blend', MAX//2), ('aux', MAX)):
                    row = defaults.copy(); row[0] = 29*MAX//30; row[1] = (2*raw+1)*MAX//256
                    row[4] = value; row[6] = MAX if held else 0
                    settings.append(row); labels.append('six op/patch '+str(patch)+('/hold/' if held else '/normal/')+output)
    with tempfile.TemporaryDirectory(prefix='weave-benchmark-') as directory:
        scratch = Path(directory); before = scratch/'before'; before.mkdir()
        paths = subprocess.check_output(['git', 'ls-tree', '-r', '--name-only', baseline, '--', 'machines/weave'], cwd=REPO, text=True).splitlines()
        for path in paths:
            relative = Path(path).relative_to('machines/weave')
            if relative.as_posix() not in ('core.h', 'engine.cpp', 'resample.h') and not (relative.parts[0] == 'vendor' and relative.suffix in ('.cpp', '.h', '.hpp')):
                continue
            target = before/relative; target.parent.mkdir(parents=True, exist_ok=True)
            content = subprocess.check_output(['git', 'show', baseline+':'+path], cwd=REPO, text=True)
            target.write_text(content.replace('weave_dsp', 'weave_before_dsp').replace('namespace weave {', 'namespace weave_before {'))
        source = SOURCE.replace('@BEFORE@', json.dumps(str(before/'engine.cpp'))).replace('@AFTER@', json.dumps(str(MACHINE/'engine.cpp')))
        source = source.replace('@SETTINGS@', ','.join('{'+','.join(map(str, row))+'}' for row in settings))
        source = source.replace('@LABELS@', ','.join(map(json.dumps, labels))).replace('@BLOCKS@', str(args.blocks))
        source = source.replace('@VERIFY_ONLY@', 'true' if args.verify_only else 'false')
        (scratch/'benchmark.cpp').write_text(source)
        executable = scratch/'benchmark'
        subprocess.run([args.cxx, '-std=c++11', '-O2', '-I'+str(sdk/'sdk/include'), str(scratch/'benchmark.cpp'), '-o', str(executable)], check=True)
        rows = [json.loads(line) for line in subprocess.check_output([str(executable)], text=True).splitlines()]
    if args.verify_only:
        print(json.dumps(dict(baseline=baseline, exact_audio_and_state_each_block=True, scenarios=len(rows),
                             checked_blocks_per_scenario=256, timing_executed=False), indent=2))
        return
    print(json.dumps(dict(baseline=baseline, host_timing_only=True, exact_audio_and_state_each_block=True,
                         scenarios=len(rows), blocks_per_timing=args.blocks, repeats=5,
                         median_reduction_percent=round(100*(1-statistics.median(row['paired_ratio'] for row in rows)), 1),
                         worst_before=max(rows, key=lambda row: row['before_ns']),
                         worst_after=max(rows, key=lambda row: row['after_ns']), results=rows), indent=2))


if __name__ == '__main__':
    main()
