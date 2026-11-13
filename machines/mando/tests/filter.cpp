#include "../engine.cpp"
#include <cmath>
#include <complex>
#include <cstdio>
#include <cstdlib>
static void require(bool value, const char *message) {
  if (!value) {
    std::fprintf(stderr, "FAIL: %s\n", message);
    std::exit(1);
  }
}
static double response(const mando::dsp::Filter &filter, double hz) {
  const auto z = std::polar(1., -6.283185307179586 * hz / 44100.);
  std::complex<double> value(1., 0.);
  for (unsigned i = 0; i < 2; ++i)
    value *= std::ldexp(double(filter.b0[i]), -31) * (1. + 2. * z + z * z) /
             (1. + std::ldexp(double(filter.a1[i]), -30) * z +
              std::ldexp(double(filter.a2[i]), -31) * z * z);
  return std::abs(value);
}
int main() {
  using namespace mando;
  using namespace mando::dsp;
  const unsigned fractions[] = {0, 1, 32768, 65535};
  long long minimum = 2147483647;
  unsigned cases = 0;
  for (unsigned note = 0; note < 290; ++note)
    for (unsigned fraction : fractions)
      for (unsigned resonance = 0; resonance <= 128; ++resonance) {
        const Filter f(pole(static_cast<int>(note * 65536 + fraction)),
                       static_cast<int>(2147483647ull * resonance / 128));
        for (unsigned i = 0; i < 2; ++i) {
          const long long a = 2147483648ll + 2ll * f.a1[i] + f.a2[i],
                          b = 2147483648ll - 2ll * f.a1[i] + f.a2[i];
          require(a > 0 && b > 0 && f.a2[i] >= 0 && f.b0[i] >= 0,
                  "every interpolated section is strictly Schur stable");
          if (a < minimum)
            minimum = a;
          if (b < minimum)
            minimum = b;
        }
        ++cases;
      }
  require(minimum > 64,
          "fixed-point stability margin exceeds interpolation rounding");
  const Filter clean(pole(static_cast<int>(std::log2(400. / 30) * 32 * 65536)),
                     0);
  const double slope =
      20 * std::log10(response(clean, 800) / response(clean, 1600));
  require(slope > 23 && slope < 27,
          "clean stopband has four-pole24dB/octave slope");
  double previous = 0;
  for (unsigned position = 0; position <= 19005440; position += 65536) {
    double current = response(Filter(pole(static_cast<int>(position)), 0), 200);
    require(
        current * 1.001 + 1e-8 >= previous,
        "clean cutoff opens monotonically within0.01dB interpolation ripple");
    previous = current;
  }
  const Filter ringing(
      pole(static_cast<int>(std::log2(400. / 30) * 32 * 65536)), 2147483647);
  double peak = 0;
  for (unsigned hz = 200; hz < 700; ++hz) {
    double value = response(ringing, hz);
    if (value > peak)
      peak = value;
  }
  require(peak > 2 && peak < 4, "high resonance gives a useful bounded peak");
  for (unsigned i = 1; i < 514; ++i)
    require(four_pole_tables::drive[i] <= four_pole_tables::drive[i - 1] &&
                four_pole_tables::drive[i] >= 512,
            "Drive ceiling softens monotonically in the upper register");
  require(four_pole_tables::drive[0] == 2048, "full bass Drive remains8x");
  int previous_knee = -14336;
  for (int input = -131072; input <= 131072; ++input) {
    const int output = output_knee(input);
    require(output >= previous_knee && std::abs(output) <= 14336,
            "output knee is monotone and bounded");
    require(output_knee(-input) == -output, "output knee has odd symmetry");
    if (std::abs(input) <= 10240)
      require(output == input, "output knee preserves the linear bass region");
    previous_knee = output;
  }
  require(28ll * unity * 1024 < 2147483647,
          "sum of all bounded recurrence terms fits signed32");
  Voice voice = {};
  unsigned random = 7139;
  for (unsigned block = 0; block < 4096; ++block) {
    random = random * 1664525u + 1013904223u;
    Filter f(pole(static_cast<int>(random % 19005441u)),
             static_cast<int>(random >> 1));
    for (unsigned i = 0; i < 64; ++i) {
      random = random * 1664525u + 1013904223u;
      int out = f.step(voice, static_cast<int>(random % 262145u) - 131072);
      require(std::abs(out) <= 14336,
              "automated filter output stays inside the soft ceiling");
      for (int h : voice.history)
        require(h >= -67108864 && h <= 67108864,
                "all histories remain bounded");
    }
  }
  Voice tail = {};
  Shared tail_shared;
  start(tail, tail_shared, 1);
  tail.history[4] = 16777216;
  int tail_parameters[8] = {0, 2147483647, 0, 0, 0, 0, 1073741824, 0},
      tail_output[64];
  unsigned tail_pitch[64] = {};
  int late = 0;
  for (unsigned block = 0; block < 1500; ++block) {
    render(tail, tail_shared, 1, tail_output, tail_parameters, tail_pitch);
    if (block > 1400)
      for (int value : tail_output)
        if (std::abs(value) > late)
          late = std::abs(value);
  }
  require(late < 131072, "maximum-resonance low-cutoff output tail decays to "
                         "quiet after DC rejection");
  Voice dc = {};
  Shared shared;
  start(dc, shared, 1);
  int p[8] = {1503238553, 1717986918, 1073741824, 0,
              0,          1073741824, 1073741824, 0},
      out[64];
  unsigned pitch[64] = {};
  int dc_late = 0;
  for (unsigned block = 0; block < 1500; ++block) {
    render(dc, shared, 1, out, p, pitch);
    if (block > 1400)
      for (int value : out)
        if (std::abs(value) > dc_late)
          dc_late = std::abs(value);
  }
  require(dc_late < 131072,
          "constant oscillator input is rejected by the2Hz DC blocker");
  std::printf("PASS: %u coefficient cases, margin%lld, clean slope%.3fdB/oct, "
              "resonance peak%.3f,262144 automated frames and quiet tails/DC\n",
              cases, minimum, slope, peak);
}
