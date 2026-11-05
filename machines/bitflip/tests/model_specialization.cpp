#include "../engine.cpp"
#include "reference/pre_model_specialization.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstddef>

static void require(bool value, const char* message) {
    if (!value) { std::puts(message); std::exit(1); }
}

int main() {
    using opxy::u32; using opxy::i32;
    u32 random = 7139;
    for (u32 scene = 0; scene < 16; ++scene) {
        bitflip::Voice actual = {}, expected = {};
        bitflip::Shared shared = {};
        bitflip::initialize(actual, shared, scene+1);
        bitflip_reference::initialize(expected, shared, scene+1);
        i32 parameters[8] = {}, a[64], b[64]; u32 pitch[64];
        for (u32 block = 0; block < 256; ++block) {
            if (!(block % 5)) {
                for (auto& value : parameters) {
                    random = random*1664525u+1013904223u;
                    value = static_cast<i32>(random>>1);
                }
            }
            const u32 model = scene < 8 ? scene : (block/17)%8;
            parameters[0] = static_cast<i32>((2*model+1)*0x08000000u);
            if (scene < 8) parameters[3] = 0x40000000;
            for (u32 i = 0; i < 64; ++i)
                pitch[i] = scene&1 ? 25479940u : 2678267u+block*75001u+i*4097u;
            if (!(block % 29)) {
                bitflip::start(actual, shared, scene+1);
                bitflip_reference::start(expected, shared, scene+1);
            }
            bitflip::render(actual, shared, scene+1, a, parameters, pitch);
            bitflip_reference::render(expected, shared, scene+1, b, parameters, pitch);
            require(!std::memcmp(a,b,sizeof(a)), "FAIL specialized model changed PCM");
            require(!std::memcmp(&actual,&expected,offsetof(bitflip::Voice,prepared_wave)), "FAIL specialized model changed original voice state");
        }
    }
    std::puts("PASS: all eight specialized models match frozen PCM/state through automation, glide, model changes and retriggers");
}
