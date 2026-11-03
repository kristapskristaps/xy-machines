# Matter

An Elements-derived physical-modeling instrument for OP-XY OS 1.2.1, with a
note-reactive isometric material mesh. Twenty-four normal modes or eight bowed
waveguide modes respond to six exciter families: mallet, plectrum, particles,
bow, granular breath and sampled strikes. Built with the
[Open-XY SDK](https://github.com/etcetc000/open-xy).

## Controls

| Encoder | Primary | Press to select alternate |
| --- | --- | --- |
| 1 | Geometry: partial spacing/material | Exciter family |
| 2 | Bright: upper-mode damping | Contour: exciter-dependent behavior |
| 3 | Damping: resonator decay | Timbre: excitation color |
| 4 | Position: pickup position | Body: raw excitation to resonant body |

Contour adjusts plectrum separation, particle persistence, bow pressure, breath
position, or sample selection. Mallet is a single strike. Bow and breath are
sustained sources; use the XY's stock envelope to shape their amplitude. The
stock filter and mixing remain downstream. Presets cover glass, wood, metal,
bowing, breath and plucked/sampled excitation.

The mesh visualizes control and note activity, including pickup position; it is
not an audio analyzer. It supports the SDK's reduced-motion preference.

## Port scope

The normal resonator, bowed feedback, excitation algorithms and sample resources
come from Émilie Gillet's Elements. High-precision integer coefficient routines
also derive from the DMDaudio fixed-point Plaits port. See
[vendor/VENDOR.md](vendor/VENDOR.md) for revisions and numerical changes.

The public input/output and excitation run at 44.1 kHz. Resonators use a
transposed two-pole bandpass at adaptive 44.1, 22.05, 11.025 or 5.5125 kHz rates.
Each normal mode selects its own rate: frequencies below 1,894.92 Hz use
5.5125 kHz, those below 3,789.84 Hz use 11.025 kHz, those below 7,579.69 Hz
use 22.05 kHz, and higher modes remain at 44.1 kHz. All eight coupled bowed
modes share a rate, keeping the highest active frequency below 0.4 cycles per
internal sample. No normal or bowed modes are removed by this optimization.
Normal-mode contributions accumulate at each rate before one interpolation of
the summed group, avoiding repeated full-rate interpolation for every mode.
Integer division ordering differs by at most 168 mixed units from separately
interpolating 24 modes; group endpoints remain exact.
Original discrete-time bandpass peak frequencies, peak gains and pole decay rates
are mapped to the slower clock during coefficient configuration. Box-filtered
excitation and linear output interpolation use generated response compensation
for the normal bank. The bowed feedback retains physical delay lengths with
fractional cubic reads; its velocity smoothing follows the same physical time
constant. Sample playback and exciter timing remain at their original rates.
Bowed delay lines store 16-bit samples. The native build is limited to two
simultaneous Matter voices across all tracks;
new notes steal the oldest voice. Two independent bowed-delay resources are
acquired only by the bow exciter. Other exciters use the modal bank without
running a second waveguide network. The browser can schedule eight audition
voices, but only two can hold bowed resources.

It exposes a focused modal voice with one selected exciter family. The original
three-way simultaneous exciter mixer, external excitation inputs, reverb/stereo
space control, string/other alternate resonator modes and Ominous are not included.
The stock XY envelope supplies the overall gate and release.

## Develop and audition

Keep the SDK beside this repository as `../../../xy-machines-sdk`:

```sh
python ../../../xy-machines-sdk/sdk.py test --machine . --cxx clang++ --firmware
python ../../../xy-machines-sdk/sdk.py preview --machine . --cxx clang++ --out ../previews/matter-001
python ../../../xy-machines-sdk/tools/serve_harness.py --preview ../previews/matter-001
```

Sound tests exercise the six exciters, pitch/control motion, output bounds and
resource ownership. An independent double-precision implementation checks 13,824
modal coefficient sets and pickup weighting. Sanitizer runs check numerical and
memory safety. `tests/arithmetic.cpp` checks every integer in the excitation
scaler's bounded range against signed division by five.

The adaptive resonator implementation intentionally changes fixed-point rounding,
filter state representation and saturation behavior. It is not bit-identical to
the preceding full-rate TPT resonator. A 102-case paired render covers six
exciters, three pitches, default/dark/maximum/dry/minimum controls and two moving-control
traces per model. Static resonant examples differ by at most 3.07 dB RMS in that
set; the largest moving-control difference is 3.84 dB. Dry Body=0 examples remain
bit-identical. The box/linear conversion changes upper partials and transients,
and changing rate during control motion can change the transient response.
Independent coefficient tests check bandpass peak tuning to within one cent and
pole decay rates to within four percent of the original double-precision laws;
the tested grid observed 0.087 cents and 0.22 percent respectively. These are
numerical checks, not listening approval or measured CPU qualification.

Coefficient-only pitch hysteresis avoids rebuilding the full bank for tiny
changes in the stock pitch-increment buffer. It tracks pitch within 0.423 cent;
exciter timing remains unchanged. Independent jitter, slow-glide and vibrato
traces observe at most 0.433 cent of filter-peak difference from freshly computed
coefficients. Larger pitch or material-control changes rebuild coefficients at
the existing block boundary. This is a bounded pitch tolerance, not evidence
that a particular hardware capture contains pitch jitter.

From the collection root, save quantitative paired audio and spectral/decay data
outside the checkout:

```sh
python machines/matter/tools/quality.py --baseline <full-rate-commit> \
  --sdk /path/to/open-xy --cxx /path/to/clang++ --automation \
  --out /external/path/matter-quality-001
```

The tool reports loudness, normalized coarse spectral energy, envelope decay
fits and dominant spectral peaks. A dominant peak is not necessarily the
fundamental pitch. Sanitizer tests cover moving controls, saturated resonator
state and interpolation headroom. The older exact-state benchmark describes
full-rate candidates and is not an acceptance test for this intentional DSP
change.

The limiter calculates its gain once per block. A smaller screen uses 31 animated
nodes, static mesh lines and native text; idle motion settles after note release.
This reduces first-open work and avoids the former raster-letter paths and
per-edge rotation calculations. Parameter order and saved sound controls remain
unchanged. The earlier 24-mode port already has fewer upper partials than Elements.

Hardware timing for this revision, the reported first-open freeze and normal
multi-track playback still require validation on the XY. Follow the SDK's
[hardware profiling guide](https://github.com/etcetc000/open-xy/blob/staging/docs/profiling.md)
before catalog submission. Keep firmware and stock content outside every checkout.

MIT, including upstream notices in [vendor/LICENSE](vendor/LICENSE).
