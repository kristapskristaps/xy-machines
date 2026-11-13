#include "../engine.cpp"
#include <cstdio>
#include <cstdlib>

static void require(bool value, const char* message) {
    if (!value) { std::fprintf(stderr, "FAIL: %s\n", message); std::exit(1); }
}

int main() {
    vader::Voice voice = {}; vader::Shared shared = {};
    vader::start(voice, shared, 1);
    unsigned random = 7139;
    int maximum = 0;
    for (unsigned mode = 0; mode < 4; ++mode) for (unsigned band = 0; band < 6; ++band) {
        for (unsigned trial = 0; trial < 64; ++trial) {
            const unsigned drive = (random = random * 1664525u + 1013904223u) % 262137u;
            const int mix = (random = random * 1664525u + 1013904223u) % 32769u;
            voice.cache_status = 0;
            require(!vader::cached_shaper(voice, mode, drive, band, mix), "a new key first uses the direct path");
            require(vader::cached_shaper(voice, mode, drive, band, mix), "an unchanged key builds its bank");
            require(vader::cached_shaper(voice, mode, drive, band, mix), "a ready bank can be reused");
            for (unsigned at = 0; at < 513; ++at)
                require(voice.cached_wave[at] == vader::shaped(mode, drive, band, mix, at << 23), "cached vertices equal direct interpolation");
            for (unsigned sample = 0; sample < 512; ++sample) {
                const unsigned phase = (random = random * 1664525u + 1013904223u);
                const int error = std::abs(vader::lookup(voice.cached_wave, phase) - vader::shaped(mode, drive, band, mix, phase));
                if (error > maximum) maximum = error;
                require(error <= 4, "changing interpolation order has at most four waveform units of error");
            }
            require(!vader::cached_shaper(voice, mode ^ 1u, drive, band, mix), "shape invalidates the cache");
            require(!vader::cached_shaper(voice, mode, drive ^ 1u, band, mix), "drive invalidates the cache");
            require(!vader::cached_shaper(voice, mode, drive, (band + 1) % 6, mix), "pitch band invalidates the cache");
            require(!vader::cached_shaper(voice, mode, drive, band, mix ^ 1), "band blend invalidates the cache");
        }
    }
    vader::start(voice, shared, 1);
    require(voice.cache_status == 0, "retrigger invalidates cached state");
    std::printf("PASS: cached vertices, all four keys, retrigger and 786,432 interpolated phases; maximum error %d waveform units.\n", maximum);
}
