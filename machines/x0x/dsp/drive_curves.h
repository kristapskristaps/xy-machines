#pragma once
#include "primitives.h"
namespace x0x { namespace tables {
extern const opxy::i16 drive_curves[128][129];
}}
namespace x0x { namespace dsp {
// The output lowpass is a convex Q15 update, bounding |sample| to 32767.
static inline i32 table_drive(i32 sample,const opxy::i16* curve){
    const u32 magnitude=sample<0?u32(-sample):u32(sample),index=magnitude>>8;
    const i32 a=curve[index],b=curve[index+1];
    const i32 shaped=a+((b-a)*i32(magnitude&255u)>>8);
    return sample<0?-shaped:shaped;
}
}}
