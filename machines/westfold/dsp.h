#pragma once
#include "dsp_common.h"
#include "tables.h"
#include "wave_points.h"

namespace westfold { namespace dsp {
inline i32 read_point(const u32* table,u32 phase) {
    const u32 point=table[(phase>>23)&255u];
    i32 value=static_cast<opxy::i16>(point),difference=static_cast<i32>(point&0xffff0000u);
    // Negate before interpolation: negating a rounded output loses a unit.
    if(phase>>31){value=-value;difference=-difference;}
    // The packed high half is slope*65536, so Q31 multiply retains the
    // original slope*fraction >>15 rounding in one native multiply.
    return value+opxy::q31::multiply(difference,static_cast<i32>((phase>>8)&32767u));
}
inline i32 sine(u32 phase) { return read_point(points::sine,phase); }
inline i32 wave(u32 fold, i32 fraction, const Band& band, u32 phase) {
    const i32 a = mix(read_point(points::waves[band.index][fold], phase),
                      read_point(points::waves[band.index][fold + 1], phase), fraction);
    if (!band.fade || band.index == 7) return a;
    const i32 b = mix(read_point(points::waves[band.index + 1][fold], phase),
                      read_point(points::waves[band.index + 1][fold + 1], phase), fraction);
    return mix(a, b, band.fade);
}
inline i32 clean(const Band& band, u32 phase) {
    const i32 a = read_point(points::waves[band.index][0], phase);
    if (!band.fade || band.index == 7) return a;
    return mix(a, read_point(points::waves[band.index + 1][0], phase), band.fade);
}
} }
