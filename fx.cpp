// Halo: a PunchKit-derived plate, bloom, echo and shimmer network.
// Copyright (c) 2026 Kristaps Dreija. See LICENSE and PROVENANCE.md.
#include "core.h"
#include "tables.h"
#include <opxy/q31.hpp>

namespace halo {
namespace {
using opxy::i32;using opxy::u32;
const i32 unit=16777216;
struct Line {unsigned offset,length;};
// Four input diffusers per side, then two allpasses and two delays per side.
const Line lines[19]={
    {0,128},{128,128},{256,256},{512,256},
    {768,128},{896,128},{1024,256},{1280,256},
    {1536,1024},{2560,8192},{10752,2048},{12800,4096},
    {16896,1024},{17920,4096},{22016,4096},{26112,4096},
    {30208,8192},{38400,8192},{46592,67584}
};
const unsigned fir_offset=114176;
static_assert(fir_offset+124<=buffer_words,"Halo scratch layout");
const unsigned diffusion_delay[8]={68,104,161,245,73,111,173,251};
const unsigned plate_delay[2][4]={{498,3299,1334,2756},{673,3124,1968,2344}};
inline i32 clamp(i32 x,i32 lo,i32 hi){return x<lo?lo:x>hi?hi:x;}
inline i32 mul(i32 x,i32 gain){return opxy::q31::multiply(x,gain*65536);}
inline i32 parameter(unsigned index,i32 word) {
    if(index==6)return word>=1073741824?32767:0;
    return word<0?0:word>>16;
}
inline i32 glide(i32 x,i32 target,unsigned divisor) {
    const i32 difference=target-x,step=difference/static_cast<i32>(divisor);
    return x+(step?step:(difference>0)-(difference<0));
}
inline i32 soft(i32 x) {
    const bool negative=x<0;
    const u32 magnitude=static_cast<u32>(negative?-x:x);
    if(magnitude>=4u*unit)return negative?-tables::soft[1024]:tables::soft[1024];
    const unsigned at=magnitude>>16;
    const i32 a=tables::soft[at],b=tables::soft[at+1];
    const i32 value=a+mul(b-a,static_cast<i32>((magnitude&65535u)>>1));
    return negative?-value:value;
}
inline i32 table(const i32* values,u32 phase) {
    const unsigned at=(phase>>8)&255u;
    return values[at]+(values[at+1]-values[at])*static_cast<i32>(phase&255u)/256;
}
inline i32 line_sample(const State& state,const i32* memory,unsigned line,unsigned age,unsigned channel=0) {
    if(!age || age>state.written)return 0;
    const Line& l=lines[line];
    const bool stereo=line==16 || line==17;
    if(age>=(stereo?l.length/2:l.length))return 0;
    if(stereo)age*=2;
    unsigned index=state.positions[line];
    index=index>=age?index-age:index+l.length-age;
    if(stereo){index+=channel;if(index>=l.length)index-=l.length;}
    const i32 packed=memory[l.offset+index];
    if(line==18) return static_cast<opxy::i16>(channel?static_cast<u32>(packed)>>16:static_cast<u32>(packed)&65535u)*512;
    return packed;
}
inline i32 tap(const State& state,const i32* memory,unsigned line,i32 delay,unsigned channel=0) {
    const unsigned age=static_cast<u32>(delay)>>8;
    const i32 a=line_sample(state,memory,line,age,channel),b=line_sample(state,memory,line,age+1,channel);
    return a+mul(b-a,(delay&255)*128);
}
inline void write(State& state,i32* memory,unsigned line,i32 value) {
    memory[lines[line].offset+state.positions[line]]=value;
    if(++state.positions[line]==lines[line].length)state.positions[line]=0;
}
inline i32 allpass(State& state,i32* memory,unsigned line,i32 input,i32 delay,i32 gain) {
    const i32 delayed=tap(state,memory,line,delay);
    const i32 output=delayed-mul(input,gain);
    write(state,memory,line,clamp(input+mul(output,gain),-3*unit,3*unit));
    return output;
}
inline i32 sized_tap(const State& state,const i32* memory,unsigned line,unsigned base,i32 size,i32 modulation) {
    // A single fractional tap preserves loop energy at every room size.
    const i32 scale=18022+mul(size,22938);  // 0.55 .. 1.25, Q15
    const i32 delay=static_cast<i32>(base)*scale/128;
    return tap(state,memory,line,delay+modulation);
}
inline i32 plate_allpass(State& state,i32* memory,unsigned line,i32 input,unsigned base,i32 size,i32 modulation,i32 gain) {
    const i32 delayed=sized_tap(state,memory,line,base,size,modulation);
    const i32 output=delayed-mul(input,gain);
    write(state,memory,line,clamp(input+mul(output,gain),-3*unit,3*unit));
    return output;
}
i32 echo_delay(i32 control,u32 bpm) {
    if(!bpm)bpm=1200;
    if(bpm<400)bpm=400;
    if(bpm>3000)bpm=3000;
    const u32 length=opxy::division_sixteenths[opxy::division(control)]*826875u;
    const u32 delay=((length/bpm)<<8)+((length%bpm)<<8)/bpm;
    const u32 maximum=(echo_frames-3)*256u;
    return static_cast<i32>(delay>maximum?maximum:delay<512u?512u:delay);
}
inline i32 pitch_up(const State& state,const i32* memory,unsigned channel,unsigned width) {
    const u32 phase=state.grain_phase>>8,other_phase=(phase+32768u)&65535u;
    const i32 a_delay=static_cast<i32>(1102u*256u+((65535u-phase)*width>>8));
    const i32 b_delay=static_cast<i32>(1102u*256u+((65535u-other_phase)*width>>8));
    const i32 a=tap(state,memory,17,a_delay+static_cast<i32>(channel*4096u),channel);
    const i32 b=tap(state,memory,17,b_delay+static_cast<i32>(channel*4096u),channel);
    return mul(a,table(tables::window,phase))+mul(b,table(tables::window,other_phase));
}
void network(State& state,i32* memory,i32* input,i32* output) {
    for(unsigned i=0;i<7;++i)state.controls[i]=clamp(state.controls[i],0,32767);
    const i32 distance=state.controls[0],bloom=state.controls[1],echo=state.controls[2];
    const i32 shimmer=state.controls[3],color=state.controls[4],hold=state.controls[6];
    const i32 color_coefficient=table(tables::tone,static_cast<u32>(color)*2u);
    const i32 input_gain=32767-hold;
    const i32 decay=22938+mul(bloom,9437);
    const i32 held_decay=decay+mul(32763-decay,hold);
    const i32 damping=5500+mul(color,19000);
    const i32 held_damping=damping+mul(32735-damping,hold);
    i32 echoes[2],predelay[2],shifted[2];
    const unsigned pre_delay=44u+(static_cast<unsigned>(distance)*2160u>>15);
    const unsigned grain_width=772u+(static_cast<unsigned>(distance)*1543u>>15);
    for(unsigned channel=0;channel<2;++channel) {
        state.input_high[channel]+=mul(input[channel]-state.input_high[channel],1118);
        const i32 high=input[channel]-state.input_high[channel];
        state.input_low[channel]+=mul(high-state.input_low[channel],color_coefficient);
        input[channel]=mul(state.input_low[channel],input_gain);
        echoes[channel]=tap(state,memory,18,state.delay,channel);
        state.echo_low[channel]+=mul(echoes[channel]-state.echo_low[channel],color_coefficient/2+1200);
        state.echo_high[channel]+=mul(state.echo_low[channel]-state.echo_high[channel],800);
        echoes[channel]=state.echo_low[channel]-state.echo_high[channel];
        predelay[channel]=tap(state,memory,16,static_cast<i32>(pre_delay*256u+channel*4096u),channel);
        const i32 previous=state.plate[channel];
        state.shimmer_high[channel]+=mul(previous-state.shimmer_high[channel],2600);
        i32 bright=previous-state.shimmer_high[channel];
        for(unsigned stage=0;stage<2;++stage){state.shimmer_low[channel][stage]+=mul(bright-state.shimmer_low[channel][stage],19600);bright=state.shimmer_low[channel][stage];}
        shifted[channel]=bright;
    }
    // Two overlapped, Hann-windowed heads read at twice the recording speed.
    for(unsigned channel=0;channel<2;++channel)output[channel]=pitch_up(state,memory,channel,grain_width);
    // Stereo rings use adjacent channel words; positions count words here.
    write(state,memory,16,input[0]);write(state,memory,16,input[1]);
    write(state,memory,17,shifted[0]);write(state,memory,17,shifted[1]);
    i32 next_plate[2],wet[2];
    for(unsigned channel=0;channel<2;++channel) {
        const unsigned other=1-channel;
        const i32 seeded=mul(predelay[channel],23593)+mul(input[channel],3277)+mul(echoes[channel],mul(echo,21000));
        i32 diffused=seeded;
        for(unsigned stage=0;stage<4;++stage)
            diffused=allpass(state,memory,channel*4+stage,diffused,static_cast<i32>(diffusion_delay[channel*4+stage]*256u),20316+mul(distance,4259));
        const i32 shimmer_gain=mul(mul(shimmer,shimmer),32767-hold);
        const i32 tank=soft(mul(diffused,23593)+mul(state.plate[other],held_decay)+mul(output[channel],mul(shimmer_gain,16384)));
        state.phase[channel]+=channel?24737u:22011u;
        const i32 sine=table(tables::sine,state.phase[channel]>>16);
        const i32 modulation=mul(sine,4000+mul(distance,18000))/4;
        const unsigned first=8+channel*4;
        const i32 ap1=plate_allpass(state,memory,first,tank,plate_delay[channel][0],distance,modulation,22282);
        const i32 d1=sized_tap(state,memory,first+1,plate_delay[channel][1],distance,0);
        write(state,memory,first+1,ap1);
        state.damping[channel]+=mul(d1-state.damping[channel],held_damping);
        const i32 ap2=plate_allpass(state,memory,first+2,mul(state.damping[channel],held_decay),plate_delay[channel][2],distance,-modulation/4,16384);
        const i32 d2=sized_tap(state,memory,first+3,plate_delay[channel][3],distance,0);
        write(state,memory,first+3,ap2);
        next_plate[channel]=clamp(d2,-unit,unit);
        const i32 tail=mul(ap1,4260)+mul(d1,10158)+mul(d2,22282)+mul(state.plate[other],5898);
        const i32 magnitude=input[channel]<0?-input[channel]:input[channel];
        state.body[channel]+=mul(magnitude-state.body[channel],38);
        const i32 transient=clamp((magnitude-state.body[channel]-state.body[channel]/3)*4,0,unit);
        const i32 follower=transient>state.duck[channel]?2800:12+mul(32767-bloom,35);
        state.duck[channel]+=mul(transient-state.duck[channel],follower);
        const i32 duck_gain=32767-mul(state.duck[channel]/512,mul(bloom,24576));
        const i32 open=mul(tail,duck_gain);
        wet[channel]=soft(mul(open,24000)+mul(echoes[channel],mul(echo,20000)));
    }
    const i32 feedback=mul(11000+mul(bloom,14000),echo);
    opxy::u32 packed=0;
    for(unsigned channel=0;channel<2;++channel) {
        const i32 echo_write=soft(input[channel]+mul(echoes[1-channel],feedback));
        const i32 shaped=echo_write+state.echo_error[channel];
        const i32 quantized=clamp((shaped+256)>>9,-32768,32767);
        state.echo_error[channel]=shaped-quantized*512;
        packed|=static_cast<u32>(static_cast<opxy::u16>(static_cast<opxy::i16>(quantized)))<<(channel*16);
        state.plate[channel]=next_plate[channel];output[channel]=wet[channel];
    }
    write(state,memory,18,static_cast<i32>(packed));
    if(state.written<echo_frames)++state.written;
}

// FIR histories are the only scratch that needs an explicit bounded reset.
inline i32 downsample(const i32* history,unsigned position) {
    unsigned at=position>=15?position-15:position+16;
    i32 output=history[at]/2;
    for(unsigned tap_index=0;tap_index<31;tap_index+=2) {
        at=position>=tap_index?position-tap_index:position+31-tap_index;
        output+=opxy::q31::multiply(history[at],tables::halfband[tap_index]);
    }
    return output;
}
inline i32 upsample(const i32* history,unsigned position,unsigned phase) {
    if(phase) return history[position>=7?position-7:position+9];
    i32 output=0;
    for(unsigned i=0;i<16;++i) {
        const unsigned at=position>=i?position-i:position+16-i;
        output+=opxy::q31::multiply(history[at],tables::halfband[i*2]*2);
    }
    return output;
}
}
void initialize(State& state){state=State();}
void process(State& state,opxy::Buffer buffer,const opxy::Transport& transport,
             opxy::Channel left,opxy::Channel right,opxy::Parameters parameters) {
    if(!buffer.words || buffer.count<buffer_words){for(unsigned i=0;i<opxy::block_frames;++i)left[i]=right[i]=0;return;}
    i32* memory=buffer.words;
    if(!state.ready) {
        for(unsigned i=0;i<124;++i)memory[fir_offset+i]=0;
        for(unsigned i=0;i<7;++i)state.controls[i]=parameter(i,parameters[i]);
        state.delay=echo_delay(parameters[5],transport.bpm_x10);state.ready=1;
    }
    const i32 target_delay=echo_delay(parameters[5],transport.bpm_x10);
    i32* down[2]={memory+fir_offset,memory+fir_offset+31};
    i32* up[2]={memory+fir_offset+62,memory+fir_offset+78};
    const unsigned grain_width=772u+(static_cast<unsigned>(state.controls[0])*1543u>>15);
    const u32 grain_step=16777216u/grain_width;
    for(unsigned i=0;i<opxy::block_frames;++i) {
        down[0][state.down_write]=left[i]/128;down[1][state.down_write]=right[i]/128;
        if(!state.half) {
            for(unsigned control=0;control<7;++control) {
                const i32 target=parameter(control,parameters[control]);
                state.controls[control]=glide(state.controls[control],target,control==6?256:512);
            }
            state.delay=glide(state.delay,target_delay,1024);
            i32 input[2]={downsample(down[0],state.down_write),downsample(down[1],state.down_write)},wet[2];
            network(state,memory,input,wet);
            state.grain_phase=(state.grain_phase+grain_step)&16777215u;
            if(++state.up_write==16)state.up_write=0;
            up[0][state.up_write]=wet[0];up[1][state.up_write]=wet[1];
        }
        left[i]=clamp(upsample(up[0],state.up_write,state.half),-unit+1,unit-1)*128;
        right[i]=clamp(upsample(up[1],state.up_write,state.half),-unit+1,unit-1)*128;
        state.half^=1;
        if(++state.down_write==31)state.down_write=0;
    }
}
}
