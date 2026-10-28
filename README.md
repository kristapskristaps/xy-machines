# Vader

A playable 808 bass and kick synthesizer for the OP-XY, with a reactive helmet
and reactor display. Original C++ synthesis, no samples or third-party DSP code.
Requires Open-XY SDK 1.4 or later, with OS 1.2.1 for the native build.

## Sound

A phase-reset oscillator feeds two parallel branches: a clean sine foundation
and a band-limited drive bank. Both follow the played note and the same pitch
drop, so the clean sub cannot drift against the driven body. A high-passed noise
impulse adds optional click. A fixed 1 ms onset fade softens phase reset.
Boost supplies 0–6 dB of output gain; at full Boost the clean oscillator uses
almost the full half-scale Q31 range permitted by the SDK.

The four drive characters are **Round** (symmetric saturation), **Tube**
(biased saturation), **Diode** (hard asymmetric clipping), and
**Fold** (sine folding). These are sound-design characters. The waveform banks
contain no DC, have a positive fundamental, and crossfade between six
harmonic-limited versions as pitch rises. Drive reaches hard saturation and
strong sine folding, with up to 63 harmonics at bass pitches. It interpolates
between nine intensities. Tone is a logarithmic two-pole low-pass from 20 Hz
to fully open; Sub blends back the clean fundamental around that filter.

The factory default is a mono 808 with portamento, a fast attack, a decaying
zero-sustain AMP envelope and a release tail. Preset volume is 100 and the
keyboard starts two octaves down. The stock AMP and play-mode pages remain
editable. Reload Vader's **default** preset after installing this revision:
changing firmware or selecting an engine does not rewrite an existing track's
envelope, volume or play-mode settings. The oscillator itself can still sustain
when AMP is set that way. The browser has its own simple audition gate and
does not reproduce the device's preset envelope, mono mode or portamento.

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
| 2 | Tone | 0–100 | Logarithmic body brightness, 20 Hz to open; the clean sub bypasses this. |
| 2 | Boost | 0–6 dB | Smoothed output gain, with full output headroom at 6 dB. |

The eight presets are Pure Force, Original 808, Dark Lord, Death Star, Red Saber,
Short Fuse, Carbon and Seismic. They ship as native presets with mono play mode,
portamento, a decaying AMP envelope and full preset volume. Short Fuse has no
glide and a shorter decay; Seismic has a longer decay. Pure Force is a clean
sine. The default preset has a small pitch drop and no drive or noise click.
Regenerate the native preset files with `node tools/generate_presets.mjs`.

This revision keeps the first six parameter slots. Tone keeps Color's seventh
slot but changes its range to a logarithmic two-pole filter. Boost replaces
Phase in the eighth slot: an old Phase value now selects gain rather than
start phase. Reload the supplied presets to adopt the new values and play
settings; old projects retain their saved control words and AMP settings.

## Display

The helmet is traced from the supplied reference in `artwork/mask-reference.png`.
Its dome, flared sides, eye shapes and triangular grille retain that mask's
silhouette; the source's badge and checkerboard are excluded. Every part is
native SVG geometry, with no embedded bitmap or browser animation.

Each note recoils the complete helmet, flashes its eyes and sweeps a highlight
across the visor. Three broken reactor rings rotate outward, sparks scatter
around the mask and small energy packets connect it to the waveform diagrams.
Four recent note impacts can overlap, so fast retriggers keep their own rings.
The grille charges from the centre outward with Drive and note activity;
control changes ease the indicators to their new positions. The corner
targeting brackets have been removed.

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
An independent frozen V2 renderer also verifies bit-identical audio across
59 million samples of long notes, control changes, glides and retriggers,
plus two million pitch-band selections. The optimized renderer reuses identical
waveform lookups, replaces integer division with an exact corrected reciprocal,
and stops click and pitch-sweep work only after it can no longer affect audio.
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
