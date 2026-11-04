#include "core.h"
#include "dsp.h"

namespace mando {
namespace {
// Each section keeps its coefficients and recurrence in a smaller register set.
__attribute__((noinline)) void first_section(Voice& voice, opxy::Output __restrict output,
                                             const dsp::Filter& filter) {
    using namespace dsp;
    i32 x1=voice.history[0],x2=voice.history[1],y1=voice.history[2],y2=voice.history[3];
    const i32 b=filter.b0[0],a=filter.a1[0],c=filter.a2[0];
    for(unsigned i=0;i<opxy::block_frames;++i) {
        const i32 x=saturate(output[i])*1024;
        const i32 y=clamp(multiply(x+2*x1+x2,b)-multiply(y1,a)*2-multiply(y2,c),-67108864,67108864);
        x2=x1;x1=x;y2=y1;y1=y;output[i]=y;
    }
    voice.history[0]=x1;voice.history[1]=x2;voice.history[2]=y1;voice.history[3]=y2;
}
__attribute__((noinline)) void second_section(Voice& voice, opxy::Output __restrict output,
                                              const dsp::Filter& filter,i32 x1,i32 x2) {
    using namespace dsp;
    i32 y1=voice.history[4],y2=voice.history[5];
    const i32 b=filter.b0[1],a=filter.a1[1],c=filter.a2[1];
    for(unsigned i=0;i<opxy::block_frames;++i) {
        const i32 x=output[i];
        const i32 y=clamp(multiply(x+2*x1+x2,b)-multiply(y1,a)*2-multiply(y2,c),-67108864,67108864);
        x2=x1;x1=x;y2=y1;y1=y;output[i]=y;
    }
    voice.history[4]=y1;voice.history[5]=y2;
}
__attribute__((noinline)) void filter_audio(Voice &voice,
                                            opxy::Output __restrict output,
                                            const dsp::Filter &filter) {
    using namespace dsp;
    const i32 previous_first=voice.history[2], previous_second=voice.history[3];
    first_section(voice,output,filter);
    second_section(voice,output,filter,previous_first,previous_second);
    for (unsigned i = 0; i < opxy::block_frames; ++i) {
        const i32 filtered = output_knee(output[i] >> 10);
        const i32 charge = voice.dc_output * 32759 + voice.dc_residue;
        const i32 dc = filtered - voice.dc_input + (charge >> 15);
        voice.dc_residue = static_cast<i32>(static_cast<u32>(charge) & 32767u);
        voice.dc_input = filtered;
        voice.dc_output = dc;
        output[i] = clamp(dc, -unity, unity) * 32768;
    }
}
template <bool Moving, bool Constant>
void render_audio(Voice &voice, opxy::Output __restrict output,
                  opxy::Pitch __restrict pitch, const dsp::Band &first,
                  const dsp::Band &second, const dsp::Oscillator &osc_a,
                  const dsp::Oscillator &osc_b, const dsp::Filter &filter,
                  const i32 *target_wave, u32 ratio, u32 octave, i32 balance,
                  i32 drive) {
    using namespace dsp;
    const Oscillator main_a = Moving ? Oscillator(first, 0) : osc_a;
    const Oscillator main_b = Moving ? Oscillator(second, 0) : osc_b;
    const Oscillator pulse_a(first, 32767), pulse_b(second, 32767);
    const u32 fixed_first = pitch[0] > 0x40000000u ? 0x40000000u : pitch[0];
    const u32 fixed_second = second_increment(fixed_first, ratio, octave);
    for (unsigned i = 0; i < opxy::block_frames; ++i) {
        if (Moving) {
            voice.wave[0] = smooth_wave(voice.wave[0], target_wave[0]);
            voice.wave[1] = smooth_wave(voice.wave[1], target_wave[1]);
        }
        const u32 increment = Constant                 ? fixed_first
                              : pitch[i] > 0x40000000u ? 0x40000000u
                                                       : pitch[i];
        const u32 other = Constant ? fixed_second
                                   : second_increment(increment, ratio, octave);
        const i32 a =
            balance == 32767 ? 0
            : Moving ? morph(main_a, pulse_a, voice.phase[0], voice.wave[0])
                     : main_a.read(voice.phase[0]);
        const i32 b =
            balance == 0 ? 0
            : Moving     ? morph(main_b, pulse_b, voice.phase[1], voice.wave[1])
                         : main_b.read(voice.phase[1]);
        // All prepared saw/pulse values lie in [-8344,8344]. Both
        // interpolation and these convex blends preserve that bound.
        const opxy::i16 difference = static_cast<opxy::i16>(b - a);
        const opxy::i16 weight = static_cast<opxy::i16>(balance);
        const i32 mixed = a + (difference * weight >> 15);
        const opxy::i16 sample = static_cast<opxy::i16>(mixed);
        const opxy::i16 gain = static_cast<opxy::i16>(drive);
        const i32 driven = sample * gain >> 8;
        voice.phase[0] += increment;
        voice.phase[1] += other;
        output[i] = driven;
    }
    filter_audio(voice, output, filter);
}

} // namespace
inline void start(Voice &voice, Shared &, u32) {
    voice.phase[0] = 0;
    voice.phase[1] = 0x20000000u;
    for (unsigned i = 0; i < 6; ++i)
        voice.history[i] = 0;
    for (unsigned i = 0; i < 8; ++i)
        voice.controls[i] = 0;
    voice.ready = 0;
    voice.wave[0] = voice.wave[1] = 0;
    voice.dc_input = voice.dc_output = voice.dc_residue = 0;
    voice.cached_filter.g = 0;
}
inline void initialize(Voice &voice, Shared &shared, u32 owner) {
    start(voice, shared, owner);
}
inline void destroy(Voice &, Shared &, u32) {}

inline void render(Voice &voice, Shared &, u32, opxy::Output __restrict output,
                   opxy::Parameters parameters, opxy::Pitch __restrict pitch) {
    using namespace dsp;
    for (unsigned j = 0; j < 8; ++j) {
        const i32 target = control(parameters[j]);
        if (!voice.ready || j == 3 || j == 6)
            voice.controls[j] = target;
        else
            voice.controls[j] += (target - voice.controls[j]) / 4;
    }
    const i32 *p = voice.controls;
    const u32 waves = option(p[3], 4), octave = option(p[6], 3),
              ratio = tuning(p[4]);
    const i32 target_wave[2] = {waves >= 2 ? 32767 : 0, waves & 1 ? 32767 : 0};
    if (!voice.ready) {
        voice.wave[0] = target_wave[0];
        voice.wave[1] = target_wave[1];
        voice.ready = 1;
    }
    u32 highest = pitch[0], lowest = pitch[0];
    for (unsigned i = 1; i < opxy::block_frames; ++i) {
        const u32 increment = pitch[i];
        if (increment > highest)
            highest = increment;
        if (increment < lowest)
            lowest = increment;
    }
    // All increments above the rail render identically after the existing
    // clamp.
    const bool constant = highest == lowest || lowest >= 0x40000000u;
    if (highest > 0x40000000u)
        highest = 0x40000000u;
    // Select safe harmonic limits for every pitch in this block.
    const Band first(highest), second(second_increment(highest, ratio, octave));
    const i32 coefficient = pole(cutoff_position(p, pitch[0]));
    if (voice.cached_filter.g != coefficient ||
        voice.cached_resonance != p[1]) {
        voice.cached_filter = Filter(coefficient, p[1]);
        voice.cached_resonance = p[1];
    }
    const Filter filter(voice.cached_filter);
    const i32 balance = p[5] >> 16;
    const u32 other_highest = second_increment(highest, ratio, octave);
    const i32 first_drive = interpolate(four_pole_tables::drive, highest, 21);
    const i32 second_drive =
        interpolate(four_pole_tables::drive, other_highest, 21);
    const i32 maximum_drive =
        first_drive + ((second_drive - first_drive) * balance >> 15);
    const i32 drive = 256 + multiply(p[2], maximum_drive - 256);
    const Oscillator osc_a(first, target_wave[0]),
        osc_b(second, target_wave[1]);
    const bool changing_waves =
        voice.wave[0] != target_wave[0] || voice.wave[1] != target_wave[1];
    if (changing_waves) {
        if (constant)
            render_audio<true, true>(voice, output, pitch, first, second, osc_a,
                                     osc_b, filter, target_wave, ratio, octave,
                                     balance, drive);
        else
            render_audio<true, false>(voice, output, pitch, first, second,
                                      osc_a, osc_b, filter, target_wave, ratio,
                                      octave, balance, drive);
    } else {
        if (constant)
            render_audio<false, true>(voice, output, pitch, first, second,
                                      osc_a, osc_b, filter, target_wave, ratio,
                                      octave, balance, drive);
        else
            render_audio<false, false>(voice, output, pitch, first, second,
                                       osc_a, osc_b, filter, target_wave, ratio,
                                       octave, balance, drive);
    }
}
} // namespace mando
