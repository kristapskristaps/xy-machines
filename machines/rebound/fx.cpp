#include "core.h"
#include "tables.h"
#include <opxy/q31.hpp>

namespace rebound {
namespace {
using opxy::i32;
using opxy::u32;
using opxy::i64;
using opxy::q31::multiply;
const i32 maximum = opxy::q31::maximum;
const i32 shortest = 512 * 256;
const i32 longest = (line_frames - 2) * 256;

i32 control(i32 value) { return value < 0 ? 0 : value; }

// Match stock's Q31 -> Q15 enum quantization, including saved boundary values.
u32 choice(i32 value, u32 count) {
    return ((static_cast<u32>(control(value)) >> 16) * count) >> 15;
}

i32 glide(i32 value, i32 target) {
    const i32 difference = target - value;
    const i32 step = difference / 512;
    return value + (step ? step : (difference > 0) - (difference < 0));
}

// Unit slope below -12 dBFS, a quadratic knee, then a -6 dBFS ceiling.
// Wide input accommodates input + gain above unity without Q31 wraparound.
i32 saturate(i64 value) {
    const i64 magnitude = value < 0 ? -value : value;
    const i32 knee = 536870912, ceiling = 1073741823;
    i32 result;
    if (magnitude <= knee) result = static_cast<i32>(magnitude);
    else if (magnitude >= 3LL * knee) result = ceiling;
    else {
        const i32 x = static_cast<i32>(magnitude - knee);
        result = knee + x - multiply(x, x);
        if (result > ceiling) result = ceiling;
    }
    return value < 0 ? -result : result;
}

i32 blend(i32 a, i32 b, i32 amount) {
    return a + multiply(b - a, amount);
}

i32 delay_for(i32 size, i32 fine, u32 tempo) {
    const u32 bpm = tempo ? tempo : 1200;
    const u32 length = opxy::division_sixteenths[opxy::division(control(size))] * 1653750;
    const i32 milliseconds = static_cast<i32>(choice(fine, 101)) - 50;
    const u32 whole = length / bpm;
    if (whole > line_frames + 2205) return longest;
    // Eight fractional bits, without a wide divide or overflowing remainder << 8.
    u32 remainder = length % bpm, fraction = 0;
    for (unsigned bit = 0; bit < 8; ++bit) {
        fraction *= 2;
        if (remainder >= bpm - remainder) {
            remainder -= bpm - remainder;
            ++fraction;
        } else remainder += remainder;
    }
    const i32 samples = static_cast<i32>(whole * 256 + fraction) + milliseconds * 441 * 256 / 10;
    return samples < shortest ? shortest : samples > longest ? longest : samples;
}

i32 sample(const i32* line, const State& state, u32 age) {
    return age > state.written ? 0 : line[(state.write + line_frames - age) & (line_frames - 1)];
}

i32 tap(const i32* line, const State& state) {
    const u32 age = static_cast<u32>(state.delay) >> 8;
    const i32 fraction = state.delay & 255;
    const i32 a = sample(line, state, age), b = sample(line, state, age + 1);
    // Both samples are bounded below +/- 0.5, so their difference cannot overflow.
    const i32 difference = b - a;
    return a + (difference / 256) * fraction + ((difference % 256) * fraction) / 256;
}

i32 filtered(State& state, unsigned side, i32 input) {
    state.dc[side] += multiply(input - state.dc[side], state.highpass);
    const i32 high = input - state.dc[side];
    // HP transients may approach +/- 1.0. Half-scale LP state keeps subtraction safe.
    const i32 half = opxy::q31::half(high);
    state.low[side] += multiply(half - state.low[side], state.lowpass);
    return state.low[side];
}
}

void initialize(State& state) { state = State(); }

void process(State& state, opxy::Buffer buffer, const opxy::Transport& transport,
             opxy::Channel left, opxy::Channel right, opxy::Parameters parameters) {
    if (!buffer.words || buffer.count < buffer_words) {
        for (unsigned i = 0; i < opxy::block_frames; ++i) left[i] = right[i] = 0;
        return;
    }
    const i32 delay = delay_for(parameters[0], parameters[4], transport.bpm_x10);
    const i32 feedback = tables::feedback_half[choice(parameters[1], 191)];
    const i32 pingpong = control(parameters[2]), level = control(parameters[3]);
    const i32 highpass = tables::highpass[choice(parameters[5], 33)];
    const i32 lowpass = tables::lowpass[choice(parameters[6], 33)];
    if (!state.ready) {
        state.delay = delay;
        state.feedback_half = feedback;
        state.pingpong = pingpong;
        state.level = level;
        state.highpass = highpass;
        state.lowpass = lowpass;
        state.ready = 1;
    }
    i32* const l = buffer.words;
    i32* const r = buffer.words + line_frames;
    for (unsigned i = 0; i < opxy::block_frames; ++i) {
        state.delay = glide(state.delay, delay);
        state.feedback_half = glide(state.feedback_half, feedback);
        state.pingpong = glide(state.pingpong, pingpong);
        state.level = glide(state.level, level);
        state.highpass = glide(state.highpass, highpass);
        state.lowpass = glide(state.lowpass, lowpass);
        const i32 out_l = tap(l, state), out_r = tap(r, state);
        const i32 fb_l = filtered(state, 0, out_l), fb_r = filtered(state, 1, out_r);
        // Attenuate injection before blending: arbitrary full-scale send inputs
        // must not overflow when L and R have opposite signs.
        const i32 in_l = opxy::q31::half(left[i]), in_r = opxy::q31::half(right[i]);
        const i32 mono = opxy::q31::half(in_l) + opxy::q31::half(in_r);
        // The filter stores half-scale samples and gain is stored half-scale;
        // multiplying then widening by four restores the actual loop gain.
        l[state.write] = saturate(static_cast<i64>(blend(in_l, mono, state.pingpong)) +
            4LL * multiply(blend(fb_l, fb_r, state.pingpong), state.feedback_half));
        r[state.write] = saturate(static_cast<i64>(multiply(in_r, maximum - state.pingpong)) +
            4LL * multiply(blend(fb_r, fb_l, state.pingpong), state.feedback_half));
        state.write = (state.write + 1) & (line_frames - 1);
        if (state.written < line_frames) ++state.written;
        left[i] = multiply(out_l, state.level);
        right[i] = multiply(out_r, state.level);
    }
}
}
