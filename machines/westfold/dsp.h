#pragma once
#include "dsp_common.h"
#include "tables.h"

namespace westfold { namespace dsp {
// Audio inputs fit signed16 and weights are≤32767. Truncating the difference
// toward zero keeps the interpolation inside its input bounds and changes it
// by at most one Q15 count while making both multiply operands signed16.
inline i32 blend16(i32 a,i32 b,i32 amount) {
    return a+(static_cast<opxy::i16>((b-a)/2)*static_cast<opxy::i16>(amount)>>14);
}
inline i32 sine(u32 phase) { return read(tables::sine, phase); }
inline i32 wave(u32 fold, i32 fraction, const Band& band, u32 phase) {
    const i32 a = mix(read(tables::waves[band.index][fold], phase),
                      read(tables::waves[band.index][fold + 1], phase), fraction);
    if (!band.fade || band.index == 7) return a;
    const i32 b = mix(read(tables::waves[band.index + 1][fold], phase),
                      read(tables::waves[band.index + 1][fold + 1], phase), fraction);
    return mix(a, b, band.fade);
}
inline i32 clean(const Band& band, u32 phase) {
    const i32 a = read(tables::waves[band.index][0], phase);
    if (!band.fade || band.index == 7) return a;
    return mix(a, read(tables::waves[band.index + 1][0], phase), band.fade);
}
} }
