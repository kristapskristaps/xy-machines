# X0X

A configurable OP-XY drum machine. Shift + black chooses a drum type for the
selected pad; two encoder pages shape that pad independently of the others.

The first sound implementation covers all **47 valid drum-type/family combinations**
(plus two kick/snare variations in the 707 recordings). The analog families are
original fixed-point synthesis inspired by their classic architectures, rather
than calibrated circuit emulations. The 707 voices and 909 hats/cymbals use
credited recordings. Host tests exercise the real SDK pad editor, storage codec
and sound callbacks. The imported standalone revision was exercised on the XY
and measured with synthetic DSP workloads. That evidence does not qualify a
new collection/SDK build or measure whole-project CPU.

## SDK

Use the collection root's `SDK_REPOSITORY` and `SDK_REVISION`, configured by
`./xy setup`. The release targets the unmodified public SDK, pinned to official
`main` commit `71566d29f0087c793f392e9c09768ae3c866a379` (SDK 1.9). It does not
require PR #422, custom `parameterEdit`, played-pad drawing fields or bundled
sample-free pad records. Drum DSP, 24 independent pads and eight voices remain.

The public builder includes one init preset with neutral track offsets. It
uses `drum_defaults` to supply the 808 Classic pad mapping. The seven authored
24-pad kits are preserved separately at [preset-packs/x0x](../../preset-packs/x0x/README.md),
not presented as builder-bundled presets. Public-SDK hardware save/reload and
CPU qualification remain pending; existing device presets are not deleted.

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
track activity animates the selected drum without changing that selection label.
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
still pose instead of inventing a hit. The public display snapshot identifies the selected pad, not the pad that a
sequencer hit played. Its artwork reacts to track note activity using that
selection; it cannot identify a different playing drum. Selected labels and
controls remain stable. This is an intentional public-API limitation.
Run `python artwork/drummer.py` to regenerate the vector scene, pivots and contact
poses. The same C++ animates the browser and native display; no bitmap is loaded
by the machine.

The kit has 24 independent pads, with these drum types: kick, snare, clap, rim,
low/mid/high tom, closed/open hat, cymbal, ride, cowbell, claves, maracas,
low/mid/high conga and tambourine. The default mapping is in `models.h`.

Style selects the available 606, 707, 808 and 909 families for that type. For
example, kick offers all four; ride offers 707/909, and congas offer 808.
Style uses ordinary one-unit encoder edits and consistent family positions across drum types. Each family occupies a 32-value zone, so crossing to another family can take several detents; saved raw values are unchanged. A missing family
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

Run these commands from the collection root. `./xy test x0x --native` checks
the machine and `./xy preview x0x --watch` opens the SDK workbench. For the
complete 24-pad browser audition, use the clean SDK configured by setup (shown
here at its default sibling path):

```sh
python ../xy-machines-sdk/sdk.py preview --machine machines/x0x --out local/x0x-browser-001 --cxx clang++
python machines/x0x/audition/serve.py --sdk ../xy-machines-sdk --preview local/x0x-browser-001 --port 8958
```

Open http://127.0.0.1:8958/. Click a pad or a style to start audio. Style's
buttons, arrows, wheel and range input step directly between available
families. Each pad retains its type, eight controls and choke group. Any main
encoder press switches pages; Shift exposes type/choke. The browser plays
one-shot tails and supports eight concurrent audition voices. A beat loop
helps compare tuning, envelopes and hat choke groups while playing.
The Kit menu offers 808 Classic, 606 Dry, 707 Studio, 909 Club, Night Wire,
Warehouse and Soft Machines. Loading one resets all 24 audition pads; each
pad remains independently editable. `python machines/x0x/tools/generate_presets.py --sdk ../xy-machines-sdk` rebuilds
the audition bank, the builder init in `presets/`, and separate files under `preset-packs/x0x/`. The separate device kits
store all 24 pads with neutral track parameters, keeping model selection and
both pages freely editable. Their open/closed hats share choke group 1.

