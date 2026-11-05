#include "../engine.cpp"
#include "cases.h"
#include "weave_reference.h"
#include <cstdio>
#include <cstring>
int main(){
 using namespace crease_ref;
 for(unsigned scene=0;scene<split_cases::count;++scene){
  Voice v={};Shared shared;initialize(v,shared,scene+1);start(v,shared,scene+1);
  unsigned long long pcm=14695981039346656037ull,state=pcm;
  for(unsigned block=0;block<96;++block){
   opxy::i32 p[8]={},out[64];opxy::u32 pitch[64];split_cases::parameters(scene,block,p);
   for(unsigned i=0;i<64;++i)pitch[i]=split_cases::pitch(scene,block,i);
   if(scene==43 && block%11==0)start(v,shared,scene+1);
   render(v,shared,scene+1,out,p,pitch);
   for(auto sample:out){if(sample < -536870912 || sample > 536870912){std::puts("FAIL output peak");return 1;}pcm=split_cases::hash(pcm,static_cast<opxy::u32>(sample));}
  }
  const auto& model=v.oscillator.e.wsh;
  for(unsigned i=0;i<sizeof(model)/4;++i){opxy::u32 word;std::memcpy(&word,reinterpret_cast<const unsigned char*>(&model)+4*i,4);state=split_cases::hash(state,word);}
  if(pcm!=reference_pcm[scene] || state!=reference_state[scene]){std::printf("FAIL frozen Weave model scene%u: pcm%llu/%llu state%llu/%llu\n",scene,pcm,reference_pcm[scene],state,reference_state[scene]);return 1;}
 }
 std::puts("PASS: 45 frozen Weave PCM and selected-state fixtures; control sweeps, extremes, glide, retriggers and automation");
}
