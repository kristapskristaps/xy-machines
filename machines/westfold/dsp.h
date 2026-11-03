#pragma once
#include "dsp_common.h"
#include "tables.h"
#include "half_waves.h"

namespace westfold { namespace dsp {
inline i32 read_half(const opxy::i16* table,u32 phase) {
    const u32 index=(phase>>23)&255u;
    i32 first=table[index],second=table[index+1];
    // Reconstruct endpoints before rounding the interpolation product.
    if(phase>>31){first=-first;second=-second;}
    return mix(first,second,static_cast<i32>((phase>>8)&32767u));
}
inline i32 sine(u32 phase) { return read_half(half_tables::sine,phase); }
inline i32 wave(u32 fold, i32 fraction, const Band& band, u32 phase) {
    const i32 a = mix(read_half(half_tables::waves[band.index][fold], phase),
                      read_half(half_tables::waves[band.index][fold + 1], phase), fraction);
    if (!band.fade || band.index == 7) return a;
    const i32 b = mix(read_half(half_tables::waves[band.index + 1][fold], phase),
                      read_half(half_tables::waves[band.index + 1][fold + 1], phase), fraction);
    return mix(a, b, band.fade);
}
inline i32 clean(const Band& band, u32 phase) {
    const i32 a = read_half(half_tables::waves[band.index][0], phase);
    if (!band.fade || band.index == 7) return a;
    return mix(a, read_half(half_tables::waves[band.index + 1][0], phase), band.fade);
}
} }
