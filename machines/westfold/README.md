# Westfold

A playable West Coast synthesis voice for [Open-XY](https://github.com/etcetc000/open-xy): a rounded triangle core, progressive harmonic folding, odd/even overtones, audio-rate phase modulation, a cycling slope and a plucked low-pass gate.

The screen follows the instrument: a teal clean core sits behind the amber folded wave, partials grow below it, and a twelve-part iris opens on each note. Overtone has an odd/even position marker. The lettering is vector artwork, so it needs no installed fonts. Reduced motion keeps note and control feedback while stopping continuous animation. The drawing illustrates the synthesis settings and note events; it does not measure the output spectrum.

## Play

Four main controls, with a second control on each encoder press:

| Encoder | Main control | Press for |
| --- | --- | --- |
| 1 | **Overtone:** odd to even harmonic families | **FM Ratio:** 0.5, 1, 1.5, 2, 3, 4, 6, 8, with continuous positions between |
| 2 | **Multiply:** progressively fold the waveform | **FM Depth:** audio-rate phase modulation with a squared response |
| 3 | **Balance:** clean fundamental to shaped overtones | **Slope Hz:** 0.05–400 Hz cycling function |
| 4 | **Decay:** 45 ms–8 s low-pass-gate contour | **Motion:** bipolar slope modulation of Multiply |

The last half-percent of Decay opens the gate continuously; its caption reads **hold**. The stock amplitude envelope still determines note release. Increase its release if a long internal contour is cut short. The factory presets provide full sustain and a longer release for the internal gate.

Start with **Initial Coast**, then try **Buchla Wood** for woody strikes, **Rubber Bass** for rounded sequences, **Copper Bells** for metallic notes and **Slow Voltage** for moving sustained tones. **Circuit Birds** takes the slope into audio rate. All twelve patches are in the audition; matching device presets are in `presets/`. Copy those JSON files into `/content/presets/westfold/` after installing firmware that contains this machine.

For a lively sequence, vary Multiply and Overtone with parameter locks, leave some clean signal in Balance, and try FM Ratio between its harmonic anchors. The stock LFOs can target all eight parameters.

## Audition and check

Keep the SDK beside this repository as `../../../xy-machines-sdk`. Use Python 3.11+, NumPy 2.3.5, Node 22.12+ and Clang with a WebAssembly linker. On macOS, Homebrew LLVM and LLD provide the preview compiler. Use `--cxx /opt/homebrew/opt/llvm/bin/clang++` if Apple Clang cannot link WebAssembly.

```sh
python ../../../xy-machines-sdk/sdk.py test --machine . --cxx clang++ --firmware
python ../../../xy-machines-sdk/sdk.py preview --machine . --cxx clang++ --out ../../../xy-machines-sdk/local/westfold-preview-001
python ../../../xy-machines-sdk/tools/serve_harness.py --preview ../../../xy-machines-sdk/local/westfold-preview-001 --port 8947
```

Open `http://127.0.0.1:8947`, enable audio and hold the on-screen keys or use `A W S E D F T G Y H U J K`. The audition has its own simple amplitude gate; the device adds its stock envelopes, filter, velocity and mixer.

Render dry WAV demonstrations and run the static folding alias check:

```sh
python resources/render_demo.py --out ../../../xy-machines-sdk/local/westfold-audition-001
```

Output directories must be new. Generated audio, previews and binaries stay outside this repository. `tests/sound.cpp` checks harmonic families, spectral growth, clean balance, pitch, decay, modulation, deterministic restarts and randomized automation. `tests/display.cpp` checks frame bounds, track changes, note response, alternate captions and reduced motion. Both also run with AddressSanitizer and UndefinedBehaviorSanitizer during development.

## Sound and implementation

The folding transfer comes from Emilie Gillet’s MIT-licensed Braids triangle folder; its original table and notice are in `vendor/`. This is an original voice inspired by the 0-Coast’s synthesis approach. Its gate and modulation behavior are musical approximations.

Offline generation projects the folding curve into 17 continuously interpolated fold levels and eight harmonic limits. Interpolating between those limits preserves smooth pitch changes. The even branch runs at twice the core phase. A separate clean branch keeps Balance independent of folding and slope modulation. All audio processing uses bounded integer arithmetic; there are no runtime allocations, OS calls or platform conditionals.

A bandwidth allowance reduces upper harmonics under modulation. Even overtones and modulation depth taper at very high pitches. The low-pass gate couples gain to two gentle filter poles, and a DC blocker removes offsets. Its exponential contour is interpolated over each 64-sample block; filter coefficients update every eight samples.

The engine retains all eight audio banks. The display uses only bank 1, so its
generated `display_tables.h` holds that bank and the lookups the drawing needs.
This avoids copying seven unused banks into the display and removes about
119 KiB from the native module. Both headers come from the same generator;
their integer values are unchanged.

An exactly zero smoothed Balance skips the unused folded branches; zero Overtone skips
the unused even branch. The high-pitch guard also skips an even branch whose
gain is zero. Controls, phase, modulation and gate state still advance on every
sample. Band crossfades and the FM guard use exact bounded reciprocal arithmetic
in place of three native software division sites. Voice storage and native
audio stack usage are unchanged.

Audio phase reads now use a 1,024-point nearest lookup. The extra points are
exact midpoints of the preceding 512-point integer interpolant; fold-level and
mip crossfades still interpolate continuously, while phase, envelope, modulation
and the gate keep their existing timing. The padded 1,025-value rows avoid
power-of-two row strides. The display retains its original data.

This trades phase interpolation for more table memory and stronger images.
Across 405 complete preset/control comparisons at 27.5–8,000 Hz, maximum level
change was 0.017 dB and relative waveform RMS error was 3.37%. Raw coherent
bank tests measured worst unwanted-image energy at -28.1 dB relative to wanted
harmonics (preceding interpolator: -47.6 dB); the largest image was -34.0 dB
relative to the strongest wanted partial. These are declared compromises,
not a claim of identical sound. Hardware CPU acceptance remains pending.

`tests/equivalence.cpp` authenticates the frozen preceding DSP in
`tests/reference/` against unchanged audio/state hashes. Production audio is
checked by `tests/quality.cpp` against that reference, with 4% RMS waveform,
0.25 dB level and 0.02 octave-band distance limits. Tuning, envelopes and
modulation phases must match exactly. Other sound/display/arithmetic tests
still cover harmonic growth, clean balance, control behavior, drawing data,
band transitions, one million phase increments and output bounds.

To regenerate the committed tables, metadata, presets and panel:

```sh
python resources/generate_tables.py
python resources/generate_dense_points.py
python resources/configure.py
```

## Device validation

The machine compiles for Blackfin through the public SDK. The source declares a pool of eight voices; usable polyphony depends on the full device workload. Swept host estimates on the development Mac varied materially, and some exceeded the SDK's 24,000-instruction advisory threshold. Treat the native module as a development build until device CPU, polyphony, rendering and sound have been validated in an authorized hardware test.

Prepare a profiling-enabled development module outside every checkout:

```sh
python ../../../xy-machines-sdk/sdk.py compile --machine . --profiling on --out /absolute/external/path/westfold-module-001
```

An installable image additionally needs the exact stock OS 1.2.1 from the owner’s device. Follow the SDK’s [build guide](https://github.com/etcetc000/open-xy/blob/staging/docs/build.md) and [hardware profiling guide](https://github.com/etcetc000/open-xy/blob/staging/docs/profiling.md). Firmware and firmware-containing packages belong outside every checkout. No device installation or diagnostics are performed by the development commands above.

Westfold is MIT licensed; see [LICENSE](LICENSE) and the [Braids attribution](vendor/README.md).
