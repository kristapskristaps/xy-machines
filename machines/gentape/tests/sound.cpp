#include "../fx.cpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

static const double pi = 3.14159265358979323846;
static void require(bool ok, const char *message) {
    if (!ok) {
        std::fprintf(stderr, "FAIL: %s\n", message);
        std::exit(1);
    }
}
static int control(double x) { return static_cast<int>(x * 2147483647); }
struct Fixture {
    gentape::State state;
    std::vector<int> memory;
    int p[8], left[64], right[64];
    Fixture() : memory(gentape::buffer_words, 0x60000000), p{} { gentape::initialize(state); }
    void run() { gentape::process(state, {memory.data(), gentape::buffer_words}, left, right, p); }
    void zero() {
        for (unsigned i = 0; i < 64; ++i)
            left[i] = right[i] = 0;
    }
};
static double rms(double frequency, unsigned model, double saturation = 0) {
    Fixture f;
    f.p[4] = control((model + .5) / 4);
    f.p[2] = control(saturation);
    double energy = 0;
    for (unsigned b = 0; b < 300; ++b) {
        for (unsigned i = 0; i < 64; ++i)
            f.left[i] = f.right[i] = control(.18 * std::sin(2 * pi * frequency * (b * 64 + i) / 44100));
        f.run();
        if (b > 100)
            for (unsigned i = 0; i < 64; ++i) {
                double x = f.left[i] / 2147483648.;
                energy += x * x;
            }
    }
    return std::sqrt(energy / (199 * 64));
}
int main() {
    require(sizeof(gentape::State) <= opxy::effect_buffered_state_bytes, "state exceeds native slot");
    Fixture flat;
    double sum = 0;
    unsigned peak_frame = 0;
    double peak = 0;
    for (unsigned b = 0; b < 48; ++b) {
        flat.zero();
        if (!b)
            flat.left[0] = control(.25);
        flat.run();
        for (unsigned i = 0; i < 64; ++i) {
            require(flat.right[i] == 0, "stereo crosstalk with Spread off");
            if (b * 64 + i < 512)
                require(flat.left[i] == 0, "stale scratch or unprocessed dry leaked");
            const double x = flat.left[i] / 2147483648.;
            sum += x;
            if (std::fabs(x) > peak) {
                peak = std::fabs(x);
                peak_frame = b * 64 + i;
            }
        }
    }
    require(peak_frame == 512, "flat mode group delay changed");
    require(std::fabs(sum - .25) < .0001, "flat mode DC gain is not unity");
    gentape::initialize(flat.state);
    for (unsigned b = 0; b < 80; ++b) {
        flat.zero();
        flat.run();
        for (int x : flat.left)
            require(x == 0, "reset exposed old scratch");
    }
    for (opxy::Buffer absent : {opxy::Buffer{nullptr, 0}, opxy::Buffer{flat.memory.data(), 1}}) {
        std::fill(flat.left, flat.left + 64, 123);
        std::fill(flat.right, flat.right + 64, 123);
        gentape::process(flat.state, absent, flat.left, flat.right, flat.p);
        for (unsigned i = 0; i < 64; ++i)
            require(!flat.left[i] && !flat.right[i], "missing buffer not silent");
    }
    const double mid = rms(1000, 0), high = rms(16000, 0);
    require(high > mid * .90, "clean path loses excessive high frequencies");
    require(rms(8000, 3) < rms(8000, 0) * .22, "dictation voice lacks bandwidth reduction");
    require(rms(80, 3) < rms(80, 0) * .45, "dictation voice lacks low cut");
    require(rms(1000, 1) > .08, "VHS output unexpectedly quiet");
    Fixture linked, wide, dry0, dry1;
    for (Fixture *f : {&linked, &wide, &dry0, &dry1}) {
        f->p[0] = control(.8);
        f->p[1] = control(.7);
        f->p[2] = control(.6);
        f->p[3] = control(.8);
    }
    wide.p[5] = control(1);
    dry1.p[7] = control(1);
    double difference = 0;
    for (unsigned b = 0; b < 1600; ++b) {
        for (unsigned i = 0; i < 64; ++i) {
            const int x = control(.25 * std::sin(2 * pi * 440 * (b * 64 + i) / 44100));
            for (Fixture *f : {&linked, &wide, &dry0, &dry1})
                f->left[i] = f->right[i] = x;
        }
        for (Fixture *f : {&linked, &wide, &dry0, &dry1})
            f->run();
        for (unsigned i = 0; i < 64; ++i) {
            require(linked.left[i] == linked.right[i], "linked stereo wandered apart");
            require(dry0.left[i] == dry1.left[i] && dry0.right[i] == dry1.right[i],
                    "Dry mixed source into callback");
            difference += std::fabs((wide.left[i] - static_cast<double>(wide.right[i])) / 2147483648.);
        }
    }
    require(difference > 100, "Spread fails to create stereo variation");
    // Every corner, fast automation, signed out-of-range controls, full-scale
    // alternating inputs and resets: also run under undefined/address sanitizers.
    Fixture stress;
    unsigned random = 123;
    for (unsigned b = 0; b < 5000; ++b) {
        for (unsigned k = 0; k < 8; ++k) {
            random = random * 1664525u + 1013904223u;
            stress.p[k] = b < 256 ? ((b >> k) & 1 ? 2147483647 : 0) : static_cast<int>(random);
        }
        for (unsigned i = 0; i < 64; ++i) {
            stress.left[i] = i & 1 ? 2147483647 : (-2147483647 - 1);
            stress.right[i] = -(stress.left[i] / 2);
        }
        if (b == 2500)
            gentape::initialize(stress.state);
        stress.run();
        require(stress.state.delay[0] >= 4 * 65536 && stress.state.delay[1] <= 1500 * 65536,
                "read head escaped line");
    }
    std::printf("PASS: Gen Tape delay, stale scratch, reset, stereo, routing, "
                "bandwidth, Spread and control stress; clean 16k/1k %.4f\n",
                high / mid);
}
