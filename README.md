# Vader

A playable 808 bass and kick synthesizer for the OP-XY, with a reactive helmet
and reactor display. Original C++ synthesis, no samples or third-party DSP code.
Requires Open-XY SDK 1.4 or later, with OS 1.2.1 for the native build.

## Sound

A phase-reset oscillator feeds two parallel branches: a clean sine foundation
and a band-limited drive bank. Both follow the played note and the same pitch
drop, so the clean sub cannot drift against the driven body. A high-passed noise
impulse adds optional click. A fixed 1 ms onset fade avoids a hard discontinuity
when choosing a nonzero start phase. Output stays within half-scale Q31.

The four drive characters are **Round** (symmetric saturation), **Tube**
(biased saturation), **Diode** (rounded clipping with an even harmonic), and
**Fold** (sine folding). These are sound-design characters. The waveform banks
contain no DC, have a positive fundamental, and crossfade between five
harmonic-limited versions as pitch rises. Drive interpolates between nine
intensities; Color darkens the body while Sub blends back the clean fundamental.

The engine sustains. Set the OP-XY's **AMP** page for a short decay and zero
sustain to make kicks, or hold notes for 808 bass. Use the OP-XY's stock glide
and note/velocity controls for slides and expression. The browser has its own
simple audition gate; it does not reproduce the device's AMP or glide behavior.

## Controls

Press any encoder to switch between the two fixed header rows.

| Page | Control | Range | Behavior |
| --- | --- | --- | --- |
| 1 | Punch | 0–48 st | Initial pitch above the played key; 0 gives a tuned sub. |
| 1 | Fall | 5–160 ms | Exponential pitch-drop time constant; the pitch approaches the key over several time constants. |
| 1 | Drive | 0–100 | Harmonic intensity; 0 is clean in every Shape. |
| 1 | Shape | Round, Tube, Diode, Fold | Drive character. |
| 2 | Click | 0–100 | Amount of a fixed 2 ms noise impulse, captured at note start. |
| 2 | Sub | 0–100 | Clean parallel foundation; 100 removes the driven body. |
| 2 | Color | 0–100 | Body brightness, darkest to brightest; the clean sub bypasses this. |
| 2 | Phase | 0–315°, in 45° steps | Oscillator start phase for alignment with other drums; captured at note start. |

The eight presets are Pure Force, Original 808, Dark Lord, Death Star, Red Saber,
Short Fuse, Carbon and Seismic. They store engine controls; use AMP to choose
the note length. The default Pure Force is clean, with Punch, Drive and Click off.

## Display

The helmet is traced from the supplied reference in `artwork/mask-reference.png`.
Its dome, flared sides, eye shapes and triangular grille retain that mask's
silhouette; the source's badge and checkerboard are excluded. Every part is
native SVG geometry, with no embedded bitmap or browser animation.

Each note recoils the complete helmet, flashes its eyes and sweeps a highlight
across the visor. Three broken reactor rings rotate outward, sparks scatter
around the mask and small energy packets connect it to the waveform diagrams.
Four recent note impacts can overlap, so fast retriggers keep their own rings.
The grille charges from the centre outward with Drive and note activity; turns
move the targeting brackets and ease the indicators to their new positions.

Motion settles within 0.2 seconds; idle and held resting poses are stationary.
Release closes the visor, reduced motion snaps to static note feedback, and
switching tracks establishes a fresh note baseline. The diagrams explain the
selected character; they are not audio meters or a live oscilloscope.
Regenerate the mask and panel with `python tools/generate_panel.py` (Pillow and
NumPy), which preserves the SDK header in `artwork/header.svg.fragment`.

## Design references

These references informed the feature choices. Research used their published
product descriptions; it did not include a comparative listening test.

- [SubLab XL](https://futureaudioworkshop.com/sublab-xl/): a useful reference for
  combining a clear bass foundation with harmonically rich layers and drive.
  Vader uses one shared phase to keep its two branches aligned.
- [Sonic Academy KICK 3](https://www.sonicacademy.com/products/kick-3): explicit
  pitch shaping, selective keytracking and phase alignment make it a strong
  kick-design reference. Vader condenses pitch shaping into Punch and Fall.
- [Initial Audio 808 Studio 2](https://initialaudio.com/): a dedicated playable
  808 instrument with synthesis, sub and distortion layers. Vader keeps those
  immediate sound-design roles within the XY's eight controls.
- [Roland TR-808 Software Rhythm Composer](https://www.roland.com/us/products/rc_tr-808/):
  the reference for original 808 character and circuit-modeling ambition.
  Vader focuses on a modern chromatic bass/kick instrument.

## Develop and audition

Keep [Open-XY](https://github.com/etcetc000/open-xy) on `staging` beside this
repository as `../open-xy`. Use Python 3.11+, NumPy 2.3.5, Node 22.12+ and
Clang with WebAssembly support. On macOS with Homebrew LLVM, add
`--cxx /opt/homebrew/opt/llvm/bin/clang++` to the preview and test commands.
Choose a fresh output folder for each preview.

```sh
python ../open-xy/sdk.py test --machine .
python ../open-xy/sdk.py preview --machine . --out ../open-xy/local/vader-preview-001
python ../open-xy/tools/serve_harness.py --preview ../open-xy/local/vader-preview-001
```

Open the printed localhost URL, enable audio, select a low octave and play.
Tests cover clean-sub purity, fundamental retention, DC, high-register alias
energy, pitch-drop settling, transient decay, deterministic retrigger, all 256
control corners and the display's note, idle, track and reduced-motion behavior.
Regenerate the checked-in analytic tables with `python tools/generate_tables.py`.
The SDK adapters own voice allocation and registration; callbacks allocate no
memory and do not perform I/O. Machine code, artwork and tests stay here.

## Hardware validation

Native compilation and host tests do not establish hardware acceptance or CPU
cost. A profiling-enabled development image can be built from committed source:

```sh
python ../open-xy/sdk.py build --machine . --profiling on \
  --stock-os "<private>/stock-1.2.1/os.ldr" --out "<private>/vader-dev-001"
```

The development build checks a fresh source rebuild locally. The separate
`sdk.py verify --package ... --stock-os ...` release verification command
requires the SDK's Windows x86_64 release host.

Keep all firmware and packages outside every checkout. Follow the SDK's
[build guide](https://github.com/etcetc000/open-xy/blob/staging/docs/build.md)
and [hardware profiling workflow](https://github.com/etcetc000/open-xy/blob/staging/docs/profiling.md).
Device installation, diagnostics and three revision-bound CPU runs require a
separate user instruction. Hardware validation and catalog publication remain
pending. A catalog submission needs the measured CPU record for the pinned
commit and a catalog-only PR against `staging`.

MIT licensed; see [LICENSE](LICENSE).
