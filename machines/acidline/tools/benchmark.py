"""Compare exact Acidline output and paired host timings against a Git revision."""
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
const int settings[][8] = {@SETTINGS@};
const char* labels[] = {@LABELS@};
volatile unsigned sink = 0;

template <class V, class S, class Initialize, class Start, class Render>
double run(V& voice, S& shared, const int* initial, unsigned mode, unsigned blocks,
           Initialize initialize, Start start, Render render, unsigned long long& hash) {
    int parameters[8], output[64]; unsigned pitch[64], seed=7139;
    for(unsigned j=0;j<8;++j) parameters[j]=initial[j];
    initialize(voice,shared,1);
    hash=14695981039346656037ull;
    const auto begin=std::chrono::steady_clock::now();
    for(unsigned b=0;b<blocks;++b) {
        if(mode==2) {
            if(b%8==0) for(unsigned j=0;j<8;++j) parameters[j]=initial[j];
            seed=seed*1664525u+1013904223u;
            parameters[b%8]=static_cast<int>(seed>>1);
        }
        for(unsigned i=0;i<64;++i)
            pitch[i]=mode==0 ? 5356535u : mode==1 ? 342818246u : 5356535u+(b%128)*720031u+i*10001u;
        if(b%32==0) start(voice,shared,1);
        render(voice,shared,1,output,parameters,pitch);
        if(blocks==128) for(unsigned i=0;i<64;++i)
            hash=(hash^static_cast<unsigned>(output[i]))*1099511628211ull;
        sink^=static_cast<unsigned>(output[b&63]);
    }
    return std::chrono::duration<double,std::nano>(std::chrono::steady_clock::now()-begin).count()/blocks;
}

int main() {
    acidline_before::Voice before; acidline_before::Shared bs;
    acidline::Voice after; acidline::Shared as;
    const char* modes[]={"bass","high note","control and pitch automation"};
    for(unsigned s=0;s<sizeof(settings)/sizeof(settings[0]);++s) for(unsigned mode=0;mode<3;++mode) {
        unsigned long long old_hash,new_hash;
        run(before,bs,settings[s],mode,128,acidline_before::initialize,acidline_before::start,acidline_before::render,old_hash);
        run(after,as,settings[s],mode,128,acidline::initialize,acidline::start,acidline::render,new_hash);
        if(sizeof(before)!=sizeof(after) || old_hash!=new_hash || std::memcmp(&before,&after,sizeof(before))) {
            std::fprintf(stderr,"FAIL: audio/state differs at %s / %s\n",labels[s],modes[mode]);return 1;
        }
        double old_times[5],new_times[5],ratios[5];
        for(unsigned repeat=0;repeat<5;++repeat) {
            if(repeat&1) {
                new_times[repeat]=run(after,as,settings[s],mode,@BLOCKS@,acidline::initialize,acidline::start,acidline::render,new_hash);
                old_times[repeat]=run(before,bs,settings[s],mode,@BLOCKS@,acidline_before::initialize,acidline_before::start,acidline_before::render,old_hash);
            } else {
                old_times[repeat]=run(before,bs,settings[s],mode,@BLOCKS@,acidline_before::initialize,acidline_before::start,acidline_before::render,old_hash);
                new_times[repeat]=run(after,as,settings[s],mode,@BLOCKS@,acidline::initialize,acidline::start,acidline::render,new_hash);
            }
            ratios[repeat]=new_times[repeat]/old_times[repeat];
        }
        std::sort(old_times,old_times+5);std::sort(new_times,new_times+5);std::sort(ratios,ratios+5);
        std::printf("{\"setting\":\"%s\",\"mode\":\"%s\",\"before_ns\":%.1f,\"after_ns\":%.1f,\"paired_ratio\":%.6f}\n",
                    labels[s],modes[mode],old_times[2],new_times[2],ratios[2]);
    }
}
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', default='0c9dddeb9b86b4f438699ad20eeb008a5f1c9fb5')
    parser.add_argument('--sdk', type=Path)
    parser.add_argument('--cxx', default='c++')
    parser.add_argument('--blocks', type=int, default=1000)
    args = parser.parse_args()
    if args.blocks < 400:
        parser.error('Use at least 400 blocks to reduce timer noise.')
    config_file = REPO / 'local/dev.json'
    config = json.loads(config_file.read_text()) if config_file.exists() else {}
    sdk = (args.sdk or Path(config.get('sdk', REPO.parent / 'xy-machines-sdk'))).resolve()
    if not (sdk / 'sdk/include/opxy/audio.hpp').is_file():
        parser.error('Pass --sdk with an Open-XY SDK checkout.')
    baseline = subprocess.check_output(['git', 'rev-parse', '--verify', args.baseline+'^{commit}'], cwd=REPO, text=True).strip()
    definition = json.loads((MACHINE / 'definition.json').read_text())
    defaults = [round(p['default'] * MAX) for p in definition['parameters']]
    settings, labels = [defaults], ['default']
    for index, parameter in enumerate(definition['parameters']):
        if parameter['mapping']['kind'] == 'enum':
            count = len(parameter['mapping']['labels'])
            values = [(2*i+1)*MAX//(2*count) for i in range(count)]
        else:
            values = [MAX*i//8 for i in range(9)]
        for step, value in enumerate(values):
            row = defaults.copy(); row[index] = value
            settings.append(row); labels.append(parameter['id']+':'+str(step))
    settings += [[MAX]*8, [0]*8]
    labels += ['all maximum', 'all zero']
    with tempfile.TemporaryDirectory(prefix='acidline-benchmark-') as folder:
        scratch = Path(folder); before = scratch / 'before'; before.mkdir()
        for name in ('core.h', 'tables.h', 'engine.cpp'):
            content = subprocess.check_output(['git', 'show', baseline+':machines/acidline/'+name], cwd=REPO, text=True)
            (before / name).write_text(content.replace('namespace acidline {', 'namespace acidline_before {'))
        source = SOURCE.replace('@BEFORE@', json.dumps(str(before/'engine.cpp'))).replace('@AFTER@', json.dumps(str(MACHINE/'engine.cpp')))
        source = source.replace('@SETTINGS@', ','.join('{'+','.join(map(str, row))+'}' for row in settings))
        source = source.replace('@LABELS@', ','.join(map(json.dumps, labels))).replace('@BLOCKS@', str(args.blocks))
        (scratch / 'benchmark.cpp').write_text(source)
        executable = scratch / 'benchmark'
        subprocess.run([args.cxx, '-std=c++11', '-O2', '-I'+str(sdk/'sdk/include'), str(scratch/'benchmark.cpp'), '-o', str(executable)], check=True)
        rows = [json.loads(line) for line in subprocess.check_output([str(executable)], text=True).splitlines()]
    print(json.dumps(dict(baseline=baseline, host_timing_only=True, exact_audio_and_final_state=True,
                         scenarios=len(rows), blocks_per_timing=args.blocks, repeats=5,
                         median_reduction_percent=round(100*(1-statistics.median(row['paired_ratio'] for row in rows)), 1),
                         worst_before=max(rows, key=lambda row: row['before_ns']),
                         worst_after=max(rows, key=lambda row: row['after_ns']), results=rows), indent=2))


if __name__ == '__main__':
    main()
