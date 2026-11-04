#include "../engine.cpp"
#include "reference/engine.cpp"

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <initializer_list>
struct Metric {
  double alias, fundamental, rms;
};
template <class V, class S, class Begin, class Render>
Metric measure(unsigned bin, int resonance, int drive, int wave, Begin begin,
               Render render) {
  V v = {};
  S s;
  begin(v, s, 1);
  int p[8] = {2147483647, resonance, drive, wave, 0, 0, 1073741824, 0}, out[64];
  unsigned pitch[64];
  const unsigned count = 8192;
  unsigned harmonic = 4096 / bin + 1;
  if (!(harmonic & 1))
    ++harmonic;
  if (harmonic < 3)
    harmonic = 3;
  unsigned folded = harmonic * bin % count;
  if (folded > count / 2)
    folded = count - folded;
  for (unsigned i = 0; i < 64; ++i)
    pitch[i] = bin * (0x80000000u / (count / 2));
  double ar = 0, ai = 0, fr = 0, fi = 0, energy = 0;
  unsigned n = 0;
  for (unsigned block = 0; block < 512; ++block) {
    render(v, s, 1, out, p, pitch);
    if (block < 384)
      continue;
    for (unsigned i = 0; i < 64; ++i, ++n) {
      double x = out[i] / 2147483648.;
      double a = 6.283185307179586 * folded * n / count,
             f = 6.283185307179586 * bin * n / count;
      ar += x * std::cos(a);
      ai += x * std::sin(a);
      fr += x * std::cos(f);
      fi += x * std::sin(f);
      energy += x * x;
    }
  }
  return {2 * std::hypot(ar, ai) / count, 2 * std::hypot(fr, fi) / count,
          std::sqrt(energy / count)};
}
static void require(bool value, const char *message) {
  if (!value) {
    std::fprintf(stderr, "FAIL: %s\n", message);
    std::exit(1);
  }
}
int main() {
  double worst = 0, old_at_worst = 0;
  unsigned cases = 0;
  for (unsigned bin : {513u, 769u, 1025u, 1537u, 2305u})
    for (int res : {0, 1073741824, 2147483647})
      for (int drive : {0, 2147483647})
        for (int wave : {0, 2147483647}) {
          auto old = measure<mando_legacy::Voice, mando_legacy::Shared>(
              bin, res, drive, wave, mando_legacy::start, mando_legacy::render);
          auto now = measure<mando::Voice, mando::Shared>(
              bin, res, drive, wave, mando::start, mando::render);
          require(now.alias < .0031623,
                  "extreme upper-register overload alias stays below-50dBFS");
          require(now.alias < .002 || now.alias < old.alias * .5,
                  "folds above-54dBFS improve on the original by at least6dB");
          require(
              now.alias < old.alias * 2 || now.alias < .000031623,
              "bright alias stays within6dB of the old design above-90dBFS");
          if (now.alias > worst) {
            worst = now.alias;
            old_at_worst = old.alias;
          }
          ++cases;
        }
  std::printf("PASS: %u brightstress cases; maximum folded alias%.9gFS "
              "(%.2fdBFS), matched original%.9gFS\n",
              cases, worst, 20 * std::log10(worst), old_at_worst);
}
