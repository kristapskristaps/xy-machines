# Granular for OP-XY

A stereo sample instrument for Open-XY SDK 1.2. It turns an assigned sample into
up to eight overlapping, Hann-windowed grains per sounding note. Scanning and
grain pitch move independently: stop the scan to sustain a texture, move it
backwards, or scatter grains across the sample. Each grain uses linear PCM
interpolation, its own pitch offset and stereo position.

The machine lives in this repository; the sibling `../../../open-xy` checkout supplies
the SDK. It registers as a `sample` machine on stock sample page 2. Assign a sample
through the usual sample browser. Saved trim, reverse, sample level, tuning and
track transpose apply. Stock envelopes, filters and mixing follow its output.
Use a sustained amp envelope when playing long textures. The stock loader's
882,000-frame limit still applies.

## Controls

| Control | Range | Default | Behavior |
| --- | --- | --- | --- |
| Position | 0–100% | 10% | Grain origin within the trimmed sample; scan adds an offset |
| Size | 6–250 ms | 67 ms | Duration of newly created grains |
| Density | 1–8 | 4 | Target number of overlapping grains |
| Spray | 0–100% | 5% | Random displacement around the grain origin, wrapping within trim |
| Scan | −2× to +2× | +0.5× | Scan speed at the original sample rate; zero holds position |
| Spread | 0–12 semitones | 0 | Random grain detuning up to this amount in either direction |
| Width | 0–100% | 50% | Random grain panning; zero preserves centered stereo playback |
| Level | 0–100% | 80% | Output level before the stock instrument's envelope and mixer |

The parameter-editor SDK build exposes these through the stock selector:

| Encoder | Click | Shift + click |
| --- | --- | --- |
| 1 | Position | Scan |
| 2 | Size | Spread |
| 3 | Density | Width |
| 4 | Spray | Level |

Choose a value, then click the left or white encoder to apply it. Back cancels.
Values are shown in the control's units. Continuous controls offer 101 points
across their range; accepting the original row keeps its exact saved value.
These are base parameters, stored with projects and presets. Ordinary encoder
turns still perform sample editing. The installed prototype without the SDK
parameter-editor change does not have this editor.

The SDK exposes all eight parameters as LFO destinations. Controls are applied
each 64-frame block; size, position, spread, width and note pitch are captured
when each grain starts. Existing grains finish their windows. Density changes
alter the spawn interval and mix normalization. Grain count remains bounded
under control motion; when every slot is busy, that spawn is skipped. Normal
steady settings space spawns so eight slots suffice. Note retrigger and trim or
reverse edits reset the grains and scan position.

Scan is free-running, without beat synchronization. Zero scan freezes the scan
offset; Position still moves the origin and Spray can still vary new grains.
Grains wrap and interpolate inside the current trim. Reverse reverses the PCM
direction, including the scan's direction through the original sample. Stereo
source channels keep their order. Mono output averages both source channels.
Playback rate is capped at 256 source frames per output frame, and combined
saved/track tuning is bounded to eight octaves in either direction.

The four presets are **Cloud**, **Freeze**, **Scatter** and **Scan**. They are
included in `definition.json`; the audition tool also renders an octave-up Scan.

## Test and audition

Requires Python 3.11+ and a C++11 host compiler. WAV rendering additionally needs
NumPy 2.3.5. From this folder:

```sh
python ../../../open-xy/sdk.py test --machine .
python tools/audition.py --out ../../../open-xy/local/granular-audition-001
```

Use a fresh output directory each time. On macOS, add
`--cxx /opt/homebrew/opt/llvm/bin/clang++` to the audition command if needed.
`--sample /path/to/sample.wav` auditions a 16-bit PCM mono/stereo WAV instead of
the generated plucked-chord source. The output contains the source WAV, five
eight-second stereo examples and numerical metrics with source hashes. The
audition calls the same `render_sample` code as the device, with a simple host
attack/release envelope. It does not model the stock filter, envelope or mixer.
The SDK currently has no interactive sample-engine workbench.

Tests cover output bounds, empty/invalid samples, live trimming, reverse,
stereo/mono routing, every control's audible effect, independent voices,
retrigger, sample-rate conversion, octave pitch, track and pad tuning, tiny
loops, extreme controls, and the portable fixed-point arithmetic. Run the C++
tests with AddressSanitizer and UndefinedBehaviorSanitizer for memory checks.
The Hann table is reproducible with `python tools/generate_window.py`.

## Native development build

Keep modules and firmware outside every checkout, including ignored folders:

```sh
python ../../../open-xy/sdk.py compile --machine . --profiling on --out /external/private/granular-module-001
python ../../../open-xy/sdk.py build --machine . --profiling on --stock-os /external/private/stock-1.2.1/os.ldr --out /external/private/granular-dev-001
```

The first command produces a module, which cannot be installed as an OS. The
second needs the SDK's exact, untouched stock OS 1.2.1 from the user's device.
Carry a previous build's `--layout` and collection forward when preserving
other custom machines. Engine 36 is this standalone build's requested number;
project identity follows the build's layout.

Hardware sound, stock page behavior, maximum practical polyphony and CPU cost
remain pending until tested on an OP-XY. Follow the SDK's
[hardware profiling guide](https://github.com/etcetc000/open-xy/blob/staging/docs/profiling.md)
for three revision-bound measurements before catalog submission. Host tests
and compilation do not provide measured device cycles. Installation, reboot
and diagnostics require a separate device instruction.

Source is under [MIT](LICENSE). The generated SDK repository skeleton is retained;
the sample DSP and mathematically generated window are implemented here. No
third-party DSP code or firmware is bundled.
