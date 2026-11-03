#include "../fx.cpp"
#define halo halo_legacy
#include "reference/fx.cpp"
#undef halo
#include <cmath>
#include <cstdio>
#include <cstring>

static opxy::i32 ma[halo::buffer_words],mb[halo_legacy::buffer_words];
static opxy::i32 la[64],ra[64],lb[64],rb[64];
int main(){
    const opxy::i32 presets[][8]={
      {858993459,1181116006,257698038,107374182,966367642,1460288880,0,2147483647},
      {1395864371,1567663063,1417339207,214748365,816043786,1460288880,0,2147483647},
      {1675037245,1825361100,601295421,1546188226,1331439861,1116691496,0,2147483647}};
    double worst_level=0,worst_error=0,worst_late=0;
    for(unsigned test=0;test<15;++test){
      halo::State a;halo::initialize(a);halo_legacy::State b;halo_legacy::initialize(b);
      std::memset(ma,0x3f,sizeof ma);std::memset(mb,0x3f,sizeof mb);
      opxy::i32 p[8];std::memcpy(p,presets[test%3],sizeof p);
      if(test>=9)p[(test-9)%6]=2147483647;
      double ea=0,eb=0,error=0,latea=0,lateb=0;
      opxy::u32 rng=7139;double low=0,low2=0;
      for(unsigned block=0;block<4096;++block){
        for(unsigned i=0;i<64;++i){
          double input=0;
          if(test<3)input=block==0&&i==0?536870912.:0;
          else if(test<6)input=block<1024?std::sin(6.283185307179586*440*(block*64+i)/44100.)*268435456.:0;
          else if(test<9)input=block<1024?std::sin(6.283185307179586*2200*(block*64+i)/44100.)*268435456.:0;
          else {rng=rng*1664525u+1013904223u;low+=(static_cast<opxy::i32>(rng)/8.-low)*.22;low2+=(low-low2)*.22;input=block<1024?low2:0;}
          la[i]=lb[i]=static_cast<opxy::i32>(input);ra[i]=rb[i]=-la[i]/2;
        }
        opxy::Transport t={1200,0,true};
        halo::process(a,{ma,halo::buffer_words},t,la,ra,p);
        halo_legacy::process(b,{mb,halo_legacy::buffer_words},t,lb,rb,p);
        for(unsigned i=0;i<64;++i){
          const double va=la[i]/2147483648.,vb=lb[i]/2147483648.;
          ea+=va*va;eb+=vb*vb;error+=(va-vb)*(va-vb);
          if(block>=2048){latea+=va*va;lateb+=vb*vb;}
        }
      }
      if(!(ea>0&&eb>0&&latea>0&&lateb>0))return 1;
      const double db=10*std::log10(ea/eb),relative=10*std::log10(error/eb),late=10*std::log10(latea/lateb);
      std::printf("case %u: level %+0.3f dB, waveform error %0.2f dB, late energy %+0.3f dB\n",test,db,relative,late);
      worst_level=std::fmax(worst_level,std::fabs(db));worst_error=std::fmax(worst_error,error/eb);worst_late=std::fmax(worst_late,std::fabs(late));
      // A shorter conversion filter may alter bright attacks. Reject large
      // body/tail level changes while testing alias rejection separately.
      if(std::fabs(db)>3 || std::fabs(late)>3)return 1;
    }
    std::printf("PASS: 15 legacy comparisons; worst level %.3f dB, relative error %.2f dB, late level %.3f dB\n",worst_level,10*std::log10(worst_error),worst_late);
}
