#include "../engine.cpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
static void require(bool ok, const char* text) {
    if (!ok) { std::fprintf(stderr, "FAIL: %s\n", text); std::exit(1); }
}
int main() {
    using namespace x0x;
    struct Buffer { opxy::i32 before, audio[64], after; } out;
    opxy::u32 pitch[64]; for (auto& value : pitch) value = 25479940;
    Shared shared;
    unsigned cases = 0;
    for (unsigned model = 0; model < model_count; ++model) for (unsigned family = 0; family < 4; ++family) {
        if (!(family_masks[model] & (1u << family))) continue;
        for (unsigned corner = 0; corner < 128; ++corner) {
            Voice voice; initialize(voice, shared, 1); start(voice, shared, 1); drum_select(voice, shared, 1, model);
            opxy::i32 p[8] = {};
            p[0] = opxy::i32(unsigned(family_raw(model, family)) << 24);
            for (unsigned s = 1; s < 8; ++s) p[s] = corner & (1u << (s - 1)) ? 0x7fffffff : 0;
            opxy::i32 original[8]; std::memcpy(original, p, sizeof(p));
            out.before = 0x12345678; out.after = 0x23456789;
            for (unsigned block = 0; block < 32; ++block) {
                for (auto& sample : out.audio) sample = 0x7fffffff;
                render(voice, shared, 1, out.audio, p, pitch);
                require(out.before == 0x12345678 && out.after == 0x23456789, "output bounds");
                for (auto sample : out.audio) require(sample >= -1073741824 && sample <= 1073741824, "declared peak at control corners");
            }
            require(!std::memcmp(original, p, sizeof(p)), "parameter buffer remains read-only");
            for (auto value : pitch) require(value == 25479940, "pitch buffer remains read-only");
            ++cases;
        }
    }
    // Interleaving another pad cannot affect oscillator, envelope or noise state.
    Voice a, reference, other;
    initialize(a, shared, 1); initialize(reference, shared, 1); initialize(other, shared, 2);
    start(a, shared, 1); start(reference, shared, 1); start(other, shared, 2);
    drum_select(a, shared, 1, snare); drum_select(reference, shared, 1, snare); drum_select(other, shared, 2, cymbal);
    opxy::i32 p[8] = {0x50000000,0x40000000,0x30000000,0x40000000,0,0x40000000,0x40000000,0};
    opxy::i32 expected[64], discard[64];
    for (unsigned block = 0; block < 1000; ++block) {
        render(a, shared, 1, out.audio, p, pitch);
        render(other, shared, 2, discard, p, pitch);
        render(reference, shared, 1, expected, p, pitch);
        require(!std::memcmp(out.audio, expected, sizeof(expected)), "pads have independent DSP state");
    }
    std::printf("PASS: %u control corners, buffer ownership, peak bounds and DSP isolation\n", cases);
}
