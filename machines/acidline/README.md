# Acidline

A subtractive acid voice for the Open-XY SDK. OP-XY supplies the sequencer,
voice mode, portamento, velocity, amplitude envelope, mixer and send effects.
The machine supplies its oscillator, resonant filter and distortion.

## On OP-XY

1. Select Acidline on an instrument track.
2. On M2, hold Shift and choose stock **mono** or **legato** play mode.
3. Adjust stock **portamento** there; preset settings also offer portamento style.
4. Use stock note velocity and velocity sensitivity for loudness.
5. For stronger filter accents, parameter-lock **Cutoff**, **Env Amt**, **Reso**
   or **Distort** on selected steps. Velocity modulation routing is available in
   stock preset settings, but its interaction with custom parameters still needs
   a hardware check; parameter locks do not require a new engine callback.

The public SDK passes stock per-sample pitch increments straight into the voice.
Acidline follows them exactly, including pitch ramps, and does not apply a second
glide. Stock velocity, envelope, filter and mixing follow its output. There is
no internal accent detector, accent gain stage or amplitude gate. XY's own
amplitude controls determine note loudness. The filter envelope remains part of
the acid sound and is adjustable per step.

TE's [instrument guide](https://teenage.engineering/guides/op-xy/instrument)
describes stock mono/legato, portamento and velocity settings. The SDK's
[firmware contract](https://github.com/etcetc000/open-xy/blob/staging/docs/abi-1.2.0.md)
describes pitch delivery and the stock processing after the oscillator callback.
Actual device behavior has not yet been exercised with this machine.

## Controls

| Main encoder | Sound | Alternate |
| --- | --- | --- |
| Cutoff | Base filter frequency, about 45 Hz–12 kHz | Decay: filter-envelope decay, about 55 ms–1.32 s |
| Reso | Filter feedback / squelch | Wave: saw → asymmetric pulse |
| Distort | Clean → asymmetric clipping → foldback | — |
| Env Amt | Filter-envelope sweep | — |

The obsolete engine Slide and Accent controls have been removed. Existing
six sound parameters keep their order and meaning. Old eight-parameter prototype
patches need those two removed parameters discarded. Stock track volume supplies
output level. The voice pool has 16 independent slots, so using Acidline on
another track does not activate a global mono lock or silence the first track.
Use XY's stock play mode for mono/legato on each track.

## DSP and optimization

The oscillator, nonlinear feedback filter and distortion run at **88.2 kHz**,
then a symmetric 31-tap half-band FIR returns 44.1 kHz output. The filter retains
four trapezoidal stages with input/feedback saturation. Individual stages are
linear within their bounded state range. This is an original musical
approximation, not a calibrated 303 circuit model.

The hot loop uses Q16 audio and Q14 coefficients, with mathematically bounded
32-bit products. Public buffers remain Q31. Tables use signed 16-bit storage;
coefficients are prepared per block and ramped over the block. Envelope decay
and DC removal run at the output/block rates instead of four times per sample.
A saw-only patch skips the extra pulse lookup. FIR symmetry avoids duplicated
multiplies. The C++ voice does no velocity scaling, portamento or voice stealing;
those responsibilities stay with the stock host and SDK adapter.

In three paired host tests against the preceding 4× version, the same sustained
patch changed from roughly 17.4 microseconds to 3.3–3.4 microseconds per block,
about **5.2× faster**. This is a host measurement, not Blackfin cycles or an
on-device acceptance result. The SDK's own host CPU estimate fell from about 175,700 to 47,400 instructions
per voice block. It remains above the 15,000 target and is advisory; this is not
an on-device CPU measurement. The pinned Blackfin native compile passed, using
116.6 KiB of total module memory (machine plus SDK).

The tradeoff is 2× instead of 4× oversampling and finite Q16/Q14 precision. In
the targeted clipped 6 kHz tone fixture, the fifth harmonic's 14.1 kHz alias is
reduced by about **73 dB** compared with processing at the output rate. That
fixture does not establish alias rejection for every note or distortion setting.

## Browser audition

The local page at **http://127.0.0.1:8947/** is optional test equipment. It is not
part of device firmware. Press Play to hear the loop, select a sound or edit the
six sound controls. The 16-step test pattern can exercise notes, rests, velocity
and ties, but OP-XY uses its own sequencer.

The browser controller simulates a fixed 60 ms glide on tied steps by preparing
pitch ramps **outside** the C++ engine. Its A switches select louder note
velocity, applied after the voice; they no longer secretly modify the filter.
For filter accents on device, use parameter locks. The browser's short amplitude
gate and glide are audition approximations, not an emulation of XY's stock voice.

## Build and test on this Mac

Keep the SDK beside this repository at `../../../xy-machines-sdk`. From the SDK checkout:

```sh
export PATH="$PWD/local/wasm-tools/lld/23.1.4/bin:$PATH"
PYTHON=/Users/tapsy/.cache/codex-runtimes/codex-primary-runtime/dependencies/python/bin/python3
"$PYTHON" sdk.py test --machine ../acidline --cxx /usr/bin/clang++ --firmware
"$PYTHON" sdk.py preview --machine ../acidline --cxx /usr/bin/clang++ --out local/acidline-preview-009
ACIDLINE_WASM="$PWD/local/acidline-preview-009/engine.wasm" node --test ../acidline/tests/audition.test.mjs
"$PYTHON" ../acidline/audition/serve.py --preview local/acidline-preview-009 --port 8947
```

Use a fresh output directory and stop the previous server before reusing the
port. The current session serves `acidline-preview-008`. The server uses the
SDK's authenticated preview bundle and allowlisted server, adding only named
audition assets. It never serves either repository as a directory. The standard
SDK workbench is available at `/harness/` on this server.

`tools/generate_tables.py` reproducibly generates the tables with standard Python.
All build outputs stay outside this machine repository; native modules and any
firmware-containing outputs stay outside every checkout.

Tests cover 81 control combinations, peak bounds, filter attenuation, distortion,
exact cumulative phase for varying stock pitch increments, voice independence,
display frames and a spectral alias fixture. C++ tests run with ASan/UBSan.
Audio-thread tests run the actual WebAssembly voice and check pattern timing,
ties, velocity outside the voice, stop/restart and live edits. The generated CI
checks source, sound, the audition scheduler and a pinned native compile.

## Hardware and publication

No firmware has been installed and no device operations have been performed.
Native compilation checks the target code, not real-time CPU headroom or stock
sequencer behavior. Follow the SDK's [hardware profiling guide](https://github.com/etcetc000/open-xy/blob/staging/docs/profiling.md)
for revision-bound measurements before publication. Keep firmware and stock
content outside every checkout. Installation and diagnostics need a separate
explicit instruction. No SDK source changes were needed for this design.

MIT; see LICENSE. DSP, tables and browser audition are original to this work.
