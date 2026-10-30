#pragma once
#include <opxy/effect.hpp>
namespace hktape {
// Normalize the imported 16-bit DSP without assuming a stock send amplitude.
// A shared stereo exponent preserves the return level across gain changes.
// Visit typed audio storage to inspect headroom or change its binary units.
// Energy detectors use power=2; positions and coefficients are never visited.
struct AudioScale {
    int direction;
    unsigned bits, room;
    AudioScale(int change,unsigned amount):direction(change),bits(amount),room(amount) {}
    template<class T> void values(T& value,unsigned power=1,opxy::u32 ceiling=32767) {
        const unsigned shift=bits*power;
        if(direction<0) value=static_cast<T>(value>>(shift<31 ? shift : 31));
        else if(direction>0) value=shift<32 ? static_cast<T>(static_cast<opxy::u32>(value)<<shift) : T(0);
        else {
            const opxy::u32 word=static_cast<opxy::u32>(value);
            const opxy::u32 magnitude=value<0 ? 0u-word : word;
            if(!magnitude) return;
            const opxy::u32 maximum=value<0 ? ceiling+1 : ceiling;
            // room only falls across the whole scan, so these decrements
            // total at most twenty, rather than twenty per stored sample.
            while(room && (room*power>=32 || magnitude>(maximum>>(room*power)))) --room;
        }
    }
    template<class T, unsigned long N> void values(T (&array)[N],unsigned power=1,opxy::u32 ceiling=32767) {
        for(unsigned long i=0;i<N && (direction || room);++i) values(array[i],power,ceiling);
    }
    template<class T> void samples(T* data,unsigned count,opxy::u32 ceiling=32767) {
        for(unsigned i=0;i<count && (direction || room);++i) values(data[i],1,ceiling);
    }
};
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
        set_lift(lift);
    }
    void set_lift(opxy::u32 value) {lift=value;shift=16-static_cast<int>(lift);}
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
