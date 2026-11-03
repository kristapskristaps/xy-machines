#pragma once
// Private implementation included after Halo's scalar network helpers.
const unsigned wet_frames=opxy::block_frames/4;
inline u32 written_at(const State& state,unsigned frame){
    if(state.written>=echo_frames)return state.written;
    const u32 count=state.written+frame;return count<echo_frames?count:echo_frames;
}
template<unsigned LineIndex>
inline unsigned position_at(const State& state,unsigned frame){
    const unsigned position=state.positions[LineIndex]+frame*(LineIndex==16||LineIndex==17?2:1);
    return LineIndex==18?(position>=lines[18].length?position-lines[18].length:position):position&(lines[LineIndex].length-1);
}
template<unsigned LineIndex>
__attribute__((always_inline)) inline i32 old_sample(const State& state,const i32* memory,unsigned frame,unsigned age,unsigned channel=0){
    static_assert(LineIndex<18,"unpacked power-of-two history");
    if(!age||age>written_at(state,frame))return 0;
    const Line& line=lines[LineIndex];enum { stereo=LineIndex==16||LineIndex==17 };
    if(age>=(stereo?line.length/2:line.length))return 0;
    const unsigned position=position_at<LineIndex>(state,frame);
    const unsigned index=(position-age*(stereo?2:1)+(stereo?channel:0))&(line.length-1);
    return memory[line.offset+index];
}
template<unsigned LineIndex>
__attribute__((always_inline)) inline i32 old_tap(const State& state,const i32* memory,unsigned frame,i32 delay,unsigned channel=0){
    const unsigned age=static_cast<u32>(delay)>>8;
    const Line& line=lines[LineIndex];enum { stereo=LineIndex==16||LineIndex==17 };
    const unsigned limit=(stereo?line.length/2:line.length)-1;
    const unsigned written=written_at(state,frame),available=written<limit?written:limit;
    if(age>available)return 0;
    const unsigned position=position_at<LineIndex>(state,frame);i32 a,b;
    if(LineIndex==18){
        const unsigned index=position>=age?position-age:position+line.length-age;
        const u32 first=age?static_cast<u32>(memory[line.offset+index]):0;
        a=static_cast<opxy::i16>(channel?first>>16:first&65535u)*512;
        if(!(delay&255))return a;
        const unsigned previous=index?index-1:line.length-1;
        const u32 second=age<available?static_cast<u32>(memory[line.offset+previous]):0;
        b=static_cast<opxy::i16>(channel?second>>16:second&65535u)*512;
    }else{
        const unsigned stride=stereo?2:1;
        const unsigned index=(position-age*stride+(stereo?channel:0))&(line.length-1);
        a=age?memory[line.offset+index]:0;
        if(!(delay&255))return a;
        b=age<available?memory[line.offset+((index-stride)&(line.length-1))]:0;
    }
    return a+mul(b-a,(delay&255)*128);
}
__attribute__((always_inline)) inline i32 old_pitch_up(const State& state,const i32* memory,unsigned frame,unsigned channel,unsigned width,u32 grain_step){
    const u32 grain=frame?(state.grain_phase+frame*grain_step)&16777215u:state.grain_phase;
    const u32 phase=grain>>8,other=(phase+32768u)&65535u;
    const i32 a_delay=static_cast<i32>(551u*256u+((65535u-phase)*width>>8));
    const i32 b_delay=static_cast<i32>(551u*256u+((65535u-other)*width>>8));
    const i32 a=old_tap<17>(state,memory,frame,a_delay+static_cast<i32>(channel*2048u),channel);
    const i32 b=old_tap<17>(state,memory,frame,b_delay+static_cast<i32>(channel*2048u),channel);
    return mul(a,table(tables::window,phase))+mul(b,table(tables::window,other));
}
template<unsigned LineIndex>
inline bool prefetch_diffuser(const State& state,const i32* memory,i32* delayed,unsigned age){
    if(age<=wet_frames)return false;
    for(unsigned i=0;i<wet_frames;++i)delayed[i]=old_sample<LineIndex>(state,memory,i,age);
    return true;
}
template<unsigned Channel>
inline bool prefetch_plate(const State& state,const i32* memory,i32 delayed[][wet_frames],i32* modulation,const Settings& config){
    const unsigned first=8+Channel*4;u32 phase=state.phase[Channel];
    for(unsigned i=0;i<wet_frames;++i){
        phase+=Channel?49474u:44022u;
        modulation[i]=opxy::q31::multiply(table(tables::sine,phase>>16),config.modulation)/4;
        if((static_cast<u32>(config.delays[Channel][0]+modulation[i])>>8)<=wet_frames ||
           (static_cast<u32>(config.delays[Channel][2]-modulation[i]/4)>>8)<=wet_frames)return false;
    }
    if((static_cast<u32>(config.delays[Channel][1])>>8)<=wet_frames ||
       (static_cast<u32>(config.delays[Channel][3])>>8)<=wet_frames)return false;
    for(unsigned i=0;i<wet_frames;++i)delayed[first][i]=old_tap<first>(state,memory,i,config.delays[Channel][0]+modulation[i]);
    for(unsigned i=0;i<wet_frames;++i)delayed[first+1][i]=old_tap<first+1>(state,memory,i,config.delays[Channel][1]);
    for(unsigned i=0;i<wet_frames;++i)delayed[first+2][i]=old_tap<first+2>(state,memory,i,config.delays[Channel][2]-modulation[i]/4);
    for(unsigned i=0;i<wet_frames;++i)delayed[first+3][i]=old_tap<first+3>(state,memory,i,config.delays[Channel][3]);
    return true;
}
template<unsigned LineIndex>
__attribute__((noinline)) void diffuser_block(State& state,i32* memory,i32* samples,const i32* delayed,i32 gain){
    const unsigned start=state.positions[LineIndex],mask=lines[LineIndex].length-1;
    i32* ring=memory+lines[LineIndex].offset;
    for(unsigned i=0;i<wet_frames;++i){
        const i32 input=samples[i],output=delayed[i]-opxy::q31::multiply(input,gain);
        ring[(start+i)&mask]=clamp(input+opxy::q31::multiply(output,gain),-3*unit,3*unit);samples[i]=output;
    }
    state.positions[LineIndex]=(start+wet_frames)&mask;
}
template<unsigned Channel>
__attribute__((noinline)) void plate_block(State& state,i32* memory,i32* input,i32 delayed[][wet_frames],i32* samples,const i32* plate_start,const i32* shifted,const Settings& config){
    const unsigned other=1-Channel,first=8+Channel*4;
    for(unsigned i=0;i<wet_frames;++i)samples[i]=mul(delayed[16+Channel][i],23593)+mul(input[i],3277)+opxy::q31::multiply(delayed[18+Channel][i],config.seed_echo);
    diffuser_block<Channel*4+0>(state,memory,samples,delayed[Channel*4+0],config.diffusion);
    diffuser_block<Channel*4+1>(state,memory,samples,delayed[Channel*4+1],config.diffusion);
    diffuser_block<Channel*4+2>(state,memory,samples,delayed[Channel*4+2],config.diffusion);
    diffuser_block<Channel*4+3>(state,memory,samples,delayed[Channel*4+3],config.diffusion);
    for(unsigned i=0;i<wet_frames;++i){
        const i32 prior=i?clamp(delayed[8+other*4+3][i-1],-unit,unit):plate_start[other];
        samples[i]=soft(mul(samples[i],23593)+opxy::q31::multiply(prior,config.decay)+opxy::q31::multiply(shifted[Channel*wet_frames+i],config.shimmer));
    }
    unsigned position=state.positions[first];
    for(unsigned i=0;i<wet_frames;++i){
        const i32 x=samples[i],ap=delayed[first][i]-mul(x,22282);
        memory[lines[first].offset+((position+i)&(lines[first].length-1))]=clamp(x+mul(ap,22282),-3*unit,3*unit);
        delayed[first][i]=ap;
    }
    state.positions[first]=(position+wet_frames)&(lines[first].length-1);
    position=state.positions[first+1];i32 damping=state.damping[Channel];
    for(unsigned i=0;i<wet_frames;++i){
        memory[lines[first+1].offset+((position+i)&(lines[first+1].length-1))]=delayed[first][i];
        damping+=opxy::q31::multiply(delayed[first+1][i]-damping,config.damping);
        samples[i]=opxy::q31::multiply(damping,config.decay);
    }
    state.damping[Channel]=damping;state.positions[first+1]=(position+wet_frames)&(lines[first+1].length-1);
    position=state.positions[first+2];
    for(unsigned i=0;i<wet_frames;++i){
        const i32 x=samples[i],ap=delayed[first+2][i]-mul(x,16384);
        memory[lines[first+2].offset+((position+i)&(lines[first+2].length-1))]=clamp(x+mul(ap,16384),-3*unit,3*unit);samples[i]=ap;
    }
    state.positions[first+2]=(position+wet_frames)&(lines[first+2].length-1);
    position=state.positions[first+3];
    for(unsigned i=0;i<wet_frames;++i)memory[lines[first+3].offset+((position+i)&(lines[first+3].length-1))]=samples[i];
    state.positions[first+3]=(position+wet_frames)&(lines[first+3].length-1);
    i32 body=state.body[Channel],duck=state.duck[Channel];
    for(unsigned i=0;i<wet_frames;++i){
        const i32 prior=i?clamp(delayed[8+other*4+3][i-1],-unit,unit):plate_start[other];
        const i32 tail=mul(delayed[first][i],4260)+mul(delayed[first+1][i],10158)+mul(delayed[first+3][i],22282)+mul(prior,5898);
        const i32 magnitude=input[i]<0?-input[i]:input[i];body+=mul(magnitude-body,76);
        const i32 transient=clamp((magnitude-body-third_nonnegative(body))*4,0,unit);
        const i32 follower=transient>duck?5361*65536:config.release;duck+=opxy::q31::multiply(transient-duck,follower);
        const i32 duck_gain=32767-opxy::q31::multiply(duck/512,config.duck);
        input[i]=soft(mul(mul(tail,duck_gain),24000)+opxy::q31::multiply(delayed[18+Channel][i],config.wet_echo));
    }
    state.body[Channel]=body;state.duck[Channel]=duck;
}
// Every read must precede every ring write. Its age guard admits only reads
// older than this whole wet block, including both fractional interpolation taps.
__attribute__((noinline)) bool network_block(State& state,i32* memory,i32* left,i32* right,i32* scratch,i32* shifted,const Settings& config,i32 target_delay,u32 grain_step){
    static_assert(wet_frames==16,"Halo network block domain");
    if(config.shimmer && state.grain_phase>16777215u)return false;
    // Rows 0..15 hold mono ring reads, 16/17 predelay and 18/19 echo.
    // Pitch heads/work samples borrow the caller's two conversion arrays.
    // Consumed AP1 rows are reused for the tail's AP1 output.
    i32 delayed[20][wet_frames];i32 delay=state.delay;
    for(unsigned i=0;i<wet_frames;++i){delay=glide(delay,target_delay,512);scratch[i]=delay;if((static_cast<u32>(delay)>>8)<=wet_frames)return false;}
    const i32 final_delay=delay;
    for(unsigned i=0;i<wet_frames;++i){delayed[18][i]=old_tap<18>(state,memory,i,scratch[i],0);delayed[19][i]=old_tap<18>(state,memory,i,scratch[i],1);}
    if(config.pre_delay<=wet_frames)return false;
    for(unsigned i=0;i<wet_frames;++i){delayed[16][i]=old_sample<16>(state,memory,i,config.pre_delay,0);delayed[17][i]=old_sample<16>(state,memory,i,config.pre_delay+8,1);}
    for(unsigned i=0;i<wet_frames;++i){shifted[i]=config.shimmer?old_pitch_up(state,memory,i,0,config.grain_width,grain_step):0;shifted[wet_frames+i]=config.shimmer?old_pitch_up(state,memory,i,1,config.grain_width,grain_step):0;}
    if(!prefetch_diffuser<0>(state,memory,delayed[0],diffusion_delay[0]) || !prefetch_diffuser<1>(state,memory,delayed[1],diffusion_delay[1]) ||
       !prefetch_diffuser<2>(state,memory,delayed[2],diffusion_delay[2]) || !prefetch_diffuser<3>(state,memory,delayed[3],diffusion_delay[3]) ||
       !prefetch_diffuser<4>(state,memory,delayed[4],diffusion_delay[4]) || !prefetch_diffuser<5>(state,memory,delayed[5],diffusion_delay[5]) ||
       !prefetch_diffuser<6>(state,memory,delayed[6],diffusion_delay[6]) || !prefetch_diffuser<7>(state,memory,delayed[7],diffusion_delay[7]))return false;
    if(!prefetch_plate<0>(state,memory,delayed,scratch,config)||!prefetch_plate<1>(state,memory,delayed,scratch,config))return false;
    const i32 plate_start[2]={state.plate[0],state.plate[1]};i32* channels[2]={left,right};
    for(unsigned channel=0;channel<2;++channel){
        i32 high=state.input_high[channel],low=state.input_low[channel];
        for(unsigned i=0;i<wet_frames;++i){high+=mul(channels[channel][i]-high,2198);low+=opxy::q31::multiply(channels[channel][i]-high-low,config.color);channels[channel][i]=opxy::q31::multiply(low,config.input_gain);}
        state.input_high[channel]=high;state.input_low[channel]=low;
        low=state.echo_low[channel];high=state.echo_high[channel];
        for(unsigned i=0;i<wet_frames;++i){low+=opxy::q31::multiply(delayed[18+channel][i]-low,config.echo_color);high+=mul(low-high,1580);delayed[18+channel][i]=low-high;}
        state.echo_low[channel]=low;state.echo_high[channel]=high;
        high=state.shimmer_high[channel];i32 low0=state.shimmer_low[channel][0],low1=state.shimmer_low[channel][1];
        const unsigned position=state.positions[17];
        for(unsigned i=0;i<wet_frames;++i){const i32 prior=i?clamp(delayed[8+channel*4+3][i-1],-unit,unit):plate_start[channel];high+=mul(prior-high,4994);low0+=mul(prior-high-low0,27476);low1+=mul(low0-low1,27476);memory[lines[17].offset+((position+2*i+channel)&(lines[17].length-1))]=low1;}
        state.shimmer_high[channel]=high;state.shimmer_low[channel][0]=low0;state.shimmer_low[channel][1]=low1;
    }
    unsigned position=state.positions[16];
    for(unsigned i=0;i<wet_frames;++i){memory[lines[16].offset+((position+2*i)&(lines[16].length-1))]=left[i];memory[lines[16].offset+((position+2*i+1)&(lines[16].length-1))]=right[i];}
    state.positions[16]=(position+2*wet_frames)&(lines[16].length-1);state.positions[17]=(state.positions[17]+2*wet_frames)&(lines[17].length-1);
    position=state.positions[18];i32 error0=state.echo_error[0],error1=state.echo_error[1];
    for(unsigned i=0;i<wet_frames;++i){
        const i32 x0=soft(left[i]+opxy::q31::multiply(delayed[19][i],config.feedback))+error0;
        const i32 x1=soft(right[i]+opxy::q31::multiply(delayed[18][i],config.feedback))+error1;
        const i32 q0=clamp((x0+256)>>9,-32768,32767),q1=clamp((x1+256)>>9,-32768,32767);
        error0=x0-q0*512;error1=x1-q1*512;
        const u32 packed=static_cast<u32>(static_cast<opxy::u16>(static_cast<opxy::i16>(q0)))|(static_cast<u32>(static_cast<opxy::u16>(static_cast<opxy::i16>(q1)))<<16);
        memory[lines[18].offset+position]=static_cast<i32>(packed);if(++position==lines[18].length)position=0;
    }
    state.positions[18]=position;state.echo_error[0]=error0;state.echo_error[1]=error1;
    plate_block<0>(state,memory,left,delayed,scratch,plate_start,shifted,config);plate_block<1>(state,memory,right,delayed,scratch,plate_start,shifted,config);
    state.plate[0]=clamp(delayed[11][wet_frames-1],-unit,unit);state.plate[1]=clamp(delayed[15][wet_frames-1],-unit,unit);
    state.phase[0]+=wet_frames*44022u;state.phase[1]+=wet_frames*49474u;
    state.delay=final_delay;state.grain_phase=(state.grain_phase+wet_frames*grain_step)&16777215u;
    if(state.written<echo_frames)state.written=state.written+wet_frames<echo_frames?state.written+wet_frames:echo_frames;
    return true;
}

__attribute__((noinline)) void scalar_network_block(State& state,i32* memory,i32* left,i32* right,
        const Settings& config,i32 target_delay,u32 grain_step){
    for(unsigned i=0;i<wet_frames;++i){
        state.delay=glide(state.delay,target_delay,512);
        i32 input[2]={left[i],right[i]},wet[2];network(state,memory,input,wet,config);
        state.grain_phase=(state.grain_phase+grain_step)&16777215u;left[i]=wet[0];right[i]=wet[1];
    }
}
