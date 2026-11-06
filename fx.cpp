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
i32 difference(i32 a, i32 b) { return a - b; }
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

// Send scratch is never cleared by stock. Read only this instance's history.
void read(const i32* memory, const State& s, u32 position, i32& left, i32& right) {
    const u32 index = (position >> 16) & (kHistory - 1);
    const u32 next = (index + 1) & (kHistory - 1);
    const u32 age = (s.write - index) & (kHistory - 1);
    const u32 next_age = (s.write - next) & (kHistory - 1);
    const bool valid = age && age <= s.written;
    const bool next_valid = next_age && next_age <= s.written;
    left = interpolate(valid ? memory[index] : 0, next_valid ? memory[next] : 0, position & 65535);
    right = interpolate(valid ? memory[kHistory + index] : 0, next_valid ? memory[kHistory + next] : 0,
                        position & 65535);
}

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
    const u32 delay = length + 441 + random(s) % (length + 1);
    g.position = ((s.write + kHistory - delay) & (kHistory - 1)) << 16;
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
    g.pan = 4096 + static_cast<i32>(random(s) % 24576);
    s.countdown = length * 32767 / (65534 + control(s.controls[1]) * 4);
}

i32 diffuse(i32 value, i32* line, u32 write, u32 written, u32 length) {
    const i32 delayed = written < length ? 0 : line[(write - length) & (kDiffuser - 1)];
    const i32 out = difference(delayed, half(value));
    line[write & (kDiffuser - 1)] = bounded(add(value, half(out)));
    return out;
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

void tick(State& s, i32* memory, i32 a, i32 b, const Coefficients& c) {
    if (!s.countdown) launch(s);
    --s.countdown;
    i32 wet[2] = {0, 0};
    for (unsigned j = 0; j < 6; ++j) {
        Grain& g = s.grains[j];
        if (!g.remaining) continue;
        const u32 p = g.phase >> 17;
        const i32 arch = static_cast<i32>((p * (32768 - p)) >> 13);
        const u32 squared = static_cast<u32>(arch * arch) >> 15;
        const opxy::u16 window = static_cast<opxy::u16>(squared < 32768 ? squared : 32767);
        // Window and pan fit unsigned halfwords. Their unshifted product is
        // already four times the Q31 gain: no wide audio product is needed.
        const opxy::u16 l_pan = static_cast<opxy::u16>(65536 - g.pan);
        const opxy::u16 r_pan = static_cast<opxy::u16>(32768 + g.pan);
        const i32 l_gain = static_cast<i32>(static_cast<u32>(window) * l_pan >> 2);
        const i32 r_gain = static_cast<i32>(static_cast<u32>(window) * r_pan >> 2);
        i32 l, r;
        read(memory, s, g.position, l, r);
        wet[0] = add(wet[0], multiply(l, l_gain));
        wet[1] = add(wet[1], multiply(r, r_gain));
        g.position = (g.position + static_cast<u32>(g.speed)) & 0x7fffffffu;
        g.phase += g.increment;
        --g.remaining;
    }
    for (unsigned ch = 0; ch < 2; ++ch) {
        s.tone_memory[ch] = add(s.tone_memory[ch], multiply(difference(wet[ch], s.tone_memory[ch]), c.tone));
        wet[ch] = s.tone_memory[ch];
        s.dc[ch] = add(s.dc[ch], multiply(difference(wet[ch], s.dc[ch]), 4000000));
        s.feedback[ch] = multiply(difference(wet[ch], s.dc[ch]), c.recycle);
    }
    memory[s.write] = bounded(add(half(a), s.feedback[1]));
    memory[kHistory + s.write] = bounded(add(half(b), s.feedback[0]));
    s.write = (s.write + 1) & (kHistory - 1);
    if (s.written < kHistory) ++s.written;

    i32* const tank = memory + 2 * kHistory;
    i32* const diff = tank + 4 * kReverb;
    const i32 l = diffuse(diffuse(wet[0], diff, s.reverb_write, s.reverb_written, kDiffLengths[0]),
                          diff + kDiffuser, s.reverb_write, s.reverb_written, kDiffLengths[1]);
    const i32 r = diffuse(diffuse(wet[1], diff + 2 * kDiffuser, s.reverb_write, s.reverb_written, kDiffLengths[2]),
                          diff + 3 * kDiffuser, s.reverb_write, s.reverb_written, kDiffLengths[3]);
    i32 taps[4];
    i32 average = 0;
    for (unsigned j = 0; j < 4; ++j) {
        const i32 tap = s.reverb_written < kLengths[j] ? 0 :
            tank[j * kReverb + ((s.reverb_write - kLengths[j]) & (kReverb - 1))];
        s.damping[j] = add(s.damping[j], multiply(difference(tap, s.damping[j]), c.damping));
        taps[j] = s.damping[j];
        average = add(average, half(half(taps[j])));
    }
    // Orthogonal Householder feedback, with loss on every traversal.
    const i32 sum_half = add(average, average);
    for (unsigned j = 0; j < 4; ++j) {
        tank[j * kReverb + (s.reverb_write & (kReverb - 1))] =
            bounded(add(half(j & 1 ? r : l), multiply(difference(taps[j], sum_half), c.gain)));
    }
    // The tank's injection and stereo projection reduce its level. Restore
    // that level before the Space crossfade; maximum Space is all reverb.
    const i32 room_l = difference(half(taps[0]), half(taps[2])) * 3;
    const i32 room_r = difference(half(taps[1]), half(taps[3])) * 3;
    s.output[0] = add(multiply(wet[0], c.direct), multiply(room_l, c.space));
    s.output[1] = add(multiply(wet[1], c.direct), multiply(room_r, c.space));
    ++s.reverb_write;
    if (s.reverb_written < kReverb) ++s.reverb_written;
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
            const i32 delta = target - s.controls[j];
            const i32 step = delta / 8;
            s.controls[j] += step ? step : (delta > 0) - (delta < 0);
        }
    }
    s.started = 1;
    const Coefficients end = coefficients(s);
    Coefficients c = started ? previous : end;
    const Coefficients step = c.step_to(end);
    for (unsigned i = 0; i < opxy::block_frames; ++i) {
        const i32 input[2] = {left[i] / 8, right[i] / 8};
        for (unsigned ch = 0; ch < 2; ++ch) {
            s.antialias[ch] = add(s.antialias[ch], multiply(difference(input[ch], s.antialias[ch]), 750000000));
            s.input[ch] = add(s.input[ch], multiply(difference(s.antialias[ch], s.input[ch]), 750000000));
        }
        if (!s.half) {
            c.advance(step);
            s.previous[0] = s.output[0];
            s.previous[1] = s.output[1];
            tick(s, buffer.words, s.input[0], s.input[1], c);
            left[i] = restore(add(half(s.previous[0]), half(s.output[0])));
            right[i] = restore(add(half(s.previous[1]), half(s.output[1])));
        } else {
            left[i] = restore(s.output[0]);
            right[i] = restore(s.output[1]);
        }
        s.half ^= 1;
    }
}
}
