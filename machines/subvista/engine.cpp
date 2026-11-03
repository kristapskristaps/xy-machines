#include "core.h"
#include "tables.h"
#include "wave_slopes.h"
#include "wave_pairs.h"
#include <opxy/q31.hpp>
namespace subvista {
namespace {
using opxy::i32;
using opxy::u32;
using opxy::q31::multiply;
using opxy::q31::add;
i32 limit(i32 value, i32 bound) { return value < -bound ? -bound : value > bound ? bound : value; }
i32 blend(i32 a, i32 b, i32 amount) { return a + multiply(b - a, amount); }
i32 wave(u32 phase, u32 band) {
    const u32 index = phase >> 23;
    return tables::saw[band][index] + tables::slopes[band][index]
           * static_cast<i32>((phase & 0x7fffffu) >> 8);
}
template<bool SawOnly>
__attribute__((always_inline)) inline i32 oscillator(u32 phase, u32 band, i32 shape) {
    if(SawOnly)return wave(phase,band);
    const tables::WavePair& point=tables::pairs[band][(phase>>23)&255u];
    const i32 fraction=static_cast<i32>((phase&0x7fffffu)>>8);
    const i32 first=point.values[0]+point.slopes[0]*fraction;
    const i32 second=point.values[1]+point.slopes[1]*fraction;
    const i32 saw=(phase>>31)?second:first;
    const i32 other=(phase>>31)?first:second;
    return blend(saw,opxy::q31::half(saw-other),shape);
}
tables::Coefficients coefficients(i32 cutoff, i32 resonance) {
    const u32 c = static_cast<u32>(cutoff) >> 25;
    const u32 r = static_cast<u32>(resonance) >> 27;
    const i32 cf = static_cast<i32>((static_cast<u32>(cutoff) & 0x1ffffffu) << 6);
    const i32 rf = static_cast<i32>((static_cast<u32>(resonance) & 0x7ffffffu) << 4);
    const tables::Coefficients &a = tables::lowpass[r][c], &b = tables::lowpass[r][c + 1];
    const tables::Coefficients &d = tables::lowpass[r + 1][c], &e = tables::lowpass[r + 1][c + 1];
    const tables::Coefficients result = {
        blend(blend(a.a1,b.a1,cf),blend(d.a1,e.a1,cf),rf),
        blend(blend(a.a2,b.a2,cf),blend(d.a2,e.a2,cf),rf),
        blend(blend(a.a3,b.a3,cf),blend(d.a3,e.a3,cf),rf)};
    return result;
}
template<bool SawOnly>
void render_block(Voice& v,opxy::Output output,opxy::Parameters parameters,opxy::Pitch pitch,
                  u32 band,i32 step1,i32 step2,i32 step3) {
    for (u32 i=0;i<opxy::block_frames;++i) {
        v.a1 += step1; v.a2 += step2; v.a3 += step3;
        const u32 increment = pitch[i], detune = increment + (increment >> 8);
        const i32 input = opxy::q31::half(oscillator<SawOnly>(v.phase,band,parameters[2]))
                        + opxy::q31::half(oscillator<SawOnly>(v.detuned,band,parameters[2]))
                        + wave(v.sub,0)/8;
        // Topology-preserving two-pole low-pass. Headroom bounds keep feedback defined.
        const i32 difference = input-v.integrator2;
        const i32 band = limit(add(multiply(v.a1,v.integrator1),multiply(v.a2,difference)),0x30000000);
        const i32 low = limit(add(add(v.integrator2,multiply(v.a2,v.integrator1)),multiply(v.a3,difference)),0x30000000);
        v.integrator1 = limit(add(band,band-v.integrator1),0x30000000);
        v.integrator2 = limit(add(low,low-v.integrator2),0x30000000);
        output[i] = limit(low,0x40000000);
        v.phase += increment; v.detuned += detune; v.sub += increment/2;
    }
}
}
inline void start(Voice& v, Shared&, opxy::u32) {
    v.phase = v.sub = 0;
    v.detuned = 0x28000000u;
    v.integrator1 = v.integrator2 = 0;
    v.envelope = 0x7fffffff;
    v.a1 = v.a2 = v.a3 = 0;
}
inline void initialize(Voice& v, Shared& shared, opxy::u32 owner) { start(v,shared,owner); }
inline void destroy(Voice&, Shared&, opxy::u32) {}
inline void render(Voice& v, Shared&, opxy::u32, opxy::Output output,
                   opxy::Parameters parameters, opxy::Pitch pitch) {
    // Contour opens the filter on each note, then settles in about 300 ms.
    const i32 base = parameters[0] < 0 ? 0 : parameters[0];
    const i32 contour = multiply(multiply(v.envelope,parameters[3]),751619276);
    const i32 cutoff = base > 0x7fffffff - contour ? 0x7fffffff : base + contour;
    const tables::Coefficients target = coefficients(cutoff,parameters[1] < 0 ? 0 : parameters[1]);
    if (!v.a1 && !v.a2 && !v.a3) { v.a1=target.a1; v.a2=target.a2; v.a3=target.a3; }
    const i32 step1=(target.a1-v.a1)/64, step2=(target.a2-v.a2)/64, step3=(target.a3-v.a3)/64;
    u32 highest=0;
    for (u32 i=0;i<opxy::block_frames;++i) if (pitch[i]>highest) highest=pitch[i];
    highest += highest >> 8;
    u32 band=0;
    while (band<7 && highest<(0x80000000u>>(band+1))) ++band;
    // Shape is fixed for this callback; the saw endpoint needs no pulse lookup.
    if(parameters[2]==0)render_block<true>(v,output,parameters,pitch,band,step1,step2,step3);
    else render_block<false>(v,output,parameters,pitch,band,step1,step2,step3);
    v.envelope = multiply(v.envelope,2136937522);
}
}
