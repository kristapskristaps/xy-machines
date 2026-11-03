#pragma once
#include "dsp_common.h"
#include "tables.h"
#include "dense_points.h"

namespace westfold { namespace dsp {
inline i32 dense_read(const opxy::i16* row,u32 phase) {return row[((phase+0x200000u)>>22)&1023u];}
inline i32 sine(u32 phase) { return read(tables::sine, phase); }
inline i32 wave(u32 fold, i32 fraction, const Band& band, u32 phase) {
    const i32 a = mix(dense_read(dense::waves[band.index][fold], phase),
                      dense_read(dense::waves[band.index][fold + 1], phase), fraction);
    if (!band.fade || band.index == 7) return a;
    const i32 b = mix(dense_read(dense::waves[band.index + 1][fold], phase),
                      dense_read(dense::waves[band.index + 1][fold + 1], phase), fraction);
    return mix(a, b, band.fade);
}
inline i32 clean(const Band& band, u32 phase) {
    const i32 a = dense_read(dense::waves[band.index][0], phase);
    if (!band.fade || band.index == 7) return a;
    return mix(a, dense_read(dense::waves[band.index + 1][0], phase), band.fade);
}
} }
