// Frozen baseline for exact arithmetic/state comparison.

namespace bitflip_reference {
using bitflip::Voice; using bitflip::Shared; namespace tables = bitflip::tables;
namespace {
using opxy::i32;
using opxy::u32;
const unsigned patterns[8][4] = {
    {0,0,0,0}, {0,12,0,12}, {0,4,7,12}, {0,3,7,12},
    {0,5,7,12}, {0,4,7,11}, {0,3,7,10}, {0,7,12,19}
};
inline i32 control(i32 value) { return value < 0 ? 0 : value >> 16; }
inline i32 approach(i32 current,i32 target) {
    const i32 delta=target-current;
    return delta>-2 && delta<2 ? target : current+delta/2;
}
inline i32 clamp(i32 value, i32 low, i32 high) { return value < low ? low : value > high ? high : value; }
inline i32 mix(i32 a, i32 b, i32 amount) { return a + ((b - a) * amount >> 15); }
inline u32 choice(i32 value) { return static_cast<u32>(control(value)) >> 12; }
inline i32 curve(const i32* values, i32 amount) {
    if (amount >= 32767) return values[128];
    const u32 at = static_cast<u32>(amount) >> 8;
    return values[at] + ((values[at + 1] - values[at]) * (amount & 255) >> 8);
}
inline i32 read(const opxy::i16* wave, u32 phase) {
    const u32 at = phase >> 24;
    return mix(wave[at], wave[at + 1], static_cast<i32>((phase >> 9) & 32767u));
}
// Q12 ratios stay in 1024..65536. Saturate before the high word can overflow.
inline u32 transpose(u32 pitch, u32 ratio) {
    const u32 high = (pitch >> 16) * ratio;
    if (high >= 0x06000000u) return 0x60000000u;
    const u32 value = high * 16u + ((pitch & 65535u) * ratio >> 12);
    return value > 0x60000000u ? 0x60000000u : value;
}
struct Band { u32 index; i32 rich; };
Band band(u32 increment) {
    Band result = {0, 32768};
    while (result.index < 4 && increment >= tables::band_ends[result.index]) ++result.index;
    if (result.index < 4) {
        const u32 end = tables::band_ends[result.index];
        if (increment > end / 2)
            result.rich = clamp(static_cast<i32>((end - increment) / ((end - end / 2) >> 15)), 0, 32768);
    }
    return result;
}
inline i32 frame_wave(u32 family, i32 color, u32 level, u32 phase) {
    const u32 at = static_cast<u32>(color) >> 12;
    return mix(read(tables::waves[family][at][level], phase),
               read(tables::waves[family][at + 1][level], phase), (color & 4095) * 8);
}
inline i32 wave(u32 family, i32 color, const Band& limit, u32 phase) {
    const i32 rich = frame_wave(family, color, limit.index, phase);
    return limit.index == 4 || limit.rich == 32768 ? rich
        : mix(frame_wave(family, color, limit.index + 1, phase), rich, limit.rich);
}
inline i32 quantize(i32 sample, unsigned shift) {
    const i32 magnitude = sample < 0 ? -sample : sample;
    const i32 reduced = (magnitude >> shift) << shift;
    return sample < 0 ? -reduced : reduced;
}
inline void noise_step(Voice& voice) {
    voice.long_noise = (voice.long_noise >> 1) | (((voice.long_noise ^ (voice.long_noise >> 1)) & 1u) << 14);
    voice.short_noise = (voice.short_noise >> 1) | (((voice.short_noise ^ (voice.short_noise >> 1)) & 1u) << 6);
}
}

inline void start(Voice& voice, Shared&, u32 owner) {
    voice.phase = voice.mod_phase = voice.noise_phase = voice.arp_phase = voice.arp_step = voice.arp_type = 0;
    voice.phase2 = 0x40000000u;
    voice.long_noise = (owner * 2654435761u & 32767u) | 1u;
    voice.short_noise = (owner * 2654435761u & 127u) | 1u;
    voice.clock_phase = voice.initialized = voice.model = 0;
    voice.color = voice.edge = voice.fade = voice.model_from = voice.last_raw = voice.held = 0;
    voice.dc_input = voice.dc_output = 0;
    voice.model_fade = voice.sweep_envelope = 32768;
    voice.sweep_ratio = 4096;
}
inline void initialize(Voice& voice, Shared& shared, u32 owner) { bitflip_reference::start(voice, shared, owner); }
inline void destroy(Voice&, Shared&, u32) {}

inline void render(Voice& voice, Shared&, u32, opxy::Output output,
                   opxy::Parameters parameters, opxy::Pitch pitch) {
    const u32 model = choice(parameters[0]), arp = choice(parameters[4]);
    const i32 target_color = control(parameters[1]), target_edge = control(parameters[2]);
    if (!voice.initialized) {
        voice.color = target_color; voice.edge = target_edge;
        voice.sweep_ratio = curve(tables::sweep, control(parameters[3]));
        voice.model = model; voice.initialized = 1;
    } else if (voice.model != model) {
        voice.model = model; voice.model_from = voice.last_raw; voice.model_fade = 0;
    }
    if (voice.arp_type != arp) { voice.arp_phase = voice.arp_step = 0; voice.arp_type = arp; }
    const i32 next_color = approach(voice.color,target_color);
    const i32 next_edge = approach(voice.edge,target_edge);
    const i32 color_step = (next_color - voice.color) / 64, edge_step = (next_edge - voice.edge) / 64;
    const i32 next_sweep = voice.sweep_envelope * 31831 >> 15;
    const i32 sweep_step = (next_sweep - voice.sweep_envelope) / 64;
    const u32 arp_rate = static_cast<u32>(tables::arp_rate[choice(parameters[5])]);
    const unsigned bits = 2 + ((control(parameters[6]) * 15) >> 15), shift = 16 - bits;
    const u32 clock = static_cast<u32>(tables::clock[choice(parameters[7])]);
    u32 highest = 0;
    for (unsigned i = 0; i < opxy::block_frames; ++i) if (pitch[i] > highest) highest = pitch[i];
    if (highest > 0x60000000u) highest = 0x60000000u;
    u32 highest_interval = 0;
    for (unsigned i = 0; i < 4; ++i) if (patterns[arp][i] > highest_interval) highest_interval = patterns[arp][i];
    const i32 sweep_now = 4096 + ((voice.sweep_ratio - 4096) * voice.sweep_envelope >> 15);
    const i32 maximum_sweep = sweep_now > 4096 ? sweep_now : 4096;
    const u32 maximum = transpose(highest, static_cast<u32>(maximum_sweep) * tables::intervals[highest_interval] >> 12);
    const bool octave = model == 0 || model == 1 || model == 3 || model == 7;
    const Band limit = band(transpose(maximum, model == 5 ? 36864u : octave && (voice.edge || next_edge) ? 8192u : 4179u));
    const u32 family = model == 1 ? 1 : model == 3 ? 2 : model == 7 ? 3 : 0;
    const i32 maximum_color = voice.color > next_color ? voice.color : next_color;
    const u32 maximum_harmonic = 1u + ((static_cast<u32>(maximum_color) * 7u + 16384u) >> 15);
    const u32 sideband = transpose(maximum, (maximum_harmonic + 1) * 4096u);
    const i32 fm_guard = sideband <= 0x02000000u ? 32768 : sideband >= 0x20000000u ? 0
        : static_cast<i32>((0x20000000u - sideband) / 15360u);
    const i32 ring_guard = sideband <= 0x40000000u ? 32768 : sideband >= 0x60000000u ? 0
        : static_cast<i32>((0x60000000u - sideband) >> 14);
    for (unsigned i = 0; i < opxy::block_frames; ++i) {
        voice.color += color_step; voice.edge += edge_step;
        const i32 sweep = 4096 + ((voice.sweep_ratio - 4096) * voice.sweep_envelope >> 15);
        const u32 ratio = static_cast<u32>(sweep) * tables::intervals[patterns[arp][voice.arp_step]] >> 12;
        const u32 increment = transpose(pitch[i] > 0x60000000u ? 0x60000000u : pitch[i], ratio);
        const u32 harmonic = 1u + ((static_cast<u32>(voice.color) * 7u + 16384u) >> 15);
        i32 sample;
        if (model == 6) {
            const u32 noise_increment = transpose(increment, 1024u + ((static_cast<u32>(voice.color) * 63u) >> 5));
            const u32 before = voice.noise_phase; voice.noise_phase += noise_increment;
            if (voice.noise_phase < before) noise_step(voice);
            sample = mix((voice.long_noise & 1u) ? 23000 : -23000,
                         (voice.short_noise & 1u) ? 23000 : -23000, voice.edge);
        } else if (model == 4) {
            const i32 depth = voice.edge * fm_guard >> 15;
            const u32 offset = static_cast<u32>(read(tables::sine, voice.mod_phase) * depth) << 2;
            sample = read(tables::sine, voice.phase + offset) * 3 / 4;
            voice.mod_phase += transpose(increment, harmonic * 4096u);
        } else if (model == 5) {
            const i32 carrier = read(tables::sine, voice.phase);
            const i32 ring = carrier * read(tables::sine, voice.mod_phase) >> 15;
            sample = mix(carrier, ring, voice.edge * ring_guard >> 15) * 3 / 4;
            voice.mod_phase += transpose(increment, harmonic * 4096u);
        } else {
            sample = wave(family, voice.color, limit, voice.phase);
            if (model == 2) {
                sample = (sample + wave(0, voice.color, limit, voice.phase2)) / 2;
                voice.phase2 += transpose(increment, 4096u + (static_cast<u32>(voice.edge) * 84u >> 15));
            } else if (voice.edge) {
                sample = mix(sample, wave(family, voice.color, limit, voice.phase * 2u), voice.edge / 2);
            }
        }
        if (voice.model_fade < 32768) {
            sample = mix(voice.model_from, sample, voice.model_fade);
            voice.model_fade += 512;
        }
        voice.last_raw = sample;
        if (shift) sample = quantize(sample, shift);
        if (clock == 65536u) { voice.held = sample; voice.clock_phase = 0; }
        else {
            voice.clock_phase += clock;
            if (voice.clock_phase >= 65536u) { voice.clock_phase -= 65536u; voice.held = sample; }
            sample = voice.held;
        }
        const i32 blocked = sample - voice.dc_input + (voice.dc_output * 32690 >> 15);
        voice.dc_input = sample; voice.dc_output = clamp(blocked, -32768, 32767);
        output[i] = (voice.dc_output * voice.fade >> 15) * 8192;
        if (voice.fade < 32768) voice.fade += 1024;
        voice.phase += increment;
        voice.sweep_envelope += sweep_step;
        if (arp) {
            const u32 before = voice.arp_phase; voice.arp_phase += arp_rate;
            if (voice.arp_phase < before) voice.arp_step = (voice.arp_step + 1) & 3u;
        }
    }
    voice.color = next_color; voice.edge = next_edge; voice.sweep_envelope = next_sweep;
}
}
