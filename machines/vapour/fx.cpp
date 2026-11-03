#include "core.h"
#include "vendor/granular.h"

namespace vapour {
namespace {
using opxy::i32;
using opxy::u32;
using namespace granular;

// Clouds' eight allpass lengths, scaled from 32 kHz to 44.1 kHz.
const unsigned lengths[8]={174,248,371,612,208,283,338,558};
const unsigned offsets[8]={0,174,422,793,1405,1613,1896,2234};

inline i32 allpass_gain(i32 x) {
    // Stored allpass samples stay within +/-131071, so this product fits i32.
    return (x * 5) >> 3;
}

// The normalization target and state stay in [0, 65534]. For this difference
// range, ceil(2^31 / 100) gives the exact quotient of a nonnegative magnitude.
inline i32 normalization_step(i32 difference) {
    const i32 magnitude = difference < 0 ? -difference : difference;
    const i32 quotient = opxy::q31::multiply(magnitude, 21474837);
    return difference < 0 ? -quotient : quotient;
}

__attribute__((always_inline)) inline i32 diffuse(State& state, Workspace& workspace, i32 x, unsigned channel) {
    for (unsigned stage=channel*4;stage<channel*4+4;++stage) {
        const unsigned index=offsets[stage]+state.diffusion_position[stage];
        const i32 delayed=state.diffusion_written>=lengths[stage] ? workspace.diffusion[index] : 0;
        const i32 written=clamp(x+allpass_gain(delayed),-131071,131071);
        workspace.diffusion[index]=written;
        x=delayed-allpass_gain(written);
        u32& position=state.diffusion_position[stage];
        if (++position==lengths[stage]) position=0;
    }
    return x;
}
inline u32 packed_sample(const State& state,const i32* ring,u32 index) {
    index&=ring_frames-1;
    return index<state.written ? static_cast<u32>(ring[index]) : 0;
}
inline i32 cubic(i32 xm1,i32 x0,i32 x1,i32 x2,i32 t) {
    const i32 c=(x1-xm1)/2,v=x0-x1,w=c+v,a=w+v+(x2-x0)/2;
    return clamp(mul15(mul15(mul15(a,t)-(w+a),t)+c,t)+x0,-32768,32767);
}
template<bool High>
__attribute__((always_inline)) inline void read_stereo(const State& state,const i32* ring,u32 phase,i32& left,i32& right) {
    const u32 index=phase>>16;
    const i32 t=static_cast<i32>((phase&65535u)>>1);
    const u32 a=packed_sample(state,ring,index),b=packed_sample(state,ring,index+1);
    const i32 al=static_cast<opxy::i16>(a&65535u),ar=static_cast<opxy::i16>(a>>16);
    const i32 bl=static_cast<opxy::i16>(b&65535u),br=static_cast<opxy::i16>(b>>16);
    if (!High) {
        left=al+gain15(bl-al,t);right=ar+gain15(br-ar,t);
    } else {
        const u32 c=packed_sample(state,ring,index+2),d=packed_sample(state,ring,index+3);
        left=cubic(al,bl,static_cast<opxy::i16>(c&65535u),static_cast<opxy::i16>(d&65535u),t);
        right=cubic(ar,br,static_cast<opxy::i16>(c>>16),static_cast<opxy::i16>(d>>16),t);
    }
}
template<bool High>
void render_grain(State& __restrict state, Workspace& __restrict workspace, const i32* __restrict ring, unsigned index, i32* left, i32* right) {
    Grain grain=workspace.grains[index];
    if (!grain.remaining) return;
    for (unsigned i=grain.pre_delay;i<opxy::block_frames && grain.remaining;++i) {
        const i32 gain=envelope(grain,High);
        i32 source_l,source_r;read_stereo<High>(state,ring,grain.phase,source_l,source_r);
        const i32 l=gain15(source_l,gain);
        const i32 r=gain15(source_r,gain);
        const i32 pan_l=gain15(l,grain.left_gain),pan_r=gain15(r,grain.right_gain);
        left[i]+=pan_l+r-pan_r;
        right[i]+=pan_r+l-pan_l;
        grain.phase+=grain.increment;
        --grain.remaining;
    }
    grain.pre_delay=0;
    workspace.grains[index]=grain;
}

i32 normalized(i32 mixed, i32 gain) {
    return mul15(mixed,gain);
}
}

void initialize(State& state) {
    state=State();
    state.random=0x21u;
    state.size_hint=1411;
    state.normalization=32767;
}

void process(State& state, opxy::Buffer buffer, opxy::Channel left,
             opxy::Channel right, opxy::Parameters parameters) {
    const unsigned required=ring_frames+sizeof(Workspace)/4;
    if (!buffer.words || buffer.count<required) {
        for(unsigned i=0;i<opxy::block_frames;++i) left[i]=right[i]=0;
        return;
    }
    Workspace& workspace=*reinterpret_cast<Workspace*>(buffer.words+ring_frames);
    if (!state.initialized) {
        for(auto& grain:workspace.grains) grain=Grain();
        for(auto& sample:workspace.feedback) sample=0;
        state.initialized=1;
    }
    const u32 position=control(parameters[0]), size=control(parameters[1]);
    const u32 ratio=lookup(tables::pitch_ratio,control(parameters[2]));
    const u32 density=control(parameters[3]), texture=control(parameters[4]);
    const i32 feedback_target=static_cast<i32>(control(parameters[5])*95u/100u);
    const i32 freeze_target=control(parameters[6])>=16384u ? 32767 : 0;
    if (freeze_target && !state.freeze_gate) state.tail_count=state.tail_read=0;
    state.freeze_gate=freeze_target ? 1u : 0u;
    u32 overlap=density<15401u ? (15401u-density)*212u/100u
        : density>17367u ? (density-17367u)*212u/100u : 0u;
    if(overlap>32767u) overlap=32767u;
    const u32 overlap_cubed=((overlap*overlap)>>15)*overlap>>15;
    const u32 desired=overlap_cubed*grain_capacity;
    const u32 interval=desired ? state.size_hint*32768u/desired : 0xffffffffu;
    const u32 probability=(desired<<12)/(state.size_hint ? state.size_hint : 1);
    unsigned available[grain_capacity], free=0, active=0;
    for(unsigned i=0;i<grain_capacity;++i) {
        if(!workspace.grains[i].remaining) available[free++]=i;
        else ++active;
    }
    for(unsigned i=0;i<opxy::block_frames;++i) {
        state.feedback+=(feedback_target-state.feedback)/64;
        state.freeze+=(freeze_target-state.freeze)/64;
        if(!freeze_target) {
            // Feedback returns from the preceding 64-frame block, as in Clouds.
            const i32 old_l=workspace.feedback[i*2], old_r=workspace.feedback[i*2+1];
            state.dc_left+=(old_l-state.dc_left)/256;
            state.dc_right+=(old_r-state.dc_right)/256;
            const i32 fb=mul15(state.feedback,32767-state.freeze);
            i32 input_l=clamp((left[i]>>16)+mul15(old_l-state.dc_left,fb),-32768,32767);
            i32 input_r=clamp((right[i]>>16)+mul15(old_r-state.dc_right,fb),-32768,32767);
            if (state.tail_read < state.tail_count) {
                const u32 packed=workspace.freeze_tail[state.tail_read++];
                const i32 old_l=static_cast<opxy::i16>(packed & 65535u);
                const i32 old_r=static_cast<opxy::i16>(packed >> 16);
                const i32 fade=static_cast<i32>((state.tail_count-state.tail_read)*128u);
                input_l+=mul15(old_l-input_l,fade);
                input_r+=mul15(old_r-input_r,fade);
            }
            buffer.words[state.write]=static_cast<i32>(static_cast<u32>(static_cast<opxy::u16>(input_l))
                | (static_cast<u32>(static_cast<opxy::u16>(input_r))<<16));
            state.write=(state.write+1)&(ring_frames-1);
            if(state.written<ring_frames) ++state.written;
        }
        if (freeze_target && state.tail_count < 256) {
            workspace.freeze_tail[state.tail_count++]=static_cast<u32>(static_cast<opxy::u16>(left[i]>>16))
                | (static_cast<u32>(static_cast<opxy::u16>(right[i]>>16))<<16);
        }
        ++state.clock;
        const bool seed=density<16384u ? state.clock>=interval : (random(state)>>5)<probability;
        if(seed && free) {
            Grain& grain=workspace.grains[available[--free]];
            schedule(state,grain,size,ratio,position,texture);
            grain.pre_delay=i;
            ++active;
            state.clock=0;
        }
    }
    i32 mixed_left[opxy::block_frames]={},mixed_right[opxy::block_frames]={};
    for(unsigned g=0;g<grain_capacity*3/4;++g) render_grain<false>(state,workspace,buffer.words,g,mixed_left,mixed_right);
    for(unsigned g=grain_capacity*3/4;g<grain_capacity;++g) render_grain<true>(state,workspace,buffer.words,g,mixed_left,mixed_right);
    const i32 window_gain=static_cast<i32>(texture<12288u ? 32767u+texture*8u/3u : 65534u);
    const i32 gain=static_cast<i32>(tables::normalization[active])+
        mul15(static_cast<i32>(tables::normalization[active]),
              mul15(window_gain-32767,static_cast<i32>(overlap)));
    const i32 diffusion=texture>24576u ? static_cast<i32>((texture-24576u)*4u) : 0;
    if(!diffusion) state.diffusion_written=0;
    for(unsigned i=0;i<opxy::block_frames;++i) {
        state.normalization+=normalization_step(gain-state.normalization);
        i32 l=clamp(normalized(mixed_left[i],state.normalization),-32768,32767);
        i32 r=clamp(normalized(mixed_right[i],state.normalization),-32768,32767);
        if(diffusion) {
            const i32 diff_l=diffuse(state,workspace,l,0), diff_r=diffuse(state,workspace,r,1);
            if(state.diffusion_written<612) ++state.diffusion_written;
            l=clamp(l+mul15(diff_l-l,diffusion),-32768,32767);
            r=clamp(r+mul15(diff_r-r,diffusion),-32768,32767);
        }
        workspace.feedback[i*2]=l;
        workspace.feedback[i*2+1]=r;
        left[i]=l*16384;
        right[i]=r*16384;
    }
}
}
