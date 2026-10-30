#pragma once
#include <opxy/effect.hpp>
namespace hkcrush {
// Normalize the imported 16-bit DSP without assuming a stock send amplitude.
// Keep the gain fixed until a louder block requires more headroom.
struct SendLevel {
    opxy::u32 lift;
    int shift;
    SendLevel(opxy::Channel left, opxy::Channel right, opxy::u32 previous) {
        opxy::u32 peak=0;
        for(unsigned i=0;i<64;++i) {
            const opxy::i32 samples[]={left[i],right[i]};
            for(opxy::i32 sample:samples) {
                const opxy::u32 bits=static_cast<opxy::u32>(sample);
                const opxy::u32 magnitude=sample<0 ? 0u-bits : bits;
                if(magnitude>peak) peak=magnitude;
            }
        }
        lift=0;
        while(lift<previous && peak<=(0x20000000u>>(lift+1))) ++lift;
        shift=16-static_cast<int>(lift);
    }
    short input(opxy::i32 sample) const {
        return static_cast<short>(shift>=0 ? sample>>shift :
            static_cast<opxy::i32>(static_cast<opxy::u32>(sample)<<(-shift)));
    }
    opxy::i32 output(short sample) const {
        return shift>=0 ? static_cast<opxy::i32>(sample)*(opxy::i32(1)<<shift) :
            static_cast<opxy::i32>(sample)>>(-shift);
    }
};
}
