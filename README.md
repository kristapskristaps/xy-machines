# X0X

A configurable OP-XY drum machine. Shift + black chooses a drum type for the
selected pad; two encoder pages shape that pad independently of the others.

The first sound implementation covers all **47 valid drum-type/family combinations**
(plus two kick/snare variations in the 707 recordings). The analog families are
original fixed-point synthesis inspired by their classic architectures, rather
than calibrated circuit emulations. The 707 voices and 909 hats/cymbals use
credited recordings. Host tests exercise the real SDK pad editor, storage codec
and sound callbacks; on-device interaction, project/preset save/reload and CPU
timing remain unverified.

## SDK

This native stepping candidate pairs with the separate local SDK draft
`335a8d064b66f31d697fa6516e6bbae6d96cafa9` on `codex/native-gui-size`,
checked out as `../xy-sdk-gui-size`. It adds `resources.parameterEdit`
to SDK 1.9's synthesis-drum support. CI can use this pin after that SDK commit
is published. The existing `../open-xy` checkout is untouched.

The draft passes public checks, and the full existing collection now links
inside the shared module limit. Windows release-host qualification and
hardware acceptance remain pending. The SDK changes keep the existing
machine sources and DSP compiler settings intact. The browser audition and
native candidate use this same machine source and SDK pin.

## Controls

| Gesture / page | Encoders 1–4 |
| --- | --- |
| Shift | Drum type / choke group / unused / unused |
| Page 1 | Style / tune / decay / tone |
| Page 2 | Attack / voice character / body character / drive where available |

Press any main encoder to switch all four between the two pages. Both rows
remain visible and the active row is emphasized. Shift exposes the same config
row from either page. Choke groups are NONE and 1–3; pads sharing a numbered
group cut each other off. Set open and closed hats to the same numbered group
when assembling a kit. Repeated hits on the same pad use the SDK's short
crossfade instead of piling up voices.

The screen shows a baby-faced Grogu drummer with Wayfarer-style sunglasses,
long ears and a wrapped robe,
in grayscale with small blue/red accents on the kit. A separate label shows
the selected drum and its 606/707/808/909 style, including single-family sounds;
sequencer hits animate the played drum without changing that selection label.
The parameter bar gives labels 50 pixels and values 70 pixels within each
encoder column, with 17px active values and 12px inactive values. All
18 drum types have their own band-style emblem printed on the kick head. All
47 type/style combinations react to note starts, with articulated shoulder and
wrist movement, alternating hands where the surface is reachable, stick
follow-through, cymbal/skin vibration, head nods, tail motion and a kick pedal.
The generator solves strike poses offline; runtime keeps the sticks attached to
the hands as the joints rebound at different rates. The 606 snaps, 707 steps,
808 rolls back smoothly and 909 recoils sharply. Movement settles within
0.2 seconds, with hit brightness following the pad's decay. Reduced motion
keeps static feedback. Entering a track or browsing a type/style establishes a
still pose instead of inventing a hit. Drawing follows the latest played pad on the track, including sequencer hits,
while the header and encoders stay on the selected pad. It uses drawing ABI 5
trigger snapshots; simultaneous hits between display frames can coalesce, and
GUI control snapshots are not a history of audio-side step locks.
Run `python artwork/drummer.py` to regenerate the vector scene, pivots and contact
poses. The same C++ animates the browser and native display; no bitmap is loaded
by the machine.

The kit has 24 independent pads, with these drum types: kick, snare, clap, rim,
low/mid/high tom, closed/open hat, cymbal, ride, cowbell, claves, maracas,
low/mid/high conga and tambourine. The default mapping is in `models.h`.

Style selects the available 606, 707, 808 and 909 families for that type. For
example, kick offers all four; ride offers 707/909, and congas offer 808.
Style uses consistent family positions across drum types. A missing family
uses the next available family, or the last available one if none is higher.
Single-family types leave the style position blank. Tune has an exact zero centre and a ±12-semitone
range. Decay shows milliseconds or seconds and controls the sound envelope. Sampled
voices also end when the recording ends; tuning changes their playback length.
Attack is the onset ramp in milliseconds, from 0 (the natural transient) to 127 ms.
Tone spans a low-pass range from 35 Hz to 18.5 kHz; 808 metal voices instead balance
their upper spectral bands. Other values are amounts from 0 to 100 without a percent sign.

