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

The port runs at 44.1 kHz with retimed sample playback, delay lengths and excitation
rates. Bowed delay lines store 16-bit samples. The native build is limited to two simultaneous Matter voices across all tracks;
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

Exciter selection happens once per block, with a separate compiled sample loop
for each family. Its low-pass coefficients are cached by the quantized Timbre
index, adding 16 bytes per voice; any changed index refreshes them before that
block renders. Completed modal gains supply the unchanged pickup weights;
coefficient reconfiguration invalidates that reuse. Bowed rendering collects
its active modes once per block. The excitation scaler uses an exact public
fractional multiply in place of per-sample software division. These changes
preserve the preceding revision's samples, rounding, resonator state, random
sequence and resource ownership.
The bowed delay storage interleaves the eight modes by frame, keeping their
simultaneous writes adjacent without changing delay lengths or stored samples.
When all eight bowed modes are active, a block-level dispatch selects consecutive
mode indices directly. Partial banks retain the active-mode list. Both paths
preserve mode order and per-sample feedback; hardware CPU validation is pending.

From the collection root, compare against the preceding revision with:

```sh
python machines/matter/tools/benchmark.py --cxx /path/to/clang++ --blocks 2000
```

The paired host benchmark covers 180 scenarios: all six exciters, six pitches,
default/minimum/maximum controls, position motion and simultaneous control/pitch
motion. Each checks 256 blocks with three voices, including live model edits,
retriggers, initialization/destruction and bow-resource stealing. It compares
audio, the complete original voice state, pool ownership metadata and every
logical delay sample after every voice
callback; the added coefficient cache is excluded from the state comparison.
Five timing repeats alternate before/after order. These timings are host evidence,
not measured OP-XY CPU usage.

The limiter calculates its gain once per block. A smaller screen uses 31 animated
nodes, static mesh lines and native text; idle motion settles after note release.
This reduces first-open work and avoids the former raster-letter paths and
per-edge rotation calculations. Parameter order and saved sound controls remain
unchanged; the shorter modal bank changes upper-partial density.

Hardware timing for this revision, the reported first-open freeze and normal
multi-track playback still require validation on the XY. Follow the SDK's
[hardware profiling guide](https://github.com/etcetc000/open-xy/blob/staging/docs/profiling.md)
before catalog submission. Keep firmware and stock content outside every checkout.

MIT, including upstream notices in [vendor/LICENSE](vendor/LICENSE).
