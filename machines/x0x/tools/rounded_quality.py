"""Compare rounded audio products against the committed truncating implementation."""
import argparse
import json
from pathlib import Path
import subprocess
import tempfile

MACHINE=Path(__file__).resolve().parents[1]
REPO=MACHINE.parents[1]
SOURCE=r'''
#include @BEFORE@
#include @AFTER@
#include <cmath>
#include <cstdio>
#include <cstdlib>
static void require(bool condition,const char* text){if(!condition){std::fprintf(stderr,"FAIL: %s\n",text);std::exit(1);}}
int main(){
 const char* modes[]={"steady","tone automation","all automation"};
 for(unsigned model=0;model<x0x::model_count;++model)for(unsigned family=0;family<4;++family){
  if(!(x0x::family_masks[model]&(1u<<family)))continue;
  for(unsigned variant=0;variant<3;++variant)for(unsigned mode=0;mode<3;++mode){
   x0x_before::Voice old;x0x_before::Shared os;x0x::Voice now;x0x::Shared ns;
   x0x_before::initialize(old,os,1);x0x::initialize(now,ns,1);
   x0x_before::start(old,os,1);x0x::start(now,ns,1);
   x0x_before::drum_select(old,os,1,model);x0x::drum_select(now,ns,1,model);
   int p[8],a[64],b[64];unsigned pitch[64]={},random=7139;
   unsigned defaults[]={x0x::family_raw(model,family),64,100,64,0,64,64,0};
   for(unsigned i=0;i<8;++i){unsigned value=i==0||variant==0?defaults[i]:variant==1?0:127;p[i]=value==127?0x7fffffff:int(value<<24);}
   double aa=0,bb=0,dd=0,edge_a=0,edge_b=0;int previous_a=0,previous_b=0;unsigned peak=0;
   for(unsigned block=0;block<512;++block){
    if(mode==1)p[x0x::tone]=int((block%128)<<24);
    if(mode==2&&block%17==0)for(unsigned i=1;i<8;++i)p[i]=int(((random=random*1664525u+1013904223u)>>25)<<24);
    if(mode&&block%127==0){x0x_before::start(old,os,1);x0x::start(now,ns,1);x0x_before::drum_select(old,os,1,model);x0x::drum_select(now,ns,1,model);}
    x0x_before::render(old,os,1,a,p,pitch);x0x::render(now,ns,1,b,p,pitch);
    for(unsigned i=0;i<64;++i){double av=a[i]/2147483648.,bv=b[i]/2147483648.;aa+=av*av;bb+=bv*bv;dd+=(av-bv)*(av-bv);edge_a+=std::pow((double(a[i])-previous_a)/2147483648.,2);edge_b+=std::pow((double(b[i])-previous_b)/2147483648.,2);previous_a=a[i];previous_b=b[i];unsigned magnitude=unsigned(std::abs(b[i]));if(magnitude>peak)peak=magnitude;}
    require(now.output_low>=-32767&&now.output_low<=32767,"rounded LP remains bounded");
    require(now.noise_low>=-16384&&now.noise_low<=16383,"rounded noise LP remains bounded");
   }
   require(peak<=1073741824,"half scale headroom");
   double error=std::sqrt(dd/32768),relative=std::sqrt(dd/(aa+1e-30)),gain=10*std::log10((bb+1e-30)/(aa+1e-30));
   require(error<0.0025,"absolute error below -52dBFS across extreme controls");
   require(aa/32768<1e-8||relative<0.025,"audible render differs by less than 2.5% RMS");
   require(aa/32768<1e-8||std::fabs(gain)<0.15,"audible render gain change below 0.15dB");
   std::printf("{\"model\":%u,\"family\":%u,\"variant\":%u,\"mode\":\"%s\",\"rms\":%.12g,\"delta_rms\":%.12g,\"relative_error\":%.12g,\"gain_db\":%.12g,\"edge_ratio\":%.12g}\n",model,family,variant,modes[mode],std::sqrt(aa/32768),error,relative,gain,std::sqrt((edge_b+1e-30)/(edge_a+1e-30)));
  }
  x0x::Voice tail;x0x::Shared ts;x0x::initialize(tail,ts,1);x0x::start(tail,ts,1);x0x::drum_select(tail,ts,1,model);
  int p[8]={int(x0x::family_raw(model,family)<<24),64<<24,127<<24,64<<24,0,64<<24,64<<24,127<<24},audio[64];unsigned pitch[64]={};
  for(unsigned block=0;block<7000;++block)x0x::render(tail,ts,1,audio,p,pitch);
  require(!tail.active,"finite full-decay release terminates");for(int value:audio)require(value==0,"release tail reaches digital zero");
 }
}
'''