Page 2 labels follow the selected drum: kick has sweep/body, snare has
snappy/body, clap has spread/noise, and hats have metal/open. Simple percussion
leaves the eighth position blank. The SDK stores each used control at full
7-bit precision: twelve eight-control types and six seven-control types fit
its existing prefix codec. Stock owns pad persistence, step-lock offsets,
publication and choke routing; there is no sidecar project format.

## Validate and preview

For the complete 24-pad browser audition, build a fresh preview and run the
machine's allowlisted audition server:

```sh
python ../xy-sdk-gui-size/sdk.py preview --machine . --out ../xy-sdk-gui-size/local/x0x-browser-001 --cxx clang++
python audition/serve.py --sdk ../xy-sdk-gui-size --preview ../xy-sdk-gui-size/local/x0x-browser-001 --port 8958
```

Open http://127.0.0.1:8958/. Click a pad or a style to start audio. Style's
buttons, arrows, wheel and range input step directly between available
families. Each pad retains its type, eight controls and choke group. Any main
encoder press switches pages; Shift exposes type/choke. The browser plays
one-shot tails and supports eight concurrent audition voices. A beat loop
helps compare tuning, envelopes and hat choke groups while playing.
The Kit menu offers 808 Classic, 606 Dry, 707 Studio, 909 Club, Night Wire,
Warehouse and Soft Machines. Loading one resets all 24 audition pads; each
pad remains independently editable. `python tools/generate_presets.py` rebuilds
the audition bank and the matching device files in `presets/`. Device presets
store all 24 pads with neutral track parameters, keeping model selection and
both pages freely editable. Their open/closed hats share choke group 1.

The browser uses the full C++ engine, including the 707 and 909 recordings,
and the actual C++ display. It is not a device/project-storage simulator.
This branch declares `parameterEdit` and maps each Style detent directly to
the next available family, in either direction, using the same saved raw-value
encoding. For example, ride steps directly between 707 and 909. Other controls
retain their unit steps. The draft SDK integration checks all 4,608 combinations
of model, stored Style value and direction through its real pad editor.

With Python 3.11+, NumPy 2.3.5 and Clang with WebAssembly linking installed:

```sh
python ../xy-sdk-gui-size/sdk.py test --machine . --cxx clang++ --host-cxx c++
python tests/sdk_integration.py --sdk ../xy-sdk-gui-size --cxx c++
python ../xy-sdk-gui-size/sdk.py preview --machine . --out ../xy-sdk-gui-size/local/x0x-preview-001 --cxx clang++
python ../xy-sdk-gui-size/tools/serve_harness.py --preview ../xy-sdk-gui-size/local/x0x-preview-001
```

On macOS, pass `/opt/homebrew/opt/llvm/bin/clang++` as `--cxx` when Apple's
Clang has no WebAssembly linker. Use fresh preview output folders.

The browser shows the shared C++ display and one model/control set. Its Model
slider corresponds to Shift + black. The browser starts its parameter set at
the track-offset defaults, rather than each pad's drum defaults; set Attack and
Drive to zero when auditioning a plain hit. It does **not** simulate the device's
independent pads or project storage. Editing a second-row control selects that
row. The additional SDK integration test covers actual per-pad editor routing,
all four page presses, eight-track isolation, config from either page, choke,
pad-copy data, and all 128 values of every used control through stock's four
preset words. These checks use the pinned SDK's existing stock-boundary doubles.

## Hardware validation

