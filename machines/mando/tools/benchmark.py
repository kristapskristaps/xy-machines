"""Compare Mando against a Git revision in the same process; no device timing."""
import argparse
import ast
import statistics
import json
from pathlib import Path
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
#include <cstring>
#include <cstdio>
const int settings[][8] = {@SETTINGS@};
const char* labels[] = {@LABELS@};
volatile unsigned sink = 0;
template <class V, class S, class Start, class Render>
double timing(V& voice, S& shared, const int* initial, unsigned mode, Start start, Render render) {
    int parameters[8], output[64]; unsigned pitch[64];
    for (unsigned j=0;j<8;++j) parameters[j]=initial[j];
    const unsigned notes[] = {5356534u,42852281u,42852281u,685636492u,42852281u,42852281u,1610612736u,42852281u,1073741824u};
    for (unsigned i=0;i<64;++i) pitch[i]=notes[mode]+((mode==2 || mode==5 || mode==6) ? i*12345u : 0u);
    start(voice,shared,1);
    for (unsigned b=0;b<400;++b) render(voice,shared,1,output,parameters,pitch);
    unsigned seed=7139;
    const auto begin=std::chrono::steady_clock::now();
    for (unsigned b=0;b<@BLOCKS@;++b) {
        if(mode==7) for(unsigned i=0;i<64;++i) pitch[i]=(seed=seed*1664525u+1013904223u);
        if(mode==8) for(unsigned i=0;i<64;++i) pitch[i]=1073741792u+(i&1u)*64u;
        if(mode==5) {
            for(unsigned j=0;j<8;++j) { seed=seed*1664525u+1013904223u;parameters[j]=static_cast<int>(seed>>1); }
            parameters[0]=2147483647-static_cast<int>(seed&0x00ffffffu);
            parameters[1]=parameters[2]=parameters[0];
            parameters[5]=536870912+static_cast<int>((seed>>1)&0x3fffffffu);
        }
        if(mode==4) parameters[3]=static_cast<int>((2u*((b/8)&3u)+1u)*(2147483647u/8u));
        render(voice,shared,1,output,parameters,pitch);sink^=static_cast<unsigned>(output[b&63]);
    }
    return std::chrono::duration<double,std::nano>(std::chrono::steady_clock::now()-begin).count()/@BLOCKS@;
}
bool equivalent(const int* initial,unsigned mode) {
    mando_before::Voice before={};mando_before::Shared bs;
    mando::Voice after={};mando::Shared as;
    int p[8],old_output[64],new_output[64];unsigned pitch[64],seed=7139;
    const unsigned notes[]={5356534u,42852281u,42852281u,685636492u,42852281u,42852281u,1610612736u,42852281u,1073741824u};
    for(unsigned i=0;i<8;++i)p[i]=initial[i];
    mando_before::start(before,bs,1);mando::start(after,as,1);
    for(unsigned block=0;block<256;++block){
        for(unsigned i=0;i<64;++i)pitch[i]=notes[mode]+((mode==2||mode==5||mode==6)?i*12345u:0u);
        if(mode==7)for(unsigned i=0;i<64;++i)pitch[i]=(seed=seed*1664525u+1013904223u);
        if(mode==6 && (block&1))pitch[0]=1073741824u;
        if(mode==8)for(unsigned i=0;i<64;++i)pitch[i]=1073741792u+(i&1u)*64u;
        if(mode==5){
            for(unsigned j=0;j<8;++j){seed=seed*1664525u+1013904223u;p[j]=int(seed>>1);}
            p[0]=2147483647-int(seed&0x00ffffffu);p[1]=p[2]=p[0];
            p[5]=536870912+int((seed>>1)&0x3fffffffu);
        }
        if(mode==4)p[3]=int((2u*((block/8)&3u)+1u)*(2147483647u/8u));
        if(block%127==0){mando_before::start(before,bs,1);mando::start(after,as,1);}
        mando_before::render(before,bs,1,old_output,p,pitch);mando::render(after,as,1,new_output,p,pitch);
        if(std::memcmp(old_output,new_output,sizeof(old_output)) ||
           (@VERIFY_STATE@ && (sizeof(before)!=sizeof(after)||std::memcmp(&before,&after,sizeof(before))))){
            std::fprintf(stderr,"audio/state differs at mode %u block %u\n",mode,block);return false;
        }
    }
    return true;
}
@REFERENCE@
int main() {
    mando_before::Voice before={}; mando_before::Shared bs;
    mando::Voice after={}; mando::Shared as;
    const char* modes[] = {"bass","steady","pitch bend","high note","wave automation","overload automation","clamped pitch","random pitch","clamp crossing"};
    for(unsigned s=0;s<sizeof(settings)/sizeof(settings[0]);++s) for(unsigned mode=0;mode<@MODES@;++mode) {
        if(!equivalent(settings[s],mode))return 1;
        double old_times[5],new_times[5],ratios[5];
        for(unsigned run=0;run<5;++run) {
            double old,now;
            if(run&1) {
                now=timing(after,as,settings[s],mode,mando::start,mando::render);
                old=timing(before,bs,settings[s],mode,mando_before::start,mando_before::render);
            } else {
                old=timing(before,bs,settings[s],mode,mando_before::start,mando_before::render);
                now=timing(after,as,settings[s],mode,mando::start,mando::render);
            }
            old_times[run]=old;new_times[run]=now;ratios[run]=now/old;
        }
        std::sort(old_times,old_times+5);std::sort(new_times,new_times+5);std::sort(ratios,ratios+5);
        const double reference = reference_ns();
        std::printf("{\"setting\":\"%s\",\"mode\":\"%s\",\"before_ns\":%.1f,\"after_ns\":%.1f,\"reference_ns\":%.3f,\"paired_ratio\":%.6f}\n",labels[s],modes[mode],old_times[2],new_times[2],reference,ratios[2]);
    }
}
'''


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--baseline', default='a5d22b3', help='collection revision before B optimization')
    parser.add_argument('--sdk', type=Path, help='SDK checkout supplying public headers')
    parser.add_argument('--cxx', default='c++')
    parser.add_argument('--blocks', type=int, default=4000)
    parser.add_argument('--verify-state', action='store_true', help='Compare complete state when both revisions use the same Voice layout')
    parser.add_argument('--extended', action='store_true', help='Also cover clamped and full-range sample pitch motion')
    args = parser.parse_args()
    if args.blocks < 400:
        parser.error('Use at least 400 blocks to reduce timer noise.')
    config_file = REPO / 'local/dev.json'
    config = json.loads(config_file.read_text()) if config_file.exists() else {}
    sdk = (args.sdk or Path(config.get('sdk', REPO.parent / 'xy-machines-sdk'))).resolve()
    if not (sdk / 'sdk/include/opxy/audio.hpp').is_file():
        parser.error('Pass --sdk with an Open-XY SDK checkout.')
    baseline = subprocess.check_output(['git','rev-parse','--verify',args.baseline+'^{commit}'],cwd=REPO,text=True).strip()
    # Use the pinned SDK's reference loop/count in this process. Return its
    # time instead of printing it, and use a defined sink operation per block.
    cpu = ast.parse((sdk / 'tooling/cpu.py').read_text())
    constants = {node.targets[0].id: ast.literal_eval(node.value) for node in cpu.body
                 if isinstance(node, ast.Assign) and isinstance(node.targets[0], ast.Name)
                 and node.targets[0].id in ('REFERENCE', 'REFERENCE_INSTRUCTIONS')}
    reference = constants['REFERENCE'].replace('int main() {', 'double reference_ns() {')
    ending = '    std::printf("%.3f\\n", best / 20000);\n    return 0;'
    if ending not in reference:
        parser.error('The pinned SDK reference reporting changed; review this benchmark adapter.')
    reference = reference.replace(ending, '    return best / 20000;')
    # Preserve one sink operation per block without the original signed overflow.
    reference = reference.replace('sink = sink + output[block & 63]', 'sink = sink ^ output[block & 63]')
    definition = json.loads((MACHINE / 'definition.json').read_text())
    defaults = [round(p['default'] * MAX) for p in definition['parameters']]
    settings, labels = [defaults], ['default']
    for index, parameter in enumerate(definition['parameters']):
        if parameter['mapping']['kind'] == 'enum':
            count = len(parameter['mapping']['labels'])
            values = [(2*i+1)*MAX//(2*count) for i in range(count)]
        else:
            values = [MAX*i//8 for i in range(9)]
        for i, value in enumerate(values):
            row = defaults.copy(); row[index] = value
            settings.append(row); labels.append(parameter['id'] + ':' + str(i))
    settings += [[MAX]*8, [MAX,MAX,MAX,0,MAX,MAX//2,MAX//2,MAX]]
    labels += ['all maximum', 'bright drive resonance']
    with tempfile.TemporaryDirectory(prefix='mando-benchmark-') as folder:
        scratch = Path(folder); before = scratch / 'before'; before.mkdir()
        for relative in ('machines/mando', 'machines/cyclops'):
            paths = subprocess.check_output(['git','ls-tree','-r','--name-only',baseline,'--',relative], cwd=REPO, text=True).splitlines()
            if paths:
                break
        if not paths:
            parser.error('Baseline machine is absent; fetch the collection history or choose --baseline.')
        for path in paths:
            if Path(path).suffix not in ('.cpp','.h','.hpp'):
                continue
            target = before / Path(path).relative_to(relative)
            target.parent.mkdir(parents=True, exist_ok=True)
            text = subprocess.check_output(['git','show',baseline+':'+path], cwd=REPO, text=True)
            target.write_text(text.replace('namespace cyclops {','namespace mando_before {').replace('namespace mando {','namespace mando_before {'))
        source = SOURCE.replace('@BEFORE@',json.dumps(str(before/'engine.cpp'))).replace('@AFTER@',json.dumps(str(MACHINE/'engine.cpp')))
        source = source.replace('@SETTINGS@',','.join('{'+','.join(map(str,row))+'}' for row in settings))
        source = source.replace('@REFERENCE@',reference).replace('@VERIFY_STATE@','true' if args.verify_state else 'false').replace('@MODES@','9' if args.extended else '6')
        source = source.replace('@LABELS@',','.join(map(json.dumps,labels))).replace('@BLOCKS@',str(args.blocks))
        (scratch/'benchmark.cpp').write_text(source)
        executable = scratch/'benchmark'
        subprocess.run([args.cxx,'-std=c++11','-O2','-I'+str(sdk/'sdk/include'),str(scratch/'benchmark.cpp'),'-o',str(executable)],check=True)
        result = subprocess.check_output([str(executable)],text=True)
        rows = [json.loads(line) for line in result.splitlines()]
    for row in rows:
        row['estimated_before'] = round(row['before_ns'] / row['reference_ns'] * constants['REFERENCE_INSTRUCTIONS'])
        row['estimated_after'] = round(row['after_ns'] / row['reference_ns'] * constants['REFERENCE_INSTRUCTIONS'])
    worst_before = max(rows,key=lambda row:row['before_ns'])
    worst_after = max(rows,key=lambda row:row['after_ns'])
    modes = {mode: {
        'max_estimated_before': max(row['estimated_before'] for row in rows if row['mode'] == mode),
        'max_estimated_after': max(row['estimated_after'] for row in rows if row['mode'] == mode)
    } for mode in sorted({row['mode'] for row in rows})}
    print(json.dumps(dict(baseline=baseline,host_timing_only=True,
                         exact_audio_each_block=True,exact_voice_state_each_block=args.verify_state,
                         checked_blocks_per_scenario=256,extended=args.extended,
                         worst_before=worst_before,worst_after=worst_after,
                         worst_reduction_percent=round(100*(1-worst_after['after_ns']/worst_before['before_ns']),1),
                         median_reduction_percent=round(100*(1-statistics.median(row['paired_ratio'] for row in rows)),1),
                         max_estimated_before=max(row['estimated_before'] for row in rows),
                         max_estimated_after=max(row['estimated_after'] for row in rows),
                         scenarios=len(rows),modes=modes,results=rows),indent=2))


if __name__ == '__main__':
    main()
