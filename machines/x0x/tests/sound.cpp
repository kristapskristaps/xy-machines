#include "../engine.cpp"
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <vector>
static void require(bool ok, const char* message, unsigned model = 99, unsigned family = 99, unsigned control = 99) {
    if (!ok) { std::fprintf(stderr, "FAIL: %s (model %u, family %u, control %u)\n", message, model, family, control); std::exit(1); }
}
static void controls(unsigned model, unsigned family, opxy::i32* p) {
    const unsigned raw[8] = {x0x::family_raw(model, family),64,48,64,0,64,64,0};
    for (unsigned s = 0; s < 8; ++s) p[s] = raw[s] == 127 ? 0x7fffffff : opxy::i32(raw[s] << 24);
}
static std::vector<double> waveform(unsigned model, const opxy::i32* p, double* energy = nullptr) {
    x0x::Voice voice; x0x::Shared shared;
    x0x::initialize(voice, shared, 1); x0x::start(voice, shared, 1); x0x::drum_select(voice, shared, 1, model);
    opxy::i32 out[64]; opxy::u32 pitch[64] = {};
    std::vector<double> audio;
    double sum = 0;
    for (unsigned block = 0; block < 700; ++block) {
        x0x::render(voice, shared, 1, out, p, pitch);
        for (auto sample : out) {
            require(sample >= -1073741824 && sample <= 1073741824, "half-scale bound", model);
            const double normalized = sample / 2147483648.0;
            audio.push_back(normalized);
            sum += normalized * normalized;
        }
    }
    if (energy) *energy = std::sqrt(sum / audio.size());
    return audio;
}
static double difference(const std::vector<double>& a, const std::vector<double>& b, bool normalize = false) {
    double aa = 0, bb = 0, ab = 0;
    for (unsigned i = 0; i < a.size(); ++i) { aa += a[i]*a[i]; bb += b[i]*b[i]; ab += a[i]*b[i]; }
    return std::sqrt(std::fmax(0., normalize ? 1-ab/std::sqrt(aa*bb+1e-30) : (aa+bb-2*ab)/(aa+bb+1e-30)));
}
static double kick_frequency(unsigned raw_tune) {
    using namespace x0x;
    Voice v; Shared shared; opxy::i32 p[8], out[64]; opxy::u32 pitch[64] = {};
    controls(kick, 2, p); p[tune] = raw_tune == 127 ? 0x7fffffff : opxy::i32(raw_tune << 24); p[decay] = 0x7fffffff;
    initialize(v, shared, 1); start(v, shared, 1); drum_select(v, shared, 1, kick);
    int previous = 0; unsigned first = 0, last = 0, crossings = 0;
    for (unsigned frame = 0; frame < 44100; frame += 64) {
        render(v, shared, 1, out, p, pitch);
        for (unsigned i = 0; i < 64; ++i) {
            if (frame + i > 4410 && previous <= 0 && out[i] > 0) {
                if (!crossings) first = frame + i;
                last = frame + i; ++crossings;
            }
            previous = out[i];
        }
    }
    require(crossings > 10, "kick yields measurable fundamental");
    return 44100.0 * (crossings - 1) / (last - first);
}
int main() {
    using namespace x0x;
    unsigned combinations = 0, tested_controls = 0;
    for (unsigned model = 0; model < model_count; ++model) {
        std::vector<double> families[4];
        for (unsigned family = 0; family < 4; ++family) {
            if (!(family_masks[model] & (1u << family))) continue;
            ++combinations;
            opxy::i32 p[8]; controls(model, family, p);
            double energy;
            families[family] = waveform(model, p, &energy);
            require(energy > 0.00015 && energy < 0.4, "audible useful default", model, family);
            for (unsigned earlier = 0; earlier < family; ++earlier)
                if (!families[earlier].empty())
                    require(difference(families[earlier],families[family],true) > .12,
                            "family change has a substantial shape difference", model, family);
            for (unsigned slot = 1; slot < parameter_count(model); ++slot) {
                opxy::i32 changed[8]; std::memcpy(changed, p, sizeof(p));
                changed[slot] = 0;
                const auto low = waveform(model, changed);
                changed[slot] = 0x7fffffff;
                const auto high = waveform(model, changed);
                require(difference(low,high) > .15, "control range has a substantial audio effect", model, family, slot);
                if (slot == character || slot == body)
                    require(difference(low,high,true) > .06, "character/body change timbre rather than only gain", model, family, slot);
                ++tested_controls;
            }
            Voice v; Shared shared; opxy::i32 out[64]; opxy::u32 pitch[64] = {};
            initialize(v, shared, 1); start(v, shared, 1); drum_select(v, shared, 1, model);
            require(!finished(v, shared), "fresh hit is not finished", model, family);
            for (unsigned block = 0; block < 5000 && !finished(v, shared); ++block) render(v, shared, 1, out, p, pitch);
            require(finished(v, shared), "voice retires after its tail", model, family);
            render(v, shared, 1, out, p, pitch);
            for (auto sample : out) require(sample == 0, "retired voice stays silent", model, family);
            start(v, shared, 1); render(v, shared, 1, out, p, pitch);
            bool audible = false; for (auto sample : out) audible = audible || sample;
            require(audible && !finished(v, shared), "retrigger restarts a retired voice", model, family);
        }
    }
    require(combinations == 47, "complete family/type inventory");
    const double low = kick_frequency(0), middle = kick_frequency(64), high = kick_frequency(127);
    require(std::abs(middle / low - 2) < 0.05 && std::abs(high / middle - 2) < 0.05, "tune spans two octaves around zero");
    std::printf("PASS: %u family/type voices, %u audible controls, bounds, tails, retrigger; kick %.2f / %.2f / %.2f Hz\n",
                combinations, tested_controls, low, middle, high);
}
