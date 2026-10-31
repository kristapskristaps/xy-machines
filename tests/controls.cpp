#include "../engine.cpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
static void require(bool ok, const char* message) {
    if (!ok) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(1); }
}
int main() {
    using namespace x0x;
    opxy::u8 models[pad_count], values[pad_count][8];
    for (opxy::u32 pad = 0; pad < pad_count; ++pad) {
        drum_defaults(53 + pad, models + pad, values[pad]);
        require(models[pad] < model_count, "canonical pad has valid model");
        require(values[pad][tune] == 64 && values[pad][drive] == 0, "pads start tuned and clean");
        for (auto value : values[pad]) require(value <= 127, "seven-bit control");
    }
    require(models[0] == kick && models[1] == snare && models[7] == closed_hat && models[8] == open_hat, "playable layout");
    require(family_index(ride, values[10][style]) == 3 && family_index(tambourine, values[17][style]) == 1, "available defaults");
    opxy::u32 full = 0;
    for (opxy::u32 m = 0; m < model_count; ++m) {
        require(std::strlen(model_name(m)) <= 7, "model caption fits header");
        const opxy::u32 count = parameter_count(m); full += count == 8;
        require(count >= 4 && count <= 8 && family_count(m), "valid controls and styles");
        for (opxy::u32 s = 0; s < 8; ++s) {
            const char* label = parameter_name(m, s);
            require(std::strlen(label) <= 8 && bool(label[0]) == (s < count), "short labels; unused slot blank");
        }
        for (opxy::u32 raw = 0; raw < 128; ++raw) {
            require(family_masks[m] & (1u << family_index(m, raw)), "style available on original family");
            if (raw) require(family_index(m, raw) >= family_index(m, raw - 1), "clockwise increases");
        }
        for (opxy::u32 f = 0; f < 4; ++f) if (family_masks[m] & (1u << f))
            require(family_index(m, family_raw(m, f)) == f, "style choice round-trips");
    }
    require(full <= 15 && !parameter_name(claves, 7)[0], "fits prefix storage without filler eighth controls");
    require(!std::strcmp(parameter_name(kick, character), "sweep") && !std::strcmp(parameter_name(snare, character), "snappy"), "type-specific controls");
    require(tune_cents(0) == -1200 && tune_cents(64) == 0 && tune_cents(127) == 1200, "tune endpoints and centre");
    Voice a = {}, b = {}; Shared shared;
    initialize(a, shared, 1); initialize(b, shared, 2);
    drum_select(a, shared, 1, snare); drum_select(b, shared, 2, closed_hat);
    require(a.model == snare && b.model == closed_hat, "independent voice models");
    opxy::i32 output[64], controls[8] = {}; opxy::u32 pitch[64] = {};
    for (auto& sample : output) sample = 123;
    render(a, shared, 1, output, controls, pitch);
    for (auto sample : output) require(sample == 0, "controls milestone writes silence");
    std::puts("PASS: 24 defaults, 18 types, available styles, two-page labels and independent voices");
}
