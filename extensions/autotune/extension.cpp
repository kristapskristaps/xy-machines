#include <opxy/extension.hpp>
#include <opxy/live_input.hpp>
#include "dynamics.hpp"
#include "tuner.hpp"

namespace opxy { namespace extensions { namespace autotune {
namespace {
Tuner tuner;
Dynamics dynamics;
constexpr const char* scales[] = {
    "off", "auto", "chromatic",
    "C major", "C# major", "D major", "Eb major", "E major", "F major",
    "F# major", "G major", "Ab major", "A major", "Bb major", "B major",
    "C minor", "C# minor", "D minor", "Eb minor", "E minor", "F minor",
    "F# minor", "G minor", "G# minor", "A minor", "Bb minor", "B minor",
};
constexpr const char* speeds[] = {"smooth", "hard"};
constexpr const char* harmonies[] = {"off", "third up", "third down", "double"};
constexpr const char* blend[] = {"0%", "10%", "20%", "30%", "40%", "50%", "60%", "70%"};
constexpr const char* dynamics_modes[] = {"off", "soft"};
}
extern constexpr Hooks hooks = Hooks();
extern constexpr live_input::Controls input_controls = {
    {scales, sizeof(scales) / sizeof(*scales)},
    {speeds, sizeof(speeds) / sizeof(*speeds)},
};
extern constexpr live_input::ExtraControls input_extra_controls = {
    {"harmony", "blend", "compress", "de-ess"},
    {{harmonies, 4}, {blend, 8}, {dynamics_modes, 2}, {dynamics_modes, 2}},
};

void prepare_input(u32 mode, u32 retune, u32 amount, u32 automatic_mask,
                   const live_input::ExtraValues& extra = {}) {
    tuner.prepare(mode, retune, amount, automatic_mask, extra.values[0], extra.values[1]);
    dynamics.prepare(extra.values[2] != 0, extra.values[3] != 0);
}

void reset_input() {
    tuner.reset();
    dynamics.reset();
}

void process_input(i32* stereo) {
    tuner.process(stereo);
    dynamics.process(stereo);
}
}}}
