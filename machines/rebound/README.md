# Rebound

A tempo-synced stereo send delay for OP-XY OS 1.2.1. Feedback reaches **150%**:
turn past unity to grow repeats into soft saturation, move Size or Fine to bend
the pitch, then pull Feedback back to let them decay. Inspired by the playable
feedback of Elektron delays; this is an original implementation, not a Digitakt
DSP emulation.

## Controls

Each encoder has an alternate selected by pressing that encoder. Effects retain
stock's independent encoder selection, rather than switching the whole page.

| Encoder | Main control | Press for |
| --- | --- | --- |
| 1 | **Size**: tempo-synced 1/32, dotted 1/32, 1/16, dotted 1/16, 1/8, dotted 1/8, 1/4, 1/2 | **Fine**: -50 to +50 ms in 1 ms steps |
| 2 | **Feedback**: actual loop gain, 0–150%; 0.5% steps from 80–120%, 1% elsewhere | **Highpass**: off, then 30 Hz–4 kHz |
| 3 | **PingPong**: independent stereo at 0, full left/right bounce at 100, blended routing between | **Lowpass**: 500 Hz–18 kHz, then off |
| 4 | **Level**: wet return level, independent of loop gain | **Dry**: source level retained by the stock mixer at full send |

Size follows project tempo, including when transport is stopped; unavailable
tempo falls back to 120 BPM. Delay time is limited to about **12 ms–1.486 s per
side**. Longer note lengths at slow tempos stop at that limit. Size/Fine changes
glide like tape, bending pitch; they do not crossfade between read heads.
Feedback, routing, level and filter changes are smoothed too.

Feedback values and filter frequencies appear as their actual values on the
stock send page. The first four controls are stock's four effect LFO destinations;
alternate controls remain manually editable and saved. Defaults are an eighth-note
ping-pong, 45% feedback, 65% return level, filters off and Dry fully retained.

## Playing the feedback

Start with Feedback around 45–75. Near 100, quiet repeats can hang for a long
time. Above 100, a sound already in the line grows into oscillation. The filters
and interpolation lose some energy, so the onset depends on frequency, delay
time and filter settings; 100 does not promise a perfectly lossless freeze.
Highpass and Lowpass shape each trip around the loop, including in ping-pong.
Even with Highpass off, a 10 Hz DC blocker prevents static offset accumulating.

A unity-slope region keeps quiet repeats clean. Louder signals enter a smooth
quadratic saturation knee inside the loop. The input is attenuated by 6 dB for
internal headroom, and that gain is restored at the return: with Level at 100,
a quiet first repeat has the same amplitude as its input. The wet return is
bounded below full scale. Level
changes the wet return without killing the circulating sound; Dry belongs to the
stock source mixer. The total dry-plus-wet mix still depends on the other tracks
and mixer settings. There is no hard cutoff at unity and no automatic feedback
reduction. Reduce Feedback to dissipate a buildup, or set it to zero to stop
recirculation after the existing line plays out.

The initial build omitted this headroom recovery, making every repeat 6 dB too
quiet. This correction leaves the loop's gain, decay and saturation unchanged;
existing projects keep their controls but have a louder wet return. Halve Level
to match the previous build's return loudness, including above-unity oscillation.

At full PingPong, stereo input is averaged into the first left repeat; subsequent
repeats cross right, left, right. At zero, both channels retain their own input
and feedback. Intermediate values blend the injection and cross-feedback.

For a dub echo, try dotted 1/8, 72% Feedback, Highpass around 100 Hz and
Lowpass around 3 kHz. For a hovering loop, try 1/16 with 99.5% Feedback and
Level around 40. Send effects have no native preset files; projects save their
parameter settings.

## Check and audition

From the collection root:

```sh
./xy test rebound --native --cpu
python machines/rebound/tools/generate_tables.py --check
```

The machine-local sound tests cover timing, stereo and intermediate routing,
actual gain around unity, sustained growth and recovery, filter response, DC,
independent return level, stale-buffer/reset behavior and extreme input/control
sweeps. Run `tests/sound.cpp` with AddressSanitizer and UndefinedBehaviorSanitizer
for additional host arithmetic and memory checks.

Effects have no SDK browser preview. A host-only renderer runs the actual
callback and adds a quiet dry test phrase for listening. Use your pinned SDK's
include path and an ignored output directory:

```sh
mkdir -p local/rebound
c++ -std=c++11 -O2 -I../xy-machines-sdk/sdk/include \
  machines/rebound/tools/audition.cpp -o local/rebound/audition
local/rebound/audition bounce local/rebound/bounce.wav
local/rebound/audition gentle local/rebound/gentle.wav
local/rebound/audition ride local/rebound/feedback-ride.wav
```

`bounce` plays a short phrase into the default-style delay; `gentle` uses 25%
feedback to hear ordinary short echoes. `ride` starts at
90% feedback, rises to 130%, bends the time while oscillating, then drops to 45%
at ten seconds. These recordings exercise DSP on the host; they do not emulate
the stock send mixer or measure hardware CPU.

`tables.h` is generated from the feedback labels and filter frequencies in
`definition.json`; rerun `tools/generate_tables.py` after changing them. The
effect uses the full 512 KiB per-send scratch allowance and 52 bytes of state.
It never clears that scratch on the audio thread or reads samples unwritten
since its last initialization. Missing/short buffers return silence.

Follow [development](../../docs/development.md) for an explicit kit and the last
installed layout before building firmware. Firmware builds stay outside every
checkout; device installation and profiling need separate authorization. Hardware
sound, performance and revision-bound CPU validation remain pending. No catalog
publication or device acceptance is implied by host/native compilation.

MIT licensed; see [LICENSE](LICENSE). No vendor DSP or stock firmware is included.
