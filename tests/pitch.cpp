#include "../sample.cpp"
#include <cmath>
#include <cstdio>

namespace {
opxy::i16 pcm[192000];
bool measure(unsigned source_rate, unsigned pitch_increment, int transpose, int cents, bool frozen, double expected) {
    for (unsigned i = 0; i < source_rate; ++i)
        pcm[i] = static_cast<opxy::i16>(18000 * std::sin(6.283185307179586 * 441 * i / source_rate));
    granular::Voice voice;
    granular::initialize(voice, opxy::SampleView{pcm, source_rate, source_rate, 1});
    opxy::i32 output[128], parameters[8] = {0, 0x7fffffff, 0x36db6db6, 0,
        frozen ? 0x40000000 : 0x60000000, 0, 0, 0x7fffffff};
    opxy::u32 pitch[64];
    for (unsigned i = 0; i < 64; ++i) pitch[i] = pitch_increment;
    opxy::SampleBlock block{output, parameters, pitch, {1200, 0, true},
        {0, source_rate, opxy::sample_unity_level, false, cents}, nullptr, 0, 2, transpose, 0};
    int last_sign = 0;
    unsigned crossings = 0, count = 0;
    for (unsigned b = 0; b < 1200; ++b) {
        granular::render_sample(voice, block);
        if (b < 400) continue;
        for (unsigned i = 0; i < 64; ++i) {
            const int sign = output[2 * i] > 0 ? 1 : output[2 * i] < 0 ? -1 : 0;
            if (sign && last_sign < 0 && sign > 0) ++crossings;
            if (sign) last_sign = sign;
            ++count;
        }
    }
    const double frequency = crossings * 44100.0 / count;
    if (std::fabs(frequency - expected) > expected * 0.025) {
        std::fprintf(stderr, "rate=%u pitch=%u transpose=%d cents=%d freeze=%d: %.2f Hz, expected %.2f\n",
            source_rate, pitch_increment, transpose, cents, frozen, frequency, expected);
        return false;
    }
    return true;
}
}
int main() {
    const unsigned rates[] = {22050u, 44100u, 48000u, 96000u};
    for (unsigned rate : rates) {
        if (!measure(rate, 25480370u, 0, 0, false, 441)) return 1;
        if (!measure(rate, 50960740u, 0, 0, false, 882)) return 2;
        if (!measure(rate, 25480370u, 12, 0, false, 882)) return 3;
        if (!measure(rate, 25480370u, 0, -1200, false, 220.5)) return 4;
        if (!measure(rate, 25480370u, 0, 0, true, 441)) return 5;
    }
    return 0;
}
