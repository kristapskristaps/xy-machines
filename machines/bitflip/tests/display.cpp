#include "../display.cpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

void require(bool value,const char* message){if(!value){std::fprintf(stderr,"FAIL: %s\n",message);std::exit(1);}}
unsigned pose(const bitflip::Frame& frame){
    unsigned active=6,count=0;
    for(unsigned i=0;i<6;++i)for(unsigned layer=0;layer<3;++layer)
        if(frame.nodes[bitflip::scene::limbs+i*3+layer].x!=640*1024){active=i;++count;}
    require(count==3,"exactly one complete runner cel is visible");return active;
}
int main(){
    static_assert(sizeof(bitflip::DisplayState)==56,"declared display storage");
    bitflip::DisplayState state={};bitflip::Frame frame={};opxy::draw::Input input={};
    input.elapsed=17;input.slots[1]=input.slots[6]=input.slots[7]=2147483647;input.slots[3]=1073741824;
    input.slots[5]=858993459;
    for(unsigned i=0;i<4;++i)input.shown[i]=static_cast<opxy::u8>(i);
    bitflip::display_reset(state);bitflip::display(state,input,frame);
    require(opxy::draw::valid(frame),"valid initial display");
    require(state.jump_age==512&&state.travel==0&&pose(frame)==0,"activation establishes a stationary note baseline");
    require(!std::strcmp(frame.header[0],"PLS")&&!std::strcmp(frame.header[3],"+0"),"main header has model and neutral sweep");
    const auto idle=frame;for(unsigned i=0;i<100;++i)bitflip::display(state,input,frame);
    require(!std::memcmp(&idle,&frame,sizeof(frame)),"idle world stays still");
    input.held_notes=1;
    unsigned poses=0;const int cloud=frame.nodes[bitflip::scene::clouds].x;
    for(unsigned i=0;i<60;++i){bitflip::display(state,input,frame);poses|=1u<<pose(frame);}
    require(state.travel>0&&cloud!=frame.nodes[bitflip::scene::clouds].x,"held notes scroll the world and parallax clouds");
    require((poses&30)==30,"all four running cels animate");
    ++input.note_serial;bitflip::display(state,input,frame);
    require(state.jump_age==0,"a new note starts a hop");
    for(unsigned i=0;i<8;++i)bitflip::display(state,input,frame);
    require(frame.nodes[bitflip::scene::body].y<184*1024&&pose(frame)==5,"notes visibly lift the runner into a jump cel");
    bool collected=false,wrapped=false;unsigned previous=state.travel;
    for(unsigned i=0;i<4000;++i){
        const unsigned coins=state.coins;bitflip::display(state,input,frame);
        collected=collected||state.coins!=coins;wrapped=wrapped||state.travel<previous;previous=state.travel;
        require(opxy::draw::valid(frame),"long running world remains bounded");pose(frame);
        require(state.collected<64&&state.coins<100,"coin mask and score stay bounded");
    }
    require(collected&&wrapped,"runner collects coins and loops through the complete world");
    input.held_notes=0;
    for(unsigned i=0;i<40;++i)bitflip::display(state,input,frame);
    require(state.activity==0&&state.jump_age==512&&pose(frame)==0,"release settles to a standing pose");
    const auto stopped=frame;for(unsigned i=0;i<100;++i)bitflip::display(state,input,frame);
    require(!std::memcmp(&stopped,&frame,sizeof(frame)),"released scene rests without an idle clock");
    input.held_notes=1;input.reduced_motion=1;bitflip::display(state,input,frame);
    const auto reduced=frame;const unsigned frozen=state.travel;
    for(unsigned i=0;i<100;++i){++input.note_serial;bitflip::display(state,input,frame);}
    require(state.travel==frozen&&pose(frame)==0&&!std::memcmp(&reduced,&frame,sizeof(frame)),"reduced motion freezes scenery and poses while preserving held-note highlight");
    require(frame.nodes[bitflip::scene::body].shade==6,"reduced motion retains static note feedback");
    input.reduced_motion=0;++input.track;++input.note_serial;bitflip::display(state,input,frame);
    require(state.jump_age==512&&state.travel==0&&state.coins==0,"track changes establish a fresh world baseline");
    for(unsigned i=0;i<4;++i)input.shown[i]=static_cast<opxy::u8>(i+4);
    input.reduced_motion=1;bitflip::display(state,input,frame);
    require(!std::strcmp(frame.header[2],"16")&&!std::strcmp(frame.header[3],"44.1k"),"DAC captions have real bit depth and sample rate");
    for(unsigned i=0;i<4;++i)input.shown[i]=static_cast<opxy::u8>(i);
    input.slots[3]=2147483647;bitflip::display(state,input,frame);
    require(!std::strcmp(frame.header[3],"+24"),"full Sweep displays its actual range");
    unsigned random=7139;
    for(unsigned tick=0;tick<4096;++tick){
        for(auto& value:input.slots)value=static_cast<int>(random=random*1664525u+1013904223u);
        for(unsigned col=0;col<4;++col)input.shown[col]=static_cast<opxy::u8>(col+(tick%2)*4);
        input.elapsed=tick%17==0?-100:static_cast<int>(tick%1024);
        input.reduced_motion=tick%23==0;input.held_notes=tick%19==0?0xffffffffu:tick%4;
        if(tick%7==0)++input.note_serial;
        bitflip::display(state,input,frame);require(opxy::draw::valid(frame),"all control/page/note/late-frame combinations remain bounded");pose(frame);
    }
    std::puts("PASS: scrolling terrain, parallax, four running cels, jumps, coins, complete world wrap, release, reduced motion, track changes, real-unit pages and 4,096 stress frames");
}
