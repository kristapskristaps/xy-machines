#include "../display.cpp"
#include <cstdio>
#include <cstring>
#include <cstdlib>
using namespace opxy::draw;
static void require(bool ok,const char* message){if(!ok){std::puts(message);std::exit(1);}}
int main(){
 weave_chords::DisplayState s={};weave_chords::Frame f={};Input in={};in.elapsed=17;unsigned random=7139;
 for(unsigned n=0;n<3000;++n){
  for(unsigned i=0;i<4;++i)in.slots[i]=static_cast<opxy::i32>((random=random*1664525u+1013904223u)>>1);
  in.track=(n/100)%8;in.held_notes=n%5;in.note_serial=n/3;in.note_off_serial=n/7;in.elapsed=n%7?17:2147483647;
  weave_chords::display(s,in,f);require(valid(f),"FAIL invalid bounded display frame");
  for(unsigned path=0;path<3;++path)for(const auto& point:f.paths[path].points){
   const auto& node=f.nodes[path];
   const long long x=node.x+((static_cast<long long>(point.x)*node.cos-static_cast<long long>(point.y)*node.sin)/unit)*node.scale/unit;
   const long long y=node.y+((static_cast<long long>(point.x)*node.sin+static_cast<long long>(point.y)*node.cos)/unit)*node.scale/unit;
   require(x>=0 && x<=480LL*unit && y>=21LL*unit && y<=220LL*unit,"FAIL geometric outline overlaps header or leaves screen");
  }

  for(unsigned p=0;p<3;++p){long long area=0;for(unsigned i=0;i<32;++i){const auto& a=f.paths[p].points[i];const auto& b=f.paths[p].points[(i+1)%32];area+=static_cast<long long>(a.x)*b.y-static_cast<long long>(b.x)*a.y;}require(area!=0,"FAIL outline collapsed");}
 }
 in=Input();in.elapsed=17;in.track=0;weave_chords::display_reset(s);weave_chords::display(s,in,f);
 require(s.pulse==180,"FAIL fabricated note on initial snapshot");
 for(unsigned i=0;i<40;++i)weave_chords::display(s,in,f);
 auto prior=f;for(unsigned i=0;i<30;++i)weave_chords::display(s,in,f);require(std::memcmp(&prior,&f,sizeof f)==0,"FAIL idle motion");
 in.held_notes=1;++in.note_serial;weave_chords::display(s,in,f);prior=f;
 for(unsigned i=0;i<10;++i){weave_chords::display(s,in,f);}
 require(std::memcmp(&prior,&f,sizeof f)!=0,"FAIL no note response");
 in.reduced_motion=1;weave_chords::display(s,in,f);prior=f;
 for(unsigned i=0;i<20;++i){++in.note_serial;weave_chords::display(s,in,f);}require(std::memcmp(&prior,&f,sizeof f)==0,"FAIL reduced motion animates");
 in.slots[1]=2147483647;weave_chords::display(s,in,f);require(std::memcmp(&prior,&f,sizeof f)!=0,"FAIL reduced motion ignores controls");
 in.reduced_motion=0;in.track=1;++in.note_serial;weave_chords::display(s,in,f);require(s.pulse==180,"FAIL track switch fabricates note");

 // The default composition occupies the playable screen, rather than a small central icon.
 Input geometry={};geometry.elapsed=17;geometry.slots[0]=858993459;geometry.slots[1]=1073741823;geometry.slots[2]=858993459;
 weave_chords::DisplayState motion={};weave_chords::Frame drawing={};weave_chords::display(motion,geometry,drawing);
 long long minx=480LL*unit,maxx=0,miny=220LL*unit,maxy=0;
 for(unsigned p=0;p<3;++p)for(const auto& point:drawing.paths[p].points){
  const auto& node=drawing.nodes[p];
  const long long x=node.x+((static_cast<long long>(point.x)*node.cos-static_cast<long long>(point.y)*node.sin)/unit)*node.scale/unit;
  const long long y=node.y+((static_cast<long long>(point.x)*node.sin+static_cast<long long>(point.y)*node.cos)/unit)*node.scale/unit;
  if(x<minx){minx=x;}if(x>maxx){maxx=x;}if(y<miny){miny=y;}if(y>maxy){maxy=y;}
 }
 for(unsigned i=3;i<24;++i){const auto& node=drawing.nodes[i];if(node.x==hidden_x*unit)continue;if(node.x<minx)minx=node.x;if(node.x>maxx)maxx=node.x;}
 require(maxx-minx>=320LL*unit && maxy-miny>=120LL*unit,"FAIL composition is too small");
 geometry.held_notes=1;++geometry.note_serial;
 for(unsigned tick=0;tick<220;++tick){auto before=drawing;weave_chords::display(motion,geometry,drawing);if(tick>20)for(unsigned n=0;n<24;++n){require(std::abs(drawing.nodes[n].cos-before.nodes[n].cos)<120 && std::abs(drawing.nodes[n].sin-before.nodes[n].sin)<120,"FAIL phase-wrap rotation discontinuity");}}

 for(unsigned control=0;control<4;++control){
  Input low={};low.reduced_motion=1;low.elapsed=17;low.slots[0]=858993459;low.slots[1]=1073741823;low.slots[2]=858993459;
  weave_chords::DisplayState check={};weave_chords::Frame first={},last={};
  low.slots[control]=0;weave_chords::display(check,low,first);
  low.slots[control]=2147483647;weave_chords::display(check,low,last);
  require(std::memcmp(first.nodes,last.nodes,sizeof first.nodes)!=0 || std::memcmp(first.paths,last.paths,sizeof first.paths)!=0,"FAIL control changes only header, not artwork");
 }
 for(unsigned mask=0;mask<16;++mask){
  Input extreme={};extreme.elapsed=17;extreme.held_notes=1;
  for(unsigned control=0;control<4;++control)extreme.slots[control]=mask&(1u<<control)?2147483647:0;
  weave_chords::DisplayState check={};weave_chords::Frame frame={};
  for(unsigned tick=0;tick<220;++tick){weave_chords::display(check,extreme,frame);require(valid(frame),"FAIL invalid moving control corner");}
 }
 std::puts("PASS: bounded vector frames, noncollapsed outlines, note response, idle settling, reduced motion and track changes");
}
