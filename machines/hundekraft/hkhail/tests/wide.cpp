// Check DSP wide arithmetic against the host's built-in 64-bit arithmetic.
#include "../wide.hpp"
#include <cstdint>
using hkhail::port::Wide;
int main() {
    uint32_t random=0x4856837u;
    for(unsigned i=0;i<10000;++i) {
        random=random*1664525u+1013904223u;const uint64_t high=random;
        random=random*1664525u+1013904223u;const int64_t a=static_cast<int64_t>((high<<32)|random);
        random=random*1664525u+1013904223u;const int64_t b=static_cast<int32_t>(random)|1;
        const uint64_t product=static_cast<uint64_t>(a)*static_cast<uint64_t>(b);
        if(static_cast<uint64_t>(Wide(a)*Wide(b))!=product)return 1;
        if(static_cast<int64_t>(Wide(a)/Wide(b))!=a/b)return 2;
        if(static_cast<int64_t>(Wide(a)%Wide(b))!=a%b)return 3;
        if(static_cast<int64_t>(Wide(a)>>unsigned(i%63))!=(a>>unsigned(i%63)))return 4;
    }
    return 0;
}
