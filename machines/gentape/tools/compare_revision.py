"""Compare callback audio and transport with a committed Gen Tape revision."""
import argparse
import json
from pathlib import Path
import subprocess
import tempfile

MACHINE = Path(__file__).resolve().parents[1]
ROOT = MACHINE.parents[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--before', default='5c186b5')
    parser.add_argument('--out', type=Path, required=True)
    args = parser.parse_args()
    config = json.loads((ROOT / 'local/dev.json').read_text())
    revision = subprocess.check_output(['git', 'rev-parse', args.before], cwd=ROOT, text=True).strip()
    with tempfile.TemporaryDirectory(prefix='gentape-compare-') as folder:
        folder = Path(folder)
        for name in ('core.h', 'tables.h', 'fx.cpp'):
            source = subprocess.check_output(['git', 'show', f'{revision}:machines/gentape/{name}'], cwd=ROOT, text=True)
            (folder / name).write_text(source.replace('namespace gentape', 'namespace previous'))
        source = folder / 'compare.cpp'
        source.write_text(CPP.replace('@CURRENT@', str(MACHINE / 'fx.cpp')))
        binary = folder / 'compare'
        subprocess.run([config['cxx'], '-std=c++11', '-O2', '-I' + str(Path(config['sdk']) / 'sdk/include'), str(source), '-o', str(binary)], check=True)
        results = [json.loads(line) for line in subprocess.check_output([str(binary)], text=True).splitlines()]
    report = {'reference_commit': revision, 'hardware_cpu_measured': False, 'scope': 'Full stereo callbacks: 65536 blocks per case, including startup, failure events, reset with dirty memory, full-scale transients and continuous control changes.', 'cases': results}
    args.out.write_text(json.dumps(report, indent=2) + '\n')
    print(json.dumps(report, indent=2))


CPP = r'''
#include "fx.cpp"
#include "@CURRENT@"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>
static void require(bool condition, const char* message) {
    if (!condition) { std::fprintf(stderr,"FAIL: %s\n",message); std::exit(1); }
}
int main() {
    const int full=2147483647;
    unsigned seed=2701;
    for(unsigned test=0;test<11;++test) {
        previous::State before; gentape::State after;
        previous::initialize(before); gentape::initialize(after);
        std::vector<int> oldmem(previous::buffer_words,0x60000000), newmem(gentape::buffer_words,0x60000000);
        int p[8]={},oldl[64],oldr[64],newl[64],newr[64];
        long long max_error=0; double error_energy=0,signal_energy=0;
        unsigned events=0;
        for(unsigned b=0;b<65536;++b) {
            if(b==32768) {previous::initialize(before);gentape::initialize(after);}
            if(test<4) {for(unsigned k=0;k<7;++k)p[k]=full;p[4]=int((2*test+1)*268435455u);}
            else if(test<8) {p[4]=int((2*(test-4)+1)*268435455u);p[2]=full/2;}
            else if(test==8) { // Moving every control, with reset and model changes.
                for(unsigned k=0;k<8;++k) {unsigned x=(b*(k*71+19))&131071;p[k]=int((x<65536?x:131071-x)*32767u);}
            }
            else if(test==9) {for(unsigned k=0;k<8;++k)p[k]=b<256?full:0;}
            else {for(unsigned k=0;k<7;++k)p[k]=full;p[4]=3*268435455u;p[2]=b<256?0:full;}
            for(unsigned i=0;i<64;++i) {
                seed=seed*1664525u+1013904223u;
                int l=static_cast<int>(seed)>>2;
                seed=seed*1664525u+1013904223u;
                int r=static_cast<int>(seed)>>2;
                if((b&1023)>256 && (b&1023)<512) {
                    const double t=(b*64.+i)/44100.;
                    l=int(.95*full*std::sin(6.283185307179586*73*t));
                    r=int(.95*full*std::sin(6.283185307179586*9013*t));
                }
                if((b&1023)>640 && (b&1023)<768)l=r=(b&64)?full:-full;
                if((b&255)==0) {l=(i&1)?full:-full;r=-l;}
                if((b&1023)>900)l=r=0;
                oldl[i]=newl[i]=l;oldr[i]=newr[i]=r;
            }
            const unsigned event=before.event;
            previous::process(before,{oldmem.data(),previous::buffer_words},oldl,oldr,p);
            gentape::process(after,{newmem.data(),gentape::buffer_words},newl,newr,p);
            if(!event&&before.event)++events;
            require(before.random==after.random && before.noise_random==after.noise_random && before.event==after.event && before.wait==after.wait && before.duration==after.duration && before.kind==after.kind,"random modulation/event sequence changed");
            require(!std::memcmp(before.delay,after.delay,sizeof(before.delay)) && !std::memcmp(before.gate,after.gate,sizeof(before.gate)),"pitch or failure depth changed");
            for(unsigned i=0;i<64;++i)for(unsigned ch=0;ch<2;++ch) {
                const long long a=ch?oldr[i]:oldl[i],z=ch?newr[i]:newl[i],e=a-z;
                max_error=std::max(max_error,std::llabs(e));
                error_energy+=double(e)*e;signal_energy+=double(a)*a;
            }
        }
        const double relative=std::sqrt(error_energy/std::max(1.,signal_energy));
        if (test >= 4 && test <= 8) require(max_error==0,"Non-maximum or moving saturation changed");
        require(relative<1e-5,"callback difference exceeds -100 dB relative RMS");
        require(max_error<=16384,"callback difference exceeds -102 dBFS peak");
        // Compare retained audio by age; physical ring sizes may differ.
        const unsigned retained=std::min(1502u,unsigned(after.written));
        for(unsigned ch=0;ch<2;++ch) for(unsigned age=1;age<=retained;++age)
            require(std::llabs(static_cast<long long>(oldmem[ch*previous::line_words+((before.write-age)&(previous::line_frames-1))])
                 - newmem[ch*gentape::line_words+((after.write-age)&(gentape::line_frames-1))]) <= 1024,
                    "retained delay history differs by more than -102 dBFS");
        std::printf("{\"case\":%u,\"blocks\":65536,\"failure_events\":%u,\"maximum_error_q31\":%lld,\"relative_rms_error\":%.12g,\"transport_exact\":true}\n",test,events,max_error,relative);
    }
}
'''

if __name__ == '__main__':
    main()
