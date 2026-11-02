"""Compare HK Arc's audio/state and paired host timings against a Git revision."""
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
#include @BEFORE_DISPLAY@
#include @AFTER_DISPLAY@
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
    for(unsigned i=0;i<64;++i)pitch[i]=s.mode==3?(random=random*1664525u+1013904223u):s.mode==2?s.pitch+((i+block)%64)*17613u:s.pitch;
}
bool equivalent(const Scenario& s){
    hkarc_before::Voice old={};hkarc_before::Shared bs={};
    hkarc::Voice now={};hkarc::Shared as={};
    hkarc_before::initialize(old,bs,1);hkarc::initialize(now,as,1);
    hkarc_before::dsp(old).note.rng=hkarc::dsp(now).note.rng=7139;
    int p[8],before[64],after[64];unsigned pitch[64],random=7139;
    for(unsigned block=0;block<256;++block){
        controls(s,block,p,pitch,random);
        if(block%127==0){hkarc_before::start(old,bs,1);hkarc::start(now,as,1);}
        if(block==191){hkarc_before::destroy(old,bs,1);hkarc::destroy(now,as,1);hkarc_before::initialize(old,bs,1);hkarc::initialize(now,as,1);hkarc_before::dsp(old).note.rng=hkarc::dsp(now).note.rng=7139;}
        hkarc_before::render(old,bs,1,before,p,pitch);hkarc::render(now,as,1,after,p,pitch);
        const auto& bd=hkarc_before::dsp(old);const auto& ad=hkarc::dsp(now);
        if(std::memcmp(before,after,sizeof(before)) || old.pending!=now.pending || old.half!=now.half ||
           std::memcmp(old.pitch,now.pitch,sizeof(old.pitch)) || std::memcmp(old.audio,now.audio,sizeof(old.audio)) ||
           std::memcmp(&bd,&ad,offsetof(hkarc_before::port::Dsp,arc_gp)) || std::memcmp(&bd.note,&ad.note,sizeof(bd.note))){
            std::fprintf(stderr,"difference at %s pitch %u mode %u block %u\n",s.label,s.pitch,s.mode,block);return false;
        }
    }
    return true;
}
template<class Voice,class Shared,class Initialize,class Dsp,class Start,class Render>
double timing(const Scenario& s,Initialize initialize,Dsp dsp,Start start,Render render){
    Voice voice={};Shared shared={};initialize(voice,shared,1);
    // The production seed includes its address; pin it only in this paired fixture.
    dsp(voice).note.rng=7139;
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
bool display_equivalent(){
    hkarc_before::DisplayState old={};hkarc::DisplayState now={};
    hkarc_before::Frame before={};hkarc::Frame after={};opxy::draw::Input input={};
    hkarc_before::display_reset(old);hkarc::display_reset(now);
    unsigned random=7139;
    for(unsigned frame=0;frame<4096;++frame){
        for(unsigned i=0;i<8;++i)input.slots[i]=int(random=random*1664525u+1013904223u);
        for(unsigned i=0;i<4;++i){input.shown[i]=(frame+i)%8;input.parameters[i]=input.slots[input.shown[i]];}
        input.elapsed=frame%103;input.reduced_motion=frame%11==0;input.held_notes=frame%4;
        if(frame%9==0)++input.note_on_serial;
        if(frame%13==0)++input.note_off_serial;
        if(frame%97==0)input.track=(input.track+1)%8;
        hkarc_before::display(old,input,before);hkarc::display(now,input,after);
        if(sizeof(old)!=sizeof(now)||std::memcmp(&old,&now,sizeof(old))||std::memcmp(&before,&after,sizeof(before))){
            std::fprintf(stderr,"display difference at frame %u\n",frame);return false;
        }
    }
    return true;
}
int main(){
    if(!display_equivalent())return 1;
    for(const auto& s:scenarios){
        if(!equivalent(s))return 1;
        double old_times[5]={},new_times[5]={},ratios[5]={};
        if(!@VERIFY_ONLY@)for(unsigned repeat=0;repeat<5;++repeat){
            if(repeat&1){
                new_times[repeat]=timing<hkarc::Voice,hkarc::Shared>(s,hkarc::initialize,hkarc::dsp,hkarc::start,hkarc::render);
                old_times[repeat]=timing<hkarc_before::Voice,hkarc_before::Shared>(s,hkarc_before::initialize,hkarc_before::dsp,hkarc_before::start,hkarc_before::render);
            }else{
                old_times[repeat]=timing<hkarc_before::Voice,hkarc_before::Shared>(s,hkarc_before::initialize,hkarc_before::dsp,hkarc_before::start,hkarc_before::render);
                new_times[repeat]=timing<hkarc::Voice,hkarc::Shared>(s,hkarc::initialize,hkarc::dsp,hkarc::start,hkarc::render);
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
    parser.add_argument('--baseline', default='6456867')
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
    if not (sdk/'sdk/include/opxy/audio.hpp').is_file():
        parser.error('Pass --sdk with an Open-XY SDK checkout.')
    baseline = subprocess.check_output(['git','rev-parse','--verify',args.baseline+'^{commit}'],cwd=REPO,text=True).strip()
    definition = json.loads((MACHINE/'definition.json').read_text())
    defaults = [round(p['default']*2147483647) for p in definition['parameters']]
    slots = [p['id'] for p in definition['parameters']]
    settings = [('defaults',defaults)] + [(name,[round(preset[slot]*2147483647) for slot in slots]) for name,preset in definition.get('presets',{}).items()]
    defaults += [0]*(8-len(defaults))
    for slot in range(7):
        for step in range(5):
            values = defaults.copy(); values[slot] = round(step*2147483647/4)
            settings.append((f'control {slot} step {step}',values))
    for page in range(2):
        for mask in range(16):
            values = defaults.copy()
            for slot in range(4):values[page*4+slot]=2147483647 if mask&(1<<slot) else 0
            settings.append((f'page {page} corner {mask}',values))
    if args.full_grid:
        settings += [(f'full corner {mask}',[2147483647 if mask&(1<<slot) else 0 for slot in range(8)]) for mask in range(256)]
    pitches = [0,5356535,10713070,42852281,171409123,685636493]
    scenarios = [(name,values,pitch,0) for name,values in settings for pitch in pitches]
    scenarios += [('automation',defaults,pitch,mode) for mode in range(1,4) for pitch in pitches]
    rows_cpp = ['{'+json.dumps(name)+',{'+','.join(map(str,values))+'},'+str(pitch)+'u,'+str(mode)+'}' for name,values,pitch,mode in scenarios]
    with tempfile.TemporaryDirectory(prefix='hkarc-benchmark-') as directory:
        scratch = Path(directory); before = scratch/'before'; before.mkdir()
        for path in ['core.h','engine.cpp','dsp.hpp','wide.hpp','sine.hpp','display.h','display.cpp','visual_geometry.h','visual_runtime.h']:
            content = subprocess.check_output(['git','show',baseline+':machines/hundekraft/hkarc/'+path],cwd=REPO,text=True)
            (before/path).write_text(content.replace('namespace hkarc {','namespace hkarc_before {'))
        source = SOURCE.replace('@BEFORE@',json.dumps(str(before/'engine.cpp'))).replace('@AFTER@',json.dumps(str(MACHINE/'engine.cpp')))
        source = source.replace('@BEFORE_DISPLAY@',json.dumps(str(before/'display.cpp'))).replace('@AFTER_DISPLAY@',json.dumps(str(MACHINE/'display.cpp')))
        (scratch/'benchmark.cpp').write_text(source.replace('@BLOCKS@',str(args.blocks)).replace('@VERIFY_ONLY@','true' if args.verify_only else 'false').replace('@SCENARIOS@','{'+',\n'.join(rows_cpp)+'}'))
        executable = scratch/'benchmark'
        subprocess.run([args.cxx,'-std=c++11','-O2','-I'+str(sdk/'sdk/include'),str(scratch/'benchmark.cpp'),'-o',str(executable)],check=True)
        rows = [json.loads(line) for line in subprocess.check_output([str(executable)],text=True).splitlines()]
    print(json.dumps(dict(baseline=baseline,host_timing_only=True,exact_audio_and_retained_voice_state_each_block=True,removed_state='deterministic tuning-table cache',fixed_fixture_rng=7139,
                         full_grid=args.full_grid,verify_only=args.verify_only,checked_display_frames=4096,scenarios=len(rows),checked_blocks_per_scenario=256,
                         blocks_per_timing=0 if args.verify_only else args.blocks,repeats=0 if args.verify_only else 5,
                         median_reduction_percent=None if args.verify_only else round(100*(1-statistics.median(row['paired_ratio'] for row in rows)),1),results=rows),indent=2))


if __name__ == '__main__':
    main()
