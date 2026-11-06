#include "core.h"
#include <opxy/q31.hpp>

namespace lustre {
namespace {
using opxy::i32;
using opxy::u32;
using opxy::q31::half;
using opxy::q31::multiply;
const i32 kOne = opxy::q31::maximum;
const i32 kSignalLimit = 268435456;
const u32 kLengths[4] = {1499, 1877, 2381, 3011};
const u32 kDiffLengths[4] = {113, 173, 241, 337};

// Audio uses Q28, leaving three guard bits for sums and differences. Only
// scratch writes and the Q31 return need limiting; coefficients remain Q31.
i32 add(i32 a, i32 b) { return a + b; }
i32 bounded(i32 value) {
    return value > kSignalLimit ? kSignalLimit : value < -kSignalLimit ? -kSignalLimit : value;
}
i32 restore(i32 value) {
    // Fourfold wet gain, with a soft knee above half-scale at the return.
    // Keep feedback independent of the output gain, so decay stays stable.
    const i32 knee = 33554432;
    const bool negative = value < 0;
    const i32 magnitude = negative ? -value : value;
    if (magnitude > knee) {
        i32 shaped = 2 * knee;
        if (magnitude < 3 * knee) {
            const i32 excess = magnitude - knee;
            shaped = knee + excess - multiply(excess * 4, excess * 4);
        }
        value = negative ? -shaped : shaped;
    }
    value = opxy::q31::add(value, value);
    value = opxy::q31::add(value, value);
    value = opxy::q31::add(value, value);
    value = opxy::q31::add(value, value);
    return opxy::q31::add(value, value);
}
i32 interpolate(i32 a, i32 b, u32 fraction) {
    const i32 weight = static_cast<i32>(fraction << 15);
    return a + multiply(b - a, weight);
}
u32 random(State& s) {
    s.random ^= s.random << 13;
    s.random ^= s.random >> 17;
    s.random ^= s.random << 5;
    return s.random;
}
u32 control(i32 value) { return value < 0 ? 0 : static_cast<u32>(value) >> 16; }

void launch(State& s) {
    unsigned searched = 0;
    while (s.grains[s.next].remaining && searched < 6) {
        s.next = (s.next + 1) % 6;
        ++searched;
    }
    if (searched == 6) { s.countdown = 16; return; }
    Grain& g = s.grains[s.next];
    s.next = (s.next + 1) % 6;
    // 60..360 ms at 22.05 kHz. Decisions latch under a zero-ended envelope.
    const u32 length = 1323 + control(s.controls[0]) * 6615 / 32767;
    u32 delay = length + 441 + random(s) % (length + 1);
    g.remaining = length;
    g.phase = 0;
    g.increment = 0xffffffffu / length;
    const u32 pitch_choice = random(s) >> 17, direction_choice = random(s) >> 17;
    const u32 shimmer = control(s.controls[4]), backwards = control(s.controls[5]);
    const bool octave = shimmer == 32767 || pitch_choice < shimmer;
    const bool reverse = backwards == 32767 || direction_choice < backwards;
    const i32 drift = static_cast<i32>(control(s.controls[6]));
    const i32 detune = (static_cast<i32>(random(s) >> 17) - 16384) * drift / 262144;
    g.speed = (octave ? 131072 : 65536) + detune;
    if (reverse) g.speed = -g.speed;
    g.position = ((s.write + kHistory - delay) & (kHistory - 1)) << 16;
    g.pan = 4096 + static_cast<i32>(random(s) % 24576);
    s.countdown = length * 32767 / (65534 + control(s.controls[1]) * 4);
}

struct Coefficients {
    i32 tone, recycle, damping, gain, space, direct;
    Coefficients step_to(const Coefficients& end) const {
        return {(end.tone-tone)/32, (end.recycle-recycle)/32, (end.damping-damping)/32,
                (end.gain-gain)/32, (end.space-space)/32, (end.direct-direct)/32};
    }
    void advance(const Coefficients& step) {
        tone += step.tone;
        recycle += step.recycle;
        damping += step.damping;
        gain += step.gain;
        space += step.space;
        direct += step.direct;
    }
};

Coefficients coefficients(const State& s) {
    const i32 decay = s.controls[2];
    Coefficients c;
    c.tone = add(100000000, multiply(s.controls[7], 1400000000));
    c.recycle = multiply(decay, 1000000000);
    c.damping = add(180000000, multiply(s.controls[7], 1100000000));
    c.gain = add(1350000000, multiply(decay, 690000000));
    // The upper 40% opens into a long hall; defaults retain their old decay.
    const i32 extra = decay > 1288490188 ? decay - 1288490188 : 0;
    c.gain += multiply(extra, 188000000);
    c.space = s.controls[3];
    c.direct = kOne - c.space;
    return c;
}

// Render old history in blocks. A read near the live writer uses a one-frame
// segment instead, preserving the original feedback and wrap behavior.
template<bool Ready>
inline void history_pair(const i32* memory, u32 index, u32 write, u32 written, i32& l, i32& r) {
    const u32 age = (write-index) & (kHistory-1);
    if (!age || age > written) { l = r = 0; return; }
    l = memory[2*index];
    r = memory[2*index+1];
}

template<>
inline void history_pair<true>(const i32* memory, u32 index, u32, u32, i32& l, i32& r) {
    l = memory[2*index];
    r = memory[2*index+1];
}

template<bool Ready, bool Fractional>
__attribute__((noinline)) void grain_span(Grain& g, const i32* __restrict memory,
                                         u32 write, u32 written, u32 frames,
                                         i32* __restrict left, i32* __restrict right) {
    const u32 count = g.remaining < frames ? g.remaining : frames;
    u32 position = g.position, phase = g.phase;
    const u32 speed = static_cast<u32>(g.speed), increment = g.increment;
    const opxy::u16 pan_l = static_cast<opxy::u16>(65536 - g.pan);
    const opxy::u16 pan_r = static_cast<opxy::u16>(32768 + g.pan);
    for (u32 i = 0; i < count; ++i) {
        const u32 index = position >> 16;
        i32 l, r;
        history_pair<Ready>(memory,index,write,written,l,r);
        if (Fractional) {
            const u32 next = (index + 1) & (kHistory - 1);
            i32 next_l, next_r;
            history_pair<Ready>(memory,next,write,written,next_l,next_r);
            l = interpolate(l,next_l,position & 65535);
            r = interpolate(r,next_r,position & 65535);
        }
        const opxy::u16 part = static_cast<opxy::u16>(phase >> 17);
        const opxy::u16 complement = static_cast<opxy::u16>(32768 - part);
        const opxy::u16 arch = static_cast<opxy::u16>((static_cast<u32>(part) * complement) >> 13);
        const u32 squared = static_cast<u32>(arch) * arch >> 15;
        const opxy::u16 window = static_cast<opxy::u16>(squared < 32768 ? squared : 32767);
        const i32 gain_l = static_cast<i32>(static_cast<u32>(window) * pan_l >> 2);
        const i32 gain_r = static_cast<i32>(static_cast<u32>(window) * pan_r >> 2);
        left[i] += multiply(l, gain_l);
        right[i] += multiply(r, gain_r);
        position = (position + speed) & 0x7fffffffu;
        phase += increment;
    }
    g.position = position;
    g.phase = phase;
    g.remaining -= count;
}

bool history_safe(const State& s, u32 frames) {
    for (unsigned j = 0; j < 6; ++j) {
        const Grain& g = s.grains[j];
        if (!g.remaining) continue;
        const u32 count = g.remaining < frames ? g.remaining : frames;
        const u32 end = (g.position + static_cast<u32>(g.speed) * (count-1)) & 0x7fffffffu;
        const u32 first_age = (s.write-(g.position>>16)) & (kHistory-1);
        const u32 last_age = (s.write-(end>>16)) & (kHistory-1);
        if (first_age < 64 || last_age < 64 || first_age > kHistory-64 || last_age > kHistory-64)
            return false;
        if ((g.speed < 0 && last_age < first_age) || (g.speed > 0 && last_age > first_age))
            return false;
    }
    return true;
}

__attribute__((noinline)) void cloud_span(State& s, const i32* memory, i32 wet[2][32],
                                         u32 offset, u32 count, bool safe) {
    for (unsigned j = 0; j < 6; ++j) {
        Grain& g = s.grains[j];
        if (!g.remaining) continue;
        const bool fractional = (g.position & 65535) || (static_cast<u32>(g.speed) & 65535);
        if (s.written == kHistory && safe) {
            if (fractional) grain_span<true,true>(g,memory,s.write,s.written,count,wet[0]+offset,wet[1]+offset);
            else grain_span<true,false>(g,memory,s.write,s.written,count,wet[0]+offset,wet[1]+offset);
        } else {
            if (fractional) grain_span<false,true>(g,memory,s.write,s.written,count,wet[0]+offset,wet[1]+offset);
            else grain_span<false,false>(g,memory,s.write,s.written,count,wet[0]+offset,wet[1]+offset);
        }
    }
}

__attribute__((noinline)) void input_block(State& s, unsigned ch, const i32* input, i32* output) {
    i32 anti = s.antialias[ch], low = s.input[ch];
    for (unsigned i = 0; i < 32; ++i) {
        anti += multiply(input[2*i] / 8 - anti, 750000000);
        low += multiply(anti-low, 750000000);
        output[i] = low;
        anti += multiply(input[2*i+1] / 8 - anti, 750000000);
        low += multiply(anti-low, 750000000);
    }
    s.antialias[ch] = anti;
    s.input[ch] = low;
}

__attribute__((noinline)) void capture(State& s, unsigned ch, i32* memory, u32 write,
                                      const i32* input, i32* wet,
                                      const Coefficients& start, const Coefficients& step, u32 frames) {
    i32 low = s.tone_memory[ch], dc = s.dc[ch], feedback = 0;
    i32 tone = start.tone, recycle = start.recycle;
    for (unsigned i = 0; i < frames; ++i) {
        tone += step.tone;
        recycle += step.recycle;
        low += multiply(wet[i]-low, tone);
        wet[i] = low;
        dc += multiply(low-dc, 4000000);
        feedback = multiply(low-dc, recycle);
        memory[2 * ((write+i) & (kHistory-1)) + 1-ch] = bounded(half(input[i]) + feedback);
    }
    s.tone_memory[ch] = low;
    s.dc[ch] = dc;
}

template<unsigned Line, unsigned Length, bool Ready>
__attribute__((noinline)) void diffuser(i32* memory, u32 write, u32 written, i32* audio) {
    for (unsigned i = 0; i < 32; ++i) {
        const u32 cursor = write+i;
        const i32 delayed = Ready || written+i >= Length
            ? memory[4 * ((cursor-Length) & (kDiffuser-1)) + Line] : 0;
        const i32 value = audio[i];
        const i32 out = delayed-half(value);
        memory[4 * (cursor & (kDiffuser-1)) + Line] = bounded(value+half(out));
        audio[i] = out;
    }
}

template<unsigned Line, unsigned Length, bool Ready>
__attribute__((noinline)) void damping(State& s, const i32* memory, u32 write, u32 written,
                                      i32 coefficient, i32 step, i32* output) {
    i32 low = s.damping[Line];
    for (unsigned i = 0; i < 32; ++i) {
        coefficient += step;
        const i32 value = Ready || written+i >= Length
            ? memory[4 * ((write+i-Length) & (kReverb-1)) + Line] : 0;
        low += multiply(value-low, coefficient);
        output[i] = low;
    }
    s.damping[Line] = low;
}

__attribute__((noinline)) void tank(i32* memory, u32 write, i32 wet[2][32], i32 diffused[2][32],
                                   i32 damped[4][32], i32 output[2][32],
                                   const Coefficients& start, const Coefficients& step) {
    i32 gain = start.gain, space = start.space, direct = start.direct;
    for (unsigned i = 0; i < 32; ++i) {
        gain += step.gain;
        space += step.space;
        direct += step.direct;
        const i32 a = damped[0][i], b = damped[1][i], c = damped[2][i], d = damped[3][i];
        const i32 average = half(half(a))+half(half(b))+half(half(c))+half(half(d));
        const i32 sum = average+average;
        const i32 l = half(diffused[0][i]), r = half(diffused[1][i]);
        i32* frame = memory + 4 * ((write+i) & (kReverb-1));
        frame[0] = bounded(l+multiply(a-sum,gain));
        frame[1] = bounded(r+multiply(b-sum,gain));
        frame[2] = bounded(l+multiply(c-sum,gain));
        frame[3] = bounded(r+multiply(d-sum,gain));
        const i32 room_l = (half(a)-half(c))*3;
        const i32 room_r = (half(b)-half(d))*3;
        output[0][i] = multiply(wet[0][i],direct)+multiply(room_l,space);
        output[1][i] = multiply(wet[1][i],direct)+multiply(room_r,space);
    }
}

__attribute__((noinline)) void output_block(State& s, unsigned ch, const i32* input, i32* output) {
    i32 previous = s.output[ch];
    for (unsigned i = 0; i < 32; ++i) {
        const i32 next = input[i];
        output[2*i] = restore(half(previous)+half(next));
        output[2*i+1] = restore(next);
        previous = next;
    }
    s.output[ch] = previous;
}
}

void initialize(State& s) {
    s = State();
    s.random = 0x9e3779b9u;
}

void process(State& s, opxy::Buffer buffer, opxy::Channel left, opxy::Channel right,
             opxy::Parameters parameters) {
    if (!buffer.words || buffer.count < kBufferWords) {
        for (unsigned i = 0; i < opxy::block_frames; ++i) left[i] = right[i] = 0;
        return;
    }
    const Coefficients previous = coefficients(s);
    const bool started = s.started != 0;
    for (unsigned j = 0; j < 8; ++j) {
        const i32 target = parameters[j] < 0 ? 0 : parameters[j];
        if (!started) s.controls[j] = target;
        else {
            const i32 delta = target-s.controls[j], step = delta/8;
            s.controls[j] += step ? step : (delta>0)-(delta<0);
        }
    }
    s.started = 1;
    const Coefficients end = coefficients(s);
    const Coefficients start = started ? previous : end;
    const Coefficients step = start.step_to(end);
    i32 input[2][32], wet[2][32] = {}, diffused[2][32], damped[4][32];
    input_block(s,0,left,input[0]);
    input_block(s,1,right,input[1]);
    u32 offset = 0;
    Coefficients tone = start;
    while (offset < 32) {
        if (!s.countdown) launch(s);
        u32 count = s.countdown < 32-offset ? s.countdown : 32-offset;
        const bool safe = history_safe(s,count);
        if (!safe) count = 1;
        cloud_span(s,buffer.words,wet,offset,count,safe);
        capture(s,0,buffer.words,s.write,input[1]+offset,wet[0]+offset,tone,step,count);
        capture(s,1,buffer.words,s.write,input[0]+offset,wet[1]+offset,tone,step,count);
        s.countdown -= count;
        s.write = (s.write+count) & (kHistory-1);
        s.written = s.written+count < kHistory ? s.written+count : kHistory;
        tone.tone += step.tone*static_cast<i32>(count);
        tone.recycle += step.recycle*static_cast<i32>(count);
        offset += count;
    }
    for (unsigned ch = 0; ch < 2; ++ch)
        for (unsigned i = 0; i < 32; ++i) diffused[ch][i] = wet[ch][i];
    i32* const reverb = buffer.words + 2*kHistory;
    i32* const diff = reverb + 4*kReverb;
    const u32 rw = s.reverb_write, written = s.reverb_written;
    if (written >= kDiffLengths[3]) {
        diffuser<0,113,true>(diff,rw,written,diffused[0]);
        diffuser<1,173,true>(diff,rw,written,diffused[0]);
        diffuser<2,241,true>(diff,rw,written,diffused[1]);
        diffuser<3,337,true>(diff,rw,written,diffused[1]);
    } else {
        diffuser<0,113,false>(diff,rw,written,diffused[0]);
        diffuser<1,173,false>(diff,rw,written,diffused[0]);
        diffuser<2,241,false>(diff,rw,written,diffused[1]);
        diffuser<3,337,false>(diff,rw,written,diffused[1]);
    }
    if (written >= kLengths[3]) {
        damping<0,1499,true>(s,reverb,rw,written,start.damping,step.damping,damped[0]);
        damping<1,1877,true>(s,reverb,rw,written,start.damping,step.damping,damped[1]);
        damping<2,2381,true>(s,reverb,rw,written,start.damping,step.damping,damped[2]);
        damping<3,3011,true>(s,reverb,rw,written,start.damping,step.damping,damped[3]);
    } else {
        damping<0,1499,false>(s,reverb,rw,written,start.damping,step.damping,damped[0]);
        damping<1,1877,false>(s,reverb,rw,written,start.damping,step.damping,damped[1]);
        damping<2,2381,false>(s,reverb,rw,written,start.damping,step.damping,damped[2]);
        damping<3,3011,false>(s,reverb,rw,written,start.damping,step.damping,damped[3]);
    }
    tank(reverb,rw,wet,diffused,damped,input,start,step);
    s.reverb_write += 32;
    s.reverb_written = written+32 < kReverb ? written+32 : kReverb;
    output_block(s,0,input[0],left);
    output_block(s,1,input[1],right);
}
}
