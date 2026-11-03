#include "../engine.cpp"
#define mando mando_legacy
#include "reference/engine.cpp"
#undef mando
#include <cmath>
#include <cstdio>
#include <cstring>

int main(){
 const opxy::i32 patches[8][8]={
 {1331439861,0,0,0,0,1073741824,1073741824,0},
 {1030792151,322122547,386547056,0,257698038,1116691496,1073741824,858993459},
 {622770258,107374182,0,1879048191,0,1073741824,0,1395864371},
 {1352914698,1438814043,687194767,805306368,343597384,923417968,1073741824,751619276},
 {1181116006,644245094,1288490188,1342177279,171798692,1288490188,0,1073741824},
 {1567663062,708669604,257698038,1879048191,429496729,1073741824,1073741824,1395864371},
 {1309965025,2018634628,171798692,0,0,0,1073741824,2147483647},
 {2147483647,0,0,0,515396075,1073741824,1073741824,0}};
 const double frequencies[]={27.5,40,65.406,110,220,440,1760,4000,8000};
 double worst_level=0,worst_error=0,worst_spectrum=0;unsigned cases=0;
 for(unsigned patch=0;patch<8;++patch)for(double hz:frequencies){
  mando::Voice a={};mando::Shared sa; mando_legacy::Voice b={};mando_legacy::Shared sb;
  mando::initialize(a,sa,1);mando_legacy::initialize(b,sb,1);
  opxy::i32 oa[64],ob[64];opxy::u32 pitch[64];
  for(auto& p:pitch)p=static_cast<opxy::u32>(hz*4294967296./44100.);
  double ea=0,eb=0,error=0,low_a[10]={},low_b[10]={},bands_a[11]={},bands_b[11]={};
  for(unsigned block=0;block<512;++block){
   mando::render(a,sa,1,oa,patches[patch],pitch);mando_legacy::render(b,sb,1,ob,patches[patch],pitch);
   if(a.phase[0]!=b.phase[0] || a.phase[1]!=b.phase[1])return 1;
   for(unsigned i=0;i<64;++i){double va=oa[i]/2147483648.,vb=ob[i]/2147483648.;
    ea+=va*va;eb+=vb*vb;error+=(va-vb)*(va-vb);
    double prior_a=va,prior_b=vb;
    for(unsigned k=0;k<10;++k){
     const double alpha=1-std::exp(-6.283185307179586*(16000./(1u<<k))/44100.);
     low_a[k]+=alpha*(va-low_a[k]);low_b[k]+=alpha*(vb-low_b[k]);
     const double ba=prior_a-low_a[k],bb=prior_b-low_b[k];
     bands_a[k]+=ba*ba;bands_b[k]+=bb*bb;prior_a=low_a[k];prior_b=low_b[k];
    }
    bands_a[10]+=prior_a*prior_a;bands_b[10]+=prior_b*prior_b;
   }
  }
  if(eb>1e-10){double db=10*std::log10(ea/eb),relative=error/eb;
   double total_a=0,total_b=0,spectrum=0;
   for(unsigned k=0;k<11;++k){total_a+=bands_a[k];total_b+=bands_b[k];}
   for(unsigned k=0;k<11;++k)spectrum+=std::fabs(bands_a[k]/total_a-bands_b[k]/total_b);
   std::printf("patch%u %.3fHz: level%+.4fdB waveform%.3fdB spectrum%.5f\n",patch,hz,db,10*std::log10(relative),spectrum);
   worst_spectrum=std::fmax(worst_spectrum,spectrum);
   worst_level=std::fmax(worst_level,std::fabs(db));worst_error=std::fmax(worst_error,relative);
   // Resonant limit cycles can drift in phase over long renders. Compare
   // their level and octave-band distribution, retaining waveform error as
   // a diagnostic rather than declaring identical sample phase.
   if(std::fabs(db)>1.0 || spectrum>.10)return 1;
  }
  ++cases;
 }
 std::printf("PASS: %u tuning-identical legacy comparisons, maxlevel%.4fdB, waveform%.3fdB spectrum%.5f\n",cases,worst_level,10*std::log10(worst_error),worst_spectrum);
}
