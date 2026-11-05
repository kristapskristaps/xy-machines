// Frozen before block-level model specialization.
#pragma once
namespace x0x { namespace dsp {
static inline i32 dispatch_baseline_source(Voice& v, const Settings& s, i32 random) {
    const i32 transient = v.transient >> 16;
    const i32 body_amount = i32(s.raw[body]) * 258;
    i32 signal = 0;
    switch (v.model) {
    case kick: {
        const i32 tonal = sine(v.phase[0]);
        const i32 harmonic = sine(v.phase[0] * 2);
        signal = s.family == 3 ? (tonal * 3 + triangle(v.phase[0] + 0x40000000u)) / 4 : tonal;
        signal = (signal * 3 + multiply16(harmonic, body_amount)) / 4;
        const i32 click = multiply16(random, transient);
        signal = limit(signal + multiply16(click, s.family == 3 ? 10000 : 3500));
        break;
    }
    case snare: {
        const i32 shell = s.family == 3 ? (triangle(v.phase[0] + 0x40000000u) + triangle(v.phase[1] + 0x40000000u)) / 2
                                      : (sine(v.phase[0]) + sine(v.phase[1])) / 2;
        const i32 shell_gain = 8192 + multiply16(24576, transient);
        signal = multiply16(multiply16(shell, shell_gain), body_amount) / 2;
        signal += multiply16(random - v.noise_low, i32(s.raw[character]) * 220);
        signal = limit(signal);
        break;
    }
    case clap: {
        i32 burst = 4096 + body_amount*3/4;
        if (v.age < 3*s.clap_period) {
            u32 local = v.age;
            if (local >= s.clap_period) local -= s.clap_period;
            if (local >= s.clap_period) local -= s.clap_period;
            const u32 remaining = s.clap_period-local;
            burst = i32(ratio_q15(remaining,s.clap_period,s.clap_inverse));
        }
        signal = multiply16(limit((random - v.noise_low) * 2), burst);
        break;
    }
    case rim:
        signal = divide_signal<3>((multiply16(sine(v.phase[0]),32767-body_amount)+multiply16(sine(v.phase[1]),body_amount))*2);
        signal += multiply(multiply16(random, transient), i32(s.raw[character]) * 512);
        break;
    case low_tom: case mid_tom: case high_tom:
    case low_conga: case mid_conga: case high_conga: {
        const i32 tonal = sine(v.phase[0]);
        signal = tonal + multiply(sine(v.phase[0]*3)-tonal,body_amount/2);
        if (v.model <= high_tom) signal += multiply16(multiply16(random, transient), s.family == 0 ? 7000 : 2500);
        break;
    }
    case closed_hat: case open_hat: case cymbal: {
        i32 metal = 0;
        for (u32 i = 0; i < 6; ++i) metal += square_sixth(v.phase[i]);
        v.metal_low += lowpass_product(metal - v.metal_low, s.family == 0 ? 12000 : 9000);
        signal = limit((metal - v.metal_low) * (v.model == cymbal ? 3 : 4));
        if (v.model == cymbal) signal = (signal * 3 + multiply16(random, body_amount)) / 4;
        break;
    }
    case cowbell:
        signal = (square(v.phase[0]) + multiply16(square(v.phase[1]), body_amount)) / 2;
        break;
    case claves: {
        const i32 tonal = sine(v.phase[0]);
        signal = tonal + multiply(sine(v.phase[0]*2)-tonal,body_amount/2);
        signal += multiply(multiply16(random, transient), i32(s.raw[character]) * 1024);
        break;
    }
    case maracas:
        signal = limit((random-v.noise_low)*2+multiply16(v.noise_low,body_amount)*2);
        break;
    default: break; // Ride and tambourine are PCM-only types.
    }
    u32 bend = 65536;
    if (v.model == kick) {
        const u32 depth = s.raw[character] * (s.family == 3 ? 2300u : s.family == 0 ? 1800u : 1300u);
        bend += scale(depth, u32(v.pitch_envelope >> 15));
    } else if ((v.model >= low_tom && v.model <= high_tom) || (v.model >= low_conga && v.model <= high_conga)) {
        bend += scale(s.raw[character] * 350u, u32(v.pitch_envelope >> 15));
    }
    if (bend == 65536) {
        for (u32 i = 0; i < 6; ++i)
            v.phase[i] += s.increments[i] > 0x60000000u ? 0x60000000u : s.increments[i];
    } else {
        // All six increments are equal for the kick/tom/conga pitch sweep.
        const u32 advance = scale(s.increments[0], bend);
        const u32 bounded = advance > 0x60000000u ? 0x60000000u : advance;
        for (u32 i = 0; i < 6; ++i) v.phase[i] += bounded;
    }
    return limit(signal);
}
template <bool Sampled, bool Metal=false> static inline void dispatch_baseline_block(Voice& v, opxy::Output output, const Settings& s) {
    if (v.pending) {
        v.pending = 0;
        v.amplitude = v.transient = v.pitch_envelope = 0x7fffffff;
    }
    bool silent = true, ended = false;
    const u32 end = s.duration * 441u / 10;
    const u32 sample_length = !Sampled ? 0 :
        s.alternate >= 0 && pcm::samples[s.alternate].frames > pcm::samples[s.sample].frames
            ? pcm::samples[s.alternate].frames : pcm::samples[s.sample].frames;
    const u32 drive_amount = control_count(v.model) == 8 ? s.raw[x0x::drive] : 0;
    i32 metallic[Metal?opxy::block_frames:1];
    if(Metal) {
        const u32 remaining=v.age<end?end-v.age:0;
        const u32 frames=v.active?(remaining<opxy::block_frames?remaining:opxy::block_frames):0;
        if(s.metal_reciprocals[1]){
            if(v.model==cymbal)metal_source_block<true,true>(v,s,metallic,frames);
            else metal_source_block<false,true>(v,s,metallic,frames);
        }else{
            if(v.model==cymbal)metal_source_block<true,false>(v,s,metallic,frames);
            else metal_source_block<false,false>(v,s,metallic,frames);
        }
    }
    for (u32 i = 0; i < opxy::block_frames; ++i) {
        i32 sample = 0;
        bool source_done = v.age >= end;
        if (Sampled) {
            source_done = source_done || v.cursor >= sample_length;
        }
        ended = source_done;
        if (v.active && !source_done) {
            if (Metal) sample = metallic[i];
            else if (Sampled) sample = pcm_source(v, s);
            else {
                const i32 random = noise(v.seed);
                v.noise_low += noise_product(random - v.noise_low, s.noise_filter);
                sample = dispatch_baseline_source(v, s, random);
            }
            if (v.age < s.attack_frames)
                sample = multiply16(sample,i32(v.age*s.attack_step >> 16));
            sample = multiply16(sample, v.amplitude >> 16);
            v.amplitude = envelope(v.amplitude, s.amplitude_rate);
            v.transient = envelope(v.transient, s.transient_rate);
            v.pitch_envelope = envelope(v.pitch_envelope, s.pitch_rate);
        }
        v.output_low += lowpass_product(sample - v.output_low, s.filter);
        sample = v.output_low;
        sample = drive(sample, drive_amount);
        const i32 clean = limit(sample - v.dc_input + dc_feedback(v.dc_output));
        v.dc_input = sample; v.dc_output = clean;
        output[i] = clean * 32768; // Declared half-scale Q31 peak.
        silent = silent && clean == 0;
        if (v.active) ++v.age;
    }
    if (silent && ended && v.age && !v.pending) v.active = 0;
}
static inline void dispatch_baseline_render(Voice& v, opxy::Output output, opxy::Parameters parameters) {
    if (!v.active && !v.pending) {
        for (u32 i = 0; i < opxy::block_frames; ++i) output[i] = 0;
        return;
    }
    const Settings& s = voice_settings(v, parameters);
    // The selected model/style is fixed for this block. Keep PCM cursor checks
    // and synthesized oscillators in separate sample loops.
    if(s.metal808)render_metal_block(v,output,s);
    else if(s.sample>=0)render_sample_block(v,output,s);
    else dispatch_baseline_block<false>(v,output,s);
}
}}
