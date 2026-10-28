#include "../sample.cpp"
#include <cstdio>
#include <cstring>

int main() {
    opxy::i16 tiny[4] = {32767, -32768, -32768, 32767};
    granular::Voice voice;
    opxy::u32 random = 17, pitch[64];
    opxy::i32 parameters[8], output[130];
    const unsigned rates[] = {8000, 22050, 44100, 48000, 96000, 192000};
    for (unsigned rate : rates) {
        granular::initialize(voice, opxy::SampleView{tiny, 2, rate, 2});
        opxy::SampleBlock block{output + 1, parameters, pitch, {3000, 0, true},
            {0, 2, opxy::sample_unity_level, false, 0}, nullptr, 0, 2, 0, 0};
        for (unsigned b = 0; b < 5000; ++b) {
            for (unsigned i = 0; i < 8; ++i) {
                random = random * 1664525u + 1013904223u;
                parameters[i] = static_cast<opxy::i32>(random & 0x7fffffffu);
            }
            if (b % 3 == 0) for (unsigned i = 0; i < 8; ++i) parameters[i] = b & 1 ? 0x7fffffff : -1;
            for (unsigned i = 0; i < 64; ++i) pitch[i] = b % 3 == 0 ? 0xffffffffu : 25480370u;
            block.region.reverse = ((b / 97) & 1) != 0;
            block.region.pitch_cents = static_cast<opxy::i32>(random % 19201u) - 9600;
            block.track_transpose = static_cast<opxy::i32>(b % 73) - 36;
            block.drum_transpose = b & 1 ? 120 : -120;
            block.region.level = b & 4 ? 0xffffffffu : opxy::sample_unity_level;
            block.output_channels = b & 8 ? 1 : 2;
            output[0] = 12345; output[129] = 67890;
            granular::render_sample(voice, block);
            if (output[0] != 12345 || output[129] != 67890) return 1;
            for (unsigned i = 0; i < block.output_channels * 64; ++i)
                if (output[i + 1] < -1073741824 || output[i + 1] > 1073741824) return 2;
        }
    }
    // A stable trim exercises grains across many blocks at the highest density.
    opxy::i16 pcm[2048];
    for (unsigned i = 0; i < 2048; ++i) pcm[i] = static_cast<opxy::i16>(i * 191u);
    granular::initialize(voice, opxy::SampleView{pcm, 2048, 192000, 1});
    opxy::SampleBlock block{output + 1, parameters, pitch, {1200, 0, true},
        {2, 2046, 0xffffffffu, false, 9600}, nullptr, 0, 2, 36, 0};
    for (unsigned i = 0; i < 8; ++i) parameters[i] = 0x7fffffff;
    for (unsigned b = 0; b < 3000; ++b) granular::render_sample(voice, block);
    return 0;
}
