// Host-only file renderer for the actual production callback. Input/output are
// interleaved stereo Q31 at 44.1 kHz; the Python driver owns WAV conversion.
#include "../fx.cpp"
#include <cstdio>
#include <cstdlib>
int main(int argc, char **argv) {
    if (argc != 11) {
        std::fprintf(stderr, "render input.raw output.raw wow flutter saturate "
                             "failure model spread noise dry\n");
        return 1;
    }
    FILE *in = std::fopen(argv[1], "rb");
    FILE *out = std::fopen(argv[2], "wb");
    if (!in || !out) {
        std::perror("render");
        return 1;
    }
    int p[8];
    for (unsigned i = 0; i < 8; ++i) {
        double x = std::atof(argv[3 + i]);
        if (x < 0 || x > 1)
            return 1;
        p[i] = static_cast<int>(x * 2147483647);
    }
    static int memory[gentape::buffer_words];
    gentape::State s;
    gentape::initialize(s);
    int interleaved[128], left[64], right[64];
    while (true) {
        const size_t words = std::fread(interleaved, sizeof(int), 128, in);
        if (!words)
            break;
        if (words % 2) {
            std::fprintf(stderr, "incomplete stereo frame\n");
            return 1;
        }
        for (unsigned i = 0; i < 64; ++i) {
            left[i] = 2 * i < words ? interleaved[2 * i] : 0;
            right[i] = 2 * i + 1 < words ? interleaved[2 * i + 1] : 0;
        }
        gentape::process(s, {memory, gentape::buffer_words}, left, right, p);
        for (unsigned i = 0; i < 64; ++i) {
            interleaved[2 * i] = left[i];
            interleaved[2 * i + 1] = right[i];
        }
        if (std::fwrite(interleaved, sizeof(int), words, out) != words)
            return 1;
    }
    const bool failed = std::ferror(in) || std::ferror(out);
    std::fclose(in);
    return std::fclose(out) || failed;
}
