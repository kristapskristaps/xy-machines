#include "../engine.cpp"
#include <cmath>
#include <cstdio>
#include <vector>

namespace {
const double pi = 3.14159265358979323846;
int failures = 0;
void require(bool condition, const char* message) {
    if (!condition) { std::printf("FAIL: %s\n", message); ++failures; }
}
opxy::i32 q(double value) { return static_cast<opxy::i32>(value * 2147483647.0); }
opxy::u32 increment(double hz) { return static_cast<opxy::u32>(hz / 44100 * 4294967296.0 + .5); }
std::vector<double> render(opxy::i32* p, double hz, unsigned blocks = 384) {
    cyclops::Voice voice = {}; cyclops::Shared shared;
    opxy::u32 pitch[64]; opxy::i32 out[64];
    for (unsigned i = 0; i < 64; ++i) pitch[i] = increment(hz);
    cyclops::initialize(voice, shared, 1); cyclops::start(voice, shared, 1);
    std::vector<double> result;
    for (unsigned b = 0; b < blocks; ++b) {
        cyclops::render(voice, shared, 1, out, p, pitch);
        for (unsigned i = 0; i < 64; ++i) {
            require(out[i] >= -cyclops::dsp::peak && out[i] <= cyclops::dsp::peak, "declared output peak");
            if (b >= 128) result.push_back(out[i] / 2147483648.0);
        }
    }
    return result;
}
double magnitude(const std::vector<double>& samples, double frequency, double rate = 44100) {
    double re = 0, im = 0, weight = 0;
    for (unsigned i = 0; i < samples.size(); ++i) {
        const double phase = 2 * pi * frequency * i / rate;
        const double w = .5 - .5 * std::cos(2 * pi * i / (samples.size() - 1));
        weight += w; re += w * samples[i] * std::cos(phase); im += w * samples[i] * std::sin(phase);
    }
    return 2 * std::sqrt(re * re + im * im) / weight;
}
double gain(double hz, double resonance) {
    cyclops::Voice voice = {};
    const int position = static_cast<int>(std::log2(400.0 / 30) * 32 * 65536);
    const cyclops::dsp::Ladder filter(cyclops::dsp::pole(position), q(resonance));
    double re = 0, im = 0;
    const unsigned count = 32768;
    for (unsigned i = 0; i < count * 2; ++i) {
        const double phase = 2 * pi * hz * i / 44100;
        const int output = filter.step(voice, static_cast<int>(std::sin(phase) * 819));
        if (i >= count) { re += output * std::cos(phase); im += output * std::sin(phase); }
    }
    return 2 * std::sqrt(re * re + im * im) / count / 819;
}
}
int main() {
    using namespace cyclops;
    using namespace cyclops::dsp;
    require(option(0, 4) == 0 && option(2147483647, 4) == 3, "wave enum endpoints");
    require(option(q(.5), 3) == 1, "zero octave is the centre option");
    require(tuning(0) == (1u << 20), "zero detune is exact unison");
    require(std::abs(tuning(2147483647) / 1048576.0 - std::pow(2., 50. / 1200)) < .000002, "50 cent tuning endpoint");
    require(second_increment(increment(55), tuning(0), 0) == increment(55) / 2, "oscillator two sub octave");
    require(second_increment(increment(55), tuning(0), 2) == increment(55) * 2, "oscillator two upper octave");
    opxy::i32 p[8] = {q(.6), 0, 0, 0, 0, 0, q(.5), q(1)};
    const int pivot = cutoff_position(p, increment(65.4063913));
    require(std::abs(pivot - multiply(p[0], 19005440)) < 256, "C2 key-tracking pivot");
    require(std::abs(cutoff_position(p, increment(130.8127826)) - pivot - 32 * 65536) < 256, "full tracking doubles cutoff per octave");
    p[7] = 0;
    require(cutoff_position(p, increment(32.7)) == cutoff_position(p, increment(523.25)), "zero tracking is pitch independent");
    require(gain(40, 0) > .96, "low frequencies pass through the ladder");
    const double g800 = gain(800, 0), g1600 = gain(1600, 0);
    require(g800 / g1600 > 10, "four-pole high frequency rolloff exceeds 20 dB per octave");
    require(gain(400, .9) > gain(400, 0) * 5, "resonance creates a peak at cutoff");
    std::printf("Ladder: gain at 40 Hz %.3f, 800/1600 Hz slope %.2f dB/oct\n", gain(40, 0), 20 * std::log10(g800 / g1600));

    // Coherent high-pitch test: the third harmonic is above Nyquist and must
    // disappear in the mip oscillator rather than fold back into the spectrum.
    const unsigned count = 8192, bin = 1537;
    const opxy::u32 step = bin * (0x80000000u / (count / 2));
    const Band band(step);
    std::vector<double> high;
    opxy::u32 phase = 0;
    for (unsigned i = 0; i < count; ++i) { high.push_back(oscillator(phase, band, 0) / double(unity)); phase += step; }
    const double fundamental = magnitude(high, bin * 44100.0 / count);
    const double alias = magnitude(high, (count - bin * 3) * 44100.0 / count);
    require(alias < fundamental * .001, "band-limited saw rejects folded third harmonic by 60 dB");

    opxy::i32 driven_parameters[8] = {q(.7), q(.3), q(.6), 0, 0, 0, q(.5), 0};
    const double driven_hz = bin * 44100.0 / count;
    const std::vector<double> shaped = render(driven_parameters, driven_hz);
    const double shaped_fundamental = magnitude(shaped, driven_hz);
    const double folded_third = magnitude(shaped, 44100 - 3 * driven_hz);
    require(folded_third < shaped_fundamental * .0005,
            "final ladder pole suppresses the driven folded third below -66 dBc");

    p[0] = q(1); p[5] = 0;
    const std::vector<double> a = render(p, 55);
    require(magnitude(a, 55) > .03 && magnitude(a, 55) > magnitude(a, 50) * 10, "A1 fundamental is in tune and audible");
    p[5] = q(1); p[6] = 0;
    const std::vector<double> sub = render(p, 55);
    require(magnitude(sub, 27.5) > magnitude(sub, 55) * 1.5, "second-oscillator octave changes audible pitch");
    p[3] = q(.875); p[6] = q(.5); p[5] = 0;
    const std::vector<double> pulse = render(p, 55);
    require(magnitude(pulse, 110) < magnitude(pulse, 55) * .015, "square has no even harmonic");
    p[3] = 0; p[0] = q(.30);
    const std::vector<double> dark = render(p, 55);
    require(magnitude(dark, 880) < magnitude(a, 880) * .02, "cutoff audibly removes upper harmonics");
    p[0] = q(1); p[2] = q(1);
    const std::vector<double> driven = render(p, 55);
    require(magnitude(driven, 165) > magnitude(a, 165) * 1.2, "drive adds harmonic energy");

    Voice voice = {}; Shared shared;
    opxy::u32 pitch[64]; opxy::i32 output[64];
    initialize(voice, shared, 1);
    opxy::u32 seed = 1927;
    // All control corners at bass, mid and high pitches, plus rapid automation.
    for (unsigned combination = 0; combination < 256; ++combination) {
        for (unsigned j = 0; j < 8; ++j) p[j] = combination & (1u << j) ? 2147483647 : 0;
        start(voice, shared, 1);
        for (unsigned b = 0; b < 96; ++b) {
            const double hz = b < 32 ? 27.5 : b < 64 ? 440 : 7040;
            for (unsigned i = 0; i < 64; ++i) pitch[i] = increment(hz);
            render(voice, shared, 1, output, p, pitch);
            for (unsigned i = 0; i < 64; ++i) require(output[i] >= -peak && output[i] <= peak, "corner stability");
            for (unsigned j = 0; j < 4; ++j) require(std::abs(2 * (voice.ladder[j] >> 15)) <= 2 * unity, "bounded ladder state");
        }
    }
    for (unsigned b = 0; b < 2048; ++b) {
        for (unsigned j = 0; j < 8; ++j) { seed = seed * 1664525u + 1013904223u; p[j] = static_cast<opxy::i32>(seed >> 1); }
        for (unsigned i = 0; i < 64; ++i) { seed = seed * 1664525u + 1013904223u; pitch[i] = seed; }
        render(voice, shared, 1, output, p, pitch);
        for (unsigned i = 0; i < 64; ++i) require(output[i] >= -peak && output[i] <= peak, "automated stability");
    }
    // A stationary oscillator is a DC input. Fractional charge in the output
    // blocker must drain it, including negative tails smaller than one step.
    for (unsigned j = 0; j < 8; ++j) p[j] = 0;
    p[0] = q(.8); p[5] = q(1); p[6] = q(.5);
    for (unsigned i = 0; i < 64; ++i) pitch[i] = 0;
    start(voice, shared, 1);
    for (unsigned b = 0; b < 2048; ++b) render(voice, shared, 1, output, p, pitch);
    for (unsigned i = 0; i < 64; ++i) require(std::abs(output[i]) <= 32768, "DC blocker drains a constant negative input");
    opxy::i32 before[64];
    start(voice, shared, 1); render(voice, shared, 1, before, p, pitch);
    start(voice, shared, 1); render(voice, shared, 1, output, p, pitch);
    for (unsigned i = 0; i < 64; ++i) require(before[i] == output[i], "retrigger resets every audio state");
    std::printf("%s: tuning, oscillator spectra, ladder response, controls, 256 corners, automation and retrigger\n", failures ? "FAIL" : "PASS");
    return failures ? 1 : 0;
}
