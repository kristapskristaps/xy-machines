#include "core.h"
#include "tables.h"
#include <opxy/q31.hpp>

namespace gentape {
namespace {
using opxy::i32;
using opxy::u32;
const i32 one = 134217728; // Internal Q27 audio leaves headroom for
                           // interpolation and filters.
const i32 full = 2147483647;
const i32 base_delay = 512 * 65536;
const i32 half = 1073741824;

i32 mul(i32 a, i32 b) { return opxy::q31::multiply(a, b); }
i32 blend(i32 a, i32 b, i32 fraction) { return a + mul(b - a, fraction); }
i32 coefficient(i32 value, i32 target) {
    const i32 difference = target - value;
    return difference >= -32 && difference <= 32 ? target : blend(value, target, 134217728);
}
u32 random(State &s) {
    u32 x = s.random;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return s.random = x;
}
i32 noise_audio(State &s) {
    s.noise_random =
        static_cast<u32>(opxy::q31::wide(static_cast<i32>(s.noise_random), 1664525)) + 1013904223u;
    return static_cast<i32>(s.noise_random >> 4) - one;
}
i32 random_audio(State &s) { return static_cast<i32>(random(s) >> 4) - one; }
i32 smooth_random(State &s, i32 *memory, i32 rate) {
    memory[0] = blend(memory[0], random_audio(s), rate);
    memory[1] = blend(memory[1], memory[0], rate);
    return memory[1];
}

// Frequency-shaped saturation: two low-cost record poles focus nonlinear
// compression on the body of the sound. The complementary band retains its
// detail. Removing high-frequency energy BEFORE the nonlinearity controls
// foldback; filtering a distorted output could not undo it.
i32 shape(i32 input) {
    // The record poles are convex, and maximum drive is 3.5. The table's
    // +/-8 domain therefore also covers every full-scale transient.
    const u32 position = static_cast<u32>(input + 8 * one), index = position >> 20;
    return blend(tanh_table[index], tanh_table[index + 1], static_cast<i32>((position & 1048575) << 11));
}
void saturation_block(i32 *input, i32 *line, State &s, u32 channel, i32 drive_step, i32 makeup_step,
                      i32 amount_step) {
    i32 record0 = s.record[channel][0], record1 = s.record[channel][1];
    i32 drive = s.drive, makeup = s.makeup, amount = s.amount;
    for (u32 i = 0; i < 64; ++i) {
        drive += drive_step;
        makeup += makeup_step;
        amount += amount_step;
        const i32 clean = input[i] >> 4;
        record0 = blend(record0, clean, 485819789); // 1.8 kHz record bandwidth
        record1 = blend(record1, record0, 485819789);
        const i32 colored = mul(shape(mul(record1, drive) * 8), makeup);
        line[i] = clean + mul(colored - record1, amount);
    }
    s.record[channel][0] = record0;
    s.record[channel][1] = record1;
}
i32 sample(const i32 *line, u32 write, u32 written, u32 age) {
    return age > written ? 0 : line[(write - age) & (line_frames - 1)];
}

// Four-point cubic Hermite interpolation. Fractional delay has 16 fractional
// bits; all taps stay behind the writer, including at maximum wow and snag.
template <bool complete> i32 tap(const i32 *line, u32 write, u32 written, i32 delay) {
    const u32 age = static_cast<u32>(delay) >> 16;
    const i32 f = static_cast<i32>((static_cast<u32>(delay) & 65535) << 15);
    if (!f)
        return sample(line, write, written, age);
    if (!complete && written < age - 1)
        return 0;
    i32 a, b, c, d;
    if (complete || written >= age + 2) {
        const i32 *const p = line + ((write - age - 2) & (line_frames - 1));
        d = p[0];
        c = p[1];
        b = p[2];
        a = p[3];
    } else {
        a = sample(line, write, written, age - 1);
        b = sample(line, write, written, age);
        c = sample(line, write, written, age + 1);
        d = sample(line, write, written, age + 2);
    }
    const i32 c1 = (c - a) >> 1;
    const i32 c3 = ((d - a) >> 1) + ((3 * (b - c)) >> 1);
    const i32 c2 = c - b - c1 - c3;
    return b + mul(f, c1 + mul(f, c2 + mul(f, c3)));
}
template <bool complete>
__attribute__((noinline)) void delay_block(i32 *out, const i32 *line, u32 write, u32 written, i32 delay,
                                           i32 step) {
    for (u32 i = 0; i < 64; ++i) {
        delay += step;
        out[i] = tap<complete>(line, write + i, written + i, delay);
    }
}
i32 output(i32 value) {
    const i32 limit = one - 1;
    return value > limit ? full : value < -limit ? -full : value * 16;
}
// Two-pole recorder voicing: one bandwidth pole, one low-band shelf/cut.
// The retained low-band gain is stored at half scale so every coefficient stays
// positive and model changes interpolate safely. Gain and Dry are folded into
// the block-rate gate target, avoiding another multiply per audio sample.
__attribute__((noinline)) void color_block(i32 *out, State &s, u32 ch, i32 hp, i32 lp, i32 bass_gain,
                                           i32 gate_step, i32 hiss_step) {
    i32 low = s.low[ch][0], bass = s.dc[ch], gate = s.gate[ch], hiss = s.hiss;
    for (u32 i = 0; i < 64; ++i) {
        gate += gate_step;
        hiss += hiss_step;
        low = blend(low, out[i], lp);
        bass = blend(bass, low, hp);
        const i32 colored = ((low - bass) >> 1) + mul(bass, bass_gain);
        i32 value = mul(colored, gate) * 4;
        if (hiss)
            value += mul(noise_audio(s), hiss * 16);
        out[i] = output(value);
    }
    s.low[ch][0] = low;
    s.dc[ch] = bass;
}

void plain_block(i32 *out, State &s, u32 ch, i32 gate_step, i32 hiss_step) {
    i32 gate = s.gate[ch], hiss = s.hiss;
    const i32 last = out[63];
    if (!hiss && !hiss_step && gate == half && !gate_step) {
        for (u32 i = 0; i < 64; ++i)
            out[i] = output(out[i]);
    } else {
        for (u32 i = 0; i < 64; ++i) {
            gate += gate_step;
            hiss += hiss_step;
            i32 value = mul(out[i], gate) * 2;
            if (hiss)
                value += mul(noise_audio(s), hiss * 16);
            out[i] = output(value);
        }
    }
    s.dc[ch] = 0;
    s.low[ch][0] = s.low[ch][1] = last;
    s.bass[ch] = last;
}

} // namespace

void initialize(State &state) {
    state = State();
    state.random = 0x6e746170;
    state.noise_random = 0x91a527cd;
    state.delay[0] = state.delay[1] = base_delay;
    state.gate[0] = state.gate[1] = half;
    state.bump = half;
    state.drive = 2 * one;
    state.makeup = full;
    state.gain = one;
    state.lp = full;
    state.wait = 120;
}

void process(State &s, opxy::Buffer buffer, opxy::Channel left, opxy::Channel right,
             opxy::Parameters parameters) {
    if (!buffer.words || buffer.count < buffer_words) {
        for (u32 i = 0; i < opxy::block_frames; ++i)
            left[i] = right[i] = 0;
        return;
    }
    if (!s.ready) {
        for (u32 i = 0; i < 7; ++i)
            s.controls[i] = parameters[i] < 0 ? 0 : parameters[i];
        s.ready = 1;
    }
    for (u32 i = 0; i < 7; ++i)
        s.controls[i] = blend(s.controls[i], parameters[i] < 0 ? 0 : parameters[i], 134217728);
    const i32 wow_amount = mul(s.controls[0], full / 2 + s.controls[0] / 2);
    const i32 flutter_amount = mul(s.controls[1], full / 2 + s.controls[1] / 2);
    const i32 sat = s.controls[2], drive_amount = mul(sat, full / 2 + sat / 2);
    const i32 fail = s.controls[3];
    const i32 wet = full - (parameters[7] < 0 ? 0 : parameters[7]);
    const u32 model = parameters[4] < 0 ? 0 : (static_cast<u32>(parameters[4]) >> 29);
    const i32 spread = s.controls[5];
    const i32 noise = mul(s.controls[6], s.controls[6]);
    const i32 drive = 2 * one + mul(5 * one, drive_amount);
    // Gentle make-up restores body as the record band compresses; the
    // complementary upper band keeps its original level.
    const i32 makeup =
        static_cast<i32>(2147483647u / (16384u + ((static_cast<u32>(drive_amount) >> 16) * 3 / 4))) * 16384;
    const Model &model_data = models[model];
    const i32 hp = coefficient(s.hp, model_data.hp), lp = coefficient(s.lp, model_data.lp);
    const i32 bump = coefficient(s.bump, model_data.bump), gain = coefficient(s.gain, model_data.gain);
    i32 wow[2], flutter[2];
    for (u32 ch = 0; ch < 2; ++ch) {
        wow[ch] = smooth_random(s, s.wow[ch], 22000000);
        flutter[ch] = smooth_random(s, s.flutter[ch], 520000000);
    }
    // Most transport movement stays common; Spread adds independent movement.
    const i32 w[2] = {wow[0], blend(wow[0], wow[1], spread)};
    const i32 f[2] = {flutter[0], blend(flutter[0], flutter[1], spread)};
    i32 event_envelope = 0;
    if (s.event) {
        const i32 t = (full / static_cast<i32>(s.duration)) * static_cast<i32>(s.event);
        event_envelope = mul(mul(t, full - t), fail) * 4;
        if (++s.event >= s.duration)
            s.event = 0;
    } else if (s.wait) {
        --s.wait;
    } else {
        const u32 density = static_cast<u32>(fail) >> 23;
        s.wait = 55 + (random(s) & 1023) + (255 - density) * 12;
        if (fail > 100000) {
            s.event = 1;
            s.duration = 24 + (random(s) & 127);
            s.kind = random(s) & 3;
        }
    }
    i32 target_delay[2], target_gate[2], step_delay[2], step_gate[2];
    for (u32 ch = 0; ch < 2; ++ch) {
        const i32 drift = (mul(w[ch], wow_amount) / 2048) * 1800;
        const i32 twitch = (mul(f[ch], flutter_amount) / 2048) * 28;
        const i32 snag = s.kind & 1 ? mul(event_envelope, 96 * 65536) : 0;
        target_delay[ch] = base_delay + drift + twitch + (s.kind & 2 ? -snag : snag);
        if (target_delay[ch] < 4 * 65536)
            target_delay[ch] = 4 * 65536;
        if (target_delay[ch] > 1500 * 65536)
            target_delay[ch] = 1500 * 65536;
        const i32 loss = s.kind & 1 ? event_envelope / 5 : event_envelope - event_envelope / 16;
        const i32 channel_loss = ch && spread ? mul(loss, full - spread / 2) : loss;
        // Flutter carries a little amplitude movement as well as pitch.
        target_gate[ch] =
            full - channel_loss - mul(mul(f[ch] < 0 ? -f[ch] : f[ch], flutter_amount), 250000000);
        target_gate[ch] =
            target_gate[ch] == full && wet == full ? gain * 8 : mul(mul(target_gate[ch], wet), gain * 8);
        step_delay[ch] = (target_delay[ch] - s.delay[ch]) / 64;
        step_gate[ch] = (target_gate[ch] - s.gate[ch]) / 64;
    }
    const i32 drive_step = (drive - s.drive) / 64, makeup_step = (makeup - s.makeup) / 64;
    const i32 amount_step = (sat - s.amount) / 64, hiss = mul(mul(noise, 600000), wet);
    const i32 hiss_step = (hiss - s.hiss) / 64;
    if (!sat && !s.amount) {
        for (u32 i = 0; i < 64; ++i) {
            buffer.words[s.write + i] = left[i] >> 4;
            buffer.words[line_words + s.write + i] = right[i] >> 4;
        }
        for (u32 ch = 0; ch < 2; ++ch)
            s.record[ch][0] = s.record[ch][1] = 0;
    } else {
        saturation_block(left, buffer.words + s.write, s, 0, drive_step, makeup_step, amount_step);
        saturation_block(right, buffer.words + line_words + s.write, s, 1, drive_step, makeup_step,
                         amount_step);
    }
    if (!s.write)
        for (u32 ch = 0; ch < 2; ++ch)
            for (u32 i = 0; i < 4; ++i)
                buffer.words[ch * line_words + line_frames + i] = buffer.words[ch * line_words + i];
    for (u32 ch = 0; ch < 2; ++ch) {
        i32 *const out = ch ? right : left;
        const i32 *const line = buffer.words + ch * line_words;
        if (s.written >= 1502)
            delay_block<true>(out, line, s.write, s.written, s.delay[ch], step_delay[ch]);
        else
            delay_block<false>(out, line, s.write, s.written, s.delay[ch], step_delay[ch]);
        if (!hp && lp == full && bump == half && gain == one)
            plain_block(out, s, ch, step_gate[ch], hiss_step);
        else
            color_block(out, s, ch, hp, lp, bump, step_gate[ch], hiss_step);
    }
    s.write = (s.write + 64) & (line_frames - 1);
    s.written = s.written + 64 < line_frames ? s.written + 64 : line_frames;
    s.drive = drive;
    s.makeup = makeup;
    s.amount = sat;
    s.hiss = hiss;
    s.hp = hp;
    s.lp = lp;
    s.bump = bump;
    s.gain = gain;
    for (u32 ch = 0; ch < 2; ++ch) {
        s.delay[ch] = target_delay[ch];
        s.gate[ch] = target_gate[ch];
    }
    // The source mixer supplies Dry. The wet return fades by (1-Dry), making
    // the complete send a level-balanced crossfade rather than an additive mix.
}
} // namespace gentape
