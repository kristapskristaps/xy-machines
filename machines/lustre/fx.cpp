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
const i32 kReturnKnee = 33554432;

// Audio uses Q28, leaving three guard bits for sums and differences. Only
// scratch writes and the Q31 return need limiting; coefficients remain Q31.
i32 add(i32 a, i32 b) { return a + b; }
i32 bounded(i32 value) {
    return value > kSignalLimit ? kSignalLimit : value < -kSignalLimit ? -kSignalLimit : value;
}
__attribute__((always_inline)) inline i32 restore(i32 value) {
    // Fourfold wet gain, with a soft knee above half-scale at the return.
    // Keep feedback independent of the output gain, so decay stays stable.
    const bool negative = value < 0;
    const i32 magnitude = negative ? -value : value;
    if (magnitude > kReturnKnee) {
        i32 shaped = 2 * kReturnKnee;
        if (magnitude < 3 * kReturnKnee) {
            const i32 excess = magnitude - kReturnKnee;
            shaped = kReturnKnee + excess - multiply(excess * 4, excess * 4);
        }
        value = negative ? -shaped : shaped;
    }
    // The knee bounds the result to +/-2^26; four gain bits fit exactly.
    const i32 scaled = value * 16;
    return opxy::q31::add(scaled,scaled);
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

inline i32 grain_envelope(u32 phase) {
    const opxy::u16 part = static_cast<opxy::u16>(phase >> 17);
    const opxy::u16 complement = static_cast<opxy::u16>(32768 - part);
    const opxy::u16 arch = static_cast<opxy::u16>((static_cast<u32>(part) * complement) >> 13);
    const u32 squared = static_cast<u32>(arch) * arch >> 15;
    return static_cast<i32>(squared < 32768 ? squared : 32767) * 16384;
}

template<bool Ready, bool Fractional, bool Contiguous = false>
__attribute__((noinline)) void grain_span(Grain& g, const i32* __restrict memory,
                                         u32 write, u32 written, u32 frames,
                                         i32* __restrict left, i32* __restrict right) {
    const u32 count = g.remaining < frames ? g.remaining : frames;
    u32 position = g.position, phase = g.phase;
    const u32 speed = static_cast<u32>(g.speed), increment = g.increment;
    const i32 pan_l = (65536 - g.pan) * 32768;
    const i32 pan_r = (32768 + g.pan) * 32768;
    for (u32 i = 0; i < count; ++i) {
        const u32 index = position >> 16;
        i32 l, r;
        history_pair<Ready>(memory,index,write,written,l,r);
        if (Fractional) {
            const u32 next = Contiguous ? index+1 : (index+1) & (kHistory-1);
            i32 next_l, next_r;
            history_pair<Ready>(memory,next,write,written,next_l,next_r);
            l = interpolate(l,next_l,position & 65535);
            r = interpolate(r,next_r,position & 65535);
        }
        const i32 envelope = grain_envelope(phase);
        const i32 gain_l = multiply(envelope,pan_l);
        const i32 gain_r = multiply(envelope,pan_r);
        left[i] += multiply(l, gain_l);
        right[i] += multiply(r, gain_r);
        position += speed;
        if (!Contiguous) position &= 0x7fffffffu;
        phase += increment;
    }
    g.position = position & 0x7fffffffu;
    g.phase = phase;
    g.remaining -= count;
}

__attribute__((noinline)) void forward_grain(Grain& g, const i32* __restrict source,
                                              u32 count, i32* __restrict left,
                                              i32* __restrict right) {
    u32 phase = g.phase;
    const u32 increment = g.increment;
    const i32 pan_l = (65536-g.pan)*32768;
    const i32 pan_r = (32768+g.pan)*32768;
    for (u32 i = 0; i < count; ++i) {
        const i32 envelope = grain_envelope(phase);
        left[i] += multiply(source[0],multiply(envelope,pan_l));
        right[i] += multiply(source[1],multiply(envelope,pan_r));
        source += 2;
        phase += increment;
    }
    g.position = (g.position+(count<<16)) & 0x7fffffffu;
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
        // The base sample and its interpolation neighbor must lie outside
        // every history slot this span will write.
        const bool fractional = (g.position & 65535) || (static_cast<u32>(g.speed) & 65535);
        const u32 minimum_age = fractional ? 2 : 1;
        const u32 maximum_age = kHistory-frames;
        if (first_age < minimum_age || last_age < minimum_age ||
            first_age > maximum_age || last_age > maximum_age)
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
        const u32 frames = g.remaining < count ? g.remaining : count;
        const u32 index = g.position >> 16;
        bool ready = s.written == kHistory && safe;
        if (safe && !ready) {
            const u32 end = (g.position + static_cast<u32>(g.speed)*(frames-1)) & 0x7fffffffu;
            const u32 first_age = (s.write-index) & (kHistory-1);
            const u32 last_age = (s.write-(end>>16)) & (kHistory-1);
            const u32 youngest = first_age < last_age ? first_age : last_age;
            const u32 oldest = first_age > last_age ? first_age : last_age;
            ready = oldest <= s.written;
            // No initialized history in this span: advance the silent grain
            // without evaluating its envelope. A fractional neighbor is newer.
            if (youngest > s.written + (fractional ? 1u : 0u)) {
                g.position = (g.position + static_cast<u32>(g.speed)*frames) & 0x7fffffffu;
                g.phase += g.increment*frames;
                g.remaining -= frames;
                continue;
            }
        }
        if (ready && !fractional && g.speed == 65536 && frames <= kHistory-index) {
            forward_grain(g,memory+2*index,frames,wet[0]+offset,wet[1]+offset);
        } else if (ready && fractional) {
            const u32 end = g.position+static_cast<u32>(g.speed)*(frames-1);
            const u32 limit = (kHistory-1)<<16;
            // The unsigned end check also rejects crossing either ring edge.
            if (g.position < limit && end < limit)
                grain_span<true,true,true>(g,memory,s.write,s.written,count,wet[0]+offset,wet[1]+offset);
            else
                grain_span<true,true>(g,memory,s.write,s.written,count,wet[0]+offset,wet[1]+offset);
        } else if (ready) {
            grain_span<true,false>(g,memory,s.write,s.written,count,wet[0]+offset,wet[1]+offset);
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
    i32 low = s.tone_memory[ch], dc = s.dc[ch];
    i32 tone = start.tone, recycle = start.recycle;
    const i32 tone_step = step.tone, recycle_step = step.recycle;
    i32* destination = memory+2*write+1-ch;
    for (unsigned i = 0; i < frames; ++i) {
        tone += tone_step;
        recycle += recycle_step;
        low += multiply(wet[i]-low, tone);
        wet[i] = low;
        dc += multiply(low-dc, 4000000);
        const i32 feedback = multiply(low-dc, recycle);
        *destination = bounded(half(input[i]) + feedback);
        destination += 2;
    }
    s.tone_memory[ch] = low;
    s.dc[ch] = dc;
}

// Sync the mirror only when a write touches the beginning or crosses the end.
__attribute__((noinline)) u32 advance_delay(i32* delay, u32 position, u32 length) {
    const u32 end = position+kFrames;
    if (position < kMirror) {
        for (u32 i = position; i < kMirror; ++i) delay[length+i] = delay[i];
    }
    if (end < length) return end;
    const u32 overflow = end-length;
    for (u32 i = 0; i < overflow; ++i) delay[i] = delay[length+i];
    return overflow;
}

u32 wait_for_delay(u32 written, u32 length) {
    return written < length ? length-written : 0;
}

template<bool Ready>
__attribute__((noinline)) void diffuse_pair(i32* first, i32* second,
                                            const i32* input, i32* output,
                                            u32 first_wait, u32 second_wait) {
    for (u32 i = 0; i < kFrames; ++i) {
        const i32 value = input[i];
        const i32 delayed_first = Ready || i >= first_wait ? first[i] : 0;
        const i32 middle = delayed_first-half(value);
        first[i] = bounded(value+half(middle));
        const i32 delayed_second = Ready || i >= second_wait ? second[i] : 0;
        const i32 out = delayed_second-half(middle);
        second[i] = bounded(middle+half(out));
        output[i] = out;
    }
}

template<bool Ready>
__attribute__((noinline)) void damping(const i32* source, i32& state, i32 coefficient,
                                      i32 step, i32* output, u32 wait) {
    i32 low = state;
    for (u32 i = 0; i < kFrames; ++i) {
        coefficient += step;
        const i32 value = Ready || i >= wait ? source[i] : 0;
        low += multiply(value-low,coefficient);
        output[i] = low;
    }
    state = low;
}

__attribute__((noinline)) void tank(i32* const* delay, i32 diffused[2][32], i32 damped[4][32],
                                   i32 gain, i32 gain_step) {
    i32* a_delay = delay[0];
    i32* b_delay = delay[1];
    i32* c_delay = delay[2];
    i32* d_delay = delay[3];
    for (u32 i = 0; i < kFrames; ++i) {
        gain += gain_step;
        const i32 a = damped[0][i], b = damped[1][i], c = damped[2][i], d = damped[3][i];
        const i32 average = half(half(a))+half(half(b))+half(half(c))+half(half(d));
        const i32 sum = average+average;
        const i32 l = half(diffused[0][i]), r = half(diffused[1][i]);
        a_delay[i] = bounded(l+multiply(a-sum,gain));
        b_delay[i] = bounded(r+multiply(b-sum,gain));
        c_delay[i] = bounded(l+multiply(c-sum,gain));
        d_delay[i] = bounded(r+multiply(d-sum,gain));
    }
}

__attribute__((noinline)) i32 space_mix(const i32* wet, const i32* first, const i32* second,
                                       i32* output, const Coefficients& start, const Coefficients& step) {
    i32 space = start.space, direct = start.direct, peak = 0;
    const i32 space_step = step.space, direct_step = step.direct;
    for (unsigned i = 0; i < kFrames; ++i) {
        space += space_step;
        direct += direct_step;
        const i32 room = (half(first[i])-half(second[i]))*3;
        const i32 sample = multiply(wet[i],direct)+multiply(room,space);
        output[i] = sample;
        const i32 magnitude = sample < 0 ? -sample : sample;
        if (magnitude > peak) peak = magnitude;
    }
    return peak;
}

template<bool Linear>
__attribute__((noinline)) void output_block(State& s, unsigned ch, const i32* input, i32* output) {
    i32 previous = half(s.output[ch]);
    for (unsigned i = 0; i < kFrames; ++i) {
        const i32 next = input[i];
        const i32 current = half(next);
        const i32 middle = previous+current;
        output[2*i] = Linear ? middle*32 : restore(middle);
        output[2*i+1] = Linear ? next*32 : restore(next);
        previous = current;
    }
    s.output[ch] = input[kFrames-1];
}

void output(State& s, unsigned ch, const i32* input, i32* channel, i32 peak) {
    // Both endpoints are below the knee, so each interpolated sample is too.
    if (peak <= kReturnKnee && s.output[ch] >= -kReturnKnee && s.output[ch] <= kReturnKnee)
        output_block<true>(s,ch,input,channel);
    else
        output_block<false>(s,ch,input,channel);
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
        if (count > kHistory-s.write) count = kHistory-s.write;
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
    // A fresh room fed only zero has no filter or delay history to advance.
    if (!s.reverb_written) {
        i32 signal = 0;
        for (unsigned i = 0; i < kFrames; ++i) signal |= wet[0][i] | wet[1][i];
        if (!signal) {
            for (unsigned i = 0; i < opxy::block_frames; ++i) left[i] = right[i] = 0;
            return;
        }
    }
    i32* const reverb = buffer.words+2*kHistory;
    i32* const diff = reverb+kRoomWords;
    const u32 written = s.reverb_written;
    i32* diff_delay[4], *room_delay[4];
    for (unsigned line = 0; line < 4; ++line) {
        diff_delay[line] = diff+delay_offset(kDiffuserLengths,line)+s.diffuser_position[line];
        room_delay[line] = reverb+delay_offset(kRoomLengths,line)+s.room_position[line];
    }
    if (written >= kDiffuserLengths[3]) {
        diffuse_pair<true>(diff_delay[0],diff_delay[1],wet[0],diffused[0],0,0);
        diffuse_pair<true>(diff_delay[2],diff_delay[3],wet[1],diffused[1],0,0);
    } else {
        diffuse_pair<false>(diff_delay[0],diff_delay[1],wet[0],diffused[0],
                            wait_for_delay(written,kDiffuserLengths[0]),wait_for_delay(written,kDiffuserLengths[1]));
        diffuse_pair<false>(diff_delay[2],diff_delay[3],wet[1],diffused[1],
                            wait_for_delay(written,kDiffuserLengths[2]),wait_for_delay(written,kDiffuserLengths[3]));
    }
    for (unsigned line = 0; line < 4; ++line) {
        if (written >= kRoomLengths[line])
            damping<true>(room_delay[line],s.damping[line],start.damping,step.damping,damped[line],0);
        else
            damping<false>(room_delay[line],s.damping[line],start.damping,step.damping,damped[line],
                           kRoomLengths[line]-written);
    }
    tank(room_delay,diffused,damped,start.gain,step.gain);
    const i32 left_peak = space_mix(wet[0],damped[0],damped[2],input[0],start,step);
    const i32 right_peak = space_mix(wet[1],damped[1],damped[3],input[1],start,step);
    for (unsigned line = 0; line < 4; ++line) {
        s.diffuser_position[line] = advance_delay(diff+delay_offset(kDiffuserLengths,line),
                                                  s.diffuser_position[line],kDiffuserLengths[line]);
        s.room_position[line] = advance_delay(reverb+delay_offset(kRoomLengths,line),
                                              s.room_position[line],kRoomLengths[line]);
    }
    const u32 room_ready = kRoomLengths[3];
    s.reverb_written = written+kFrames < room_ready ? written+kFrames : room_ready;
    output(s,0,input[0],left,left_peak);
    output(s,1,input[1],right,right_peak);
}
}
