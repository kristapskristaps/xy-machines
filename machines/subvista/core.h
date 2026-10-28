#pragma once
#include <opxy/audio.hpp>
namespace subvista {
struct Voice {
    opxy::u32 phase, detuned, sub;
    opxy::i32 integrator1, integrator2, envelope, a1, a2, a3;
};
struct Shared {};
}