The browser uses the full C++ engine, including the 707 and 909 recordings,
and the actual C++ display. It is not a device/project-storage simulator.
The audition's family buttons can jump between available families. Device
Style edits use the public SDK's ordinary one-unit stepping rather than that
browser shortcut. Host integration checks verify raw-value boundaries and
preservation of the established saved-family encoding.

The integration checks also exercise the real SDK pad editor and native audio adapter:

```sh
python machines/x0x/tests/sdk_integration.py --sdk ../xy-machines-sdk --cxx c++
python machines/x0x/tests/native_audio.py --sdk ../xy-machines-sdk --cxx c++
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
[build guide](https://github.com/etcetc000/open-xy/blob/main/docs/build.md).
A development package can test pad selection, Shift config, both control pages,
copy/paste, project save/reload, preset save/reload and the individual drum sounds.
Installation/device diagnostics require a separate user instruction. Do not
replace an existing custom collection without preserving its layout and machines.

Before publishing the finished sound engine, follow
[hardware profiling](https://github.com/etcetc000/open-xy/blob/main/docs/profiling.md)
and collect revision-bound CPU evidence. A host check or successful build
proves neither hardware acceptance nor measured CPU cost.

## Sound implementation

Voice settings are cached per voice and refreshed whenever its model or any
7-bit control changes. Unbent oscillators advance directly; swept tonal voices
share one increment calculation across their identical oscillators. The cache
and cached clap constants use 152 additional bytes per voice (1,216 bytes for
eight voices). The native kit supports eight simultaneous voices; additional
hits use the SDK's voice stealing, while all 24 pads and 47 type/style sounds
remain available. The original 47 captured waveform fingerprints still
authenticate frozen legacy helpers. Current renders are compared quantitatively for level and normalized
edge energy across every control value, live edit and model change.
The retained display also reuses settled poses between triggers and updates
their brightness when a tail ends.

Exact reciprocal arithmetic replaces signed division in the sample loop.
The three-partial square sources use 1025-point interpolated tables instead of
three sine interpolations for every oscillator. Their exhaustive phase-grid
comparison differs by less than 0.1% RMS. Smaller /3 and /6 signals keep their
single-word reciprocals. Clap bursts use a bounded remainder and a corrected
reciprocal; PCM clap delay is cached with its controls. These divisions retain
their original integer rounding. `tests/arithmetic.cpp` exhaustively
checks each supported division range, every Drive sample/control pair and every
supported clap ratio.

Proven bounded products preserve both operands as signed16 through the native
compiler. This covers oscillator/stem interpolation slopes, source envelope and
body gains, bounded noise differences, attack/amplitude, the limited DC feedback
state and all three analytic Drive products. Negative products retain the
original truncation toward zero; this adds no precision or sound change beyond
the synthesized stems. Full-scale low-pass/PCM differences and gains above 32767
keep their original wider arithmetic. Exhaustive Drive tests and 423 paired
PCM/full-state scenes authenticate equivalence to the stem-only revision.
The complete PCM callback is compiled separately once per block to keep the
native generated kernels bounded; outlining adds one call per PCM block.

PCM and synthesized sources select separate sample loops once per callback.
The 808 hats/cymbal read three periodic synthesized stems: bell, bright and air.
They were rendered from the preceding six rectangular oscillators and five
spectral filters, not sampled from a drum machine. Three Character anchors
retain different oscillator ratios; intermediate values crossfade adjacent
anchors. Each half-second loop rounds oscillator frequencies to the nearest
2 Hz and includes a duplicate interpolation endpoint. Envelopes, attack, decay,
tone/body balances and drive stay live. Tune resamples the stems, so filter
centres now transpose with oscillator pitch. The finite loops and Character
crossfade change beating and phase compared with the former live generator.

The nine i16 stems occupy 396918 bytes, plus 4100 bytes for the square shapes.
Voice capacity and the 332-byte voice state are unchanged. This is a deliberate
sound/performance tradeoff; a fresh device sweep must meet 3.5% before acceptance.

`tools/benchmark.cpp` reports host nanoseconds per sample for all 47 sounds,
with steady controls or a changing Tone control (`changing` argument). Compile
it with `-O2`, the SDK include path, `dsp/tables.cpp`, `dsp/sample_data.cpp` and
`dsp/metal_stems.cpp`.
For a paired comparison against the preceding collection revision, run from
the repository root:

```sh
python machines/x0x/tools/benchmark.py --cxx /path/to/clang++ --blocks 4000
```

It reports whether output/state are identical and compares 423 host scenarios:
all 47 sounds, default/minimum/maximum controls, retriggers, Tone automation and
all-control automation. Intentional stem/table changes mean equivalence is not
required; sound checks live in the machine tests. Timed voices restart after
retirement, so silence callbacks do not dominate short sounds. Host timings
cannot qualify this revision's hardware CPU or catalog acceptance.

The original 47 waveform and 18 metallic-state hashes have not been replaced.
They authenticate frozen reference helpers; current audio must retain RMS
level and normalized edge energy within a factor of two across those scenes.
The 808 default spectral centroid must stay within 25% and its upper-band
proportion within 43% of the live reference, alongside the original high-band
and fundamental-suppression checks. Broader timbre and device listening still
need evaluation at the revision being measured.

- 606: swept tonal oscillators for kick/toms, dual-tone/noise snare and a six-oscillator
  metallic source for hats/cymbal.
- 808: sine kick with pitch fall, tonal/noise snare, burst clap, tuned toms and
  congas, rim/claves, metallic hats/cymbal, dual-oscillator cowbell and maracas.
  Hats/cymbal use pre-rendered six-oscillator spectral branches, with the
  cymbal's bell/bright/air envelopes mixed independently rather than adding
  white noise. Tone balances the high bands; Bell shapes the lower strike.
  The source generator follows the architecture described by
  [Werner, Abel and Smith (2014)](https://www.icmc14-smc14.net/images/proceedings/OS24-B10-TheTR-808Cymbal.pdf),
  with original code and coefficients.
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
Compile `tools/generate_stems.cpp` with the SDK include path and `dsp/tables.cpp`
to an external executable, then redirect its output to `dsp/metal_stems.cpp` to
regenerate the synthesized stems and square tables.

The envelope lookup uses 85 knots spaced more closely at short durations,
saving 5,812 bytes compared with a uniform four-millisecond table.
`tests/envelopes.cpp` checks all 6,141 supported millisecond durations: the
coefficient-derived decay time stays within 0.35% of the requested time.

Feed-forward Q15 products round to nearest, with half-count ties toward positive.
Recursive low-pass updates and DC feedback retain truncation toward zero so
quiet tails terminate. `tools/rounded_quality.py` compares 423 renders against
the committed truncating renderer and checks complete releases for all 47 voices.
Across those renders, maximum waveform RMS difference is 1.96% above -80dBFS
and maximum level change is 0.105dB; default steady voices stay below 0.73%
RMS difference and 0.023dB. These are sound comparisons, not hardware timings.

`tests/sound.cpp` checks all 47 combinations, 323 shaping controls with a substantial measured waveform change,
gain-normalized distinctions between all available family pairs, character/body
timbre changes, output bounds, voice completion, retriggering and the measured two-octave tuning
span. `tests/numerics.cpp` covers 6,016 control corners, buffer ownership and DSP
isolation. `tests/metallic.cpp` checks upper-band energy and suppression of the
808 hat/cymbal's pitched fundamentals, plus independence from the white-noise
generator. These tests establish neither circuit accuracy nor device CPU cost.

To render a four-kit demonstration and a timed catalogue of all 47 sounds,
compile `audition/render.cpp` with `dsp/tables.cpp`, `dsp/sample_data.cpp`,
`dsp/metal_stems.cpp` and the
SDK include directory, then run the executable with two external WAV output
paths. The demo plays 606, 707, 808 and 909 for eight seconds each. The catalogue
prints its timestamps. Both use the actual machine DSP and per-pad defaults.

Machine source, artwork and tests live here. SDK/platform changes belong in a
separate SDK PR against `staging`. No firmware or OP-XY stock content belongs in this
repository. The source is covered by [LICENSE](LICENSE).
