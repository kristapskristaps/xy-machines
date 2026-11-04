# Mando

A Minitaur-inspired bass synth for the OP-XY: two band-limited oscillators,
a voiced resonant four-pole lowpass, and a silver Mandalorian-inspired helmet
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
| 1 | Cutoff | 30 Hz–16.0 kHz, exponential. Nominal filter frequency before key tracking; voiced darker at low resonance. |
| 1 | Reso | 0–100. Emphasizes the filter corner. High settings ring when excited by the oscillators; this design is stable and does not self-oscillate. |
| 1 | Drive | 0–100. Adds input saturation. Bass uses up to 8× gain; the upper register smoothly limits this toward 2× for cleaner high notes. Zero is cleanest. |
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

## Adaptive sample rate

Bass notes use a 22.05 kHz oscillator/filter core with cubic reconstruction to
44.1 kHz. Both oscillator frequencies must stay below 2,756 Hz. A voice leaving
that range switches to full-rate synthesis until its next note start; a short
crossfade smooths the handoff. Pitch phase accumulation stays exact, including
glide and the second oscillator's octave/detune. The bass path caps its filter
design at 8.5 kHz and adds four output frames of reconstruction latency. This
softens the highest harmonics compared with the full-rate design, while retaining
the dual oscillators, resonant four-pole lowpass and all 16 factory presets.
No extra voices, envelopes or LFOs are added.

Generate the low-rate coefficient bank with
`python machines/mando/tools/generate_half_rate.py` after the generators below.
Tests cover its coefficient stability, reconstruction images, rate handoff,
output headroom and the existing preset/alias/pitch checks. Hardware timing is
revision-bound and must be measured before this candidate is accepted.

## Sound and display

This experimental redesign keeps the twin harmonic-limited saw/square
oscillators, but replaces the global ladder feedback solve with two stable
lowpass biquads. One section supplies a fixed smooth response; the other adds
resonance. Their symmetric numerators need six fractional products per sample
in total. The cascade shares its intermediate output/input histories.

At zero resonance, the clean response has a four-pole Butterworth shape.
Its prewarp is scaled to retain the old ladder's dark bass corner. Increasing
Reso moves the peak toward the nominal Cutoff frequency. High-cutoff resonance
is softened above 2.5 kHz, and input Drive smoothly decreases with played
register. These choices retain strong resonant bass while avoiding excessive
high-frequency gain and folding. Drive still changes gain and saturation at
the top of the keyboard; it becomes less aggressive there.

One soft clip shapes the input. A static output knee is linear below 0.625 of
internal unity and bends peaks toward a 0.875 ceiling. It uses the same small
lookup and adds no feedback loop or audio-rate product. A 2 Hz DC blocker follows
the knee, preserving sub-bass and removing any nonlinear DC offset.

The sound deliberately differs from the previous ladder. Drive and resonance
interact differently, and **Resonant Wire** is now input-excited ringing rather
than a free-running tone; it is quieter. No independent sine is added. Other
presets keep their exact saved controls, AMP settings and voice modes. The
engine retains eight voices and its saved ID and parameter contracts.

The original phase grid is retained. Offline rich/half/lean mip blends select
one active table per oscillator. The shared tanh lookup differs by at most eight
Q14 least significant bits from the original interpolant and is exact around
zero. Generate tables from the repository root with the original generator,
then `python machines/mando/tools/generate_baked_tables.py` and
`python machines/mando/tools/generate_four_pole.py`.
Callbacks allocate nothing and use no floating point or OS services. Coefficients
update only when cutoff or resonance changes; the Drive ceiling is prepared
once per block. There is no callback-built wavetable or long cold-start cache.

The dual saw/square layout was inspired by
[Moog's Minitaur manual](https://api.moogmusic.com/sites/default/files/2018-02/Minitaur_Manual.pdf).
Mando is an independent digital design, not a circuit model or measured
reproduction. No Moog source, firmware or artwork is included, and Mando is not
affiliated with or endorsed by Moog Music.

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

Musical tests check tuning, four-pole rolloff, a useful smooth resonant peak,
cutoff/key tracking, full control corners, rapid automation, reset, quiet tails,
DC rejection and display motion. Integer Schur inequalities cover 149,640
interpolated coefficient settings; the minimum margin is 7,436 integer counts.
The clean coefficient response falls about 24.2 dB/octave in the checked stopband,
and the high-resonance peak is 2.78 times unity in its bass fixture. Histories and
all recurrence arithmetic have explicit bounds.

All 16 presets are rendered at nine pitches, during automation, over an
eight-octave glide and around mip transitions: 192 scenes and 49,152 callbacks.
The tested bass notes have RMS levels of roughly 0.011–0.152 FS, with no samples
on the hard output rails. Comparisons with the old ladder report level,
sample and spectral changes as diagnostics; they do not assert PCM or timbre
equivalence. The old fingerprints and original implementation remain frozen
and independently authenticated.

A coherent overload fixture keeps its folded third below −60 dBc and −100 dBFS.
A separate 60-case bright-overload sweep compares absolute aliases with the
original ladder; strong folds must improve, and changes above the quiet floor
cannot grow by more than 6 dB. The largest measured fold in that sweep is
0.00213 FS (−53.4 dBFS), versus 0.00796 FS for the matched original case.
Extreme driven upper notes can still alias; this
is not an alias-free oscillator or distortion model. These checks do not replace
listening on the instrument.

The oscillator tests verify every narrowed table slope and 16,777,216 half-mip
phase positions. Each voice now uses 120 bytes. Native compilation and host
checks do not measure device CPU; this exact revision must meet the 3.5%
hardware requirement before release. No catalog CPU result is inferred from
host timing or native instruction counts.

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
