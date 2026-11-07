#include "../fx.cpp"
#include <cmath>
#include <cstdio>
#include <vector>
struct Scenario {
    const char *name;
    double controls[8];
    double amplitude;
    double frequency;
};
const Scenario cases[] = {{"clean", {0, 0, 0, 0, 0, 0, 0, 0}, .2, 441},
                          {"wow25", {.25, 0, 0, 0, 0, 0, 0, 0}, .2, 441},
                          {"wow50", {.5, 0, 0, 0, 0, 0, 0, 0}, .2, 441},
                          {"wow100", {1, 0, 0, 0, 0, 0, 0, 0}, .2, 441},
                          {"flutter25", {0, .25, 0, 0, 0, 0, 0, 0}, .2, 441},
                          {"flutter50", {0, .5, 0, 0, 0, 0, 0, 0}, .2, 441},
                          {"flutter100", {0, 1, 0, 0, 0, 0, 0, 0}, .2, 441},
                          {"saturate25", {0, 0, .25, 0, 0, 0, 0, 0}, .2, 441},
                          {"saturate50", {0, 0, .5, 0, 0, 0, 0, 0}, .2, 441},
                          {"saturate100", {0, 0, 1, 0, 0, 0, 0, 0}, .2, 441},
                          {"saturate100 hot", {0, 0, 1, 0, 0, 0, 0, 0}, .6, 441},
                          {"failure25", {0, 0, 0, .25, 0, 0, 0, 0}, .2, 441},
                          {"failure50", {0, 0, 0, .5, 0, 0, 0, 0}, .2, 441},
                          {"failure100", {0, 0, 0, 1, 0, 0, 0, 0}, .2, 441},
                          {"VHS", {0, 0, 0, 0, .375, 0, 0, 0}, .2, 441},
                          {"cassette", {0, 0, 0, 0, .625, 0, 0, 0}, .2, 441},
                          {"dictation", {0, 0, 0, 0, .875, 0, 0, 0}, .2, 441},
                          {"dictation bass", {0, 0, 0, 0, .875, 0, 0, 0}, .2, 100},
                          {"dictation bright", {0, 0, 0, 0, .875, 0, 0, 0}, .2, 8000},
                          {"spread alone", {0, 0, 0, 0, 0, 1, 0, 0}, .2, 441},
                          {"spread wow", {1, 0, 0, 0, 0, 1, 0, 0}, .2, 441},
                          {"noise25", {0, 0, 0, 0, 0, 0, .25, 0}, 0, 441},
                          {"noise50", {0, 0, 0, 0, 0, 0, .5, 0}, 0, 441},
                          {"noise100", {0, 0, 0, 0, 0, 0, 1, 0}, 0, 441},
                          {"dry100", {0, 0, 0, 0, 0, 0, 0, 1}, .2, 441}};
int main() {
    const double pi = 3.14159265358979323846;
    for (const auto &c : cases) {
        gentape::State state;
        gentape::initialize(state);
        std::vector<int> memory(gentape::buffer_words, 0x60000000);
        int p[8];
        for (unsigned k = 0; k < 8; ++k)
            p[k] = static_cast<int>(c.controls[k] * 2147483647);
        double energy = 0, delta = 0, cents2 = 0, centsmax = 0, mingate = 1, delaymin = 1e10,
               delaymax = -1e10;
        unsigned events = 0, count = 0, blocks = 0;
        int l[64], r[64];
        unsigned prior_event = 0;
        for (unsigned b = 0; b < 20672; ++b) { // 30 seconds; ignore first second
            int previous = state.delay[0];
            for (unsigned i = 0; i < 64; ++i)
                l[i] = r[i] = static_cast<int>(
                    c.amplitude * std::sin(2 * pi * c.frequency * (b * 64 + i) / 44100) * 2147483647);
            gentape::process(state, {memory.data(), gentape::buffer_words}, l, r, p);
            if (!prior_event && state.event)
                ++events;
            prior_event = state.event;
            if (b < 690)
                continue;
            const double speed = 1 - (state.delay[0] - double(previous)) / (64 * 65536.);
            const double cents = 1200 * std::log2(speed);
            cents2 += cents * cents;
            centsmax = std::fmax(centsmax, std::fabs(cents));
            ++blocks;
            delaymin = std::fmin(delaymin, state.delay[0] / 65536.);
            delaymax = std::fmax(delaymax, state.delay[0] / 65536.);
            mingate = std::fmin(mingate, state.gate[0] / 1073741824.);
            for (unsigned i = 0; i < 64; ++i) {
                const double x = l[i] / 2147483648., y = r[i] / 2147483648.;
                energy += x * x;
                delta += (x - y) * (x - y);
                ++count;
            }
        }
        std::printf(
            "{\"case\":\"%s\",\"rms\":%.8f,\"gain_db\":%.3f,\"pitch_rms_cents\":%.3f,\"pitch_max_cents\":%."
            "3f,\"delay_span_samples\":%.3f,\"min_gain\":%.5f,\"events_per_30s\":%u,\"stereo_delta_rms\":%."
            "8f}\n",
            c.name, std::sqrt(energy / count),
            c.amplitude > 0 && energy > 0
                ? 20 * std::log10(std::sqrt(energy / count) / (c.amplitude / std::sqrt(2.)))
                : 0,
            std::sqrt(cents2 / blocks), centsmax, delaymax - delaymin, mingate, events,
            std::sqrt(delta / count));
    }
}
