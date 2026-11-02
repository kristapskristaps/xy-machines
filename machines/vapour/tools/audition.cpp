// Host-only rendering of the real callback; no stock files or device access.
#include "../fx.cpp"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

static void word(FILE* output, unsigned value, unsigned bytes) {
    for (unsigned i = 0; i < bytes; ++i) std::fputc((value >> (8*i)) & 255, output);
}

int main(int argc, char** argv) {
    if (argc != 3 || (std::strcmp(argv[1], "cloud") && std::strcmp(argv[1], "freeze"))) {
        std::fprintf(stderr, "usage: audition cloud|freeze output.wav\n");
        return 1;
    }
    FILE* output = std::fopen(argv[2], "wb");
    if (!output) { std::perror(argv[2]); return 1; }
    const bool freeze = std::strcmp(argv[1], "freeze") == 0;
    const unsigned frames = 44100 * 14;
    std::fwrite("RIFF", 1, 4, output); word(output, 36 + frames*4, 4);
    std::fwrite("WAVEfmt ", 1, 8, output); word(output, 16, 4); word(output, 1, 2); word(output, 2, 2);
    word(output, 44100, 4); word(output, 44100*4, 4); word(output, 4, 2); word(output, 16, 2);
    std::fwrite("data", 1, 4, output); word(output, frames*4, 4);
    std::vector<opxy::i32> words(282624/4);
    vapour::State state; vapour::initialize(state);
    opxy::i32 p[8] = {644245094, 966367642, 1073741824, 214748365,
                     1932735282, 536870912, 0, 2147483647};
    for (unsigned start = 0; start < frames; start += 64) {
        opxy::i32 l[64], r[64]; double dry_l[64], dry_r[64];
        const double seconds = start / 44100.0;
        p[6] = freeze && seconds >= 3 && seconds < 9 ? 2147483647 : 0;
        if (freeze && seconds >= 6) p[2] = seconds < 9 ? 536870912 : 1073741824;
        for (unsigned i = 0; i < 64; ++i) {
            const double time = (start+i) / 44100.0;
            const double local = std::fmod(time, 0.5);
            const bool play = freeze ? time < 3 || (time >= 9 && time < 11) : time < 8;
            const double frequency = 165 * std::pow(2.0, (static_cast<unsigned>(time*2)%5)*3.0/12);
            dry_l[i] = play ? 0.12 * std::exp(-local*10) *
                (std::sin(6.283185307179586*frequency*time) + 0.2*std::sin(18.84955592153876*frequency*time)) : 0;
            dry_r[i] = play ? 0.1 * std::exp(-local*12) * std::sin(6.283185307179586*frequency*1.5*time) : 0;
            l[i] = static_cast<opxy::i32>(dry_l[i] * 2147483647);
            r[i] = static_cast<opxy::i32>(dry_r[i] * 2147483647);
        }
        vapour::process(state, {words.data(), static_cast<unsigned>(words.size())}, l, r, p);
        for (unsigned i = 0; i < 64 && start+i < frames; ++i) {
            const double mixed[2] = {dry_l[i] + l[i]/2147483648.0, dry_r[i] + r[i]/2147483648.0};
            for (double value : mixed) {
                const double bounded = value < -1 ? -1 : value > 1 ? 1 : value;
                word(output, static_cast<unsigned>(static_cast<int>(bounded * 32767)), 2);
            }
        }
    }
    const bool failed = std::ferror(output) != 0;
    if (std::fclose(output) != 0 || failed) return 1;
    std::printf("Rendered %s\n", argv[2]);
}
