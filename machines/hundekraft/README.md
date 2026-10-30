# Hundekraft for Open-XY — development collection

26 private development ports of the Hundekraft 3.0.3 workshop collection:
12 synths, 13 stereo send effects and one sample engine. Machine source lives
outside the SDK, in one standalone machine folder per port. The Open-XY SDK
checkout is unchanged.

## Start testing

The audition server is running at **http://127.0.0.1:9137/**.

- **Effects:** choose an effect, enable audio and play the phrase. Every knob
  controls the same C++ callback compiled for the XY. Keys, pluck, drums and
  local audio files are available as sources. Send and Monitor are audition
  controls. A/B compares the processed return with the dry source.
- **Synths:** open a machine's workbench from the page, enable audio and play
  its keyboard or phrase. Export/import keeps all controls in a patch file.
- **Screens:** open [the 26-screen gallery](http://127.0.0.1:9137/gallery.html),
  hold a note and change the controls. It recreates the original artwork in
  monochrome with selective red, orange and blue. Native synths use these
  displays; effect and sample displays are browser studies pending an SDK interface.
- **Recordings:** the page includes one WAV for every machine and a common dry
  reference. Granular's sample-engine audition is a recording; the SDK does
  not supply an interactive sample-engine workbench.
- **In a room:** play some input and change Capture from Off to On. Each
  change starts a new capture; the captured loop then evolves through the room.
- **Freeze / Latch:** these controls are exposed as Off/On parameters, including
  HAIL, Clouds, Snowglobe and Palme. Start with input playing before freezing.

Host auditions do not establish device acceptance. The first hardware audition
reported crackling on the effects and degraded Mixtur audio; the native word-type
correction needs a fresh hardware comparison. CPU measurements remain pending.

To restart the server after stopping it, from this folder:

```sh
/Users/tapsy/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/bin/python3 tools/serve_audition.py --folder local/audition-009 --port 9137
```

The server binds only to localhost and serves a hash-checked file allowlist.
Its synth workbenches use the SDK's authenticated preview server on ports
9138–9149. It never exposes a checkout or firmware directory.

## Included machines

| Kind | Machines |
| --- | --- |
| Synth | Wavetable, Taffy, PEW, Elg, Supersaw, FM Bell, SID, MBIRA, Oxbow, EVA, Arc, Mixtur |
| Send effect | Washer, Crush, Tape, Warble, Plate, Snowglobe, Father, Reverb, Clouds, HAIL, Stutter, Palme, In a room |
| Sample | Granular: four overlapping grains, Hann windows, spray, tuning, trim, reverse and stereo PCM |

Each machine has `machine.json`, `definition.json`, its C++ implementation,
source attribution and host tests. `collection.json` builds all 26 together.

Not included: Aether needs a tilt input service, Amp needs a live-input engine
service, and Gin's complete kit/editor workflow needs native drum-synth services.
Those services are absent from the current public XY machine API. Tape tools,
sequencers, MIDI Tape and other OP-1 system features are separate firmware work.

## What the ports change

The effects and Mixtur use SDK `opxy::i32`/`u32` for their imported 32-bit
words. Host-preprocessed `int` is a different type from Blackfin's SDK `long`,
even though both are 32 bits; mixing them violates sample-buffer aliasing.
`tests/target_words.cpp` checks pointer compatibility and is also compiled with
the pinned Blackfin compiler during validation. The importer preserves SDK word
types for future regenerations.

Synth DSP keeps its original 128-sample internal block. The adapter queues the
pitch input and emits 64 samples per XY callback, adding **64 samples / 1.45 ms**
of latency. This preserves the algorithms' original block-based coefficient,
envelope and modulation timing. Heavy work runs on alternate callbacks, so
hardware profiling must examine the peak cost as well as averages.

All mutable OP-1 DSP globals are owned by a voice or effect instance. Original
instrument-wide modulation becomes independent per voice; the ports do not
reproduce the OP-1's shared six-voice modulation behavior. Each synth has a
16-voice SDK pool shared by tracks using that machine. XY stock envelopes and
mixing follow the callback; OP-1-specific key-release gestures, screens and
preset files are not imported. The original vector scenes and their moving
parts are reconstructed through the XY display API; [VISUALS.md](VISUALS.md)
lists every scene and the fidelity limits. Five-to-eight-control synths now
show both parameter pages through the SDK 1.4 header.

Discrete wave/interval/ratio controls use ordinary enum ranges instead of the
OP-1's repeating 512-unit selector bands. Effects return wet audio; Dry belongs
to the stock XY source mixer. The source's mono effects run independent left
and right processors. Father, Crush, Warble and Plate retain their stereo DSP.
Delay memory is cleared in bounded pieces before first use or reset. A short
silent warmup after changing an effect is expected.

The imported coefficient calculations use tested, portable two-word arithmetic
where the XY compiler does not provide a 64-bit runtime helper. No firmware
addresses, assembly, platform conditionals, allocation or file I/O are in the
machine callbacks.

## Validation completed

- `sdk.py test --no-cpu` passed for **all 26 machines**, including all 12 WASM synth
  previews and 77 host test executables. Voice interleaving, audio/input guards,
  dirty send buffers, reset tails, dry routing, sample trim and stereo handling
  are covered. Wide arithmetic is compared with host 64-bit arithmetic.
- All 26 sound checks passed AddressSanitizer plus bounds, alignment, null and
  division-by-zero checks.
- All 26 display invariant tests also passed AddressSanitizer and undefined-behavior
  checks (excluding the imported DSP's signed-wrap/shift conventions). Each covers
  1,800 control/activity frames plus activation, idle and reduced motion.
- The pinned Blackfin compiler linked all 26 machines, including the 12 custom
  synth displays, and passed the SDK's module checks:
  **1,035,280 bytes / 1011.0 KiB of 2040 KiB**. All 26 drawing callbacks also
  cross-compile independently; effect/sample studies remain unregistered.
- `sdk.py check` passed its public suite (209 tests); its 11 private-fixture
  tests were explicitly skipped by that public command. This is not firmware
  release verification.
- All 13 live effect WASM modules loaded in the browser without console errors.
  Clouds produced audio through the effect workbench; Wavetable's SDK workbench
  played its phrase. Local WAV loading and playback after switching effects were also checked. All 26 recorded auditions have finite, nonzero output.
- All 26 reconstructed screens load in the gallery without rendering errors.
  Note/control motion, reduced motion and effect playback/switching were checked.
  The 127 audio implementation/header files match the pre-visual-pass hashes.

Logs, audition metrics and compiled previews are in ignored `local/`. The main
test log is `local/sdk-visual-tests-final.log`; display sanitizer results are in
`local/visual-checks.log`, and the earlier sound sanitizers are in `local/sanitizers.log`.
Host timing estimates exceed the SDK's preferred aim
for Arc, Wavetable, SID, Clouds, Reverb and Palme. They are estimates, not
measured device cycles. Start hardware profiling with a small workload.

## Native development module and firmware

The verified compile output is outside every checkout:

```text
/Users/tapsy/Library/Application Support/OPXY-SDK/private/hundekraft-visual-module-004/
```

It contains a **non-bootable module**, compile analysis, source hashes and an
identity layout. Do not install `module.bin` or `module.elf` as an OS.

An installable development OS has not been made: the available file at
`/Users/tapsy/Documents/os.ldr` does not match the stock 1.2.1 hash required by
the SDK. The required SHA-256 is
`e9dbdaa16e39446b267bee4c9e7256c491d167a1594a3a97343af35d74bc98f2`.
No expected hashes or authentication checks were changed.

Once the user's matching stock file is available, build with the sibling SDK:

```sh
python ../../../xy-machines-sdk/sdk.py build --collection collection.json --profiling on --stock-os /external/private/stock-1.2.1/os.ldr --out /external/private/hundekraft-dev-001
```

For an existing custom firmware, carry its layout forward with `--layout` and
include the machines it needs. Keep all stock inputs and firmware-containing
outputs outside every checkout. Installation, reboot and three hardware
profiling runs require a separately authorized device test, following
[Open-XY's profiling guide](https://github.com/etcetc000/open-xy/blob/staging/docs/profiling.md).
There are no catalog submissions or hardware CPU records yet.

## Rebuild auditions

Use Python 3.11+ with NumPy and LLVM with WebAssembly support. The local tool
paths are recorded in the scripts. Choose a fresh audition directory each time:

```sh
python tools/visuals.py
python tools/check_visuals.py
python tools/audition.py --out local/audition-010 --previews
python tools/live_effects.py local/audition-010
python tools/visual_previews.py local/audition-010
python tools/serve_audition.py --folder local/audition-010 --port 9137
```

The source import is reproducible with `tools/port_workshop.py` and the pinned
source archive. It deliberately works in separate machine folders; it does
not edit the SDK or install anything on a device.

## Source and licenses

Imported source: [Hundekraft 3.0.3 corresponding source](https://hundekraft.github.io/op1-workshop/source/hundekraft-3.0.3-source.tar.gz),
SHA-256 `2f745e3858f9fabf8e653ecfe3511679b47f795ff09f13e9bd5590e26995b1d7`.
The archive checksum was verified before importing code.

Retained GPL-3.0-or-later notices and component licenses are in each machine's
`LICENSE`, `NOTICE` and `UPSTREAM.md`. `SOURCE-PROVENANCE.md` retains the
upstream component/permission record, including the Oxbow and Maiko ports.
The additional proprietary-firmware permission names the OP-1. Clarification
for OP-XY distribution remains necessary; these are private development ports.
No code, packages, catalog entries or firmware were published or sent to anyone.
