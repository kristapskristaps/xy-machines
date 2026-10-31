#pragma once
#include <opxy/types.hpp>
namespace x0x {
enum Model : opxy::u32 {
    kick, snare, clap, rim, low_tom, mid_tom, high_tom, closed_hat, open_hat,
    cymbal, ride, cowbell, claves, maracas, low_conga, mid_conga, high_conga,
    tambourine, model_count
};
enum Control : opxy::u32 { style, tune, decay, tone, attack, character, body, drive };
const opxy::u32 pad_count = 24;
// Styles present on the original families: 606, 707, 808, 909.
const opxy::u8 family_masks[model_count] = {
    15, 15, 14, 14, 15, 14, 15, 15, 15, 15, 10, 6, 4, 4, 4, 4, 4, 2
};
static inline opxy::u32 family_count(opxy::u32 model) {
    opxy::u32 count = 0;
    if (model >= model_count) return 0;
    for (opxy::u32 i = 0; i < 4; ++i) count += (family_masks[model] >> i) & 1u;
    return count;
}
static inline opxy::u32 family_index(opxy::u32 model, opxy::u32 raw) {
    if (model >= model_count) return 0;
    if (raw > 127) raw = 127;
    opxy::u32 rank = raw * family_count(model) / 128;
    for (opxy::u32 i = 0; i < 4; ++i) {
        if (!(family_masks[model] & (1u << i))) continue;
        if (!rank) return i;
        --rank;
    }
    return 0;
}
static inline opxy::u8 family_raw(opxy::u32 model, opxy::u32 family) {
    if (model >= model_count) return 0;
    opxy::u32 rank = 0;
    for (opxy::u32 i = 0; i < 4; ++i) {
        if (!(family_masks[model] & (1u << i))) continue;
        if (i == family) return opxy::u8((2 * rank + 1) * 64 / family_count(model));
        ++rank;
    }
    return 0;
}
static inline const char* family_name(opxy::u32 model, opxy::u32 raw) {
    static const char* const names[] = {"606", "707", "808", "909"};
    return names[family_index(model, raw)];
}
static inline opxy::u32 raw_control(opxy::i32 value) { return value < 0 ? 0 : opxy::u32(value) >> 24; }
static inline opxy::i32 tune_cents(opxy::u32 raw) {
    if (raw > 127) raw = 127;
    return raw <= 64 ? (opxy::i32(raw) - 64) * 1200 / 64 : (opxy::i32(raw) - 64) * 1200 / 63;
}
static inline opxy::u32 decay_ms(opxy::u32 raw) { return 20 + raw * raw / 4; }
static inline opxy::u32 control_count(opxy::u32 model) {
    // Simple percussion has no drive: twelve full models fit the SDK prefix
    // codec (at most 15 eight-control models in a kit with >16 models).
    return model >= model_count ? 0 : model >= claves ? 7 : 8;
}
static inline const char* short_name(opxy::u32 model) {
    static const char* const names[model_count] = {
        "kick", "snare", "clap", "rim", "tom lo", "tom mid", "tom hi", "cl.hat",
        "op.hat", "cymbal", "ride", "cowbell", "claves", "maracas", "conga l",
        "conga m", "conga h", "tamb"
    };
    return model < model_count ? names[model] : "";
}
static inline const char* control_name(opxy::u32 model, opxy::u32 slot) {
    if (slot >= control_count(model)) return "";
    static const char* const common[] = {"style", "tune", "decay", "tone", "attack", "", "", "drive"};
    if (slot != character && slot != body) return common[slot];
    switch (model) {
    case snare: return slot == character ? "snappy" : "body";
    case clap: return slot == character ? "spread" : "noise";
    case closed_hat: case open_hat: return slot == character ? "metal" : "open";
    case cymbal: case ride: return slot == character ? "metal" : "bell";
    case cowbell: return slot == character ? "ratio" : "body";
    case rim: case claves: return slot == character ? "click" : "body";
    case maracas: case tambourine: return slot == character ? "noise" : "body";
    default: return slot == character ? "sweep" : "body";
    }
}
static inline void default_pad(opxy::u32 key, opxy::u8* model, opxy::u8* controls) {
    static const opxy::u8 layout[pad_count] = {
        kick, snare, clap, rim, low_tom, mid_tom, high_tom, closed_hat, open_hat,
        cymbal, ride, cowbell, claves, maracas, low_conga, mid_conga, high_conga,
        tambourine, kick, snare, closed_hat, open_hat, clap, cymbal
    };
    *model = layout[key >= 53 && key <= 76 ? key - 53 : 0];
    const opxy::u8 defaults[8] = {family_raw(*model, 2), 64, 48, 64, 0, 64, 64, 0};
    for (opxy::u32 i = 0; i < 8; ++i) controls[i] = defaults[i];
    if (*model == ride) controls[style] = family_raw(*model, 3);
}
}
