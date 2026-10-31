// Host-only rendering of the real effect callback. No device or stock files.
#include "../fx.cpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

static const double pi = 3.14159265358979323846;
static opxy::i32 option(unsigned index, unsigned count) {
    return static_cast<opxy::i32>((static_cast<opxy::i64>(2 * index + 1) * 2147483648LL) / (2 * count));
}
static void word(FILE* output, unsigned value, unsigned bytes) {
    for (unsigned i = 0; i < bytes; ++i) std::fputc((value >> (8*i)) & 255, output);
}
int main(int argc, char** argv) {
    if (argc != 3 || (std::strcmp(argv[1], "bounce") && std::strcmp(argv[1], "ride"))) {
        std::fprintf(stderr, "usage: audition bounce|ride output.wav\n");
        return 1;
    }
    FILE* output = std::fopen(argv[2], "wb");
    if (!output) { std::perror(argv[2]); return 1; }
    const bool ride = std::strcmp(argv[1], "ride") == 0;
    const unsigned frames = 44100 * (ride ? 16 : 8);
    std::fwrite("RIFF", 1, 4, output); word(output, 36 + frames*4, 4);
    std::fwrite("WAVEfmt ", 1, 8, output); word(output, 16, 4); word(output, 1, 2); word(output, 2, 2);
    word(output, 44100, 4); word(output, 44100*4, 4); word(output, 4, 2); word(output, 16, 2);
    std::fwrite("data", 1, 4, output); word(output, frames*4, 4);
    std::vector<opxy::i32> words(rebound::buffer_words);
    rebound::State state; rebound::initialize(state);
    opxy::i32 p[8] = {option(ride ? 2 : 4, 8), option(ride ? 100 : 65, 191),
                     2147483647, 1395864370, option(50, 101), 0, 2147483647, 2147483647};
    const opxy::Transport transport = {1200, 0, true};
    for (unsigned start = 0; start < frames; start += 64) {
        opxy::i32 l[64], r[64]; double dry[64];
        if (ride) {
            const double seconds = start / 44100.0;
            // 90 -> 130% during 2..7 s; hold until 10 s, then pull to 45%.
            const double percent = seconds < 2 ? 90 : seconds < 7 ? 90 + 8*(seconds-2) : seconds < 10 ? 130 : 45;
            const unsigned index = percent < 80 ? static_cast<unsigned>(percent) : percent <= 120 ? 80 + static_cast<unsigned>((percent-80)*2) :
                                                  161 + static_cast<unsigned>(percent-121);
            p[1] = option(index, 191);
            // A performance sweep of the delay time while the loop is singing.
            if (seconds >= 8 && seconds < 10) p[4] = option(static_cast<unsigned>(50 + 20*std::sin((seconds-8)*pi)), 101);
        }
        for (unsigned i = 0; i < 64; ++i) {
            const unsigned n = start + i;
            const double time = n / 44100.0;
            const double local = ride ? time : std::fmod(time, 0.75);
            const bool play = ride ? time < 1.5 : time < 4.5;
            const double frequency = ride ? 440 : 220 * std::pow(2.0, (static_cast<unsigned>(time / 0.75) % 4)*3.0/12);
            dry[i] = play ? 0.14 * std::exp(-local*8) * (std::sin(2*pi*frequency*time) +
                                0.25*std::sin(2*pi*frequency*3*time)) : 0;
            l[i] = r[i] = static_cast<opxy::i32>(dry[i] * 2147483647);
        }
        rebound::process(state, {words.data(), rebound::buffer_words}, transport, l, r, p);
        for (unsigned i = 0; i < 64 && start+i < frames; ++i) {
            for (opxy::i32 wet : {l[i], r[i]}) {
                const double value = dry[i] + wet / 2147483648.0;
                const int sample = static_cast<int>(value * 32767);
                word(output, static_cast<unsigned>(sample), 2);
            }
        }
    }
    const bool failed = std::ferror(output) != 0;
    if (std::fclose(output) != 0 || failed) return 1;
    std::printf("Rendered %s\n", argv[2]);
}
