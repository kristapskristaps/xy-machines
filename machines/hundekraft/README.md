# Hundekraft for Open-XY — development collection

25 private development ports of the Hundekraft 3.0.3 workshop collection:
12 synths and 13 stereo send effects. Machine source lives
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
- **Screens:** open [the 25-screen gallery](http://127.0.0.1:9137/gallery.html),
  hold a note and change the controls. It recreates the original artwork in
  monochrome with selective red, orange and blue. Native synths use these
  displays; effect displays are browser studies pending an SDK interface.
- **Recordings:** the page includes one WAV for every machine and a common dry
  reference.
- **In a room:** play some input and change Capture from Off to On. Each
  change starts a new capture; the captured loop then evolves through the room.
- **Freeze / Latch:** these controls are exposed as Off/On parameters, including
  HAIL, Clouds, Snowglobe and Palme. Start with input playing before freezing.

Host auditions do not establish device acceptance. The first hardware audition
reported crackling on the effects and degraded Mixtur audio. After the native
word-type correction, the user confirmed Mixtur improved; the FX remained bad.
The subsequent collection-wide level correction also failed hardware acceptance:
the user reported the HK effects still sounded bad while core Ring Mod and Ping
Pong sounded correct on the same setup. FX debugging is stopped at the user's
request. The 13 effect sources and their histories are retained, but
`collections/hundekraft-test.json` now selects only the 12 synth ports. The full
source inventory remains available in `collections/hundekraft.json`.

The experimental FX source includes working-level handling and typed history
rescaling, with passing host/native/sanitizer checks. Those checks do not
establish usable hardware audio. Do not include these FX in normal device kits
unless explicitly requested for renewed investigation.

The corrected word-type build (`c3690f4`) completed one hardware CPU capture:
Crush 0.954%, Father 11.374%, Clouds 13.481%, Reverb 17.887% and Palme 28.396%
of the audio-block budget at defaults. In a room's default workload was inactive,
so it has no valid CPU number. These default-setting measurements do not establish
maximum load or audio acceptance, and predate the collection-wide level correction.

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

Each machine has `machine.json`, `definition.json`, its C++ implementation,
source attribution and host tests. `collection.json` builds all 25 together.

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

## Validation

Each remaining machine includes host sound, arithmetic and display checks. Run
`./xy test` from the repository root for the current collection. Earlier local
logs and compiled outputs describe historical snapshots, not this inventory.
Host tests and browser auditions do not establish hardware acceptance or CPU.

## Native development module and firmware

The verified compile output is outside every checkout:

```text
/Users/tapsy/Library/Application Support/OPXY-SDK/private/hundekraft-visual-module-004/
```

It contains a **non-bootable module**, compile analysis, source hashes and an
identity layout. Do not install `module.bin` or `module.elf` as an OS.

Development firmware is built from the official pinned SDK using the root
`./xy build` and `./xy stage` workflow in [the development guide](../../docs/development.md).
The default HK test kit contains the 12 synths; its 13 unaccepted FX are excluded. Carry the last verified device
layout into each build and keep firmware, packages and captures in external
private storage. Installation and profiling require an authorized device test.

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
