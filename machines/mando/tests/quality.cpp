#include "../engine.cpp"
#include "quality_presets.h"
#include "reference/engine.cpp"
#include <cmath>
#include <complex>
#include <cstdio>
#include <cstdlib>
#include <vector>
static void require(bool ok, const char *text) {
  if (!ok) {
    std::fprintf(stderr, "FAIL: %s\n", text);
    std::exit(1);
  }
}
static std::vector<double> bands(const std::vector<double> &samples) {
  const unsigned count = static_cast<unsigned>(samples.size());
  std::vector<std::complex<double>> spectrum(count);
  for (unsigned i = 0; i < count; ++i)
    spectrum[i] =
        samples[i] * (.5 - .5 * std::cos(6.283185307179586 * i / (count - 1)));
  for (unsigned i = 1, j = 0; i < count; ++i) {
    unsigned bit = count >> 1;
    for (; j & bit; bit >>= 1)
      j ^= bit;
    j ^= bit;
    if (i < j)
      std::swap(spectrum[i], spectrum[j]);
  }
  for (unsigned length = 2; length <= count; length *= 2) {
    const auto step = std::polar(1., -6.283185307179586 / length);
    for (unsigned start = 0; start < count; start += length) {
      std::complex<double> phase(1., 0.);
      for (unsigned i = 0; i < length / 2; ++i) {
        const auto a = spectrum[start + i],
                   b = phase * spectrum[start + i + length / 2];
        spectrum[start + i] = a + b;
        spectrum[start + i + length / 2] = a - b;
        phase *= step;
      }
    }
  }
  std::vector<double> result(33, 0.);
  double energy = 0;
  // Overlapping linear bands keep a tiny frequency drift from crossing a
  // hard bin boundary, while detecting changed spectral energy/timbre.
  for (unsigned i = 1; i <= count / 2; ++i) {
    const double position = double(i) * 32 / (count / 2),
                 power = std::norm(spectrum[i]);
    const unsigned band = static_cast<unsigned>(position);
    const double fraction = position - band;
    result[band] += power * (1 - fraction);
    if (band < 32)
      result[band + 1] += power * fraction;
    energy += power;
  }
  for (auto &value : result)
    value = std::sqrt(value / energy);
  return result;
}
int main() {
  const double pi = 3.14159265358979323846;
  const double notes[] = {27.5, 55, 110, 220, 440, 880, 1760, 3520, 7040};
  double max_error = 0, max_abs = 0, min_level = 2, max_level = 0, min_edge = 2,
         max_edge = 0, max_spectral = 0;
  unsigned scenes = 0, render_blocks = 0, waveform_difference_cases = 0;
  for (unsigned patch = 0;
       patch < sizeof(quality_presets) / sizeof(quality_presets[0]); ++patch)
    for (unsigned note = 0; note < 12; ++note) {
      mando::Voice now = {};
      mando::Shared shared;
      mando_legacy::Voice old = {};
      mando_legacy::Shared legacy;
      mando::start(now, shared, 1);
      mando_legacy::start(old, legacy, 1);
      int p[8], a[64], b[64];
      unsigned pitch[64], seed = 7139;
      for (unsigned j = 0; j < 8; ++j)
        p[j] = quality_presets[patch][j];
      double aa = 0, bb = 0, error = 0, ae = 0, be = 0, ap = 0, bp = 0;
      unsigned clipped = 0;
      std::vector<double> before, after;
      before.reserve(16384);
      after.reserve(16384);
      for (unsigned block = 0; block < 256; ++block) {
        if (note == 9 && block % 8 == 0)
          for (unsigned j = 0; j < 8; ++j)
            p[j] = int((seed = seed * 1664525u + 1013904223u) >> 1);
        for (unsigned i = 0; i < 64; ++i) {
          const unsigned frame = block * 64 + i;
          const double hz =
              note == 10 ? 27.5 * std::pow(256., frame / 16383.)
                         : 53.8330078125 *
                               std::pow(2., (patch % 6) +
                                                .08 * std::sin(2 * pi * 5 *
                                                               frame / 44100.));
          pitch[i] = note < 9 ? unsigned(notes[note] / 44100 * 4294967296. + .5)
                     : note == 9 ? (seed = seed * 1664525u + 1013904223u)
                                 : unsigned(hz / 44100 * 4294967296. + .5);
        }
        mando::render(now, shared, 1, b, p, pitch);
        mando_legacy::render(old, legacy, 1, a, p, pitch);
        require(
            now.cached_filter.g > 0,
            "normal presets/automation exercise the original four-pole path");
        ++render_blocks;
        require(now.phase[0] == old.phase[0] && now.phase[1] == old.phase[1],
                "stock pitch and oscillator phases stay exact");
        for (unsigned i = 0; i < 64; ++i) {
          require(b[i] >= -536870912 && b[i] <= 536870912,
                  "declared output headroom");
          const double x = a[i] / 2147483648., y = b[i] / 2147483648.;
          if (b[i] == 536870912 || b[i] == -536870912)
            ++clipped;
          aa += x * x;
          bb += y * y;
          error += (x - y) * (x - y);
          ae += (x - ap) * (x - ap);
          be += (y - bp) * (y - bp);
          ap = x;
          bp = y;
          before.push_back(x);
          after.push_back(y);
        }
      }
      const double relative = std::sqrt(error / aa),
                   absolute = std::sqrt(error / (256 * 64)),
                   level = std::sqrt(bb / aa),
                   edge = std::sqrt((be / bb) / (ae / aa));
      if (relative > max_error)
        max_error = relative;
      if (absolute > max_abs)
        max_abs = absolute;
      if (level < min_level)
        min_level = level;
      if (level > max_level)
        max_level = level;
      if (edge < min_edge)
        min_edge = edge;
      if (edge > max_edge)
        max_edge = edge;
      const auto old_bands = bands(before), new_bands = bands(after);
      double spectral = 0;
      for (unsigned i = 0; i < old_bands.size(); ++i) {
        const double delta = old_bands[i] - new_bands[i];
        spectral += delta * delta;
      }
      spectral = std::sqrt(spectral);
      if (spectral > max_spectral)
        max_spectral = spectral;
      if (relative >= .1)
        ++waveform_difference_cases;
      std::printf("Mando four-pole patch%u note%u level%.8f edge%.8f "
                  "relative%.8f absolute%.8f spectral%.8f\n",
                  patch, note, level, edge, relative, absolute, spectral);
      require(std::isfinite(relative) && std::isfinite(edge),
              "finite sound comparisons");
      // These are musical acceptance checks; old-ladder similarity remains
      // diagnostic.
      if (note < 6)
        require(std::sqrt(bb / 16384) > .005 && std::sqrt(bb / 16384) < .20,
                "every factory bass preset is audible and has headroom");
      require(clipped == 0, "factory scenes avoid hard-rail clipping");
      std::printf("MUSICAL patch%u note%u rms%.9g clip%.9g\n", patch, note,
                  std::sqrt(bb / 16384), clipped / 16384.);
      ++scenes;
    }
  // Reuse the coherent high-note overload fixture; compare the folded third
  // directly with the original Q14 ladder; the quantized nonlinear lookup can
  // change its alias floor.
  mando::Voice now = {};
  mando::Shared shared;
  mando_legacy::Voice old = {};
  mando_legacy::Shared legacy;
  mando::start(now, shared, 1);
  mando_legacy::start(old, legacy, 1);
  int p[8] = {1503238553, 644245094, 1288490188, 0, 0, 0, 1073741824, 0}, a[64],
      b[64];
  unsigned pitch[64];
  const unsigned count = 8192, bin = 1537,
                 step = bin * (0x80000000u / (count / 2));
  for (auto &value : pitch)
    value = step;
  double ar = 0, ai = 0, br = 0, bi = 0, fr = 0, fi = 0;
  unsigned n = 0;
  for (unsigned block = 0; block < 512; ++block) {
    mando::render(now, shared, 1, b, p, pitch);
    mando_legacy::render(old, legacy, 1, a, p, pitch);
    if (block < 384)
      continue;
    for (unsigned i = 0; i < 64; ++i, ++n) {
      const double angle = 2 * pi * ((count - 3 * bin) % count) * n / count,
                   fund = 2 * pi * bin * n / count;
      const double x = a[i] / 2147483648., y = b[i] / 2147483648.;
      ar += x * std::cos(angle);
      ai += x * std::sin(angle);
      br += y * std::cos(angle);
      bi += y * std::sin(angle);
      fr += y * std::cos(fund);
      fi += y * std::sin(fund);
    }
  }
  const double old_alias = 2 * std::hypot(ar, ai) / count,
               new_alias = 2 * std::hypot(br, bi) / count,
               fundamental = 2 * std::hypot(fr, fi) / count;
  require(new_alias < fundamental * .001 && new_alias < .00001,
          "coherent overload alias stays below-60dBc/-100dBFS");
  std::printf("alias old %.12g new %.12g relative %.3fdBc\n", old_alias,
              new_alias, 20 * std::log10(new_alias / fundamental));
  std::printf(
      "SUMMARY %u scenes/%u four-pole blocks: RMSlevel%.8f..%.8f "
      "normalizededge%.8f..%.8f maxrelative%.8f abs%.8f spectral%.8f "
      "waveform-difference-cases%u; aliasQ14%.9f redesigned%.9f (-%.2fdBc)\n",
      scenes, render_blocks, min_level, max_level, min_edge, max_edge,
      max_error, max_abs, max_spectral, waveform_difference_cases, old_alias,
      new_alias, -20 * std::log10(new_alias / fundamental));
  return 0;
}
