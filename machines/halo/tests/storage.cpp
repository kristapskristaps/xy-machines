#include "../fx.cpp"
#define halo halo_storage_reference
#include "storage_reference/fx.cpp"
#undef halo
#include <cstdio>
#include <cstring>
static opxy::i32 ma[halo::buffer_words],mb[halo_storage_reference::buffer_words],la[64],ra[64],lb[64],rb[64];
int main(){
 const opxy::i32 presets[][8]={{858993459,1181116006,257698038,107374182,966367642,1460288880,0,2147483647},{1395864371,1567663063,1417339207,214748365,816043786,1460288880,0,2147483647},{1675037245,1825361100,601295421,1546188226,1331439861,1116691496,0,2147483647}};
 for(unsigned scenario=0;scenario<12;++scenario){
  halo::State a;halo::initialize(a);halo_storage_reference::State b;halo_storage_reference::initialize(b);
  std::memset(ma,0x7f,sizeof ma);std::memset(mb,0x3f,sizeof mb);
  for(unsigned line=0;line<19;++line)for(unsigned i=0;i<halo::lines[line].length;++i){
   const opxy::i32 dirty=static_cast<opxy::i32>((i*17239u+17u)*(line+1));
   ma[halo::lines[line].offset+i]=mb[halo_storage_reference::lines[line].offset+i]=dirty;
  }
  opxy::i32 p[8];std::memcpy(p,presets[scenario%3],sizeof p);opxy::u32 random=7139,phase=0;
  for(unsigned block=0;block<4096;++block){
   if(scenario>=3 && block%31==0)for(auto&v:p)v=static_cast<opxy::i32>((random=random*1664525u+1013904223u)>>1);
   if(scenario>=6 && block%127==0)for(unsigned c=0;c<8;++c)p[c]=(block/127+c)%2?2147483647:0;
   if(scenario>=9)p[6]=block>=512 && block<3072?2147483647:0;
   if(scenario%2 && block==2048){halo::initialize(a);halo_storage_reference::initialize(b);}
   for(unsigned i=0;i<64;++i){phase+=23192823u;la[i]=lb[i]=block<256||scenario>=3?static_cast<opxy::i32>(phase)/4:0;ra[i]=rb[i]=block%113==0&&i==0?536870912:-la[i]/2;}
   const opxy::Transport t={scenario>=3?(block%3==0?0u:block%3==1?400u:3000u):1200u,0,true};
   halo::process(a,{ma,halo::buffer_words},t,la,ra,p);halo_storage_reference::process(b,{mb,halo_storage_reference::buffer_words},t,lb,rb,p);
   if(std::memcmp(la,lb,sizeof la)||std::memcmp(ra,rb,sizeof ra)||std::memcmp(&a,&b,sizeof a)){std::printf("FAIL storage scenario%u block%u\n",scenario,block);return 1;}
  }
  for(unsigned line=0;line<19;++line)if(std::memcmp(ma+halo::lines[line].offset,mb+halo_storage_reference::lines[line].offset,halo::lines[line].length*sizeof(opxy::i32)))return 1;
  if(std::memcmp(ma+halo::fir_offset,mb+halo_storage_reference::fir_offset,halo::fir_words*sizeof(opxy::i32)))return 1;
 }
 for(unsigned line=1;line<19;++line)if(halo::lines[line].offset-halo::lines[line-1].offset-halo::lines[line-1].length!=16)return 1;
 if(halo::fir_offset+halo::fir_words>halo::buffer_words)return 1;
 std::puts("PASS: padded rings preserve every PCM/state block and complete logical delay/FIR storage across12longscenarios");
}
