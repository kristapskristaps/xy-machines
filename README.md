# Matter

An Elements-derived physical-modeling instrument for OP-XY OS 1.2.1, with a
note-reactive isometric material mesh. Fifty-two normal modes and eight bowed
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
rates. Bowed delay lines store 16-bit samples. Each of eight voice-state slots
can own one of eight independent bowed-delay resources. Storage capacity does
not establish real-time hardware polyphony.

It exposes a focused modal voice with one selected exciter family. The original
three-way simultaneous exciter mixer, external excitation inputs, reverb/stereo
space control, string/other alternate resonator modes and Ominous are not included.
The stock XY envelope supplies the overall gate and release.

## Develop and audition

Keep the SDK beside this repository as `../open-xy`:

```sh
python ../open-xy/sdk.py test --machine . --cxx clang++ --firmware
python ../open-xy/sdk.py preview --machine . --cxx clang++ --out ../previews/matter-001
python ../open-xy/tools/serve_harness.py --preview ../previews/matter-001
```

Sound tests exercise the six exciters, pitch/control motion, output bounds and
resource ownership. An independent double-precision implementation checks 29,952
modal coefficient sets and pickup weighting. Sanitizer runs check numerical and
memory safety.

Hardware timing and normal multi-track playback remain pending. Follow the SDK's
[hardware profiling guide](https://github.com/etcetc000/open-xy/blob/staging/docs/profiling.md)
before catalog submission. Keep firmware and stock content outside every checkout.

MIT, including upstream notices in [vendor/LICENSE](vendor/LICENSE).
