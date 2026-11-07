"""Time combined settings with the SDK's host reference; never device CPU."""
import argparse
import json
from pathlib import Path
import subprocess
import sys
import tempfile

MACHINE=Path(__file__).resolve().parents[1]
ROOT=MACHINE.parents[1]

def main():
    p=argparse.ArgumentParser(description=__doc__);p.add_argument('--out',type=Path,required=True);a=p.parse_args()
    config=json.loads((ROOT/'local/dev.json').read_text());sdk=Path(config['sdk']);sys.path.insert(0,str(sdk))
    from tooling.cpu import reference_cost,REFERENCE_INSTRUCTIONS
    cases=[('clean',[0]*8),('cassette keys',[.36,.23,.38,0,.625,.35,0,0]),('VHS pad',[.62,.34,.42,.25,.375,.55,.06,0]),('worn drums',[.42,.48,.7,.7,.375,.4,.035,0])]
    cases += [(f'all enabled model {model}',[1,1,1,1,(model+.5)/4,1,1,0]) for model in range(4)]
    rows=',\n'.join('{'+','.join(str(round(x*2147483647)) for x in values)+'}' for _,values in cases)
    with tempfile.TemporaryDirectory(prefix='gentape-benchmark-') as folder:
        folder=Path(folder);source=folder/'bench.cpp';binary=folder/'bench'
        source.write_text('''#include "@SOURCE@"
#include <chrono>
#include <cstdio>
static const int cases[][8]={@CASES@};
volatile unsigned sink=0;
int main(){
 static int memory[gentape::buffer_words];gentape::State s;int left[64],right[64],noise[64];unsigned seed=1;
 for(unsigned i=0;i<64;++i){seed=seed*1664525u+1013904223u;noise[i]=static_cast<int>(seed)>>2;}
 for(const auto& parameters:cases){
  double best=1e30;
  for(unsigned repeat=0;repeat<5;++repeat){
   gentape::initialize(s);
   std::chrono::steady_clock::time_point begin;
   for(unsigned b=0;b<6500;++b){
    if(b==500)begin=std::chrono::steady_clock::now();
    for(unsigned i=0;i<64;++i){left[i]=noise[i];right[i]=noise[(i+17)&63];}
    gentape::process(s,{memory,gentape::buffer_words},left,right,parameters);sink^=unsigned(left[b&63]);
   }
   const double ns=std::chrono::duration<double,std::nano>(std::chrono::steady_clock::now()-begin).count()/6000;
   if(ns<best)best=ns;
  }
  std::printf("%.3f\\n",best);
 }
}
'''.replace('@SOURCE@',str(MACHINE/'fx.cpp')).replace('@CASES@',rows))
        host='c++'
        subprocess.run([host,'-std=c++11','-O2','-I'+str(sdk/'sdk/include'),str(source),'-o',str(binary)],check=True)
        times=list(map(float,subprocess.check_output([str(binary)],text=True).split()))
        reference=reference_cost(host,folder)
        report={'host_only':True,'hardware_cpu_measured':False,'reference_ns':reference,'scope':'Stereo callback; combined settings, 500 warmup + 6000 timed blocks, best of five. Not Blackfin cycles or a hardware percentage.','results':[{'case':name,'ns_per_block':ns,'estimated_instructions':round(ns/reference*REFERENCE_INSTRUCTIONS)} for (name,_),ns in zip(cases,times)]}
    a.out.write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(report,indent=2))

if __name__=='__main__':main()