Build into a fresh folder outside every checkout using the SDK's
[build guide](https://github.com/etcetc000/open-xy/blob/sdk/drum-synthesis/docs/build.md).
A development package can test pad selection, Shift config, both control pages,
copy/paste, project save/reload, preset save/reload and the individual drum sounds.
Installation/device diagnostics require a separate user instruction. Do not
replace an existing custom collection without preserving its layout and machines.

Before publishing the finished sound engine, follow
[hardware profiling](https://github.com/etcetc000/open-xy/blob/sdk/drum-synthesis/docs/profiling.md)
and collect revision-bound CPU evidence. A host check or successful build
proves neither hardware acceptance nor measured CPU cost.

## Sound implementation

Voice settings are cached per voice and refreshed whenever its model or any
7-bit control changes. Unbent oscillators advance directly; swept tonal voices
share one increment calculation across their identical oscillators. The cache
and cached clap constants use 152 additional bytes per voice (4,864 bytes for
32 voices). They preserve
the working sound output: `tests/waveforms.cpp` checks 47 captured fingerprints,
covering every control value, unchanged blocks, live edits and model changes.
The retained display also reuses settled poses between triggers and updates
their brightness when a tail ends.

Exact reciprocal arithmetic replaces signed division in the sample loop.
The reciprocals use each signal's proven range rather than a full-word high
product; the 606 oscillator combines consecutive divisions by 23 and 6 into
one division by 138. Clap bursts use a bounded remainder and a corrected
reciprocal; PCM clap delay is cached with its controls. These preserve integer
rounding and the original output samples. `tests/arithmetic.cpp` exhaustively
checks each supported division range, every Drive sample/control pair and every
supported clap ratio.

`tools/benchmark.cpp` reports host nanoseconds per sample for all 47 sounds,
with steady controls or a changing Tone control (`changing` argument). Compile
it with `-O2`, the SDK include path, `dsp/tables.cpp` and `dsp/sample_data.cpp`.
Host measurements do not capture the Blackfin compiler's software-divider
cost. Compare native hardware callback workloads before claiming an OP-XY CPU
improvement; hardware timing of this revision remains pending.

- 606: swept tonal oscillators for kick/toms, dual-tone/noise snare and a six-oscillator
  metallic source for hats/cymbal.
- 808: sine kick with pitch fall, tonal/noise snare, burst clap, tuned toms and
  congas, rim/claves, metallic hats/cymbal, dual-oscillator cowbell and maracas.
  Hats/cymbal use six band-limited rectangular oscillators, with upper-overtone
  bands near 3.44/7.1 kHz and separate high-pass branches. The cymbal has three
  spectral branches with different envelope shapes, rather than added white
  noise. Tone balances the high bands; Bell shapes the cymbal's lower strike.
  These lightweight filters follow the spectral architecture described by
  [Werner, Abel and Smith (2014)](https://www.icmc14-smc14.net/images/proceedings/OS24-B10-TheTR-808Cymbal.pdf),
  with original code and coefficients, rather than their complete circuit model.
- 707: all 15 original recordings, including the two kick and snare variants.
  Body blends the variants on those two voices. Other controls shape pitch,
  envelope, onset, spectral balance and transient/body character.
- 909: swept sine/triangle kick, dual-triangle/noise snare, burst clap, tonal
  rim/toms and sampled closed/open hats, crash and ride.

Each voice has independent oscillator, envelope, filter, PCM cursor and noise
state. Audio uses bounded fixed-point arithmetic, static data and no allocation,
file access, logging or graphics. Ended voices retire after the tail has become
silent. The core adds no sequencer or effects page.

The two 707 kick/snare recordings blend with Body. On sampled voices, Character
shapes the spectrum or onset/pitch behavior rather than replacing the source
with another synthesized drum. Hats' Open control also scales their envelope.
None of these additional shaping controls implies it existed on the original
hardware.

The embedded sample data takes 202,625 bytes, compressed with an 8-bit
logarithmic curve and decoded/interpolated at playback. See
[recording credits and adaptations](assets/CREDITS.md) and
[source hashes](assets/provenance.json). To rebuild sample data, run
`python assets/generate.py --cache <external-cache>` with ffmpeg installed;
`python dsp/generate_tables.py` rebuilds the fixed-point math tables.
`python dsp/generate_metallic.py` rebuilds the 808 spectral filter coefficients.

The envelope lookup uses 85 knots spaced more closely at short durations,
saving 5,812 bytes compared with a uniform four-millisecond table.
`tests/envelopes.cpp` checks all 6,141 supported millisecond durations: the
coefficient-derived decay time stays within 0.35% of the requested time.

`tests/sound.cpp` checks all 47 combinations, 323 shaping controls with a substantial measured waveform change,
gain-normalized distinctions between all available family pairs, character/body
timbre changes, output bounds, voice completion, retriggering and the measured two-octave tuning
span. `tests/numerics.cpp` covers 6,016 control corners, buffer ownership and DSP
isolation. `tests/metallic.cpp` checks upper-band energy and suppression of the
808 hat/cymbal's pitched fundamentals, plus independence from the white-noise
generator. These tests establish neither circuit accuracy nor device CPU cost.

To render a four-kit demonstration and a timed catalogue of all 47 sounds,
compile `audition/render.cpp` with `dsp/tables.cpp`, `dsp/sample_data.cpp` and the
SDK include directory, then run the executable with two external WAV output
paths. The demo plays 606, 707, 808 and 909 for eight seconds each. The catalogue
prints its timestamps. Both use the actual machine DSP and per-pad defaults.

Machine source, artwork and tests live here. SDK/platform changes belong in a
separate SDK PR against `staging`. No firmware or OP-XY stock content belongs in this
repository. The source is covered by [LICENSE](LICENSE).
