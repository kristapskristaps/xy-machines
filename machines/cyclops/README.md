# Cyclops

A Minitaur-inspired bass synth for the OP-XY: two band-limited oscillators,
nonlinear four-pole ladder filtering, and a single-eyed stone character that
responds to notes and controls. Cyclops is an independent digital design,
not a component-level circuit model or a measured reproduction of the hardware.

## Play

From the XY Machines repository root, use the collection's configured SDK
and compiler:

```sh
./xy test cyclops --native --cpu
./xy preview cyclops --watch
```

The preview prints a free localhost URL. Open it and play the keyboard or
**Play phrase**. The repository's [development guide](../../docs/development.md)
covers setup and hardware builds.

For bass, use the keyboard's minus button to move down an octave or two.
Watch mode rebuilds successful edits automatically. The preview applies its own
simple amplitude gate; it does not emulate the OP-XY's envelopes, glide,
LFOs, stock filter or effects.

## Controls

Press any encoder to switch between the two rows; both remain visible.
Shift + turn provides fine adjustment. The screen header shows physical
units; the preview's Cutoff slider shows its normalized amount.

| Page | Control | Range and use |
| --- | --- | --- |
| 1 | Cutoff | 30 Hz–16.0 kHz, exponential. The ladder's pole frequency, before key tracking. |
| 1 | Reso | 0–100. Emphasizes the filter corner; the top approaches ringing without promising sustained self-oscillation. |
| 1 | Drive | 0–100. Increases gain into the nonlinear ladder from 1× to 8×. Zero is the cleanest setting. |
| 1 | Waves | `S S`, `S P`, `P S`, `P P`: saw or square for oscillators 1 and 2. |
| 2 | Detune | 0–50 cents, oscillator 2 sharp. The header uses cents; the preview uses semitones. |
| 2 | Balance | 0 = oscillator 1; 50 = equal; 100 = oscillator 2. |
| 2 | Octave | Oscillator 2 at −12, 0 or +12 semitones. |
| 2 | Keytrack | 0–100. No tracking through one octave of filter movement per played octave, pivoting at C2 (65.406 Hz). |

Use the OP-XY's existing envelope, glide, LFO and FX controls for performance
shaping. Its engine LFO destinations can address Cyclops Cutoff and every
other declared parameter. There is no additional engine envelope or LFO;
Cyclops does not reproduce Minitaur's independent filter ADSR, external input,
CV/MIDI implementation or global monophonic note-priority logic. Its eight
voices follow the OP-XY's allocation and amplitude processing.

The bank contains 16 native presets with engine controls, AMP envelopes and
voice mode. Most are mono bass patches; **Open Twins** and **Velvet Twin** are
polyphonic. **Slow Giant** adds portamento. Browser presets recall the engine
controls only; the full AMP and voice settings apply on the OP-XY.

| Preset | Character |
| --- | --- |
| Init | Clean unison saws, open sustain, no drive or resonance. |
| Bull Bass | Rounded detuned saw bass with a little bite. |
| Round Sub | Dark square oscillators with a lower octave underneath. |
| Copper Bite | Resonant saw/pulse edge for rhythmic bass lines. |
| Octave Growl | Driven lower-octave saw/pulse weight. |
| Hollow Pulse | Detuned square pair with a hollow middle. |
| Resonant Wire | Thin, strongly resonant single-oscillator tone. |
| Open Twins | Bright detuned saw pair, polyphonic. |
| Deep Pillar | Low, restrained sub with strong key tracking. |
| Rubber Pluck | Short zero-sustain saw/pulse bass. |
| Acid Stare | Resonant single-saw bass for cutoff performance. |
| Iron Fang | Hard-driven lower-octave bass. |
| Slow Giant | Soft attack, detuned saws and glide. |
| Wooden Knock | Short zero-sustain pulse with upper-octave body. |
| Velvet Twin | Soft, lightly detuned saw/pulse blend, polyphonic. |
| Upper Eye | Brighter upper-octave saw/pulse lead bass. |

The original eight engine presets keep their exact parameter values.

## Sound and display

The original fixed-point DSP uses nine smoothly blended harmonic-limited
wavetable levels. A trapezoidal four-stage ladder solves instantaneous linear
feedback, with a tanh input in the feedback path, integrator charge retention,
mild resonance compensation and a 10 Hz output DC blocker. It runs at 44.1 kHz;
nonlinear drive can introduce aliasing at bright, high-register settings.
The table generator is reproducible with Python's standard library:
`python machines/cyclops/tools/generate_tables.py` from the repository root.
Audio callbacks allocate nothing and use
no floating point, OS services or per-sample general division.

This follows the dual saw/square oscillators and resonant 24 dB/octave low-pass
architecture described in [Moog's Minitaur manual](https://api.moogmusic.com/sites/default/files/2018-02/Minitaur_Manual.pdf).
No Moog source code, firmware or artwork is included. Cyclops is not affiliated
with or endorsed by Moog Music.

The cyclops opens its eye and iris with Cutoff, tightens its pupil with Reso,
brightens with Drive and looks between the oscillators with Balance. Detune
tilts its brow. Notes trigger a short blink that eases back to rest within
0.2 seconds. Idle screens stay still; reduced motion disables the blink and
page transitions snap. Track changes establish fresh note baselines.

## Validation and hardware

The machine tests check tuning, harmonic content, folded-harmonic suppression,
ladder rolloff and resonance, key tracking, control corners, rapid automation,
quiet-note/DC behavior and animation. Frozen fingerprints cover all eight
original presets plus automation and pitch changes. The expanded saturation
cache is checked exhaustively against the original integer interpolant.

The optimized engine preserves the original audio exactly. It packs each
pole's even state and fractional charge into one accumulator, explicitly
unrolls the fixed four stages, prepares waveform/pitch paths once per block,
and caches exact saturation values in 16 KiB. Voice state is 80 bytes (was 96).
The native module uses about 189 KiB of the shared 2032 KiB region.

The latest SDK host estimate is about **11,600 instructions per voice per
64-sample block**, below the advisory 15,000 target. A longer paired benchmark
against the imported `2c40746` engine covers 384 cases, including both
oscillators, control extremes, high notes, pitch bends and simultaneous
waveform/overload automation. Its median host-time reduction is about **34%**;
the highest calibrated estimate was about **14,000 instructions per block**.
The paired tool runs before/after in alternating order, warms each case and
uses the pinned SDK reference loop within the same process:

```sh
python machines/cyclops/tools/benchmark.py --baseline 2c40746 > local/cyclops-performance.json
```

Host timings vary and do not measure Blackfin cycles, stock processing or
whole-project utilization. Hardware CPU profiling and normal playback validation
remain pending; no catalog CPU record is claimed from these estimates.

For hardware work, follow the collection's
[development guide](../../docs/development.md#repeatable-device-updates):
commit the source, select a kit containing Cyclops and the machines needed by
existing projects, and preserve the last installed layout. `./xy build` and
`./xy stage` keep firmware outside every checkout. Device installation and
diagnostics require the user's separate instruction.

A catalog submission must wait for the exact revision's hardware control-sweep
CPU record and use the catalog path `machines/cyclops`. A catalog-only PR targets
`main`, unless it depends on SDK changes only available on `staging`.

Source and generated tables are MIT licensed; see [LICENSE](LICENSE).
