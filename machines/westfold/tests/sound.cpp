#include "../engine.cpp"
#include <cmath>
#include <cstdio>
#include <vector>
#include <cstring>

namespace {
using opxy::i32;
using opxy::u32;
int failures = 0;
void check(const char* label, bool pass) {
    if (!pass) { ++failures; std::printf("FAIL %s\n", label); }
}
i32 q(double value) { return static_cast<i32>(value * 2147483647.0); }
std::vector<i32> render(const i32* parameters, double hz, unsigned blocks = 690) {
    westfold::Voice voice = {};
    westfold::Shared shared;
    westfold::initialize(voice, shared, 1);
    westfold::start(voice, shared, 1);
    u32 pitch[64];
    for (unsigned i = 0; i < 64; ++i) pitch[i] = static_cast<u32>(hz * 4294967296.0 / 44100.0);
    std::vector<i32> out(blocks * 64);
    for (unsigned b = 0; b < blocks; ++b)
        westfold::render(voice, shared, 1, out.data() + b * 64, parameters, pitch);
    return out;
}
double energy(const std::vector<i32>& data, unsigned start, unsigned length) {
    double result = 0;
    for (unsigned i = start; i < start + length; ++i) result += double(data[i]) * data[i];
    return result / length;
}
double partial(const std::vector<i32>& data, double hz, unsigned harmonic) {
    double re = 0, im = 0;
    for (unsigned i = 0; i < 4410; ++i) {
        const double angle = i * hz * harmonic * 6.283185307179586 / 44100;
        re += data[4410 + i] * std::cos(angle);
        im += data[4410 + i] * std::sin(angle);
    }
    return re * re + im * im;
}
double parity(const std::vector<i32>& data, double hz) {
    double odd = 1, even = 1;
    for (unsigned h = 1; h <= 16; ++h) (h & 1 ? odd : even) += partial(data, hz, h);
    return even / odd;
}
double brightness(const std::vector<i32>& data, double hz) {
    double energy = 1, weighted = 0;
    for (unsigned h = 1; h <= 24; ++h) {
        const double e = partial(data, hz, h);
        energy += e; weighted += e * h;
    }
    return weighted / energy;
}
bool bounded(const std::vector<i32>& data) {
    for (i32 sample : data) if (sample > 268435456 || sample < -268435456) return false;
    return true;
}
}

int main() {
    i32 p[8] = {0,q(.6),q(.8),q(1),q(1.0/7),0,q(.3),0};
    auto odd = render(p, 220);
    p[0] = q(2.0/3);
    auto even = render(p, 220);
    check("Overtone selects odd/even harmonic families", parity(odd,220) < .01 && parity(even,220) > 3);
    p[0] = q(.2); p[1] = 0;
    auto simple = render(p, 110);
    p[1] = q(.85);
    auto folded = render(p, 110);
    check("Multiply creates a substantially richer spectrum", brightness(folded,110) > brightness(simple,110) * 2);
    p[2] = 0;
    auto dry = render(p, 220);
    p[0] = q(.95); p[1] = q(.12); p[7] = q(.9);
    auto dry2 = render(p, 220);
    check("Balance retains an independent clean fundamental", dry == dry2);
    check("Clean core has its strongest partial at the played pitch",
          partial(dry,220,1) > partial(dry,220,2) * 100 && partial(dry,220,1) > partial(dry,220,3) * 20);
    for (double hz : {55.0,110.0,440.0,1760.0,7040.0}) {
        p[3] = q(1); p[2] = q(.7); p[0] = q(.3); p[1] = q(.8);
        auto note = render(p,hz,200);
        check("Notes across seven octaves are audible and bounded", bounded(note) && energy(note,4410,4000) > 1e9);
    }
    p[3] = q(.20); p[5] = p[7] = 0;
    auto short_note = render(p,220);
    p[3] = q(.80);
    auto long_note = render(p,220);
    check("Decay controls the gate duration", energy(short_note,22050,4000) < energy(long_note,22050,4000) * .001);
    check("The low-pass gate darkens as it decays", energy(long_note,30000,4000) < energy(long_note,2000,4000));
    p[3] = q(1); p[4] = q(4.0/7); p[5] = 0;
    auto unmodulated = render(p,220);
    p[5] = q(.7);
    auto fm = render(p,220);
    check("Audio-rate modulation changes timbre without clipping", fm != unmodulated && bounded(fm));
    p[5] = 0; p[7] = q(.6); p[6] = q(.65);
    auto moving = render(p,220);
    check("Cycling slope animates the folding", moving != unmodulated && bounded(moving));
    // The high Overtone range exposes Slope in the audio path even with Motion at zero.
    p[0] = q(2.0/3); p[1] = q(.7); p[2] = q(1); p[3] = q(1); p[5] = 0; p[6] = q(.9); p[7] = 0;
    const auto harmonic_only = render(p,220);
    p[0] = q(1);
    const auto slope_audio = render(p,220);
    const double slope_hz = westfold::dsp::lookup(westfold::tables::rates,westfold::dsp::control(p[6])) * 44100.0 / 4294967296.0;
    check("High Overtone introduces the cycling Slope's ring-like sidebands",
          partial(slope_audio,440+slope_hz,1) > partial(harmonic_only,440+slope_hz,1)*20 && bounded(slope_audio));
    p[0] = q(.3); p[1] = q(.8); p[2] = q(.7); p[5] = 0; p[7] = q(.6); p[6] = q(.65);
    auto repeat = render(p,220);
    check("A restruck voice is deterministic", repeat == moving);
    westfold::Voice voice = {};
    westfold::Shared shared;
    u32 seed = 1, pitch[64];
    i32 out[64];
    westfold::initialize(voice,shared,1);
    for (unsigned block=0; block<4096; ++block) {
        if (block % 83 == 0) westfold::start(voice,shared,1);
        for (unsigned c=0;c<8;++c) {seed=1664525u*seed+1013904223u; p[c]=static_cast<i32>(seed);}
        for (unsigned i=0;i<64;++i) {seed=1664525u*seed+1013904223u; pitch[i]=seed;}
        westfold::render(voice,shared,1,out,p,pitch);
        for (i32 x:out) check("Automation and extreme pitch remain inside declared headroom",x>=-268435456&&x<=268435456);
    }
    if (!failures) std::puts("PASS parity, harmonic growth, clean balance, pitch, gate duration, modulation, lifecycle and automation");
    return failures ? 1 : 0;
}
