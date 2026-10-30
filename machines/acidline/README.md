# Acidline

An acid synth for Open-XY: band-limited saw/pulse, a resonant four-pole
filter, a pluck envelope and distortion. Stock OP-XY supplies pitch,
portamento, velocity, amplitude envelopes and mixing.

## Mono playback

The default and all five named factory presets select stock **mono** play mode.
This is mono independently on each track: using Acidline on another track does
not steal the first track's voice. Existing projects retain their saved play
mode; load an updated factory preset or switch M2 / Shift play mode to mono
once. Loading a preset replaces its sound settings, so save an edited sound
first. The machine API does not lock the stock play-mode menu: selecting poly
manually enables overlapping voices and multiplies DSP cost again.

Stock portamento follows the supplied sample-by-sample pitch ramp. No second
glide, global voice lock, velocity detector or internal amplitude gate is added.
Use parameter locks on Cutoff, Env Amt, Reso or Distort for filter accents.
The 16-slot DSP pool remains available for independent tracks and stock fades;
it is not a per-track polyphony setting.

## Controls and display

Press any encoder to switch the complete page. The SDK's shared two-page
header shows both rows and highlights the active row. Saved parameter order
and mappings are unchanged; this update adds no filler controls.

| Encoder | Page 1 | Page 2 |
| --- | --- | --- |
| 1 | Cutoff | Decay |
| 2 | Reso | Wave |
| 3 | Distort | Distort (same saved control) |
| 4 | Env Amt | Env Amt (same saved control) |

A laboratory bench contains red acid, blue catalyst and a pale base, connected
by a recirculating glass pipe network. Coloured fluid packets travel through
the pipes while notes sound; cutoff controls flow speed. Turning valves, two
pressure gauges, reaction flashes and droplets respond to the controls and
note pulses. The apparatus sits against a tiled lab wall with pump housings
and an instrument cabinet. Cutoff,
resonance and distortion change the liquid levels. Notes trigger ripples,
bubbles and short vapour bursts; held notes keep the reaction active. Idle
motion settles, track changes reset activity, and reduced motion removes the
continuous movement. Display work is separate from the audio callback.

## DSP

The oscillator, nonlinear feedback filter and distortion run at 88.2 kHz.
A symmetric 31-tap half-band FIR returns 44.1 kHz output. Filter saturation,
anti-aliasing, signal levels and control mappings are unchanged.

The FIR's seven nonzero symmetric pairs and four filter stages are unrolled
without changing accumulation order or rounding. A before/after comparison of
153,600 output samples with changing controls, pitch and retriggers was
bit-identical. The host advisory estimate changed from roughly 45,300 to
42,000 Blackfin instructions per voice block in this run; these estimates are
noisy and are not hardware measurements. The main overload fix is using stock
mono rather than accumulating heavy polyphonic voices. Several Acidline tracks
still add CPU cost.

## Development

From the XY Machines root:

```sh
./xy test acidline --native --cpu
node --test machines/acidline/tests/presets.test.mjs
./xy preview acidline --watch
```

The C++ tests cover control extremes, peak bounds, filter attenuation,
distortion, exact stock pitch ramps, independent DSP states, display bounds,
both header pages, idle/reduced-motion behavior, full-convolution equivalence
and spectral alias rejection. Preset tests check stock mono mode and unchanged
sound parameters. The optional dedicated acid sequencer under `audition/`
provides its own simple mono gate and glide; it does not emulate stock's
allocator or envelopes.

Use the root browser workflow to refresh the cached builder after source
changes. This revision still needs device testing with rapid notes and several
independent tracks, plus a revision-bound CPU profile before publication.
Host/native checks do not establish real-time hardware headroom. Keep firmware
and stock content outside every checkout; device operations require a separate
user instruction.

MIT; see LICENSE. DSP, tables and browser audition are original to this work.
