#include "core.h"
#include <opxy/q31.hpp>

namespace lustre {
namespace {
using opxy::i32;
using opxy::u32;
using opxy::q31::add;
using opxy::q31::half;
using opxy::q31::multiply;
const i32 kOne = opxy::q31::maximum;
const u32 kLengths[4] = {1499, 1877, 2381, 3011};
const u32 kDiffLengths[4] = {113, 173, 241, 337};

i32 negate(i32 value) { return value == -kOne - 1 ? kOne : -value; }
i32 difference(i32 a, i32 b) { return add(a, negate(b)); }
i32 interpolate(i32 a, i32 b, u32 fraction) {
    const i32 weight = static_cast<i32>(fraction << 15);
    return add(multiply(a, kOne - weight), multiply(b, weight));
}
i32 follow(i32 value, i32 target) {
    const i32 step = (target - value) / 256;
    return value + (step ? step : (target > value) - (target < value));
}
u32 random(State& s) {
    s.random ^= s.random << 13;
    s.random ^= s.random >> 17;
    s.random ^= s.random << 5;
    return s.random;
}
u32 control(i32 value) { return value < 0 ? 0 : static_cast<u32>(value) >> 16; }

// Send scratch is never cleared by stock. Read only this instance's history.
i32 history(const i32* line, const State& s, u32 index) {
    const u32 age = (s.write - index) & (kHistory - 1);
    return !age || age > s.written ? 0 : line[index];
}
i32 read(const i32* line, const State& s, u32 position) {
    const u32 index = (position >> 16) & (kHistory - 1);
    return interpolate(history(line, s, index), history(line, s, (index + 1) & (kHistory - 1)),
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
    line[write & (kDiffuser - 1)] = add(value, half(out));
    return out;
}

void tick(State& s, i32* memory, i32 a, i32 b) {
    if (!s.countdown) launch(s);
    --s.countdown;
    i32 wet[2] = {0, 0};
    for (unsigned j = 0; j < 6; ++j) {
        Grain& g = s.grains[j];
        if (!g.remaining) continue;
        const u32 p = g.phase >> 17;
        const i32 arch = static_cast<i32>((p * (32768 - p)) >> 13);
        const u32 squared = static_cast<u32>(arch * arch) >> 15;
        const i32 envelope = static_cast<i32>((squared < 32768 ? squared : 32767) << 16);
        const i32 l = half(half(multiply(read(memory, s, g.position), envelope)));
        const i32 r = half(half(multiply(read(memory + kHistory, s, g.position), envelope)));
        const i32 pan = g.pan * 65536;
        wet[0] = add(wet[0], multiply(l, kOne - half(pan)));
        wet[1] = add(wet[1], multiply(r, add(1073741824, half(pan))));
        g.position = (g.position + static_cast<u32>(g.speed)) & 0x7fffffffu;
        g.phase += g.increment;
        --g.remaining;
    }
    const i32 decay = s.controls[2];
    const i32 tone = add(100000000, multiply(s.controls[7], 1400000000));
    const i32 recycle = multiply(decay, 1000000000);
    for (unsigned ch = 0; ch < 2; ++ch) {
        s.tone_memory[ch] = add(s.tone_memory[ch], multiply(difference(wet[ch], s.tone_memory[ch]), tone));
        wet[ch] = s.tone_memory[ch];
        s.dc[ch] = add(s.dc[ch], multiply(difference(wet[ch], s.dc[ch]), 4000000));
        s.feedback[ch] = multiply(difference(wet[ch], s.dc[ch]), recycle);
    }
    memory[s.write] = add(half(a), s.feedback[1]);
    memory[kHistory + s.write] = add(half(b), s.feedback[0]);
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
    const i32 damping = add(180000000, multiply(s.controls[7], 1100000000));
    for (unsigned j = 0; j < 4; ++j) {
        const i32 tap = s.reverb_written < kLengths[j] ? 0 :
            tank[j * kReverb + ((s.reverb_write - kLengths[j]) & (kReverb - 1))];
        s.damping[j] = add(s.damping[j], multiply(difference(tap, s.damping[j]), damping));
        taps[j] = s.damping[j];
        average = add(average, half(half(taps[j])));
    }
    // Orthogonal Householder feedback, with loss on every traversal.
    const i32 gain = add(1350000000, multiply(decay, 690000000));
    const i32 sum_half = add(average, average);
    for (unsigned j = 0; j < 4; ++j) {
        tank[j * kReverb + (s.reverb_write & (kReverb - 1))] =
            add(half(j & 1 ? r : l), multiply(difference(taps[j], sum_half), gain));
    }
    const i32 room_l = difference(half(taps[0]), half(taps[2]));
    const i32 room_r = difference(half(taps[1]), half(taps[3]));
    const i32 space = s.controls[3];
    s.output[0] = add(multiply(wet[0], kOne - half(space)), multiply(room_l, space));
    s.output[1] = add(multiply(wet[1], kOne - half(space)), multiply(room_r, space));
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
    i32 target[8];
    for (unsigned j = 0; j < 8; ++j) {
        target[j] = parameters[j] < 0 ? 0 : parameters[j];
        if (!s.started) s.controls[j] = target[j];
    }
    s.started = 1;
    for (unsigned i = 0; i < opxy::block_frames; ++i) {
        const i32 input[2] = {left[i], right[i]};
        for (unsigned ch = 0; ch < 2; ++ch) {
            s.antialias[ch] = add(s.antialias[ch], multiply(difference(input[ch], s.antialias[ch]), 750000000));
            s.input[ch] = add(s.input[ch], multiply(difference(s.antialias[ch], s.input[ch]), 750000000));
        }
        if (!s.half) {
            for (unsigned j = 0; j < 8; ++j) s.controls[j] = follow(s.controls[j], target[j]);
            s.previous[0] = s.output[0];
            s.previous[1] = s.output[1];
            tick(s, buffer.words, s.input[0], s.input[1]);
            left[i] = add(half(s.previous[0]), half(s.output[0]));
            right[i] = add(half(s.previous[1]), half(s.output[1]));
        } else {
            left[i] = s.output[0];
            right[i] = s.output[1];
        }
        s.half ^= 1;
    }
}
}
