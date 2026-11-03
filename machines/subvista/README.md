# Subvista

An eight-voice subtractive synth prototype for the Open-XY SDK, ID `subvista`.
Machine code and tests live in this folder within XY Machines.

Two slightly detuned, band-limited wavetable oscillators blend saw and pulse,
with a quiet sine sub-oscillator, into a resonant two-pole low-pass filter.
A per-note contour opens the cutoff and decays over roughly 300 ms. The SDK
browser supplies its audition amplitude gate; native stock envelopes follow
this oscillator on the device.

| Control | Behavior |
| --- | --- |
| Cutoff | Low-pass cutoff, exponential sweep from 60 Hz to 15 kHz |
| Reso | Filter resonance, damped through a pronounced peak |
| Shape | Saw to pulse blend |
| Contour | Amount of the per-note downward filter sweep |

The display animates a signal-flow schematic: raw waveform, filter aperture,
filtered waveform and cutoff curve. These drawings describe the controls;
they are not measured audio scopes. The workbench's Waveform tab is the live
output scope. Reduced motion stops continuous movement.

Five starting presets: Soft Current, Glass Teeth, Hollow Reed, Acid Bloom,
and Open Circuit. Click Enable audio, then Play phrase or use
`A W S E D F T G Y H U J K`; Escape stops all notes.

## Current validation

The SDK machine checks, WebAssembly preview, and host sound/display tests pass.
Host tests cover filter attenuation, 27 control combinations, pitch extremes,
output bounds, deterministic retriggering, and 800 display frames including
reduced motion. The host tests also pass AddressSanitizer and UBSan.

The filter retains its two-pole topology, saturating Q31 arithmetic and
per-sample coefficient ramps. The eight mip bands, phase increments, detune,
sub-oscillator, envelope, voice capacity and saved controls remain unchanged.

This revision deliberately replaces runtime linear wavetable interpolation with
nearest-point lookup in 1024-point mip tables. Original samples alternate with
exact midpoint samples from the preceding interpolator; phase rounds to the
nearest point. This reduces oscillator work and constant memory while adding a
small phase-grid error and stronger high-frequency reconstruction images.
Audio is intentionally not bit-identical.

Across 540 rendered cases (all control corners, defaults/presets, six pitches,
control sweeps, random automation and glide), median relative RMS error is
0.31%; the worst is 2.00% at a bright resonant 27.5 Hz saw setting. Maximum measured
RMS level change above -60dBFS is 0.018dB. A coherent single-oscillator check finds
worst out-of-band/image energy of -35.9dBc, compared with -48.3dBc for the
original interpolator. This check includes reconstruction images and folded
images; it does not claim a perfect alias-free reference. These are host audio
comparisons, not measured device CPU.

The original PCM/state golden values remain unchanged in
`tests/reference_goldens.cpp`, checked against the frozen exact implementation
in `tests/reference_engine.hpp`. Current sound tests use explicit error and
headroom budgets in `tests/fidelity.cpp`; they do not pass the old golden hashes
off as the new output. Phase, envelope and coefficient state remain exact.

Oscillator generation and filtering remain separate block stages, using the
output buffer for temporary oscillator inputs before filtering in place. The
pure-saw endpoint still skips pulse lookup/blending. No scratch array is added.
Regenerate `dense_waves.h` with
`python machines/subvista/tools/generate_dense_waves.py` from the collection root.

This revision declares SDK 1.8 because the public saturating add is currently
available on upstream `staging`, rather than the released `main`. It builds with
the collection's pinned SDK. Submit its catalog entry against `staging` until
that interface reaches `main`; no SDK changes are bundled with the machine.

## Develop and audition

Use the collection's pinned SDK through the root commands:

```sh
./xy test subvista --native --cpu
./xy preview subvista --watch
```

For the quantitative audio comparison against the frozen exact revision:

```sh
python machines/subvista/tools/fidelity.py --out /external/private/subvista-audio-check-001
```

The output includes the full error/loudness/spectral report, a coherent oscillator
image analysis and reference/candidate WAV pairs for listening. The comparison
checks 540 cases and verifies phase, envelope and coefficient state after each
block. It fails beyond 2.5% relative RMS error or 0.05 dB RMS change for reference
levels above -60dBFS, beyond 0.0015 FS absolute RMS error, or above -35dBc image
energy in the specified coherent oscillator cases. Normal playback and final
hardware CPU qualification remain required.

## Hardware and publication

Follow the SDK's [firmware build guide](https://github.com/etcetc000/open-xy/blob/staging/docs/build.md)
and [hardware profiling workflow](https://github.com/etcetc000/open-xy/blob/staging/docs/profiling.md).
Keep firmware and stock content outside every checkout. Commit the machine,
build with profiling enabled, install when authorized and collect the required sweep. Three captures help
check repeatability.
The host estimate printed by `test` cannot substitute for those measurements.

For the builder, follow [catalog submission](https://github.com/etcetc000/open-xy/blob/staging/docs/catalog.md#submit-a-machine).
Every new or changed draft/published entry needs a hardware CPU record tied to
its pinned commit. Hardware validation may be performed by a maintainer if you
do not own an XY. Publication remains pending until that evidence is available.

The generated starter is covered by [LICENSE](LICENSE). Preserve notices for
code you reuse and include the licences of any dependencies or ports.
