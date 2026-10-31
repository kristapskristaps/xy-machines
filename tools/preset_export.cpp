// Host authoring helper. The SDK owns the packed-pad codec used by the device.
#include "../models.h"
#include <drum_storage.hpp>
#include <cstdio>
#include <cstdlib>

int main(int argc, char** argv) {
    if (argc != 9) return 1;
    int offsets[8];
    for (unsigned i = 0; i < 8; ++i) {
        char* end;
        const long value = std::strtol(argv[i+1],&end,10);
        if (*end || value < -64 || value > 63) return 1;
        offsets[i] = int(value);
    }
    using namespace opxy;
    using namespace opxy::platform;
    const DrumCodec codec = drum_codec(x0x::model_count,x0x::control_count);
    if (!codec.fits()) return 1;
    std::puts("[");
    for (unsigned i = 0; i < 24; ++i) {
        u8 model, controls[8]; x0x::default_pad(53+i,&model,controls);
        for (unsigned s = 0; s < 8; ++s) {
            const int value = int(controls[s])+offsets[s];
            controls[s] = u8(value < 0 ? 0 : value > 127 ? 127 : value);
        }
        i32 words[4];
        if (!drum_pack(model,controls,words,codec)) return 1;
        const char* mode = model == x0x::closed_hat || model == x0x::open_hat ? "mute1" : "oneshot";
        std::printf("{\"model\":\"fm slap\",\"params\":[%u,%u,%u,%u],\"playmode\":\"%s\",\"hikey\":%u}%s\n",
            u32(words[0])>>16,u32(words[1])>>16,u32(words[2])>>16,u32(words[3])>>16,mode,53+i,i==23?"":",");
    }
    std::puts("]");
}
