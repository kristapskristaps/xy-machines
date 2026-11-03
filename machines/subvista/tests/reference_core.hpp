// Frozen exact reference at 87ef0be94632955bf72c6560ff91ea588f7111d3.
#pragma once
#include <opxy/audio.hpp>
namespace subvista_reference {
struct Voice {
    opxy::u32 phase, detuned, sub;
    opxy::i32 integrator1, integrator2, envelope, a1, a2, a3;
};
struct Shared {};
}
