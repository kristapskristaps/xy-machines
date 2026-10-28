// Host-only audition driver. The instrument itself uses no host libraries.
#include "../sample.cpp"
#include <cstdio>
#include <cstdlib>
#include <vector>

int main(int argc, char** argv) {
    if (argc != 14) return 2;
    const unsigned frames = static_cast<unsigned>(std::strtoul(argv[2], nullptr, 10));
    const unsigned rate = static_cast<unsigned>(std::strtoul(argv[3], nullptr, 10));
    const unsigned rendered = static_cast<unsigned>(std::strtoul(argv[4], nullptr, 10));
    const unsigned increment = static_cast<unsigned>(std::strtoul(argv[5], nullptr, 10));
    std::vector<opxy::i16> pcm(frames * 2);
    std::FILE* input = std::fopen(argv[1], "rb");
    if (!input) return 3;
    const auto read = std::fread(pcm.data(), sizeof(opxy::i16), pcm.size(), input);
    std::fclose(input);
    if (read != pcm.size()) return 4;
    granular::Voice voice;
    granular::initialize(voice, opxy::SampleView{pcm.data(), frames, rate, 2});
    opxy::i32 output[128], parameters[8];
    for (unsigned i = 0; i < 8; ++i) parameters[i] = static_cast<opxy::i32>(std::strtol(argv[6 + i], nullptr, 10));
    opxy::u32 pitch[64];
    for (unsigned i = 0; i < 64; ++i) pitch[i] = increment;
    opxy::SampleBlock block{output, parameters, pitch, {1200, 0, true},
        {0, frames, opxy::sample_unity_level, false, 0}, nullptr, 0, 2, 0, 0};
    for (unsigned frame = 0; frame < rendered; frame += 64) {
        granular::render_sample(voice, block);
        float audio[128];
        for (unsigned i = 0; i < 64; ++i) {
            const unsigned at = frame + i;
            float gain = at < 441 ? at / 441.0f : 1;
            if (at >= rendered) gain = 0;
            else if (rendered - at < 6615) gain *= (rendered - at) / 6615.0f;
            audio[2 * i] = output[2 * i] / 2147483648.0f * gain;
            audio[2 * i + 1] = output[2 * i + 1] / 2147483648.0f * gain;
        }
        const unsigned count = rendered - frame < 64 ? rendered - frame : 64;
        if (std::fwrite(audio, sizeof(float), count * 2, stdout) != count * 2) return 5;
    }
    return 0;
}
