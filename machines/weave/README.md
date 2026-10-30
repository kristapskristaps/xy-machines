# Weave

Fifteen Plaits-derived synthesis models for OP-XY OS 1.2.1, with a miniature animated
industrial room. Built with the [Open-XY SDK](https://github.com/etcetc000/open-xy).

The model bank contains waveshaping, two-operator FM, filtered noise, particles,
kick, snare, hi-hat, grain formants, chords, swarm, wavetable, virtual analog,
modal, string and six-operator FM. The six-operator model includes Plaits' 96
factory FM patches across its three banks. This is a selected Plaits port, not
the complete 24-model module.

## Controls

Model is encoder 1 on page 1; Output is encoder 1 on page 2.
Press any encoder to switch all four controls between pages. Shift + turn
keeps the SDK's normal fine adjustment. Both rows stay visible above the room.

| Encoder | Page 1 | Page 2 |
| --- | --- | --- |
| 1 | Model | Output: main through a crossfade to auxiliary |
| 2 | Harmonic | Strike: stock, soft, mid, hard |
| 3 | Timbre | Mode: normal, hold |
| 4 | Morph | Seed: free, 1–127 |

Strike changes the excitation of Kick, Snare, Hi-hat, Modal and String. Drum
settings scale the initial shell/noise excitation; Modal changes the click and
brightness/damping, and String changes its pluck noise and brightness/damping.
Stock preserves the previous fixed excitation exactly. Soft, mid and hard are
three additional strength presets, not a second volume control.

Hold keeps Swarm's grain/glissando density evolving instead of slowing after
its initial burst. In 6-op it keeps the active FM patch's internal gate open
while the XY voice runs; the stock amp envelope still shapes the final sound.
Normal retains the previous behaviour. Hold can keep DSP active for longer,
but adds no voices, oscillators or sample-rate stages.

Seed 0 (`free`) retains the owner-specific evolving random stream. Values
1–127 restart a fixed random sequence at each note or model retrigger, useful
for Noise, Dust, drum noise, Swarm, String and random 6-op LFOs. Turning Seed
takes effect on the next trigger. It does not clear oscillator/filter/resonator
history, so a ringing retrigger is not promised to repeat an entire waveform.
Its effect depends on the output and, for FM, the patch's LFO type. Controls
that do not apply to the selected model show `--`.

The model name stays engraved on the cabinet, with a page-1 Model hint below
the room. The selector changes GUI routing only; all eight saved parameter
slots and every DSP file are unchanged by this interaction update.

The three added slots default to zero and zero preserves old sounds. Existing
parameter order, mappings and presets are retained. Strike/Mode changes run at
block or excitation rate; Seed only changes random state at a trigger. The
string's existing noise shift selects its excitation strength without another
per-sample processing stage. No new audio state or voice capacity is required.

Harmonic, Timbre and Morph follow each model's original meanings. The underlying
integer port accepts 128 steps for these controls and interpolates within its
render blocks. Changing model retriggers it. The XY's stock amp envelope,
filter and mixing remain downstream. Percussive models supply their own decay;
use a sufficiently long stock amp envelope to hear their full tails. Controls use the upstream SDK 1.6 paged interface.

The display is a cutaway machine room. A belt-driven flywheel powers a cabinet,
with a hanging light, pressure gauge, conveyor and a small maintenance robot.
Each model swaps the physical cartridge inside the cabinet: meshing operators
for FM, a grain hopper for noise, drums and cymbals for percussion, organ pipes
for chords, and bars or strings for the resonant models.

Harmonic changes the mechanism's size and gear ratios. Timbre changes drive
speed and the conveyor load; Morph changes pressure and the press stroke;
Output turns the routing valve. Notes drive the motor, press, fan, parcels,
steam and the robot's wrench. Motion follows controls and per-track note gates.
Release settles in about 0.18 seconds, idle poses remain still, and reduced
motion keeps the room static while preserving the control positions and lamp.

The retained artwork uses 65 actors (including eight header steps and fifteen model labels), two 32-point outlines, sixteen native text slots
and nine palette colors. Most room detail is static. Mutable geometry is cached
at about 20 Hz while the motor runs, with immediate note-start updates; moving
parts follow display frames. Regenerate the fragment and `art.h` with
`python tools/generate_art.py`. Update the firmware and
`content/display/weave.svg` together when installing this display revision.

## Port fidelity

The DSP derives from Émilie Gillet's MIT-licensed Plaits, via the integer port
by gdeo607 and DMDaudio. [vendor/VENDOR.md](vendor/VENDOR.md) records the source
revisions and changes. The engine runs internally at 48 kHz, followed by a
24-tap, 147-phase fixed-point converter to the SDK's 44.1 kHz. Original sample
counts and time constants therefore keep their intended duration.

There is no module-panel CV calibration, user-data/SysEx import, or Plaits LPG.
The stock XY envelope supplies the voice gate. The native build is limited to two simultaneous Weave voices across all tracks,
with oldest-note stealing. The browser audition can schedule eight voices. All
fifteen models remain available. Dust uses four particle resonators, Swarm four
oscillators, Modal twelve modes, and String one internal string per external
note. Chords keeps its five-note voicing and 6-op keeps all six operators and
96 patches. The reduced banks trade density and upper partials for headroom;
String relies on the external voices for overlapping notes. All source DSP
still runs at 48 kHz, preserving pitch and time constants. The limiter and FM
soft clip use an exact reciprocal division with a checked remainder; wavetable
cubic interpolation drops only sub-LSB rounding work. Normal playback and
worst-case model CPU still require device validation.
The 24-tap converter uses mirrored histories to remove ring-wrap work while
retaining bit-identical audio.

## Develop and audition

Keep the SDK beside this repository as `../../../xy-machines-sdk`, using Python 3.11+ and a
Clang with WebAssembly support:

```sh
python ../../../xy-machines-sdk/sdk.py test --machine . --cxx clang++ --firmware
python ../../../xy-machines-sdk/sdk.py preview --machine . --cxx clang++ --out ../previews/weave-001
python ../../../xy-machines-sdk/tools/serve_harness.py --preview ../previews/weave-001
```

Tests cover all models and both outputs, deterministic voice ownership, moving
controls, fractional arithmetic, 41 original/corrected reference renders plus 19 bounded-quality fixtures, exhaustive divisor checks, cubic interpolation error bounds, sample-rate
conversion, and regressions for wavetable bank bounds and the FM sine LFO. Additional checks cover excitation on all five supported models, free/fixed random streams, held Swarm/FM, live mode changes, all eight moving controls, and the two-page header.

[Hardware profiling](https://github.com/etcetc000/open-xy/blob/staging/docs/profiling.md)
remains required before catalog submission. Host CPU estimates are not measured
device cost. No firmware is included; keep device builds outside every checkout.

MIT, with the third-party notices in [vendor/LICENSE](vendor/LICENSE).
