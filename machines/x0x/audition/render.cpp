#include "../engine.cpp"
#include <cstdio>
#include <cstdlib>
#include <vector>

namespace {
struct Player { x0x::Voice voice; opxy::i32 controls[8]; unsigned gain; };
Player players[32]; x0x::Shared shared; unsigned next_voice;
void hit(unsigned model, unsigned family, unsigned gain = 100) {
    Player& player = players[next_voice++ % 32];
    const unsigned raw[8] = {x0x::family_raw(model, family),64,48,64,0,64,64,0};
    for (unsigned s = 0; s < 8; ++s) player.controls[s] = opxy::i32(raw[s] << 24);
    x0x::start(player.voice, shared, next_voice % 32 + 1);
    x0x::drum_select(player.voice, shared, next_voice % 32 + 1, model);
    player.gain = gain;
}
void word(std::FILE* file, unsigned value, unsigned bytes) {
    for (unsigned i = 0; i < bytes; ++i) std::fputc(int(value >> (8 * i)) & 255, file);
}
void write(const char* path, const std::vector<opxy::i64>& samples) {
    const char credits[] = "X0X DSP audition. TR-707: Francois Dion, public domain. "
        "TR-909 hats/cymbals: Janne G:son Berg (2005), edited/published by altemark; "
        "CC BY 4.0 https://creativecommons.org/licenses/by/4.0/ ; "
        "https://freesound.org/people/altemark/packs/1643/ . "
        "Adapted: mono, resampled, trimmed, normalized, logarithmic encoding, pitch/envelope/EQ. No endorsement.";
    const unsigned comment_bytes = (sizeof(credits) + 1) & ~1u;
    opxy::i64 peak = 1;
    for (auto sample : samples) { const auto magnitude = sample < 0 ? -sample : sample; if (magnitude > peak) peak = magnitude; }
    std::FILE* file = std::fopen(path, "wb");
    if (!file) { std::perror(path); std::exit(1); }
    std::fwrite("RIFF", 1, 4, file); word(file, unsigned(samples.size()) * 2 + 56 + comment_bytes, 4);
    std::fwrite("WAVEfmt ", 1, 8, file); word(file, 16, 4); word(file, 1, 2); word(file, 1, 2);
    word(file, 44100, 4); word(file, 88200, 4); word(file, 2, 2); word(file, 16, 2);
    std::fwrite("data", 1, 4, file); word(file, unsigned(samples.size()) * 2, 4);
    for (auto sample : samples) word(file, unsigned(int(sample * 27850 / peak)) & 65535u, 2);
    std::fwrite("LIST", 1, 4, file); word(file, 12 + comment_bytes, 4);
    std::fwrite("INFOICMT", 1, 8, file); word(file, sizeof(credits), 4);
    std::fwrite(credits, 1, sizeof(credits), file);
    if (sizeof(credits) != comment_bytes) std::fputc(0, file);
    std::fclose(file);
}
}
int main(int argc, char** argv) {
    if (argc != 3) { std::fprintf(stderr, "usage: render showcase.wav voices.wav\n"); return 1; }
    using namespace x0x;
    for (unsigned mode = 0; mode < 2; ++mode) {
        for (unsigned i = 0; i < 32; ++i) initialize(players[i].voice, shared, i + 1);
        next_voice = 0;
        const unsigned frames = (mode ? 27 : 34) * 44100;
        std::vector<opxy::i64> samples(frames);
        unsigned step_index = 0, catalogue = 0;
        for (unsigned frame = 0; frame < frames; frame += 64) {
            if (!mode && step_index < 256 && frame >= step_index * 44100 / 8) {
                const unsigned family = step_index / 64, step = step_index % 16, bar = step_index % 64 / 16;
                if (step % 4 == 0) hit(kick, family, 100);
                if (step == 4 || step == 12) { hit(snare, family, 80); if (family != 0) hit(clap, family, 45); }
                if (step % 2 == 0 && step != 14) hit(closed_hat, family, 60);
                if (step == 14) hit(open_hat, family, 50);
                if (step == 0 && bar == 0) hit(cymbal, family, 40);
                if (bar == 3 && step == 11) hit(low_tom, family, 65);
                if (bar == 3 && step == 15) hit(high_tom, family, 65);
                if (family == 1 && step == 6) hit(tambourine, family, 40);
                if ((family == 1 || family == 2) && step == 10) hit(cowbell, family, 40);
                if (family == 2 && step == 7) hit(low_conga, family, 60);
                if (family == 2 && step == 11) hit(high_conga, family, 45);
                if (family == 3 && step == 10) hit(ride, family, 35);
                ++step_index;
            } else if (mode && catalogue < 47 && frame >= catalogue * 22050) {
                unsigned index = 0;
                for (unsigned family = 0; family < 4; ++family) for (unsigned model = 0; model < model_count; ++model) {
                    if (!(family_masks[model] & (1u << family))) continue;
                    if (index++ == catalogue) {
                        hit(model, family);
                        std::printf("%5.1f s  %s  %s\n", catalogue * 0.5, family_name(model, family_raw(model, family)), model_name(model));
                    }
                }
                ++catalogue;
            }
            opxy::i32 output[64]; opxy::u32 pitch[64] = {};
            for (unsigned p = 0; p < 32; ++p) if (players[p].voice.active) {
                render(players[p].voice, shared, p + 1, output, players[p].controls, pitch);
                for (unsigned i = 0; i < 64 && frame + i < frames; ++i) samples[frame + i] += opxy::i64(output[i]) * players[p].gain / 100;
            }
        }
        write(argv[mode + 1], samples);
    }
}
