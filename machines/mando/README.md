# Mando

A Minitaur-inspired bass synth for the OP-XY: two band-limited oscillators,
nonlinear four-pole ladder filtering, and a silver Mandalorian-inspired helmet
with a shoulder cape that responds to notes and controls. Mando is an independent
digital design, not a component-level circuit model or a measured reproduction
of the hardware.

## Play

From the XY Machines repository root, use the collection's configured SDK
and compiler:

```sh
./xy test mando --native --cpu
./xy preview mando --watch
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
shaping. Its engine LFO destinations can address Mando Cutoff and every
other declared parameter. There is no additional engine envelope or LFO;
Mando does not reproduce Minitaur's independent filter ADSR, external input,
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
`python machines/mando/tools/generate_tables.py` from the repository root.
Audio callbacks allocate nothing and use
no floating point, OS services or per-sample general division.

This follows the dual saw/square oscillators and resonant 24 dB/octave low-pass
architecture described in [Moog's Minitaur manual](https://api.moogmusic.com/sites/default/files/2018-02/Minitaur_Manual.pdf).
No Moog source code, firmware or artwork is included. Mando is not affiliated
with or endorsed by Moog Music.

The display is an articulated beskar portrait: bevelled armour, a black T-shaped
visor, layered cape folds and a rifle with a telescoping barrel and two moving
muzzle prongs. Blue accents belong to the scope and rangefinder; small red cells
show weapon charge. There is no mark or light inside the visor.

| Parameter | Visual response |
| --- | --- |
| Cutoff | Flares the cape outward, moving its layered folds and hem. |
| Reso | Swings the rifle from a resting angle into an aiming position. |
| Drive | Extends the barrel and enlarges the red charge cells. |
| Waves | Selects four weapon stances and matching saw/pulse chest badges. |
| Detune | Spreads and angles the rifle's two muzzle prongs. |
| Balance | Turns the helmet and transfers the light across its metal surfaces. |
| Octave | Changes the whole portrait between three framing distances. |
| Keytrack | Extends the rangefinder mast and rotates its sensor head. |

A full-range change moves a visible feature by at least 7–26 screen pixels,
depending on the control; the test measures transformed feature positions rather
than header text. Continuous visual changes settle within about 0.2 seconds.
Notes add a helmet nod, metal glint, rifle recoil and cape movement. Idle screens
stay still; reduced motion jumps to the resting pose and suppresses note
movement. Track changes establish fresh note baselines.

The engine ID is now `mando`, so the native XY menu, catalog and factory
preset bank use Mando. The 16 factory patches retain their sound settings;
their type changes from `cyclops` to `mando`. This is an intentional identity
migration: older projects and user presets referring to `cyclops` may need the
engine reselected or their preset type migrated. The development kit keeps the
previous numeric engine/page assignments. Original device files are backed up;
legacy user presets are not overwritten or deleted.

## Validation and hardware

The machine tests check tuning, harmonic content, folded-harmonic suppression,
ladder rolloff and resonance, key tracking, control corners, rapid automation,
quiet-note/DC behavior and animation. Frozen fingerprints cover the auditioned B response at all eight
original preset settings plus automation and pitch changes. The expanded saturation
cache is checked exhaustively against the original integer interpolant.

The B filter response puts resonance compensation and the second saturation
before the final ladder pole, inside the feedback path. This lets the final
pole filter the saturation harmonics. Oscillators, control mappings and preset
values are unchanged; the sound intentionally differs from the first release.
This remains a digital approximation rather than a measured Minitaur model.

The engine packs each pole's even state and fractional charge into one
accumulator, unrolls the four stages and prepares waveform/pitch paths once per
block. Wavetables store their original Q14 values in 16-bit words; a 32 KiB
cache expands the exact integer saturation interpolant. Each voice caches
filter coefficients until cutoff or resonance changes. Voice state is 116
bytes; the SDK compile report gives the native footprint for its selected revision.
Compatibility fingerprints bind the optimized render to the B audition.

Pitch maximum, minimum and block constancy are determined in one scan. Blocks
whose increments all reach the existing maximum-pitch clamp use the constant
pitch path too; oscillator and filter state retain the same samples and rounding.


Use the paired host benchmark to compare CPU cost against a collection revision:

```sh
python machines/mando/tools/benchmark.py --baseline a5d22b3 > local/mando-performance.json
# For a current-layout comparison, including complete voice-state equivalence:
python machines/mando/tools/benchmark.py --baseline 400ddee --verify-state --extended --cxx /path/to/clang++
```

The benchmark accepts both the historical Cyclops path and current Mando path.
It checks audio at every block over 256 blocks per scenario; `--verify-state`
also compares complete voice state when the selected baseline has the current
layout. `--extended` adds above-limit, full-range and clamp-crossing pitch motion,
for 576 scenarios. Five timing repeats alternate order and report paired medians.
The pitch regression test checks exact phase advance for constant, clipped,
clamp-crossing, zero and random inputs, including a single differing sample in
every position of the block.

The historical 384-case benchmark against the unoptimized B response measured
about 10% lower worst-case host render time. The longer 440 Hz control sweep
estimated about 15,300 instructions per voice per 64-sample block; simultaneous
overload/control automation reached about 15,700. The short SDK run reported
21,400, illustrating the host estimator's substantial variation. These readings
remain above the 15,000 advisory aim; they do not qualify device CPU safety.

Host timings do not measure Blackfin cycles, stock processing or whole-project
utilization. The new response needs hardware CPU profiling and normal playback
validation; no catalog CPU record is claimed from host estimates.

For hardware work, follow the collection's
[development guide](../../docs/development.md#repeatable-device-updates):
commit the source, select a kit containing Mando and the machines needed by
existing projects, and preserve the last installed layout. `./xy build` and
`./xy stage` keep firmware outside every checkout. Device installation and
diagnostics require the user's separate instruction.

A catalog submission must wait for the exact revision's hardware control-sweep
CPU record and use the catalog path `machines/mando`. A catalog-only PR targets
`main`, unless it depends on SDK changes only available on `staging`.

Source and generated tables are MIT licensed; see [LICENSE](LICENSE).
