// Halo: a PunchKit-derived plate, bloom, echo and shimmer network.
// Copyright (c) 2026 Kristaps Dreija. See LICENSE and PROVENANCE.md.
#include "core.h"
#include "tables.h"
#include <opxy/q31.hpp>

namespace halo_plate {
namespace {
using opxy::i32;using opxy::u32;
const i32 unit=16777216;
struct Line {unsigned offset,length;};
// Four input diffusers per side, then two allpasses and two delays per side.
const Line lines[19]={
    {0,64},{64,64},{128,128},{256,128},
    {384,64},{448,64},{512,128},{640,128},
    {768,512},{1280,4096},{5376,1024},{6400,2048},
    {8448,512},{8960,2048},{11008,2048},{13056,2048},
    {15104,4096},{19200,4096},{23296,33792}
};
const unsigned fir_offset=57088;
const unsigned fir_words=376;
static_assert(fir_offset+fir_words<=buffer_words,"Halo scratch layout");
const unsigned diffusion_delay[8]={34,52,81,123,37,56,87,126};
const unsigned plate_delay[2][4]={{249,1650,667,1378},{337,1562,984,1172}};
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
    const unsigned position=state.positions[line];
    if(line==18) {
        const unsigned index=position>=age?position-age:position+l.length-age;
        const u32 packed=static_cast<u32>(memory[l.offset+index]);
        return static_cast<opxy::i16>(channel?packed>>16:packed&65535u)*512;
    }
    // All unpacked lines have power-of-two lengths, including stereo rings.
    const unsigned index=(position-age+(stereo?channel:0))&(l.length-1);
    return memory[l.offset+index];
}
inline i32 tap(const State& state,const i32* memory,unsigned line,i32 delay,unsigned channel=0) {
    const unsigned age=static_cast<u32>(delay)>>8;
    const Line& l=lines[line];
    const bool stereo=line==16 || line==17;
    const unsigned limit=(stereo?l.length/2:l.length)-1;
    const unsigned available=state.written<limit?state.written:limit;
    if(age>available)return 0;
    const unsigned position=state.positions[line];
    i32 a,b;
    if(line==18) {
        const unsigned index=position>=age?position-age:position+l.length-age;
        const u32 first=age?static_cast<u32>(memory[l.offset+index]):0;
        a=static_cast<opxy::i16>(channel?first>>16:first&65535u)*512;
        if(!(delay&255))return a;
        const unsigned previous=index?index-1:l.length-1;
        const u32 second=age<available?static_cast<u32>(memory[l.offset+previous]):0;
        b=static_cast<opxy::i16>(channel?second>>16:second&65535u)*512;
    } else {
        const unsigned stride=stereo?2:1;
        const unsigned index=(position-age*stride+(stereo?channel:0))&(l.length-1);
        a=age?memory[l.offset+index]:0;
        if(!(delay&255))return a;
        b=age<available?memory[l.offset+((index-stride)&(l.length-1))]:0;
    }
    return a+mul(b-a,(delay&255)*128);
}
inline void write(State& state,i32* memory,unsigned line,i32 value) {
    memory[lines[line].offset+state.positions[line]]=value;
    if(line==18){if(++state.positions[line]==lines[line].length)state.positions[line]=0;}
    else state.positions[line]=(state.positions[line]+1)&(lines[line].length-1);
}
inline i32 allpass(State& state,i32* memory,unsigned line,i32 input,i32 delay,i32 gain) {
    const i32 delayed=line_sample(state,memory,line,static_cast<u32>(delay)>>8);
    const i32 output=delayed-opxy::q31::multiply(input,gain);
    write(state,memory,line,clamp(input+opxy::q31::multiply(output,gain),-3*unit,3*unit));
    return output;
}
inline i32 plate_allpass(State& state,i32* memory,unsigned line,i32 input,i32 delay,i32 modulation,i32 gain) {
    const i32 delayed=tap(state,memory,line,delay+modulation);
    const i32 output=delayed-mul(input,gain);
    write(state,memory,line,clamp(input+mul(output,gain),-3*unit,3*unit));
    return output;
}
i32 echo_delay(i32 control,u32 bpm) {
    if(!bpm)bpm=1200;
    if(bpm<400)bpm=400;
    if(bpm>3000)bpm=3000;
    const u32 length=opxy::division_sixteenths[opxy::division(control)]*network_rate*600u;
    const u32 denominator=bpm*16u;
    const u32 delay=((length/denominator)<<8)+((length%denominator)<<8)/denominator;
    const u32 maximum=(echo_frames-3)*256u;
    return static_cast<i32>(delay>maximum?maximum:delay<512u?512u:delay);
}
inline i32 pitch_up(const State& state,const i32* memory,unsigned channel,unsigned width) {
    const u32 phase=state.grain_phase>>8,other_phase=(phase+32768u)&65535u;
    const i32 a_delay=static_cast<i32>(551u*256u+((65535u-phase)*width>>8));
    const i32 b_delay=static_cast<i32>(551u*256u+((65535u-other_phase)*width>>8));
    const i32 a=tap(state,memory,17,a_delay+static_cast<i32>(channel*2048u),channel);
    const i32 b=tap(state,memory,17,b_delay+static_cast<i32>(channel*2048u),channel);
    return mul(a,table(tables::window,phase))+mul(b,table(tables::window,other_phase));
}
struct Settings {
    // Network gains are Q31; control shaping below retains its Q15 rounding.
    i32 color,echo_color,input_gain,decay,damping,diffusion,shimmer,seed_echo,wet_echo,feedback,duck,release,modulation;
    i32 delays[2][4];
    unsigned pre_delay,grain_width;
};
Settings settings(const State& state) {
    Settings result={};
    const i32 distance=state.controls[0],bloom=state.controls[1],echo=state.controls[2];
    const i32 shimmer=state.controls[3],color=state.controls[4],hold=state.controls[6];
    result.color=table(tables::tone,static_cast<u32>(color)*2u);
    result.input_gain=32767-hold;
    const i32 decay=22938+mul(bloom,9437);
    result.decay=decay+mul(32763-decay,hold);
    const i32 damping=5500+mul(color,19000);
    result.damping=damping+mul(32735-damping,hold);
    result.damping=clamp(2*result.damping-mul(result.damping,result.damping),0,32767);
    result.diffusion=20316+mul(distance,4259);
    result.shimmer=mul(mul(mul(shimmer,shimmer),32767-hold),16384);
    result.seed_echo=mul(echo,21000);result.wet_echo=mul(echo,20000);
    result.feedback=mul(11000+mul(bloom,14000),echo);
    result.duck=mul(bloom,24576);result.release=24+mul(32767-bloom,70);
    result.modulation=2000+mul(distance,9000);
    result.pre_delay=22u+(static_cast<unsigned>(distance)*1080u>>15);
    result.grain_width=386u+(static_cast<unsigned>(distance)*771u>>15);
    const i32 scale=18022+mul(distance,22938);
    for(unsigned channel=0;channel<2;++channel)for(unsigned line=0;line<4;++line)
        result.delays[channel][line]=static_cast<i32>(plate_delay[channel][line])*scale/128;
    result.echo_color=(result.color/2+2400)*65536;
    result.color*=65536;result.input_gain*=65536;result.decay*=65536;
    result.damping*=65536;result.diffusion*=65536;result.shimmer*=65536;
    result.seed_echo*=65536;result.wet_echo*=65536;result.feedback*=65536;
    result.duck*=65536;result.release*=65536;result.modulation*=65536;
    return result;
}
template<unsigned channel>
__attribute__((always_inline)) inline void plate(State& state,i32* memory,const i32* input,const i32* output,
        const i32* echoes,const i32* predelay,i32* next_plate,i32* wet,const Settings& config) {
        const unsigned other=1-channel;
        const i32 seeded=mul(predelay[channel],23593)+mul(input[channel],3277)+opxy::q31::multiply(echoes[channel],config.seed_echo);
        i32 diffused=seeded;
        diffused=allpass(state,memory,channel*4+0,diffused,static_cast<i32>(diffusion_delay[channel*4+0]*256u),config.diffusion);
        diffused=allpass(state,memory,channel*4+1,diffused,static_cast<i32>(diffusion_delay[channel*4+1]*256u),config.diffusion);
        diffused=allpass(state,memory,channel*4+2,diffused,static_cast<i32>(diffusion_delay[channel*4+2]*256u),config.diffusion);
        diffused=allpass(state,memory,channel*4+3,diffused,static_cast<i32>(diffusion_delay[channel*4+3]*256u),config.diffusion);
        const i32 tank=soft(mul(diffused,23593)+opxy::q31::multiply(state.plate[other],config.decay)+opxy::q31::multiply(output[channel],config.shimmer));
        state.phase[channel]+=channel?49474u:44022u;
        const i32 sine=table(tables::sine,state.phase[channel]>>16);
        const i32 modulation=opxy::q31::multiply(sine,config.modulation)/4;
        const unsigned first=8+channel*4;
        const i32 ap1=plate_allpass(state,memory,first,tank,config.delays[channel][0],modulation,22282);
        const i32 d1=tap(state,memory,first+1,config.delays[channel][1]);
        write(state,memory,first+1,ap1);
        state.damping[channel]+=opxy::q31::multiply(d1-state.damping[channel],config.damping);
        const i32 ap2=plate_allpass(state,memory,first+2,opxy::q31::multiply(state.damping[channel],config.decay),config.delays[channel][2],-modulation/4,16384);
        const i32 d2=tap(state,memory,first+3,config.delays[channel][3]);
        write(state,memory,first+3,ap2);
        next_plate[channel]=clamp(d2,-unit,unit);
        const i32 tail=mul(ap1,4260)+mul(d1,10158)+mul(d2,22282)+mul(state.plate[other],5898);
        const i32 magnitude=input[channel]<0?-input[channel]:input[channel];
        state.body[channel]+=mul(magnitude-state.body[channel],76);
        const i32 transient=clamp((magnitude-state.body[channel]-state.body[channel]/3)*4,0,unit);
        const i32 follower=transient>state.duck[channel]?5361*65536:config.release;
        state.duck[channel]+=opxy::q31::multiply(transient-state.duck[channel],follower);
        const i32 duck_gain=32767-opxy::q31::multiply(state.duck[channel]/512,config.duck);
        const i32 open=mul(tail,duck_gain);
        wet[channel]=soft(mul(open,24000)+opxy::q31::multiply(echoes[channel],config.wet_echo));
}
void network(State& __restrict state,i32* __restrict memory,i32* __restrict input,i32* __restrict output,const Settings& __restrict config) {
    i32 echoes[2],predelay[2],shifted[2];
    for(unsigned channel=0;channel<2;++channel) {
        state.input_high[channel]+=mul(input[channel]-state.input_high[channel],2198);
        const i32 high=input[channel]-state.input_high[channel];
        state.input_low[channel]+=opxy::q31::multiply(high-state.input_low[channel],config.color);
        input[channel]=opxy::q31::multiply(state.input_low[channel],config.input_gain);
        echoes[channel]=tap(state,memory,18,state.delay,channel);
        state.echo_low[channel]+=opxy::q31::multiply(echoes[channel]-state.echo_low[channel],config.echo_color);
        state.echo_high[channel]+=mul(state.echo_low[channel]-state.echo_high[channel],1580);
        echoes[channel]=state.echo_low[channel]-state.echo_high[channel];
        predelay[channel]=line_sample(state,memory,16,config.pre_delay+channel*8u,channel);
        const i32 previous=state.plate[channel];
        state.shimmer_high[channel]+=mul(previous-state.shimmer_high[channel],4994);
        i32 bright=previous-state.shimmer_high[channel];
        for(unsigned stage=0;stage<2;++stage){state.shimmer_low[channel][stage]+=mul(bright-state.shimmer_low[channel][stage],27476);bright=state.shimmer_low[channel][stage];}
        shifted[channel]=bright;
    }
    // Two overlapped, Hann-windowed heads read at twice the recording speed.
    for(unsigned channel=0;channel<2;++channel)output[channel]=config.shimmer?pitch_up(state,memory,channel,config.grain_width):0;
    // Stereo rings use adjacent channel words; positions count words here.
    write(state,memory,16,input[0]);write(state,memory,16,input[1]);
    write(state,memory,17,shifted[0]);write(state,memory,17,shifted[1]);
    i32 next_plate[2],wet[2];
    plate<0>(state,memory,input,output,echoes,predelay,next_plate,wet,config);
    plate<1>(state,memory,input,output,echoes,predelay,next_plate,wet,config);
    opxy::u32 packed=0;
    for(unsigned channel=0;channel<2;++channel) {
        const i32 echo_write=soft(input[channel]+opxy::q31::multiply(echoes[1-channel],config.feedback));
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
    const i32* latest=history+position+31;
    i32 output=latest[-15]/2;
    // Only seven mirrored pairs are nonzero. Keep each product's rounding
    // and the original accumulation order while removing the tap loop.
    output+=opxy::q31::multiply(latest[-2]+latest[-28],tables::halfband[2]);
    output+=opxy::q31::multiply(latest[-4]+latest[-26],tables::halfband[4]);
    output+=opxy::q31::multiply(latest[-6]+latest[-24],tables::halfband[6]);
    output+=opxy::q31::multiply(latest[-8]+latest[-22],tables::halfband[8]);
    output+=opxy::q31::multiply(latest[-10]+latest[-20],tables::halfband[10]);
    output+=opxy::q31::multiply(latest[-12]+latest[-18],tables::halfband[12]);
    output+=opxy::q31::multiply(latest[-14]+latest[-16],tables::halfband[14]);
    return output;
}
inline i32 upsample(const i32* history,unsigned position,unsigned phase) {
    const i32* latest=history+position+16;
    if(phase) return latest[-7];
    i32 output=0;
    output+=opxy::q31::multiply(latest[-1]+latest[-14],tables::halfband[2]*2);
    output+=opxy::q31::multiply(latest[-2]+latest[-13],tables::halfband[4]*2);
    output+=opxy::q31::multiply(latest[-3]+latest[-12],tables::halfband[6]*2);
    output+=opxy::q31::multiply(latest[-4]+latest[-11],tables::halfband[8]*2);
    output+=opxy::q31::multiply(latest[-5]+latest[-10],tables::halfband[10]*2);
    output+=opxy::q31::multiply(latest[-6]+latest[-9],tables::halfband[12]*2);
    output+=opxy::q31::multiply(latest[-7]+latest[-8],tables::halfband[14]*2);
    return output;
}
}
void initialize(State& state){state=State();}
void process(State& state,opxy::Buffer buffer,const opxy::Transport& transport,
             opxy::Channel left,opxy::Channel right,opxy::Parameters parameters) {
    if(!buffer.words || buffer.count<buffer_words){for(unsigned i=0;i<opxy::block_frames;++i)left[i]=right[i]=0;return;}
    i32* memory=buffer.words;
    if(!state.ready) {
        for(unsigned i=0;i<fir_words;++i)memory[fir_offset+i]=0;
        for(unsigned i=0;i<7;++i)state.controls[i]=parameter(i,parameters[i]);
        state.delay=echo_delay(parameters[5],transport.bpm_x10);state.ready=1;
    }
    const i32 target_delay=echo_delay(parameters[5],transport.bpm_x10);
    i32* down[2][2]={{memory+fir_offset,memory+fir_offset+62},
                     {memory+fir_offset+188,memory+fir_offset+250}};
    i32* up[2][2]={{memory+fir_offset+124,memory+fir_offset+156},
                   {memory+fir_offset+312,memory+fir_offset+344}};
    for(unsigned control=0;control<7;++control) {
        const i32 target=parameter(control,parameters[control]);
        const i32 delta=mul(target-state.controls[control],control==6?3858:1987);
        state.controls[control]=clamp(state.controls[control]+(delta?delta:(target>state.controls[control])-(target<state.controls[control])),0,32767);
    }
    const Settings config=settings(state);
    const u32 grain_step=16777216u/config.grain_width;
    for(unsigned i=0;i<opxy::block_frames;++i) {
        down[0][0][state.down_write[0]]=down[0][0][state.down_write[0]+31]=left[i]/128;
        down[0][1][state.down_write[0]]=down[0][1][state.down_write[0]+31]=right[i]/128;
        if(!state.half) {
            for(unsigned channel=0;channel<2;++channel)
                down[1][channel][state.down_write[1]]=down[1][channel][state.down_write[1]+31]=downsample(down[0][channel],state.down_write[0]);
            if(!state.quarter) {
                state.delay=glide(state.delay,target_delay,512);
                i32 input[2]={downsample(down[1][0],state.down_write[1]),downsample(down[1][1],state.down_write[1])},wet[2];
                network(state,memory,input,wet,config);
                state.grain_phase=(state.grain_phase+grain_step)&16777215u;
                if(++state.up_write[1]==16)state.up_write[1]=0;
                for(unsigned channel=0;channel<2;++channel)
                    up[1][channel][state.up_write[1]]=up[1][channel][state.up_write[1]+16]=wet[channel];
            }
            if(++state.up_write[0]==16)state.up_write[0]=0;
            for(unsigned channel=0;channel<2;++channel)
                up[0][channel][state.up_write[0]]=up[0][channel][state.up_write[0]+16]=upsample(up[1][channel],state.up_write[1],state.quarter);
            state.quarter^=1;
            if(++state.down_write[1]==31)state.down_write[1]=0;
        }
        left[i]=clamp(upsample(up[0][0],state.up_write[0],state.half),-unit+1,unit-1)*128;
        right[i]=clamp(upsample(up[0][1],state.up_write[0],state.half),-unit+1,unit-1)*128;
        state.half^=1;
        if(++state.down_write[0]==31)state.down_write[0]=0;
    }
}
}
