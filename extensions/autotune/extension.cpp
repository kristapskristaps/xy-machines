#include <opxy/extension.hpp>
#include <opxy/live_input.hpp>
#include "tuner.hpp"

namespace opxy { namespace extensions { namespace autotune {
namespace {
Tuner tuner;
constexpr const char* scales[] = {
    "off", "auto", "chromatic",
    "C major", "C# major", "D major", "Eb major", "E major", "F major",
    "F# major", "G major", "Ab major", "A major", "Bb major", "B major",
    "C minor", "C# minor", "D minor", "Eb minor", "E minor", "F minor",
    "F# minor", "G minor", "G# minor", "A minor", "Bb minor", "B minor",
};
constexpr const char* speeds[] = {"smooth", "hard"};
}
extern constexpr Hooks hooks = Hooks();
extern constexpr live_input::Controls input_controls = {
    {scales, sizeof(scales) / sizeof(*scales)},
    {speeds, sizeof(speeds) / sizeof(*speeds)},
};

void prepare_input(u32 mode, u32 retune, u32 amount, u32 automatic_mask) {
    tuner.prepare(mode, retune, amount, automatic_mask);
}

void reset_input() {
    tuner.reset();
}

void process_input(i32* stereo) {
    tuner.process(stereo);
}
}}}
