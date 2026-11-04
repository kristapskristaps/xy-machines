#include "../dsp.h"
#include <cstdio>
#include <cstdlib>
static void require(bool ok, const char *message) {
  if (!ok) {
    std::fprintf(stderr, "FAIL: %s\n", message);
    std::exit(1);
  }
}
static int reference(const opxy::i16 *values, unsigned at, unsigned fraction) {
  return values[at] +
         static_cast<int>(
             (static_cast<long long>(values[at + 1] - values[at]) * fraction) >>
             10);
}
int main() {
  unsigned comparisons = 0;
  int maximum_slope = 0, maximum_difference = 0;
  for (unsigned shape = 0; shape < 2; ++shape)
    for (unsigned band = 0; band < 9; ++band)
      for (unsigned mix = 0; mix < 3; ++mix) {
        const auto table = shape ? mando::baked_tables::square[band][mix]
                                 : mando::baked_tables::saw[band][mix];
        for (unsigned at = 0; at <= 1024; ++at)
          require(table[at] >= -8344 && table[at] <= 8344,
                  "all waveform endpoints preserve signed16 blend bounds");
        for (unsigned at = 0; at < 1024; ++at) {
          const int difference = table[at + 1] - table[at];
          require(difference >= -32768 && difference <= 32767,
                  "every generated interpolation slope fits signed16");
          if (std::abs(difference) > maximum_slope)
            maximum_slope = std::abs(difference);
        }
      }
  for (unsigned shape = 0; shape < 2; ++shape)
    for (unsigned band = 1; band < 9; ++band) {
      const auto rich =
          shape ? mando::tables::square[band] : mando::tables::saw[band];
      const auto lean = shape ? mando::tables::square[band - 1]
                              : mando::tables::saw[band - 1];
      const auto half = shape ? mando::baked_tables::square[band][1]
                              : mando::baked_tables::saw[band][1];
      for (unsigned at = 0; at < 1024; ++at)
        for (unsigned fraction = 0; fraction < 1024; ++fraction) {
          const int a = reference(rich, at, fraction),
                    b = reference(lean, at, fraction);
          const int expected = a + ((b - a) >> 1);
          const int actual =
              mando::dsp::wave_linear(half, (at << 22) | (fraction << 12));
          const int error = std::abs(expected - actual);
          require(error <= 1,
                  "preblending and interpolation commute within1Q14LSB");
          if (error > maximum_difference)
            maximum_difference = error;
          ++comparisons;
        }
    }
  std::printf("PASS: all54 table slopes fit signed16(max%d); %u half-mip phase "
              "comparisons(max%dQ14LSB)\n",
              maximum_slope, comparisons, maximum_difference);
}
