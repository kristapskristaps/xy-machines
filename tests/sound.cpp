#include "../sample.cpp"
#include <cstdio>
#include <cstring>

namespace {
opxy::i16 pcm[32768 * 2];
struct Fixture {
    granular::Voice voice;
    opxy::i32 guarded[130], parameters[8];
    opxy::u32 pitch[64];
    opxy::SampleBlock block;
    Fixture(unsigned channels = 2) : block{guarded + 1, parameters, pitch,
        {1200, 0, true}, {0, 32768, opxy::sample_unity_level, false, 0},
        nullptr, 0, channels, 0, 0} {
        const opxy::i32 defaults[8] = {0x20000000, 0x20000000, 0x36db6db6, 0,
            0x60000000, 0, 0, 0x7fffffff};
        std::memcpy(parameters, defaults, sizeof(parameters));
        for (unsigned i = 0; i < 64; ++i) pitch[i] = 25480370u;
        granular::initialize(voice, opxy::SampleView{pcm, 32768, 44100, 2});
    }
    bool render(bool silent = false) {
        guarded[0] = 0x12345678; guarded[129] = 0x23456789;
        for (unsigned i = 1; i < 129; ++i) guarded[i] = 0x3456789a;
        granular::render_sample(voice, block);
        if (guarded[0] != 0x12345678 || guarded[129] != 0x23456789) return false;
        for (unsigned i = 0; i < 64 * block.output_channels; ++i) {
            if (block.output[i] < -1073741824 || block.output[i] > 1073741824) return false;
            if (silent && block.output[i]) return false;
        }
        if (block.output_channels == 1 && guarded[65] != 0x3456789a) return false;
        return true;
    }
};
bool fail(const char* message) { std::fprintf(stderr, "%s\n", message); return false; }
bool sound_and_channels() {
    for (unsigned i = 0; i < 32768; ++i) { pcm[2 * i] = 12000; pcm[2 * i + 1] = -6000; }
    Fixture stereo, mono(1);
    bool audible = false;
    for (unsigned b = 0; b < 120; ++b) {
        if (!stereo.render() || !mono.render()) return fail("Output bounds/overwrite");
        for (unsigned i = 0; i < 64; ++i) {
            if (stereo.block.output[2 * i] < 0 || stereo.block.output[2 * i + 1] > 0)
                return fail("Stereo source order changed");
            const opxy::i32 expected = (stereo.block.output[2 * i] / 32768 +
                stereo.block.output[2 * i + 1] / 32768) / 2;
            const opxy::i32 actual = mono.block.output[i] / 32768;
            if (actual < expected - 3 || actual > expected + 3) return fail("Mono dropped a source channel");
            audible = audible || actual > 100;
        }
    }
    return audible || fail("Valid sample is silent");
}
bool invalid_and_trim() {
    Fixture f;
    f.block.region.begin = f.block.region.end = 400;
    if (!f.render(true)) return fail("Empty region sounds");
    f.block.region.begin = 32769; f.block.region.end = 100;
    if (!f.render(true)) return fail("Inverted/outside region sounds");
    f.block.region.begin = 0; f.block.region.end = 32768;
    const opxy::SampleView invalid[] = {
        {nullptr, 32768, 44100, 2}, {pcm, 0, 44100, 2}, {pcm, 1, 44100, 2},
        {pcm, 32768, 0, 2}, {pcm, 32768, 192001, 2}, {pcm, 32768, 44100, 3}
    };
    for (const auto& sample : invalid) {
        granular::initialize(f.voice, sample);
        if (!f.render(true)) return fail("Invalid sample sounds");
    }
    for (unsigned i = 0; i < 32768; ++i) pcm[2 * i] = pcm[2 * i + 1] = i < 16384 ? 16000 : 0;
    granular::initialize(f.voice, opxy::SampleView{pcm, 32768, 44100, 2});
    for (unsigned b = 0; b < 40; ++b) if (!f.render()) return false;
    f.block.region.begin = 16384;
    for (unsigned b = 0; b < 160; ++b) {
        for (unsigned p = 0; p < 8; ++p) f.parameters[p] = b & 1 ? 0x7fffffff : 0;
        f.parameters[7] = 0x7fffffff;
        f.block.region.reverse = (b & 2) != 0;
        if (!f.render(true)) return fail("Read outside silent trim after live edit");
    }
    return true;
}
bool reverse_and_level() {
    for (unsigned i = 0; i < 32768; ++i) pcm[2 * i] = pcm[2 * i + 1] = i < 16384 ? 14000 : -14000;
    Fixture forward, reverse;
    forward.parameters[0] = reverse.parameters[0] = 0;
    forward.parameters[4] = reverse.parameters[4] = 0x40000000;
    reverse.block.region.reverse = true;
    for (unsigned b = 0; b < 40; ++b) {
        if (!forward.render() || !reverse.render()) return false;
        for (unsigned i = 0; i < 128; ++i) {
            if (forward.block.output[i] < 0 || reverse.block.output[i] > 0)
                return fail("Reverse does not follow saved direction");
        }
    }
    forward.parameters[7] = 0;
    if (!forward.render(true)) return fail("Level zero is not silent");
    forward.parameters[7] = 0x7fffffff; forward.block.region.level = 0;
    return forward.render(true) || fail("Pad level zero is not silent");
}
bool isolation_and_restart() {
    for (unsigned i = 0; i < 32768 * 2; ++i) pcm[i] = static_cast<opxy::i16>((i * 193u) % 60000u - 30000);
    Fixture a, b, unrelated;
    a.parameters[3] = b.parameters[3] = 0x40000000;
    a.parameters[5] = b.parameters[5] = 0x40000000;
    a.parameters[6] = b.parameters[6] = 0x7fffffff;
    unrelated.parameters[0] = 0x7fffffff;
    for (unsigned i = 0; i < 150; ++i) {
        if (!a.render() || !unrelated.render() || !b.render()) return false;
        if (std::memcmp(a.block.output, b.block.output, 128 * sizeof(opxy::i32)))
            return fail("One voice changes another voice's grains");
    }
    granular::start(a.voice);
    granular::initialize(b.voice, opxy::SampleView{pcm, 32768, 44100, 2});
    for (unsigned i = 0; i < 80; ++i) {
        if (!a.render() || !b.render()) return false;
        if (std::memcmp(a.block.output, b.block.output, 128 * sizeof(opxy::i32)))
            return fail("Note restart retains stale grains");
    }
    return true;
}
bool controls_change_audio() {
    for (unsigned i = 0; i < 32768; ++i)
        pcm[2 * i] = pcm[2 * i + 1] = static_cast<opxy::i16>((i * 137u + i * i / 79u) % 40000u - 20000);
    for (unsigned parameter = 0; parameter < 7; ++parameter) {
        Fixture a, b;
        a.parameters[parameter] = 0;
        b.parameters[parameter] = 0x7fffffff;
        bool different = false;
        for (unsigned block = 0; block < 160; ++block) {
            if (!a.render() || !b.render()) return false;
            different = different || std::memcmp(a.block.output, b.block.output, 128 * sizeof(opxy::i32)) != 0;
        }
        if (!different) return fail("A declared control does not affect audio");
    }
    return true;
}
}
int main() {
    if (!sound_and_channels() || !invalid_and_trim() || !reverse_and_level() || !isolation_and_restart() || !controls_change_audio()) return 1;
    return 0;
}
