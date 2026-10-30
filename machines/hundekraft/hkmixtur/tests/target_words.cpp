// Compile on Blackfin too: equal widths do not guarantee compatible sample types.
#include <opxy/types.hpp>
#include "../sine.hpp"
#include "../dsp.hpp"
int main() {
    hkmixtur::port::int32_t signed_word = 0;
    hkmixtur::port::uint32_t unsigned_word = 0;
    opxy::i32* sdk_signed = &signed_word;
    opxy::u32* sdk_unsigned = &unsigned_word;
    return static_cast<int>(*sdk_signed | *sdk_unsigned);
}
