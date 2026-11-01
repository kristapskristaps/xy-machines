// Host comparison only: these timings are not OP-XY CPU measurements.
#include "../engine.cpp"
#include <chrono>
#include <cstdio>
#include <cstring>

int main(int argc, char** argv) {
    using namespace x0x;
    const bool changing = argc > 1 && !std::strcmp(argv[1], "changing");
    opxy::u32 pitch[64] = {};
    volatile opxy::u32 checksum = 0;
    for (unsigned model = 0; model < model_count; ++model) {
        for (unsigned family = 0; family < 4; ++family) {
            if (!(family_masks[model] & (1u << family))) continue;
            unsigned hash = 2166136261u;
            double best = 1e30;
            for (unsigned run = 0; run < 4; ++run) {
                const auto begin = std::chrono::steady_clock::now();
                for (unsigned hit = 0; hit < 32; ++hit) {
                    Voice voice; Shared shared;
                    initialize(voice, shared, hit + 1); start(voice, shared, hit + 1);
                    drum_select(voice, shared, hit + 1, model);
                    opxy::i32 parameters[8], output[64];
                    const unsigned raw[8] = {family_raw(model,family),64,100,64,0,64,64,0};
                    for (unsigned slot = 0; slot < 8; ++slot) parameters[slot] = opxy::i32(raw[slot] << 24);
                    for (unsigned block = 0; block < 128; ++block) {
                        if (changing) parameters[tone] = opxy::i32((block % 128) << 24);
                        render(voice, shared, hit + 1, output, parameters, pitch);
                        checksum = checksum ^ opxy::u32(output[block % 64]);
                        if (!run) for (auto sample : output) hash = (hash ^ unsigned(sample)) * 16777619u;
                    }
                }
                const double ns = std::chrono::duration<double,std::nano>(std::chrono::steady_clock::now()-begin).count();
                if (run && ns < best) best = ns;
            }
            std::printf("%u %u %.3f %08x\n", model, family, best / (32*128*64), hash);
        }
    }
    std::fprintf(stderr, "host ns/sample; %s controls; checksum %08x\n", changing ? "changing" : "steady", unsigned(checksum));
}
