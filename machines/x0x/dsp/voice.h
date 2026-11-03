#pragma once
#include "../core.h"
#include "../models.h"
#include "primitives.h"
#include "pcm.h"
#include "metallic.h"
#include "drive_curves.h"

namespace x0x { namespace dsp {
static inline i32 sample_for(u32 model, u32 family) {
    static const i32 bank707[model_count] = {0,2,12,13,4,5,6,7,8,9,10,11,-1,-1,-1,-1,-1,14};
    if (family == 1) return model < model_count ? bank707[model] : -1;
    return family == 3 && model >= closed_hat && model <= ride ? i32(15 + model - closed_hat) : -1;
}
static inline Settings settings(u32 model, opxy::Parameters parameters) {
    Settings s = {};
    for (u32 i = 0; i < 8; ++i) s.raw[i] = raw_control(parameters[i]);
    s.family = family_index(model, s.raw[style]);
    s.metal808 = s.family == 2 && model >= closed_hat && model <= cymbal;
    s.duration = decay_ms(model, s.raw[decay]);
    if (model == closed_hat || model == open_hat) s.duration = s.duration * (64 + s.raw[body]) / 128;
    s.amplitude_rate = rate(s.duration);
    s.transient_rate = rate(7);
    s.attack_frames = s.raw[attack] * 441u / 10;
    s.attack_step = s.attack_frames ? 0x7fffffffu / s.attack_frames : 0;
    s.pitch_rate = rate(s.family == 3 ? 18 : 8);
    s.filter = tables::tone[s.raw[tone]];
    if (s.metal808) s.filter = 32767; // Tone balances the spectral branches below.
    s.noise_filter = i32(scale(2048 + s.raw[tone] * 32, tables::tuning[s.raw[tune]]));
    if (model == snare && s.family == 3) s.noise_filter = 8192 + s.raw[tone] * 64;
    if (s.noise_filter > 16384) s.noise_filter = 16384;
    if (model == maracas) {
        s.noise_filter = i32(scale(4096+s.raw[character]*128,tables::tuning[s.raw[tune]]));
        if (s.noise_filter > 28000) s.noise_filter = 28000;
    }
    s.sample = sample_for(model, s.family);
    s.alternate = s.family == 1 && (model == kick || model == snare) ? s.sample + 1 : -1;
    if (s.sample >= 0) s.sample_step = scale(pcm::samples[s.sample].rate * 65536u / 44100u, tables::tuning[s.raw[tune]]);
    if (model == clap) {
        s.clap_period = s.family == 3 ? 128+s.raw[character]*4 : 180+s.raw[character]*5;
        s.clap_inverse = (32767u << 16) / s.clap_period;
        if (s.sample >= 0) s.pcm_delay = pcm::samples[s.sample].rate * s.raw[character] / 4000;
    }
    if(s.metal808){
        s.sample_step=tables::tuning[s.raw[tune]];
        const u32 character=s.raw[x0x::character];
        s.metal_reciprocals[0]=character<64?0:1;
        s.metal_reciprocals[1]=character<64?character*512:(character-64)*32768/63;
        return s;
    }
    static const u32 base[model_count] = {53,180,800,835,98,131,175,205,205,205,440,540,2500,1200,165,220,330,1200};
    u32 hz = base[model];
    if (s.family == 0) {
        if (model == kick) hz = 58;
        if (model == snare) hz = 220;
        if (model == low_tom) hz = 110;
        if (model == high_tom) hz = 185;
    } else if (s.family == 3) {
        if (model == kick) hz = 52;
        if (model >= low_tom && model <= high_tom) hz = 110 + (model - low_tom) * 55;
    }
    for (u32 i = 0; i < 6; ++i) s.increments[i] = scale(step(hz), tables::tuning[s.raw[tune]]);
    if (model == snare) s.increments[1] = scale(step(s.family == 0 ? 350 : 330), tables::tuning[s.raw[tune]]);
    if (model == rim) s.increments[1] = scale(step(s.family == 3 ? 1800 : 1250), tables::tuning[s.raw[tune]]);
    if (model == cowbell) s.increments[1] = scale(step(u32(800 + (i32(s.raw[character]) - 64) * 4)), tables::tuning[s.raw[tune]]);
    if (model >= closed_hat && model <= cymbal) {
        static const u32 metallic[2][6] = {{240,400,560,640,800,1040},{205,370,304,523,541,800}};
        for (u32 i = 0; i < 6; ++i) {
            const u32 frequency = metallic[s.family == 0 ? 0 : 1][i];
            const i32 detune = (i32(s.raw[character]) - 64) * i32(i) / 8;
            s.increments[i] = scale(step(u32(i32(frequency) + detune)), tables::tuning[s.raw[tune]]);
            if (s.metal808) {
                static const u32 nominal[6] = {2053,3696,3044,5227,5400,8000};
                s.increments[i] = scale(step(u32(i32(nominal[i])+10*detune))/10, tables::tuning[s.raw[tune]]);
                s.metal_reciprocals[i] = 0x7fffffffu/(s.increments[i]>>16);
            }
        }
    }
    return s;
}
static inline const Settings& voice_settings(Voice& v, opxy::Parameters parameters) {
    bool changed = !v.configured || v.configured_model != v.model;
    for (u32 i = 0; i < 8; ++i)
        changed = changed || v.settings.raw[i] != raw_control(parameters[i]);
    if (changed) {
        v.settings = settings(v.model, parameters);
        v.configured_model = v.model;
        v.configured = 1;
    }
    return v.settings;
}
static inline i32 read(const pcm::Sample& sample, u32 cursor, u32 fraction) {
    if (cursor >= sample.frames) return 0;
    const i32 a = pcm::decode[sample.data[cursor]];
    const i32 b = cursor + 1 < sample.frames ? pcm::decode[sample.data[cursor + 1]] : 0;
    return a + multiply(b - a, i32(fraction >> 1));
}
static inline i32 pcm_source(Voice& v, const Settings& s) {
    const pcm::Sample& sample = pcm::samples[s.sample];
    i32 signal = read(sample, v.cursor, v.fraction);
    if (s.alternate >= 0) {
        const i32 other = read(pcm::samples[s.alternate], v.cursor, v.fraction);
        signal += multiply(other - signal, i32(s.raw[body]) * 258);
    }
    if (v.model == clap && s.raw[character]) {
        const u32 delay = s.pcm_delay;
        signal = limit(signal*3/4+(v.cursor >= delay ? read(sample,v.cursor-delay,v.fraction)/2 : 0));
    }
    // A low/high split exposes snappy/metal/click/noise without replacing the
    // recording's character. Both gains are neutral at their centre values.
    v.metal_low += multiply(signal - v.metal_low, 4096);
    if (v.model != kick && !(v.model >= low_tom && v.model <= high_tom) && v.model != clap) {
        const i32 high = limit(signal - v.metal_low);
        signal = limit(v.metal_low + multiply(high, i32(s.raw[character]) * 512));
    }
    if (s.alternate < 0) {
        const i32 high = limit(signal - v.metal_low);
        signal = limit(multiply(high,32767+(64-i32(s.raw[body]))*128)
                     + multiply(v.metal_low,32767+(i32(s.raw[body])-64)*384));
    }
    u32 increment = s.sample_step;
    if (v.model == kick || (v.model >= low_tom && v.model <= high_tom)) {
        const i32 bend = (i32(s.raw[character]) - 64) * (v.pitch_envelope >> 16) / 64;
        increment = scale(increment, u32(65536 + bend));
    }
    const u32 next = v.fraction + increment;
    v.cursor += next >> 16;
    v.fraction = next & 65535;
    return signal;
}
static inline i32 synth_source(Voice& v, const Settings& s, i32 random) {
    const i32 transient = v.transient >> 16;
    const i32 body_amount = i32(s.raw[body]) * 258;
    i32 signal = 0;
    switch (v.model) {
    case kick: {
        const i32 tonal = sine(v.phase[0]);
        const i32 harmonic = sine(v.phase[0] * 2);
        signal = s.family == 3 ? (tonal * 3 + triangle(v.phase[0] + 0x40000000u)) / 4 : tonal;
        signal = (signal * 3 + multiply(harmonic, body_amount)) / 4;
        const i32 click = multiply(random, transient);
        signal = limit(signal + multiply(click, s.family == 3 ? 10000 : 3500));
        break;
    }
    case snare: {
        const i32 shell = s.family == 3 ? (triangle(v.phase[0] + 0x40000000u) + triangle(v.phase[1] + 0x40000000u)) / 2
                                      : (sine(v.phase[0]) + sine(v.phase[1])) / 2;
        const i32 shell_gain = 8192 + multiply(24576, transient);
        signal = multiply(multiply(shell, shell_gain), body_amount) / 2;
        signal += multiply(random - v.noise_low, i32(s.raw[character]) * 220);
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
        signal = multiply(limit((random - v.noise_low) * 2), burst);
        break;
    }
    case rim:
        signal = divide_signal<3>((multiply(sine(v.phase[0]),32767-body_amount)+multiply(sine(v.phase[1]),body_amount))*2);
        signal += multiply(multiply(random, transient), i32(s.raw[character]) * 512);
        break;
    case low_tom: case mid_tom: case high_tom:
    case low_conga: case mid_conga: case high_conga: {
        const i32 tonal = sine(v.phase[0]);
        signal = tonal + multiply(sine(v.phase[0]*3)-tonal,body_amount/2);
        if (v.model <= high_tom) signal += multiply(multiply(random, transient), s.family == 0 ? 7000 : 2500);
        break;
    }
    case closed_hat: case open_hat: case cymbal: {
        i32 metal = 0;
        for (u32 i = 0; i < 6; ++i) metal += square_sixth(v.phase[i]);
        v.metal_low += multiply(metal - v.metal_low, s.family == 0 ? 12000 : 9000);
        signal = limit((metal - v.metal_low) * (v.model == cymbal ? 3 : 4));
        if (v.model == cymbal) signal = (signal * 3 + multiply(random, body_amount)) / 4;
        break;
    }
    case cowbell:
        signal = (square(v.phase[0]) + multiply(square(v.phase[1]), body_amount)) / 2;
        break;
    case claves: {
        const i32 tonal = sine(v.phase[0]);
        signal = tonal + multiply(sine(v.phase[0]*2)-tonal,body_amount/2);
        signal += multiply(multiply(random, transient), i32(s.raw[character]) * 1024);
        break;
    }
    case maracas:
        signal = limit((random-v.noise_low)*2+multiply(v.noise_low,body_amount)*2);
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
static inline i32 stem_read(u32 anchor,u32 branch,u32 cursor,u32 fraction){
    const i32 a=tables::metal_stems[anchor][branch][cursor],b=tables::metal_stems[anchor][branch][cursor+1];
    return a+multiply(b-a,i32(fraction>>1));
}
template <bool Cymbal,bool Blend> static inline void metal_source_block(Voice& v,const Settings& s,
        i32* output,u32 frames) {
    if(!frames)return;
    if(!v.metal.initialized){metal_start(v.metal,v.phase);v.cursor=v.fraction=0;}
    u32 cursor=v.cursor,fraction=v.fraction;
    if(cursor>=22050)cursor%=22050;
    i32 fast=v.metal.fast,medium=v.metal.medium;
    const i32 fast_rate=Cymbal?rate(180):0,medium_rate=Cymbal?rate(700):0;
    const u32 anchor=s.metal_reciprocals[0];const i32 blend=i32(s.metal_reciprocals[1]);
    for(u32 i=0;i<frames;++i){
        i32 bright=stem_read(anchor,1,cursor,fraction),air=stem_read(anchor,2,cursor,fraction);
        if(Blend){bright+=multiply(stem_read(anchor+1,1,cursor,fraction)-bright,blend);
                  air+=multiply(stem_read(anchor+1,2,cursor,fraction)-air,blend);}
        if(!Cymbal)output[i]=limit((multiply(bright,32767-i32(s.raw[tone])*160)
                                  +multiply(air,i32(s.raw[tone])*512))*32);
        else{
            i32 bell=stem_read(anchor,0,cursor,fraction);
            if(Blend)bell+=multiply(stem_read(anchor+1,0,cursor,fraction)-bell,blend);
            const i32 strike=multiply(bell,fast>>16);
            const i32 shimmer=multiply(bright,8192+(medium>>17));
            fast=envelope(fast,fast_rate);medium=envelope(medium,medium_rate);
            output[i]=limit((multiply(strike,8192+i32(s.raw[body])*192)+shimmer
                            +multiply(air,i32(s.raw[tone])*512))*18);
        }
        const u32 next=fraction+s.sample_step;
        cursor+=next>>16;fraction=next&65535;
        if(cursor>=22050)cursor-=22050;
    }
    v.cursor=cursor;v.fraction=fraction;v.metal.fast=fast;v.metal.medium=medium;
}
template <bool Sampled, bool Metal=false> static inline void render_block(Voice& v, opxy::Output output, const Settings& s) {
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
    const opxy::i16* curve=tables::drive_curves[drive_amount];
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
                v.noise_low += multiply(random - v.noise_low, s.noise_filter);
                sample = synth_source(v, s, random);
            }
            if (v.age < s.attack_frames)
                sample = multiply(sample,i32(v.age*s.attack_step >> 16));
            sample = multiply(sample, v.amplitude >> 16);
            v.amplitude = envelope(v.amplitude, s.amplitude_rate);
            v.transient = envelope(v.transient, s.transient_rate);
            v.pitch_envelope = envelope(v.pitch_envelope, s.pitch_rate);
        }
        v.output_low += multiply(sample - v.output_low, s.filter);
        sample = v.output_low;
        sample = drive_amount?table_drive(sample,curve):sample;
        const i32 clean = limit(sample - v.dc_input + multiply(v.dc_output, 32600));
        v.dc_input = sample; v.dc_output = clean;
        output[i] = clean * 32768; // Declared half-scale Q31 peak.
        silent = silent && clean == 0;
        if (v.active) ++v.age;
    }
    if (silent && ended && v.age && !v.pending) v.active = 0;
}
// Keep the metallic scratch frame out of PCM and other synthesized voices.
__attribute__((noinline)) static void render_metal_block(Voice& v, opxy::Output output, const Settings& s) {
    render_block<false,true>(v,output,s);
}
static inline void render(Voice& v, opxy::Output output, opxy::Parameters parameters) {
    if (!v.active && !v.pending) {
        for (u32 i = 0; i < opxy::block_frames; ++i) output[i] = 0;
        return;
    }
    const Settings& s = voice_settings(v, parameters);
    // The selected model/style is fixed for this block. Keep PCM cursor checks
    // and synthesized oscillators in separate sample loops.
    if(s.metal808)render_metal_block(v,output,s);
    else if(s.sample>=0)render_block<true>(v,output,s);
    else render_block<false>(v,output,s);
}
}}