def main():
 parser=argparse.ArgumentParser(description=__doc__)
 parser.add_argument('--sdk',required=True,type=Path)
 parser.add_argument('--out',required=True,type=Path)
 parser.add_argument('--cxx',default='c++')
 parser.add_argument('--sanitize',action='store_true')
 args=parser.parse_args();args.out.mkdir(parents=True,exist_ok=False)
 baseline='9b371e81e70b979ffe65bbc7bc16b091ff5edbe9'
 with tempfile.TemporaryDirectory(prefix='x0x-rounding-',dir=args.out) as temporary:
  work=Path(temporary);before=work/'before';before.mkdir()
  paths=subprocess.check_output(['git','ls-tree','-r','--name-only',baseline,'--','machines/x0x'],cwd=REPO,text=True).splitlines()
  for path in paths:
   relative=Path(path).relative_to('machines/x0x')
   if relative.as_posix() not in ('core.h','models.h','engine.cpp') and not(relative.parts[0]=='dsp' and relative.suffix in ('.cpp','.h','.hpp')):continue
   target=before/relative;target.parent.mkdir(parents=True,exist_ok=True)
   text=subprocess.check_output(['git','show',baseline+':'+path],cwd=REPO,text=True)
   target.write_text(text.replace('namespace x0x {','namespace x0x_before {').replace('x0x::','x0x_before::'))
  source=SOURCE.replace('@BEFORE@',json.dumps(str(before/'engine.cpp'))).replace('@AFTER@',json.dumps(str(MACHINE/'engine.cpp')))
  (work/'quality.cpp').write_text(source)
  options=['-O1','-g','-fsanitize=address,undefined','-fno-sanitize-recover=all'] if args.sanitize else ['-O2']
  subprocess.run([args.cxx,'-std=c++11',*options,'-I'+str(args.sdk/'sdk/include'),str(work/'quality.cpp'),*[str(p) for p in (before/'dsp').glob('*.cpp')],*[str(p) for p in (MACHINE/'dsp').glob('*.cpp')],'-o',str(work/'quality')],check=True)
  result=subprocess.run([str(work/'quality')],text=True,capture_output=True);(args.out/'execution.log').write_text(result.stdout+result.stderr);result.check_returncode()
  rows=[json.loads(row) for row in result.stdout.splitlines()]
  require_count=47*3*3
  if len(rows)!=require_count:raise RuntimeError('Incomplete comparison')
  report=dict(candidate_commit=subprocess.check_output(['git','rev-parse','HEAD'],cwd=REPO,text=True).strip(),candidate_clean=not subprocess.check_output(['git','status','--porcelain','--untracked-files=all','--','machines/x0x'],cwd=REPO,text=True).strip(),baseline=baseline,scenarios=len(rows),blocks=512,release_tail_blocks=7000,sanitized=args.sanitize,hardware_cpu_pending=True,max_delta_rms=max(r['delta_rms'] for r in rows),max_audible_relative_error=max(r['relative_error'] for r in rows if r['rms']>=0.0001),max_audible_gain_db=max(abs(r['gain_db']) for r in rows if r['rms']>=0.0001),results=rows)
  (args.out/'quality.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({k:v for k,v in report.items() if k!='results'},indent=2))
if __name__=='__main__':main()
