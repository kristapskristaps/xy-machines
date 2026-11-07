// Offline audition: compile against the SDK headers, then pass an output .wav.
#include "../fx.cpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

void word(std::FILE* f, unsigned value, unsigned bytes) {
    for (unsigned i = 0; i < bytes; ++i) std::fputc((value >> (i * 8)) & 255, f);
}
int main(int argc, char** argv) {
    if (argc < 3 || argc > 4) { std::fprintf(stderr, "usage: render output.wav preset(0..5) [send 0..1, default .75]\n"); return 1; }
    const int preset = std::atoi(argv[2]);
    if (preset < 0 || preset > 5) return 1;
    const double send = argc == 4 ? std::atof(argv[3]) : .75;
    if (!(send >= 0 && send <= 1)) return 1;
    const double presets[6][8] = {
        {.45,.65,.6,.65,0,0,0,.65},
        {.5,.85,.72,.8,.35,.1,.2,.72},
        {.85,.95,.85,.95,.08,.3,.3,.42},
        {.62,.7,.7,.65,.55,.85,.12,.8},
        {.08,.3,.45,.3,.65,.15,.15,.8},
        {.9,1,1,1,.18,.2,.25,.65}
    };
    // All files share the same source. 0..4 s dry; 4..14 s dry+wet;
    // 14..22 s wet only, with the last source note at 13.5 s.
    const unsigned seconds = preset == 5 ? 64 : 22;
    const unsigned frames = seconds * 44100;
    const double notes[] = {220, 261.625565, 329.627557, 493.883301, 440, 329.627557, 293.664768, 391.995436};
    std::vector<opxy::i32> memory(lustre::kBufferWords, 0x12345678);
    lustre::State state;
    lustre::initialize(state);
    opxy::i32 parameters[8], left[64], right[64];
    for (unsigned j = 0; j < 8; ++j) parameters[j] = static_cast<opxy::i32>(presets[preset][j] * 2147483647.);
    std::vector<short> pcm;
    pcm.reserve(frames * 2);
    double peak = 0;
    for (unsigned start = 0; start < frames; start += 64) {
        double dry[64];
        for (unsigned i = 0; i < 64; ++i) {
            const double t = (start + i) / 44100.;
            const unsigned step = static_cast<unsigned>(t * 2);
            const double age = t - step * .5;
            const double f = notes[step % 8];
            const double env = (1-std::exp(-age * 700)) * std::exp(-age * 6);
            dry[i] = t < 14 ? .32 * env * (std::sin(6.283185307179586 * f * age) +
                .26 * std::sin(6.283185307179586 * f * 2 * age) +
                .12 * std::sin(6.283185307179586 * f * 3 * age)) : 0;
            left[i] = right[i] = static_cast<opxy::i32>(send * dry[i] * 2147483647.);
            if (t < 4) left[i] = right[i] = 0;
        }
        lustre::process(state, {memory.data(),lustre::kBufferWords}, left, right, parameters);
        for (unsigned i = 0; i < 64 && start + i < frames; ++i) {
            for (unsigned ch = 0; ch < 2; ++ch) {
                // Isolated stock-send model with dryLevel 0: attenuate the
                // source by 1-send and add the actual wet return at unity.
                const double dry_gain = start + i < 4 * 44100 ? 1 : 1-send;
                const double value = .85 * (dry_gain * dry[i] + (ch ? right[i] : left[i]) / 2147483648.);
                if (std::abs(value) > peak) peak = std::abs(value);
                if (std::abs(value) >= 1) { std::fprintf(stderr, "clipped demo\n"); return 1; }
                pcm.push_back(static_cast<short>(value * 32767));
            }
        }
    }
    std::FILE* f = std::fopen(argv[1], "wb");
    if (!f) return 1;
    const unsigned bytes = static_cast<unsigned>(pcm.size() * 2);
    std::fwrite("RIFF",1,4,f); word(f,36+bytes,4); std::fwrite("WAVEfmt ",1,8,f);
    word(f,16,4); word(f,1,2); word(f,2,2); word(f,44100,4); word(f,176400,4);
    word(f,4,2); word(f,16,2); std::fwrite("data",1,4,f); word(f,bytes,4);
    for (unsigned i = 0; i < pcm.size(); ++i) word(f,static_cast<unsigned>(pcm[i]) & 65535,2);
    const bool failed = std::ferror(f);
    std::fclose(f);
    std::printf("Audition %d: send %.2f, unity wet return; dry 0..4s, effect 4..14s, tail 14..%us; peak %.3f\n",preset,send,seconds,peak);
    return failed ? 1 : 0;
}
