# Cyclops

A Minitaur-inspired bass synth for the OP-XY: two band-limited oscillators,
nonlinear four-pole ladder filtering, and a single-eyed stone character that
responds to notes and controls. Cyclops is an independent digital design,
not a component-level circuit model or a measured reproduction of the hardware.

## Play

Keep the Open-XY SDK beside this repository as `../open-xy`, on `staging`.
Use Python 3.11+, NumPy 2.3.5 and Clang with WebAssembly support. On macOS,
use Homebrew LLVM rather than Apple's Clang. See the SDK's
[first-machine guide](https://github.com/etcetc000/open-xy/blob/staging/docs/first-machine.md).

```sh
python ../open-xy/sdk.py test --machine . --cxx /opt/homebrew/opt/llvm/bin/clang++
python ../open-xy/sdk.py preview --machine . --out ../open-xy/local/cyclops-preview-001 --cxx /opt/homebrew/opt/llvm/bin/clang++
python ../open-xy/tools/serve_harness.py --preview ../open-xy/local/cyclops-preview-001 --port 8947
```

Open http://127.0.0.1:8947/ and play the keyboard or **Play phrase**.
For bass, use the keyboard's minus button to move down an octave or two.
Use a fresh output directory after editing. The preview applies its own
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

**Init** starts with two unison saws and no resonance, drive, detune or key
tracking. **Bull Bass**, **Round Sub**, **Copper Bite**, **Octave Growl**,
**Hollow Pulse**, **Resonant Wire** and **Open Twins** explore the useful range.
Presets store the eight engine controls, not the OP-XY's other pages.

## Sound and display

The original fixed-point DSP uses nine smoothly blended harmonic-limited
wavetable levels. A trapezoidal four-stage ladder solves instantaneous linear
feedback, with a tanh input in the feedback path, integrator charge retention,
mild resonance compensation and a 10 Hz output DC blocker. It runs at 44.1 kHz;
nonlinear drive can introduce aliasing at bright, high-register settings.
The table generator is reproducible with Python's standard library:
`python tools/generate_tables.py`. Audio callbacks allocate nothing and use
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

The machine tests check tuning, harmonic content, suppression of a folded
oscillator harmonic, ladder rolloff and resonance, key tracking, control
corners, rapid automation, deterministic retriggers and display behavior.
Host tests and browser playback do not establish hardware CPU cost or device
acceptance. The final host estimate is about 23,000 instructions per voice
block, above the SDK’s 15,000 target. Device profiling must determine the
actual cost before catalog submission or relying on many simultaneous voices.
CPU profiling and normal hardware playback remain pending.

Prepare a development build from committed source, keeping all firmware
outside every checkout:

```sh
python ../open-xy/sdk.py build --machine . --profiling on --stock-os "<private>/stock-1.2.1/os.ldr" --out "<private>/cyclops-dev-001"
```

Follow the SDK's [hardware profiling guide](https://github.com/etcetc000/open-xy/blob/staging/docs/profiling.md).
Device installation and diagnostics require the user's separate instruction.
A catalog submission must wait for the exact revision's hardware control-sweep
CPU record. A catalog-only PR targets `main`, unless it depends on SDK changes
only available on `staging`.

Source and generated tables are MIT licensed; see [LICENSE](LICENSE).
