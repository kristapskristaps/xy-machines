#include "../engine.cpp"
#include <cstdio>
#include <cstdlib>

namespace {
using opxy::i32; using opxy::u32;
u32 random_word(u32& state) { return state = state * 1664525u + 1013904223u; }
u32 hash_word(u32 hash, u32 value) {
    for (unsigned byte = 0; byte < 4; ++byte) { hash = (hash ^ (value & 255u)) * 16777619u; value >>= 8; }
    return hash;
}
u32 reference_run(unsigned model) {
    bitflip::Voice voice = {}; bitflip::Shared shared = {};
    i32 parameters[8], output[64]; u32 pitch[64], random = 7139 + model * 917u, hash = 2166136261u;
    bitflip::start(voice, shared, model + 1);
    for (unsigned block = 0; block < 4096; ++block) {
        if (block % 127 == 0) bitflip::start(voice, shared, block / 127 + 1);
        parameters[0] = static_cast<i32>((model * 2u + 1u) * 134217728u);
        for (unsigned c = 1; c < 8; ++c) {
            const u32 word = random_word(random);
            parameters[c] = block < 128 ? ((block & (1u << (c - 1))) ? 2147483647 : 0)
                : block % 31 == 0 ? static_cast<i32>(word) : static_cast<i32>(word >> 1);
        }
        const u32 base = 0x00100000u + (random_word(random) & 0x07ffffffu);
        for (unsigned i = 0; i < 64; ++i)
            pitch[i] = block % 13 == 0 ? random_word(random) : base + i * 8191u;
        bitflip::render(voice, shared, model + 1, output, parameters, pitch);
        for (i32 sample : output) hash = hash_word(hash, static_cast<u32>(sample));
        const u32 state[] = {
            voice.phase, voice.phase2, voice.mod_phase, voice.noise_phase, voice.arp_phase,
            voice.arp_step, voice.arp_type, voice.long_noise, voice.short_noise, voice.clock_phase,
            voice.initialized, voice.model, static_cast<u32>(voice.color), static_cast<u32>(voice.edge),
            static_cast<u32>(voice.sweep_envelope), static_cast<u32>(voice.sweep_ratio),
            static_cast<u32>(voice.fade), static_cast<u32>(voice.model_fade), static_cast<u32>(voice.model_from),
            static_cast<u32>(voice.last_raw), static_cast<u32>(voice.held),
            static_cast<u32>(voice.dc_input), static_cast<u32>(voice.dc_output)
        };
        for (u32 value : state) hash = hash_word(hash, value);
    }
    return hash;
}
}
int main() {
    // PCM and sound-state fingerprints of the original 72bd776 renderer.
    const u32 expected[8] = { 0x68c5b53du,0xbd48206fu,0x90de8b1fu,0x6bf8b9cbu,0x304b379eu,0xe4f782d7u,0xef2a4dc7u,0x7b30ed11u };
    for (unsigned model = 0; model < 8; ++model) {
        const u32 actual = reference_run(model);
        if (actual != expected[model]) {
            std::fprintf(stderr, "FAIL: model %u changed its exact sound/state: %08x != %08x\n",
                         model, actual, expected[model]);
            return 1;
        }
    }
    std::puts("PASS: all eight models retain original PCM and sound state under control motion, pitch glides and retriggers");
}
