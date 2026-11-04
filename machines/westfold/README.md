# Westfold

A playable West Coast voice for [Open-XY](https://github.com/etcetc000/open-xy):
a rounded triangle foundation, animated folding harmonics, audio-rate phase
modulation, a cycling slope and a bright, plucked low-pass gate. Twelve presets
cover woody strikes, rubbery bass, coppery bells, moving keys and unstable drones.

The design takes cues from Make Noise's [0-Coast manual](https://www.makenoise-manuals.com/0-coast/0-coast-manual.pdf):
its triangle foundation, odd/even Overtone, Slope-to-Multiply motion, extreme
Overtone audio interaction and transistor Dynamics section. Westfold is an
original digital instrument, with musical approximations of those behaviors.
It is not a circuit model or a patch-compatible reproduction.

## Play

Press an encoder to switch between the two fixed header rows.

| Encoder | Main control | Second row |
| --- | --- | --- |
| 1 | **Overtone:** odd through mixed to even harmonics; the upper quarter brings Slope into the audio | **FM Ratio:** 0.5, 1, 1.5, 2, 3, 4, 6, 8, with continuous positions between |
| 2 | **Multiply:** increase folding complexity | **FM Depth:** audio-rate phase modulation with a squared response |
| 3 | **Balance:** triangle foundation to shaped overtones | **Slope Hz:** 0.05–400 Hz cycling function |
| 4 | **Decay:** 45 ms–8 s gate contour | **Motion:** bipolar slope modulation of Multiply |

Overtone reaches its even family around two-thirds. Above three-quarters the
caption reads **!!** and the slope progressively multiplies the shaped audio,
creating sidebands even with Motion at zero. Slow Slope rates give amplitude
movement; audio rates become metallic and unruly. Balance at zero preserves the
clean foundation independently of Overtone, Multiply and Motion.

Decay's last half-percent selects **hold**. The stock AMP envelope still controls
note release; increase its release if it cuts off a long internal contour.
Factory presets supply sustain and a longer release for the internal gate.
The existing eight saved parameter IDs, order and ranges remain unchanged.
This revision intentionally changes the sound and retunes the factory presets;
existing projects keep their saved words and stock AMP settings.

Start with **Initial Coast**, **Buchla Wood**, **Rubber Bass** and **Copper Bells**.
Try **Slow Voltage** for moving sustained notes and **Circuit Birds** or
**Open Drone** for extreme slope interaction. Vary Multiply and Overtone with
parameter locks, keep some triangle in Balance, and move FM Ratio between its
harmonic anchors. The stock LFOs can target all eight parameters.

The teal core, amber folded wave, partials and note-responsive iris illustrate
the settings; they do not measure the output spectrum. Reduced motion preserves
note and control feedback without continuous animation.

## Develop and audition

From the XY Machines collection root, with its pinned SDK configured:

```sh
./xy test westfold --native --cpu
./xy preview westfold --watch
```

The preview prints a localhost URL. Playing a key starts audio. It has a simple
audition gate; the device adds stock envelopes, filter, velocity and mixer.
Render the twelve dry demonstrations and static folding alias checks with:

```sh
python machines/westfold/resources/render_demo.py --sdk /absolute/path/to/sdk \
  --out /absolute/external/path/westfold-audition-001
```

Use a fresh external output folder. Generated audio, previews, native objects
and firmware stay outside every checkout. Device installation and diagnostics
require separate authorization.

## Implementation and checks

The original MIT-licensed Braids folding transfer remains in `vendor/`.
Offline generation projects it into seventeen odd/even harmonic frames. Nine
band-limited partials retain the triangle foundation and folding character;
phase modulation and slope interaction add further motion and sidebands.
This deliberately limits the unmodulated upper detail compared with the previous
128-harmonic wavetable implementation.

A Q13 Clenshaw recurrence evaluates the partials using bounded 32-bit products.
Generated coefficient bounds leave product headroom at both oscillator rails.
Coefficients and gate brightness update every sixteen samples and interpolate
between updates; phase, gate, DC blocker and slope interaction run each sample.
Individual partial gains taper before Nyquist with additional modulation room.
The gate couples gain and brightness through two filter poles, with a fast onset
and exponential decay. Audio callbacks allocate no memory and perform no I/O.

The audio coefficient bank is about 0.6 KiB, replacing the former 136 KiB folding
bank at runtime. The native module is about 79 KiB including its SDK adapter.
The illustrative display retains its compact drawing bank. Voice storage grows
from 72 to 108 bytes; the eight-voice pool is preserved.

Tests cover all 65,536 quantized oscillator values across every harmonic frame,
independent trigonometric agreement within a conservative fixed-point error
bound, coefficient/product bounds, monotonic partial guards, parity, spectral
growth, dry balance, pitch, decay, phase modulation, slope sidebands, lifecycle,
automation and display. The frozen pre-redesign renderer and its fingerprints
remain in `tests/reference/` and `tests/legacy_vectors.cpp` as historical vectors;
they do not assert that the redesigned sound is unchanged.

The paired host timing tool supports an explicit redesign comparison:

```sh
python machines/westfold/tools/benchmark.py --timing-only \
  --cxx /path/to/clang++ --blocks 2000
```

This compares the same controls and pitches, skipping intentional audio/state
and display differences. Sound acceptance comes from the dedicated tests and
audition. Host timing varied with workload: the 528-scenario median improved
about 5%, while the local SDK estimate was approximately 11,700 instructions per
voice block against its 15,000 advisory target. Neither is measured Blackfin CPU.

To regenerate committed coefficients, drawing tables, metadata and presets:

```sh
python machines/westfold/resources/generate_coast.py
python machines/westfold/resources/generate_tables.py
python machines/westfold/resources/configure.py
```

## Hardware validation

Native compilation and host checks pass. The redesigned revision has no device
CPU measurement yet and is not qualified below the recommended 3.5%. The older
9.13% readout describes the previous engine. Hardware sound, polyphony, display
and a revision-bound control-sweep CPU capture remain pending. Follow the SDK's
[build guide](https://github.com/etcetc000/open-xy/blob/staging/docs/build.md) and
[profiling guide](https://github.com/etcetc000/open-xy/blob/staging/docs/profiling.md)
when device work is authorized. Building locally does not install anything.

Westfold is MIT licensed; see [LICENSE](LICENSE) and [Braids attribution](vendor/README.md).
