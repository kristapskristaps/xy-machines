#include "../display.cpp"
#include <cstdio>
#include <cstring>
#include <cstdlib>
using namespace opxy::draw;
static void require(bool ok,const char* message){if(!ok){std::puts(message);std::exit(1);}}
int main(){
 weave_analog::DisplayState s={};weave_analog::Frame f={};Input in={};in.elapsed=17;unsigned random=7139;
 for(unsigned n=0;n<3000;++n){
  for(unsigned i=0;i<4;++i)in.slots[i]=static_cast<opxy::i32>((random=random*1664525u+1013904223u)>>1);
  in.track=(n/100)%8;in.held_notes=n%5;in.note_serial=n/3;in.note_off_serial=n/7;in.elapsed=n%7?17:2147483647;
  weave_analog::display(s,in,f);require(valid(f),"FAIL invalid bounded display frame");
  for(unsigned p=0;p<3;++p){long long area=0;for(unsigned i=0;i<32;++i){const auto& a=f.paths[p].points[i];const auto& b=f.paths[p].points[(i+1)%32];area+=static_cast<long long>(a.x)*b.y-static_cast<long long>(b.x)*a.y;}require(area!=0,"FAIL outline collapsed");}
 }
 in=Input();in.elapsed=17;in.track=0;weave_analog::display_reset(s);weave_analog::display(s,in,f);
 require(s.pulse==180,"FAIL fabricated note on initial snapshot");
 for(unsigned i=0;i<40;++i)weave_analog::display(s,in,f);
 auto prior=f;for(unsigned i=0;i<30;++i)weave_analog::display(s,in,f);require(std::memcmp(&prior,&f,sizeof f)==0,"FAIL idle motion");
 in.held_notes=1;++in.note_serial;weave_analog::display(s,in,f);prior=f;
 for(unsigned i=0;i<10;++i){weave_analog::display(s,in,f);}
 require(std::memcmp(&prior,&f,sizeof f)!=0,"FAIL no note response");
 in.reduced_motion=1;weave_analog::display(s,in,f);prior=f;
 for(unsigned i=0;i<20;++i){++in.note_serial;weave_analog::display(s,in,f);}require(std::memcmp(&prior,&f,sizeof f)==0,"FAIL reduced motion animates");
 in.slots[1]=2147483647;weave_analog::display(s,in,f);require(std::memcmp(&prior,&f,sizeof f)!=0,"FAIL reduced motion ignores controls");
 in.reduced_motion=0;in.track=1;++in.note_serial;weave_analog::display(s,in,f);require(s.pulse==180,"FAIL track switch fabricates note");
 std::puts("PASS: bounded vector frames, noncollapsed outlines, note response, idle settling, reduced motion and track changes");
}
