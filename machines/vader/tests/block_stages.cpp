#include "../engine.cpp"
#include <cstdio>
#include <cstring>

namespace {
using opxy::i32;
using opxy::u32;
using Hash = unsigned long long;
unsigned covered = 0;
Hash word(Hash hash, u32 value) { return (hash ^ value) * 1099511628211ull; }
Hash scenario(unsigned setting) {
    vader::Voice voice;
    std::memset(&voice, 0, sizeof(voice));
    vader::Shared shared = {};
    vader::start(voice, shared, 7);
    i32 parameters[8] = {268435455, 644245094, 1073741824, 0, 0, 429496729, 1610612735, 2147483647};
    parameters[3] = static_cast<i32>((setting % 4) * 536870912u);
    if (setting == 0) parameters[2] = 0;
    if (setting == 8) parameters[4] = 2147483647;
    u32 random = 7139, pitch[opxy::block_frames];
    i32 output[opxy::block_frames];
    Hash hash = 14695981039346656037ull;
    const u32 edges[] = {0, 1, 1u << 22, 29996596, 60960826, 125985707, 0x70000000u, 0xffffffffu};
    for (unsigned block = 0; block < 2048; ++block) {
        if (setting == 4 && block % 13 == 0) parameters[2] = block % 26 ? 2147483647 : 0;
        if ((setting == 5 || setting == 6) && block % 31 == 0) {
            for (i32& value : parameters) value = static_cast<i32>((random = random * 1664525u + 1013904223u) >> 1);
        }
        if (setting == 6 && block % 79 == 0) vader::start(voice, shared, block + 7);
        if (setting == 8 && block == 64) parameters[4] = 0;
        if (setting == 9 || block > 1024) voice.sweep = 0;
        for (unsigned i = 0; i < opxy::block_frames; ++i) {
            pitch[i] = setting == 7 ? edges[(block + i) % 8] : 25478335u;
            if (setting == 6) pitch[i] += (block * 64 + i) * 7919u;
        }
        const i32 old_drive = voice.drive;
        vader::render(voice, shared, 7, output, parameters, pitch);
        covered |= 1u << voice.cache_status;
        if (old_drive != voice.drive) covered |= 8;
        if (voice.sweep == 0) covered |= 16;
        for (i32 sample : output) hash = word(hash, static_cast<u32>(sample));
        static_assert(sizeof(voice) % 4 == 0, "Vader state is word aligned");
        u32 state[sizeof(voice) / 4];
        std::memcpy(state, &voice, sizeof(voice));
        for (u32 value : state) hash = word(hash, value);
    }
    return hash;
}
}

int main() {
    // PCM and full zero-initialized state from main 71045781d5b70cae3cf936a7ba676cd91a80efb8.
    const Hash reference[] = {
        9029417311586065576ull,
        1639591624220103847ull,
        14972929511546508385ull,
        3359921560146292301ull,
        16907258125562767050ull,
        6912011002676587543ull,
        1073910356976033597ull,
        8824759215693541564ull,
        17069156557332508527ull,
        11841376127176681395ull,
    };
    for (unsigned setting = 0; setting < 10; ++setting) {
        const Hash actual = scenario(setting);
        if (actual != reference[setting]) {
            std::fprintf(stderr, "FAIL: Vader cached block-stage audio/state %u: %llu\n", setting, actual);
            return 1;
        }
    }
    if (covered != 31) {
        std::fprintf(stderr, "FAIL: cache/ramp/tail coverage %u\n", covered);
        return 1;
    }
    std::puts("PASS: exact PCM and complete initialized voice state over 1,310,720 samples, including cache miss/build/hit, drive ramps, retriggers and pitch extremes");
}
