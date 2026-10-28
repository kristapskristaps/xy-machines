#include "granular.h"
#include "window.h"

namespace granular {
namespace {
using opxy::i32;
using opxy::u32;
const u32 root_increment = 25480370u; // C4, matching the SDK's sample benchmark.
const u32 maximum_rate = 256u << 16;

u32 control(i32 value) {
    if (value < 0) return 0;
    return (static_cast<u32>(value) + 32768u) >> 16;
}
u32 random_word(Voice& voice) {
    voice.random = voice.random * 1664525u + 1013904223u;
    return voice.random;
}
u32 scale(u32 frames, u32 fraction) {
    return (frames >> 15) * fraction + (((frames & 32767u) * fraction) >> 15);
}
u32 high_product(u32 a, u32 b) {
    const u32 low = (a & 65535u) * (b & 65535u);
    const u32 middle = (a >> 16) * (b & 65535u) + (low >> 16);
    const u32 carry = (middle & 65535u) + (a & 65535u) * (b >> 16);
    return (a >> 16) * (b >> 16) + (middle >> 16) + (carry >> 16);
}
u32 multiply_rate(u32 a, u32 b) {
    const u32 high = high_product(a, b);
    if (high >= 256u) return maximum_rate;
    const u32 value = (high << 16) | ((a * b) >> 16);
    return value > maximum_rate ? maximum_rate : value;
}
u32 divide_fraction(u32 numerator, u32 denominator) {
    const u32 whole = numerator / denominator;
    u32 remainder = numerator % denominator, fraction = 0;
    for (unsigned i = 0; i < 16; ++i) {
        remainder <<= 1;
        fraction <<= 1;
        if (remainder >= denominator) { remainder -= denominator; fraction |= 1; }
    }
    return (whole << 16) | fraction;
}
u32 tuning(i32 cents) {
    static const u32 semitones[13] = {
        65536, 69433, 73562, 77936, 82570, 87480, 92682,
        98193, 104032, 110218, 116772, 123715, 131072
    };
    if (cents < -9600) cents = -9600;
    if (cents > 9600) cents = 9600;
    i32 octave = cents / 1200, remainder = cents % 1200;
    if (remainder < 0) { remainder += 1200; --octave; }
    const u32 index = static_cast<u32>(remainder) / 100u;
    const u32 part = static_cast<u32>(remainder) % 100u;
    const u32 ratio = semitones[index] + (semitones[index + 1] - semitones[index]) * part / 100u;
    return octave < 0 ? ratio >> static_cast<u32>(-octave) : ratio << static_cast<u32>(octave);
}
i32 window(u32 phase) {
    const u32 index = phase >> 16;
    const i32 a = hann[index], b = hann[index + 1];
    return a + ((b - a) * static_cast<i32>(phase & 65535u)) / 65536;
}
void advance(u32& frame, u32& fraction, u32 whole, u32 part, u32 length, bool backwards) {
    if (backwards) {
        if (fraction < part) { fraction += 65536u - part; ++whole; }
        else fraction -= part;
        if (whole == length) whole = 0;
        frame = frame >= whole ? frame - whole : length - (whole - frame);
    } else {
        fraction += part;
        if (fraction >= 65536u) { fraction -= 65536u; ++whole; }
        frame += whole;
        if (frame >= length) frame -= length;
    }
}
i32 read(const Voice& voice, const Grain& grain, u32 channel) {
    const u32 length = voice.end - voice.begin;
    const u32 next = grain.frame + 1 == length ? 0 : grain.frame + 1;
    const u32 a_frame = voice.reverse ? voice.end - 1 - grain.frame : voice.begin + grain.frame;
    const u32 b_frame = voice.reverse ? voice.end - 1 - next : voice.begin + next;
    const i32 a = opxy::sample_at(voice.sample, a_frame, channel);
    const i32 b = opxy::sample_at(voice.sample, b_frame, channel);
    // Q15 interpolation keeps the largest signed product below 2^31.
    return a + ((b - a) * static_cast<i32>(grain.fraction >> 1)) / 32768;
}
void spawn(Voice& voice, const u32* controls, u32 duration, u32 rate) {
    unsigned slot = 0;
    while (slot < grain_count && voice.grains[slot].remaining) ++slot;
    if (slot == grain_count) return;
    Grain& grain = voice.grains[slot];
    const u32 length = voice.end - voice.begin;
    u32 position = scale(length - 1, controls[0]) + voice.scan_frame;
    if (position >= length) position -= length;
    const u32 radius = scale(length - 1, controls[3]);
    if (radius) {
        const u32 offset = scale(radius, random_word(voice) >> 17);
        if (random_word(voice) & 0x80000000u)
            position = position >= offset ? position - offset : length - (offset - position);
        else { position += offset; if (position >= length) position -= length; }
    }
    if (controls[5]) {
        const i32 spread = static_cast<i32>(controls[5] * 1200u >> 15);
        const i32 detune = static_cast<i32>(random_word(voice) >> 16) - 32768;
        rate = multiply_rate(rate, tuning(detune * spread / 32768));
    }
    grain.frame = position;
    grain.fraction = voice.scan_fraction;
    grain.whole_step = (rate >> 16) % length;
    grain.fractional_step = rate & 65535u;
    grain.phase = 0;
    grain.phase_step = (1u << 24) / duration;
    grain.remaining = duration;
    const i32 pan = (static_cast<i32>(random_word(voice) >> 16) - 32768) *
        static_cast<i32>(controls[6]) / 32768;
    grain.left = pan > 0 ? 32768 - pan : 32768;
    grain.right = pan < 0 ? 32768 + pan : 32768;
}
i32 output(i32 sum, u32 normalization, u32 level, u32 region_level) {
    i32 value = sum * static_cast<i32>(normalization) / 4096;
    if (value > 32767) value = 32767;
    if (value < -32768) value = -32768;
    value = value * static_cast<i32>(level) / 32768;
    const u32 magnitude = static_cast<u32>(value < 0 ? -value : value);
    const u32 scaled = magnitude * (region_level >> 15) >> 12;
    const i32 bounded = static_cast<i32>(scaled > 32768u ? 32768u : scaled);
    return (value < 0 ? -bounded : bounded) * 32768;
}
}

void start(Voice& voice) {
    for (unsigned i = 0; i < grain_count; ++i) voice.grains[i] = Grain{};
    voice.scan_frame = voice.scan_fraction = voice.countdown = 0;
    voice.random = 0x9e3779b9u ^ voice.sample.frames;
    voice.configured = false;
}
void initialize(Voice& voice, const opxy::SampleView& sample) {
    voice = Voice{};
    voice.sample = sample;
    start(voice);
}
void render_sample(Voice& voice, const opxy::SampleBlock& block) {
    const u32 channels = block.output_channels == 2 ? 2u : 1u;
    for (u32 i = 0; i < 64 * channels; ++i) block.output[i] = 0;
    const opxy::SampleView& sample = voice.sample;
    if (!sample.data || sample.frames < 2 || sample.frames > 0x3fffffffu ||
        sample.rate < 8000 || sample.rate > 192000 ||
        (sample.channels != 1 && sample.channels != 2)) { start(voice); return; }
    const u32 begin = block.region.begin < sample.frames ? block.region.begin : sample.frames;
    const u32 end = block.region.end < sample.frames ? block.region.end : sample.frames;
    if (end <= begin || end - begin < 2) { start(voice); return; }
    if (!voice.configured || voice.begin != begin || voice.end != end || voice.reverse != block.region.reverse) {
        start(voice);
        voice.begin = begin; voice.end = end; voice.reverse = block.region.reverse;
        voice.configured = true;
    }
    u32 controls[8];
    for (unsigned i = 0; i < 8; ++i) controls[i] = control(block.parameters[i]);
    // Size: 6..250 ms, density: 1..8 overlapping grains.
    const u32 duration = 265u + (10760u * controls[1] >> 15);
    const u32 overlap_q8 = 256u + (1792u * controls[2] >> 15);
    const u32 interval = (duration * 256u + overlap_q8 - 1) / overlap_q8;
    if (voice.countdown > interval) voice.countdown = interval;
    const u32 normalization = overlap_q8 > 512u ? 2097152u / overlap_q8 : 4096u;
    const u32 source_rate = divide_fraction(sample.rate, 44100u);
    const u32 pitch_rate = divide_fraction(block.pitch[0], root_increment);
    const i32 transpose = block.track_transpose < -36 ? -36 : block.track_transpose > 36 ? 36 : block.track_transpose;
    const i32 drum = block.drum_transpose < -120 ? -120 : block.drum_transpose > 120 ? 120 : block.drum_transpose;
    const i32 saved = block.region.pitch_cents < -9600 ? -9600 : block.region.pitch_cents > 9600 ? 9600 : block.region.pitch_cents;
    const u32 rate = multiply_rate(multiply_rate(source_rate, pitch_rate), tuning(saved + (transpose + drum) * 100));
    const i32 scan = static_cast<i32>(controls[4]) - 16384;
    const u32 scan_rate = multiply_rate(source_rate, static_cast<u32>(scan < 0 ? -scan : scan) * 8u);
    const u32 length = end - begin;
    const u32 scan_whole = (scan_rate >> 16) % length, scan_part = scan_rate & 65535u;
    for (unsigned i = 0; i < 64; ++i) {
        if (!voice.countdown) { spawn(voice, controls, duration, rate); voice.countdown = interval; }
        --voice.countdown;
        i32 left = 0, right = 0;
        for (unsigned g = 0; g < grain_count; ++g) {
            Grain& grain = voice.grains[g];
            if (!grain.remaining) continue;
            const i32 weight = window(grain.phase);
            const i32 l = read(voice, grain, 0) * weight / 32768;
            const i32 r = sample.channels == 2 ? read(voice, grain, 1) * weight / 32768 : l;
            left += l * grain.left / 32768;
            right += r * grain.right / 32768;
            advance(grain.frame, grain.fraction, grain.whole_step, grain.fractional_step, length, false);
            grain.phase += grain.phase_step;
            --grain.remaining;
        }
        if (channels == 1) block.output[i] = output((left + right) / 2, normalization, controls[7], block.region.level);
        else {
            block.output[2 * i] = output(left, normalization, controls[7], block.region.level);
            block.output[2 * i + 1] = output(right, normalization, controls[7], block.region.level);
        }
        advance(voice.scan_frame, voice.scan_fraction, scan_whole, scan_part, length, scan < 0);
    }
}
}
