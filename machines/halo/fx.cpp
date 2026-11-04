// Halo: a colored stereo FDN, tempo echo and shared octave-up cloud.
// Copyright (c) 2026 Kristaps Dreija. See LICENSE and PROVENANCE.md.
#include "core.h"
#include "fdn_tables.h"
#include "tables.h"
#include <opxy/q31.hpp>
namespace halo {
namespace {
using opxy::i32;
using opxy::u32;
using opxy::q31::multiply;
const i32 unit = 16777216;
const unsigned fdn_stride = 1040, fdn_length = 1024;
const unsigned diffuser_offset = 4160, predelay_offset = 4416,
               shimmer_offset = 6464, echo_offset = 7488;
static_assert(echo_offset + echo_frames <= buffer_words, "Halo scratch layout");
inline i32 clamp(i32 value, i32 low, i32 high) {
    return value < low ? low : value > high ? high : value;
}
inline i32 mul(i32 value, i32 gain) { return multiply(value, gain * 65536); }
inline i32 signed_word(u32 value) {
    return value <= 2147483647u ? static_cast<i32>(value)
                                : -static_cast<i32>(~value) - 1;
}
// Three eight-sample boxcars form a positive 22-tap FIR.
// Unsigned integrator arithmetic intentionally wraps.
inline void integrate(Cic &cic, u32 value) {
    cic.integrator[0] += value;
    cic.integrator[1] += cic.integrator[0];
    cic.integrator[2] += cic.integrator[1];
}
inline u32 comb(Cic &cic, u32 value) {
    for (unsigned i = 0; i < 3; ++i) {
        const u32 previous = cic.comb[i];
        cic.comb[i] = value;
        value -= previous;
    }
    return value;
}
inline i32 decimated(Cic &cic) {
    return signed_word(comb(cic, cic.integrator[2])) >> 7;
}
inline i32 interpolated(Cic &cic, u32 value) {
    integrate(cic, value);
    return signed_word(cic.integrator[2]) >> 6;
}
inline i32 soft(i32 value) {
    const bool negative = value < 0;
    const u32 magnitude = static_cast<u32>(negative ? -value : value);
    if (magnitude >= 2u * unit)
        return negative ? -unit : unit;
    const i32 a = static_cast<i32>(magnitude);
    const i32 curved = a - multiply(a, a * 32);
    return negative ? -curved : curved;
}
inline i32 parameter(unsigned index, i32 word) {
    if (index == 6)
        return word >= 1073741824 ? 32767 : 0;
    return word < 0 ? 0 : word >> 16;
}
inline i32 glide(i32 value, i32 target, unsigned divisor) {
    const i32 difference = target - value,
              step = difference / static_cast<i32>(divisor);
    return value + (step ? step : (difference > 0) - (difference < 0));
}
inline i32 table(const i32 *values, u32 phase) {
    const unsigned at = (phase >> 8) & 255u;
    return values[at] +
           (values[at + 1] - values[at]) * static_cast<i32>(phase & 255u) / 256;
}
i32 echo_delay(i32 control, u32 bpm) {
    if (!bpm)
        bpm = 1200;
    if (bpm < 400)
        bpm = 400;
    if (bpm > 3000)
        bpm = 3000;
    const u32 length = opxy::division_sixteenths[opxy::division(control)] *
                       rate_numerator * (600u / rate_denominator),
              denominator = bpm * 16u;
    const u32 delay = ((length / denominator) << 8) +
                      ((length % denominator) << 8) / denominator;
    const u32 maximum = (echo_frames - 3) * 256u;
    return static_cast<i32>(delay > maximum ? maximum
                            : delay < 512u  ? 512u
                                            : delay);
}
inline i32 sample(const State &state, const i32 *memory, unsigned offset,
                  unsigned length, unsigned age, unsigned stride = 1,
                  unsigned channel = 0) {
    if (!age || age > state.written || age >= length)
        return 0;
    return memory[offset + ((state.clock - age) & (length - 1)) * stride +
                  channel];
}
inline i32 tap(const State &state, const i32 *memory, unsigned offset,
               unsigned length, i32 delay, unsigned stride = 1,
               unsigned channel = 0) {
    const unsigned age = static_cast<u32>(delay) >> 8;
    if (age >= length)
        return 0;
    const i32 first =
        sample(state, memory, offset, length, age, stride, channel);
    if (!(delay & 255))
        return first;
    return first +
           mul(sample(state, memory, offset, length, age + 1, stride, channel) -
                   first,
               (delay & 255) * 128);
}
inline i32 echo_sample(const State &state, const i32 *memory, unsigned age,
                       unsigned channel) {
    if (!age || age > state.written || age >= echo_frames)
        return 0;
    const unsigned at = state.echo_position >= age
                            ? state.echo_position - age
                            : state.echo_position + echo_frames - age;
    const u32 packed = static_cast<u32>(memory[echo_offset + at]);
    return static_cast<opxy::i16>(channel ? packed >> 16 : packed & 65535u) *
           512;
}
inline i32 echo_tap(const State &state, const i32 *memory, i32 delay,
                    unsigned channel) {
    const unsigned age = static_cast<u32>(delay) >> 8;
    if (age >= echo_frames)
        return 0;
    const i32 first = echo_sample(state, memory, age, channel);
    if (!(delay & 255))
        return first;
    return first + mul(echo_sample(state, memory, age + 1, channel) - first,
                       (delay & 255) * 128);
}
inline i32 diffuse(State &state, i32 *memory, unsigned channel, i32 input) {
    const unsigned offset = diffuser_offset + channel * 128;
    const i32 delayed = sample(state, memory, offset, 128, channel ? 19 : 17);
    const i32 output = delayed - (input >> 1);
    memory[offset + (state.clock & 127u)] =
        clamp(input + (output >> 1), -3 * unit, 3 * unit);
    return output;
}
inline i32 pitch_up(const State &state, const i32 *memory, unsigned width) {
    const u32 phase = state.grain_phase >> 8, other = (phase + 32768u) & 65535u;
    const i32 first =
        static_cast<i32>(276u * 256u + ((65535u - phase) * width >> 8));
    const i32 second =
        static_cast<i32>(276u * 256u + ((65535u - other) * width >> 8));
    return mul(tap(state, memory, shimmer_offset, 1024, first),
               table(tables::window, phase)) +
           mul(tap(state, memory, shimmer_offset, 1024, second),
               table(tables::window, other));
}
struct Settings {
    i32 color, echo_color, input_gain, decay[4], damping[4], shimmer, seed_echo,
        wet_echo, feedback, duck, release, modulation, delays[4];
    unsigned predelay, grain_width;
};
inline i32 response(const opxy::i16 values[257][4], i32 control,
                    unsigned line) {
    const unsigned at = static_cast<u32>(control) >> 7;
    return values[at][line] +
           (values[at + 1][line] - values[at][line]) * (control & 127) / 128;
}
Settings settings(const State &state) {
    Settings result = {};
    const i32 distance = state.controls[0], bloom = state.controls[1],
              echo = state.controls[2], shimmer = state.controls[3],
              color = state.controls[4], hold = state.controls[6];
    const i32 tone = table(tables::tone, static_cast<u32>(color) * 2u);
    const i32 doubled_tone = clamp(2 * tone - mul(tone, tone), 0, 32767);
    result.color = doubled_tone * 65536;
    const i32 echo_tone = tone / 2 + 2400;
    result.echo_color = (2 * echo_tone - mul(echo_tone, echo_tone)) * 65536;
    result.input_gain = (32767 - hold) * 65536;
    const i32 pitched = mul(mul(shimmer, shimmer), 32767 - hold);
    const i32 raw_decay = 22938 + mul(bloom, 9437);
    const i32 base_decay = raw_decay + mul(32763 - raw_decay, mul(pitched, 28672)),
              decay = base_decay + mul(32763 - base_decay, hold);
    const i32 base_damping = 5500 + mul(color, 19000),
              damp = base_damping + mul(32735 - base_damping, hold);
    const i32 damping = clamp(2 * damp - mul(damp, damp), 0, 32767);
    const i32 reserve = 32767 - mul(mul(pitched, pitched), 1966);
    const i32 scale = 18022 + mul(distance, 22938);
    const unsigned lengths[4] = {659, 797, 953, 1151};
    for (unsigned i = 0; i < 4; ++i) {
        result.delays[i] = static_cast<i32>(lengths[i]) * scale / 256;
        result.decay[i] =
            mul(response(fdn_tables::decay, decay, i), reserve) * 65536;
        const i32 pole = response(fdn_tables::damping, damping, i);
        result.damping[i] = clamp(2 * pole - mul(pole, pole), 0, 32767) * 65536;
    }
    result.shimmer = mul(pitched, 16384) * 65536;
    result.seed_echo = mul(echo, 21000) * 65536;
    result.wet_echo = mul(echo, 20000) * 65536;
    result.feedback = mul(11000 + mul(bloom, 14000), echo) * 65536;
    result.duck = mul(bloom, 24576) * 65536;
    result.release = (24 + mul(32767 - bloom, 70)) * 65536;
    result.modulation = (2000 + mul(distance, 9000)) * 65536;
    result.predelay = 11u + (static_cast<u32>(distance) * 540u >> 15);
    result.grain_width = 193u + (static_cast<u32>(distance) * 386u >> 15);
    return result;
}
void network(State &__restrict state, i32 *__restrict memory,
             i32 *__restrict input, i32 *__restrict output,
             const Settings &__restrict config) {
    i32 echoes[2], seed[2];
    for (unsigned channel = 0; channel < 2; ++channel) {
        state.input_high[channel] +=
            mul(input[channel] - state.input_high[channel], 4248);
        const i32 high = input[channel] - state.input_high[channel];
        state.input_low[channel] +=
            multiply(high - state.input_low[channel], config.color);
        input[channel] = multiply(state.input_low[channel], config.input_gain);
        const i32 echo = echo_tap(state, memory, state.delay, channel);
        state.echo_low[channel] +=
            multiply(echo - state.echo_low[channel], config.echo_color);
        echoes[channel] = state.echo_low[channel];
        const i32 predelay = sample(state, memory, predelay_offset, 1024,
                                    config.predelay + channel * 4u, 2, channel);
        seed[channel] =
            diffuse(state, memory, channel,
                    (predelay * 3 >> 2) + (input[channel] >> 3) +
                        multiply(echoes[channel], config.seed_echo));
        memory[predelay_offset + (state.clock & 1023u) * 2 + channel] =
            input[channel];
    }
    state.phase += 352176u;
    const i32 modulation =
        multiply(table(tables::sine, state.phase >> 16), config.modulation) / 4;
    const i32 mod[4] = {modulation, modulation / 2, -modulation,
                        -modulation / 2};
    i32 delayed[4], damped[4];
    for (unsigned i = 0; i < 4; ++i) {
        delayed[i] = tap(state, memory, i * fdn_stride, fdn_length,
                         config.delays[i] + mod[i]);
        state.damping[i] +=
            multiply(delayed[i] - state.damping[i], config.damping[i]);
        // The DC servo retains six fractional bits and needs no audio multiply.
        state.feedback_dc[i] +=
            (state.damping[i] - (state.feedback_dc[i] >> 6)) >> 2;
        damped[i] = state.damping[i] - (state.feedback_dc[i] >> 6);
    }
    const i32 tail[2] = {(damped[0] + damped[1] - damped[2] + damped[3]) / 2,
                         (damped[0] - damped[1] + damped[2] + damped[3]) / 2};
    i32 shifted = 0;
    if (config.shimmer) {
        const i32 previous = clamp(tail[0], -unit, unit);
        state.shimmer_high += mul(previous - state.shimmer_high, 9227);
        i32 bright = previous - state.shimmer_high;
        for (unsigned i = 0; i < 2; ++i) {
            state.shimmer_low[i] += mul(bright - state.shimmer_low[i], 31913);
            bright = state.shimmer_low[i];
        }
        shifted = multiply(pitch_up(state, memory, config.grain_width),
                           config.shimmer);
        memory[shimmer_offset + (state.clock & 1023u)] = bright;
    } else {
        state.shimmer_high = state.shimmer_low[0] = state.shimmer_low[1] = 0;
        memory[shimmer_offset + (state.clock & 1023u)] = 0;
    }
    const i32 left = seed[0] + shifted, right = seed[1] - shifted;
    const i32 injection[4] = {(left + right) / 2, (left - right) / 2,
                              (right - left) / 2, (left + right) / 2};
    const i32 feedback[4] = {
        (damped[0] + damped[1] + damped[2] + damped[3]) / 2,
        (damped[0] - damped[1] + damped[2] - damped[3]) / 2,
        (damped[0] + damped[1] - damped[2] - damped[3]) / 2,
        (damped[0] - damped[1] - damped[2] + damped[3]) / 2};
    for (unsigned i = 0; i < 4; ++i)
        memory[i * fdn_stride + (state.clock & 1023u)] = clamp(
            multiply(feedback[i], config.decay[i]) + mul(injection[i], 14746),
            -unit, unit);
    const i32 a = input[0] < 0 ? -input[0] : input[0],
              b = input[1] < 0 ? -input[1] : input[1],
              magnitude = a > b ? a : b;
    const i32 release = state.peak >> 6;
    state.peak -= release ? release : state.peak ? 1 : 0;
    if (magnitude > state.peak)
        state.peak = magnitude;
    state.body += (state.peak - state.body) >> 8;
    const i32 transient =
        clamp((state.peak - state.body - (state.body >> 2)) * 4, 0, unit);
    state.duck +=
        multiply(transient - state.duck,
                 transient > state.duck ? 9845 * 65536 : clamp(config.release / 32768, 0, 32767) * 65536);
    const i32 duck_gain = 32767 - multiply(state.duck / 512, config.duck);
    u32 packed = 0;
    for (unsigned channel = 0; channel < 2; ++channel) {
        const i32 open = mul(tail[channel] + (seed[channel] >> 4), duck_gain);
        const i32 wet = mul(open, 19661) + multiply(echoes[channel], config.wet_echo);
        output[channel] = soft(wet + (wet >> 1));
        const i32 echo_write = soft(
            input[channel] + multiply(echoes[1 - channel], config.feedback));
        const i32 quantized = clamp(
            (echo_write + (echo_write < 0 ? 255 : 256)) >> 9, -32768, 32767);
        packed |= static_cast<u32>(
                      static_cast<opxy::u16>(static_cast<opxy::i16>(quantized)))
                  << (channel * 16);
    }
    memory[echo_offset + state.echo_position] = static_cast<i32>(packed);
    if (++state.echo_position == echo_frames)
        state.echo_position = 0;
    ++state.clock;
    if (state.written < echo_frames)
        ++state.written;
}
} // namespace
void initialize(State &state) { state = State(); }
void process(State &state, opxy::Buffer buffer,
             const opxy::Transport &transport, opxy::Channel left,
             opxy::Channel right, opxy::Parameters parameters) {
    if (!buffer.words || buffer.count < buffer_words) {
        for (unsigned i = 0; i < opxy::block_frames; ++i)
            left[i] = right[i] = 0;
        return;
    }
    if (!state.ready) {
        for (unsigned i = 0; i < 7; ++i)
            state.controls[i] = parameter(i, parameters[i]);
        state.delay = echo_delay(parameters[5], transport.bpm_x10);
        state.ready = 1;
    }
    for (unsigned i = 0; i < 7; ++i) {
        const i32 target = parameter(i, parameters[i]);
        const i32 delta = mul(target - state.controls[i], i == 6 ? 3858 : 1987);
        state.controls[i] = clamp(
            state.controls[i] + (delta ? delta
                                       : (target > state.controls[i]) -
                                             (target < state.controls[i])),
            0, 32767);
    }
    const Settings config = settings(state);
    const i32 target_delay = echo_delay(parameters[5], transport.bpm_x10);
    const u32 grain_step = 16777216u / config.grain_width;
    i32 wet[2][opxy::block_frames / 8];
    for (unsigned channel = 0; channel < 2; ++channel) {
        const i32 *source = channel ? right : left;
        Cic down = state.down[channel];
        for (unsigned i = 0; i < opxy::block_frames; i += 8) {
            for (unsigned j = 0; j < 8; ++j)
                integrate(down, static_cast<u32>(source[i + j] / 512));
            wet[channel][i / 8] = decimated(down);
        }
        state.down[channel] = down;
    }
    for (unsigned i = 0; i < opxy::block_frames / 8; ++i) {
        state.delay = glide(state.delay, target_delay, 512);
        i32 input[2] = {wet[0][i], wet[1][i]}, output[2];
        network(state, buffer.words, input, output, config);
        wet[0][i] = output[0];
        wet[1][i] = output[1];
        state.grain_phase = (state.grain_phase + grain_step) & 16777215u;
    }
    for (unsigned channel = 0; channel < 2; ++channel) {
        i32 *destination = channel ? right : left;
        Cic up = state.up[channel];
        for (unsigned i = 0; i < opxy::block_frames; i += 8) {
            const u32 value = comb(up, static_cast<u32>(wet[channel][i / 8]));
            destination[i] = clamp(interpolated(up, value), -unit + 1, unit - 1) * 128;
            for (unsigned j = 1; j < 8; ++j)
                destination[i + j] = clamp(interpolated(up, 0), -unit + 1, unit - 1) * 128;
        }
        state.up[channel] = up;
    }
}
} // namespace halo
