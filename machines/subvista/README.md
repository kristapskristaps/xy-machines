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

The filter uses the SDK's saturating Q31 add, preserving the previous clipping
while using the native DSP operation. A separate sample loop handles Shape at
zero, skipping the unused pulse lookups and blend. Both loops retain the same
filter interpolation, wavetable arithmetic, oscillator phase and contour decay;
voice memory and saved controls are unchanged. Exact interpolation points are
generated from the original waves in `tables.h`, replacing per-lookup subtraction
and signed division. Each point stores both half-cycle waveform values and
their slopes together, so a pulse blend reuses one phase index and fraction.
The native build uses the 24 KiB paired bank and omits the original separate
wave/slope banks, including their unused guard samples.
Regenerate `wave_pairs.h` and the reference `wave_slopes.h` with
`python machines/subvista/tools/generate_slopes.py` from the collection root.
Arithmetic tests check the slopes and interpolated samples against the original
waves. The oscillator helper
stays inlined so native sample loops use hardware loop control without two
oscillator function calls per sample.

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

For a paired comparison against the preceding collection revision:

```sh
python machines/subvista/tools/benchmark.py --cxx /path/to/clang++ --blocks 4000
```

It checks exact audio and complete voice state across 555 scenarios: the 81-point
three-position control grid at six pitches, defaults, all five presets, control
motion, sample-by-sample pitch motion, and both sides of every wavetable band
transition. Each scenario compares 256 blocks including retriggers and
reinitialization, then alternates five before/after timing repeats. The arithmetic
test compares saturation with an independent wide sum at the signed boundaries
and one million random pairs. Host timings and advisory CPU estimates do not
measure the device's CPU or establish usable polyphony.

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
