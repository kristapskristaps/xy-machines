#include "../fx.cpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
static void require(bool value, const char *message) {
  if (!value) {
    std::fprintf(stderr, "FAIL: %s\n", message);
    std::exit(1);
  }
}
static int memory[halo::buffer_words];
static int floor_div(long long value, int scale) {
  return static_cast<int>(value / scale - (value < 0 && value % scale != 0));
}
int main() {
  const int kernel[10] = {1, 3, 6, 10, 12, 12, 10, 6, 3, 1};
  halo::Cic down = {}, up = {};
  int history[10] = {}, stuffed[10] = {};
  unsigned random = 7139;
  for (unsigned frame = 0; frame < 1048576; ++frame) {
    random = random * 1664525u + 1013904223u;
    const int input = static_cast<int>(random % 33554433u) - halo::unit;
    const unsigned at = frame % 10;
    history[at] = input;
    halo::integrate(down, static_cast<unsigned>(input));
    if ((frame & 3) == 3) {
      long long expected = 0;
      for (unsigned tap = 0; tap < 10; ++tap)
        expected +=
            static_cast<long long>(history[(at + 10 - tap) % 10]) * kernel[tap];
      require(halo::decimated(down) == floor_div(expected, 64),
              "CIC downsampling matches direct ten-tap convolution through "
              "unsigned wrap");
    }
    int value = 0;
    unsigned injected = 0;
    if (!(frame & 3)) {
      random = random * 1664525u + 1013904223u;
      value = static_cast<int>(random % 33554433u) - halo::unit;
      injected = halo::comb(up, static_cast<unsigned>(value));
    }
    stuffed[at] = value;
    long long expected = 0;
    for (unsigned tap = 0; tap < 10; ++tap)
      expected +=
          static_cast<long long>(stuffed[(at + 10 - tap) % 10]) * kernel[tap];
    require(halo::interpolated(up, injected) == floor_div(expected, 16),
            "CIC interpolation matches the independent zero-stuffed FIR at "
            "every phase");
  }
  double powers[3] = {};
  const double frequencies[3] = {1000, 8000, 16000};
  for (unsigned tone = 0; tone < 3; ++tone) {
    double re = 0, im = 0;
    for (unsigned tap = 0; tap < 10; ++tap) {
      double phase = 6.283185307179586 * frequencies[tone] * tap / 44100;
      re += kernel[tap] * std::cos(phase);
      im += kernel[tap] * std::sin(phase);
    }
    powers[tone] = re * re + im * im;
  }
  const double alias = 10 * std::log10(powers[1] / powers[0]),
               high = 10 * std::log10(powers[2] / powers[0]);
  require(alias < -26 && high < -32,
          "CIC anti-alias response is bounded in the upper input band");
  halo::State state = {};
  state.clock = 0xfffff000u;
  for (unsigned frame = 0; frame < 5000; ++frame) {
    for (unsigned line = 0; line < 4; ++line)
      memory[line * halo::fdn_stride + (state.clock & 2047u)] =
          static_cast<int>(frame * 21 + line);
    memory[halo::predelay_offset + (state.clock & 2047u) * 2] =
        static_cast<int>(frame * 17);
    memory[halo::predelay_offset + (state.clock & 2047u) * 2 + 1] =
        -static_cast<int>(frame * 13);
    memory[halo::shimmer_offset + (state.clock & 2047u)] =
        static_cast<int>(frame * 11);
    const unsigned packed =
        (static_cast<unsigned short>(static_cast<short>(frame * 19))) |
        (static_cast<unsigned>(
             static_cast<unsigned short>(-static_cast<int>(frame * 23)))
         << 16);
    memory[halo::echo_offset + state.echo_position] = static_cast<int>(packed);
    if (++state.echo_position == halo::echo_frames)
      state.echo_position = 0;
    ++state.clock;
    ++state.written;
  }
  for (unsigned age = 1; age < 2047; ++age) {
    const unsigned frame = 5000 - age;
    for (unsigned line = 0; line < 4; ++line)
      require(halo::sample(state, memory, line * halo::fdn_stride, 2048, age) ==
                  static_cast<int>(frame * 21 + line),
              "global-clock FDN reads survive32-bit wrap");
    require(halo::sample(state, memory, halo::predelay_offset, 2048, age, 2,
                         0) == static_cast<int>(frame * 17) &&
                halo::sample(state, memory, halo::predelay_offset, 2048, age, 2,
                             1) == -static_cast<int>(frame * 13),
            "stereo predelay channels stay independent");
    require(halo::echo_sample(state, memory, age, 0) ==
                    static_cast<short>(frame * 19) * 512 &&
                halo::echo_sample(state, memory, age, 1) ==
                    static_cast<short>(-static_cast<int>(frame * 23)) * 512,
            "packed echo preserves both signed channels");
  }
  for (unsigned division = 0; division < 8; ++division)
    for (unsigned bpm = 400; bpm <= 3000; bpm += 100) {
      const int control = static_cast<int>(division * 268435456u);
      const double expected = halo::network_rate * 600.0 / bpm *
                              opxy::division_sixteenths[division] / 16.;
      require(std::fabs(halo::echo_delay(control, bpm) / 256. - expected) <
                  .004,
              "tempo echo divisions keep their physical timing");
    }
  unsigned random_soft = 9137;
  for (unsigned i = 0; i < 1048576; ++i) {
    random_soft = random_soft * 1664525u + 1013904223u;
    const int x =
        static_cast<int>(random_soft % (6u * halo::unit)) - 3 * halo::unit;
    const unsigned a = static_cast<unsigned>(x < 0 ? -x : x);
    const int y = halo::soft(x);
    const int expected =
        a >= 2u * halo::unit
            ? halo::unit
            : static_cast<int>(a - (static_cast<long long>(a) * a) /
                                       (4ll * halo::unit));
    require(y == (x < 0 ? -expected : expected) && halo::soft(-x) == -y &&
                std::abs(y) <= halo::unit,
            "analytic soft curve is odd, bounded and matches wide arithmetic");
  }
  halo::initialize(state);
  const unsigned width = halo::network_rate / 10, step = 16777216u / width;
  double re[2] = {}, im[2] = {};
  for (unsigned i = 0; i < 66150; ++i) {
    const int shifted = halo::pitch_up(state, memory, width);
    if (i > 11025)
      for (unsigned bin = 0; bin < 2; ++bin) {
        const double angle =
            6.283185307179586 * (bin ? 440 : 220) * i / halo::network_rate;
        re[bin] += shifted * std::cos(angle);
        im[bin] += shifted * std::sin(angle);
      }
    memory[halo::shimmer_offset + (state.clock & 2047u)] = static_cast<int>(
        std::sin(6.283185307179586 * 220 * i / halo::network_rate) *
        halo::unit / 4);
    ++state.clock;
    if (state.written < halo::echo_frames)
      ++state.written;
    state.grain_phase = (state.grain_phase + step) & 16777215u;
  }
  const double octave = 10 * std::log10((re[1] * re[1] + im[1] * im[1]) /
                                        (re[0] * re[0] + im[0] * im[0]));
  require(octave > 30, "shared shimmer heads produce an audible octave up");
  std::printf("PASS:1048576 CIC frames, global-ring wraps, stereo echo/tempo, "
              "analytic clip;8k rejection%.2fdB,16k%.2fdB, octave%.2fdB\n",
              alias, high, octave);
}
