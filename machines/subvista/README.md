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
zero, skipping the unused pulse lookups and blend. Both oscillator loops retain the same
wavetable arithmetic, phase and contour decay; voice memory and saved controls
are unchanged. Exact i16 interpolation slopes
are generated from the original waves in `tables.h`, replacing per-lookup
subtraction and signed division. This adds 8 KiB of constant data. Regenerate
`wave_slopes.h` with `python machines/subvista/tools/generate_slopes.py` from the
collection root. Arithmetic tests check every committed slope against the
original expression and interpolation across the waves.
The pulse/blend path pairs each waveform point with its half-cycle partner,
sharing the phase index and interpolation fraction. The pure-saw endpoint and
quiet sub-oscillator retain their original separate wave/slope banks to avoid
variable-half lookups. This adds a 24 KiB read-only paired bank; CPU improvement
and memory-layout effects require a fresh hardware comparison. Regenerate the
paired bank and slopes with `python machines/subvista/tools/generate_slopes.py`.
The oscillator helper
stays inlined so native sample loops use hardware loop control without two
oscillator function calls per sample.

Oscillator generation and the recursive filter run as separate block stages.
The first stage keeps the original pure-saw fast path and uses the output buffer
for 64 temporary oscillator inputs; the second filters those inputs in place.
Phase updates have no feedback from the filter, so oscillator arithmetic and
phase state remain unchanged. The private stages stay outlined to reduce shared
register pressure without adding a scratch array. Native loop control and exact
host comparisons pass; CPU improvement still requires hardware measurement.

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

For quantitative audio comparison against the frozen exact staged revision:

```sh
python machines/subvista/tools/fidelity.py --out /private/output/subvista-quality
```

The filter shares one product in its symmetric two-state matrix, reducing four
Q31 multiplies to three. Smooth coefficient ramps use their block midpoint;
changes larger than 0.2% of Q31 per block retain the original per-sample ramp.
The coefficient endpoints, oscillator phases, contour, resonant topology and
integrator/output bounds remain unchanged. A redundant intermediate band clamp
is removed; a boundary/random arithmetic test proves that its bounded state
update is identical. Integer rounding in the shared matrix can differ slightly.

The quality tool compares 540 actual C++ renders at six pitches, including the
three-position control grid, defaults, all five presets, smooth/random automation,
retrigger and glide. Measured worst RMS error is 0.000172 FS, audible level change
is at most 0.0041 dB, and factory preset relative RMS error is below 0.66%.
The largest relative error, 2.34%, occurs in a nearly muted control corner.
The documented limits are 2.5% audible relative RMS error, 0.0015 FS absolute RMS
error and 0.05 dB audible level change. Oscillator generation and its images are
unchanged. The tool includes spectral error analysis and exports reference and
candidate WAV pairs. The original exact audio/state golden values remain
unchanged and run against the frozen reference; a separate fidelity test checks
the current DSP rather than replacing those goldens.

Native loops and host arithmetic checks pass, including UBSan; these checks do
not measure device CPU. The shared kernel and added fallback change native
register allocation, so hardware must establish whether this implementation is
faster and meets the 3.5% target.

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
