#include "../engine.cpp"
#include "cases.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
static void require(bool okay, const char *why) {
    if (!okay) {
        std::puts(why);
        std::exit(1);
    }
}
int main() {
    unsigned prepared = 0;
    unsigned long long total = 0;
    for (unsigned scene = 0; scene < 45; ++scene) {
        sift::Voice voice = {};
        sift::Shared shared = {};
        sift::initialize(voice, shared, scene + 1);
        opxy::i32 p[8] = {}, out[64];
        opxy::u32 pitch[64];
        for (unsigned block = 0; block < 1024; ++block) {
            split_cases::parameters(scene, block, p);
            for (unsigned i = 0; i < 64; ++i)
                pitch[i] = split_cases::pitch(scene, block, i);
            if (scene == 43 && block % 11 == 0)
                sift::start(voice, shared, scene + 1);
            const unsigned previous = voice.build_at;
            const bool building = voice.building;
            const opxy::u32 before = voice.phase;
            sift::render(voice, shared, scene + 1, out, p, pitch);
            opxy::u32 expected = before;
            for (unsigned i = 0; i < 64; ++i) {
                expected += pitch[i] > 0x22d3b6a0u ? 0x22d3b6a0u : pitch[i];
                require(out[i] >= -536870912 && out[i] <= 536870912, "FAIL output peak");
                total += static_cast<unsigned long long>(
                    out[i] < 0 ? -static_cast<long long>(out[i]) : out[i]);
            }
            require(voice.phase == expected, "FAIL sample-accurate pitch/glide");
            require(voice.build_at <= 256, "FAIL table extent");
            if (building)
                require(voice.build_at == previous + 16, "FAIL unbounded preparation tile");
            if (voice.valid)
                ++prepared;
        }
        if (scene != 42)
            require(voice.valid, "FAIL no prepared wave");
    }
    require(prepared > 40000 && total > 0, "FAIL inactive renderer");
    // Every control, including Z mix and Morph in the Z-only sound, must change
    // the actual prepared waveform, not merely its animation or cache key.
    for (unsigned control = 0; control < 4; ++control) {
        unsigned long long hashes[2] = {};
        for (unsigned setting = 0; setting < 2; ++setting) {
            sift::Voice voice = {};
            sift::Shared shared = {};
            sift::initialize(voice, shared, 1);
            opxy::i32 p[8] = {858993459, 1073741823, 858993459, 0}, out[64];
            opxy::u32 pitch[64];
            p[control] = setting ? 2147483647 : 0;
            if (control == 2)
                p[3] = 2147483647;
            for (auto &word : pitch)
                word = 25479940;
            for (unsigned block = 0; block < 64; ++block)
                sift::render(voice, shared, 1, out, p, pitch);
            auto h = 14695981039346656037ull;
            for (unsigned i = 0; i < 256; ++i)
                h = split_cases::hash(h, static_cast<opxy::u32>(voice.cycle[voice.active][i]));
            hashes[setting] = h;
        }
        require(hashes[0] != hashes[1], "FAIL inaudible control");
    }
    std::puts("PASS: 45 long redesign scenes, bounded preparation under automation, exact "
              "pitch/glide, output limits, retriggers and audible control changes");
}
