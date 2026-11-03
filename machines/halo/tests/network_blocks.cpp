#include "../fx.cpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
static void require(bool ok,const char* text){if(!ok){std::fprintf(stderr,"FAIL: %s\n",text);std::exit(1);}}
static opxy::i32 memory[halo::buffer_words],copy[halo::buffer_words];
template<unsigned LineIndex>static void readers(){
    using namespace halo;State start={};const auto line=lines[LineIndex];const unsigned stride=LineIndex==16||LineIndex==17?2:1;
    const unsigned positions[]={0,1,line.length-16,line.length-1};
    const unsigned counts[]={0,1,15,16,17,echo_frames-8,echo_frames,0xffffffffu};
    for(auto position:positions)for(auto count:counts){start.positions[LineIndex]=position;start.written=count;
        for(unsigned frame=0;frame<wet_frames;++frame){State sequential=start;for(unsigned i=0;i<frame;++i){sequential.positions[LineIndex]=(sequential.positions[LineIndex]+stride)%line.length;if(sequential.written<echo_frames)++sequential.written;}
            const unsigned ages[]={0,1,16,17,line.length/stride-1,line.length/stride};
            for(auto age:ages)for(unsigned fraction:{0u,1u,128u,255u})for(unsigned channel=0;channel<(stride==2||LineIndex==18?2u:1u);++channel){
                const auto delay=static_cast<opxy::i32>((age<<8)|fraction);
                require(old_tap<LineIndex>(start,memory,frame,delay,channel)==tap(sequential,memory,LineIndex,delay,channel),"prefetched tap matches independent sequential cursor/history at wraps and availability edges");
            }
        }
    }
}
int main(){
    using namespace halo;using opxy::i32;using opxy::u32;
    u32 random=7139;for(auto& value:memory)value=static_cast<i32>((random=random*1664525u+1013904223u)&33554431u)-unit;
    readers<0>();readers<1>();readers<2>();readers<3>();readers<4>();readers<5>();readers<6>();readers<7>();readers<8>();readers<9>();readers<10>();readers<11>();readers<12>();readers<13>();readers<14>();readers<15>();readers<16>();readers<17>();readers<18>();
    unsigned smallest_ap=1000000,smallest_echo=1000000;
    State domain={};
    for(i32 control=0;control<=32767;++control){
        domain.controls[0]=control;const auto config=settings(domain);
        const i32 maximum_modulation=(config.modulation/65536+4)/4;
        require(config.pre_delay>wet_frames,"all legal pre-delays exceed a wet block");
        for(unsigned channel=0;channel<2;++channel){
            const unsigned ap0=static_cast<u32>(config.delays[channel][0]-maximum_modulation)>>8;
            const unsigned ap1=static_cast<u32>(config.delays[channel][2]-maximum_modulation/4)>>8;
            if(ap0<smallest_ap)smallest_ap=ap0;if(ap1<smallest_ap)smallest_ap=ap1;
            require(ap0>wet_frames&&ap1>wet_frames&&(static_cast<u32>(config.delays[channel][1])>>8)>wet_frames&&(static_cast<u32>(config.delays[channel][3])>>8)>wet_frames,"all legal modulated/simple tank delays exceed a wet block");
        }
        const unsigned age=static_cast<u32>(echo_delay(control*65536,3000))>>8;if(age<smallest_echo)smallest_echo=age;
        require(age>wet_frames,"fastest legal tempo/division still exceeds a wet block");
    }
    for(auto age:diffusion_delay)require(age>wet_frames,"all input diffusers exceed a wet block");require(551>wet_frames,"both pitch-head minima exceed a wet block");
    i32 left[16],right[16],scratch[32],shifted[32],out_left[16],out_right[16];
    const unsigned histories[]={0,1,15,16,17,echo_frames-8,echo_frames,0xffffffffu};
    for(auto history:histories){
        State scalar={},blocked={};scalar.ready=1;scalar.written=history;scalar.delay=17*256;scalar.plate[0]=unit/4;scalar.plate[1]=-unit/4;scalar.phase[0]=0xfffffff0u;scalar.phase[1]=0xffffff00u;scalar.grain_phase=16777205u;
        for(unsigned i=0;i<19;++i)scalar.positions[i]=lines[i].length-8;
        for(auto& value:memory)value=static_cast<i32>((random=random*1664525u+1013904223u)&33554431u)-unit;
        std::memcpy(copy,memory,sizeof(copy));blocked=scalar;
        Settings config=settings(scalar);config.pre_delay=17;config.modulation=0;config.shimmer=3000*65536;
        for(auto& channel:config.delays)for(auto& delay:channel)delay=17*256;
        for(unsigned i=0;i<16;++i){left[i]=out_left[i]=static_cast<i32>(i*102151u)-unit/8;right[i]=out_right[i]=-left[i]/2;}
        const unsigned grain_step=16777216u/config.grain_width;
        for(unsigned i=0;i<16;++i){scalar.delay=glide(scalar.delay,17*256,512);i32 input[2]={left[i],right[i]},output[2];network(scalar,memory,input,output,config);scalar.grain_phase=(scalar.grain_phase+grain_step)&16777215u;left[i]=output[0];right[i]=output[1];}
        require(network_block(blocked,copy,out_left,out_right,scratch,shifted,config,17*256,grain_step),"age17 pipeline boundary is admitted");
        require(!std::memcmp(left,out_left,sizeof(left))&&!std::memcmp(right,out_right,sizeof(right))&&!std::memcmp(&scalar,&blocked,sizeof(scalar))&&!std::memcmp(memory,copy,sizeof(memory)),"age17 pipeline keeps complete sequential audio/state/scratch at wraps, counter saturation and dirty/unwritten history");
    }
    for(unsigned unsafe=0;unsafe<5;++unsafe){
        State state={};state.delay=200*256;Settings config=settings(state);config.modulation=0;config.shimmer=1;
        if(unsafe==0)state.delay=2*256;if(unsafe==1)config.pre_delay=16;if(unsafe==2)config.delays[0][0]=16*256;if(unsafe==3)config.delays[1][2]=16*256;if(unsafe==4)state.grain_phase=0x80000000u;
        const State original=state;std::memcpy(copy,memory,sizeof(copy));for(unsigned i=0;i<16;++i)left[i]=out_left[i]=right[i]=out_right[i]=static_cast<i32>(i*102151u);
        require(!network_block(state,memory,left,right,scratch,shifted,config,275*256,16777216u/config.grain_width),"unsafe delay domain selects the original scalar fallback");
        require(!std::memcmp(&state,&original,sizeof(state))&&!std::memcmp(memory,copy,sizeof(memory))&&!std::memcmp(left,out_left,sizeof(left))&&!std::memcmp(right,out_right,sizeof(right)),"rejected block leaves all scalar-fallback inputs untouched");
    }
    State fallback={},sequential={};fallback.delay=sequential.delay=2*256;
    Settings config=settings(fallback);config.modulation=0;const unsigned grain_step=16777216u/config.grain_width;
    std::memcpy(copy,memory,sizeof(copy));for(unsigned i=0;i<16;++i)left[i]=out_left[i]=right[i]=out_right[i]=static_cast<i32>(i*102151u);
    for(unsigned i=0;i<16;++i){sequential.delay=glide(sequential.delay,275*256,512);i32 in[2]={left[i],right[i]},wet[2];network(sequential,memory,in,wet,config);sequential.grain_phase=(sequential.grain_phase+grain_step)&16777215u;left[i]=wet[0];right[i]=wet[1];}
    scalar_network_block(fallback,copy,out_left,out_right,config,275*256,grain_step);
    require(!std::memcmp(&fallback,&sequential,sizeof(fallback))&&!std::memcmp(memory,copy,sizeof(memory))&&!std::memcmp(left,out_left,sizeof(left))&&!std::memcmp(right,out_right,sizeof(right)),"short-age scalar fallback retains exact frame-by-frame processing");
    std::printf("PASS:19-ring reader/written/wrap proofs, full age17 pipeline equivalence, minimum AP%u/echo%u, five no-mutation fallback guards\n",smallest_ap,smallest_echo);
}
