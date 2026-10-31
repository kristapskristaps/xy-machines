# X0X

A configurable OP-XY drum machine. Shift + black chooses a drum type for the
selected pad; two encoder pages shape that pad independently of the others.

This revision is the **controls prototype**. It renders silence: drum DSP and
sample assets are the next milestone. Host tests exercise the real SDK pad
editor and codec; on-device interaction and project/preset save/reload remain
unverified.

## SDK

Requires SDK 1.9 drum synthesis support, currently on
[`sdk/drum-synthesis`](https://github.com/etcetc000/open-xy/tree/sdk/drum-synthesis),
not `staging`. CI pins `c08d2d023473e989792ad1feadec75cc58ce4de2`.
The local development checkout is `../open-xy-x0x-sdk`; the existing `../open-xy`
checkout is untouched. Use a separate clone/worktree at the pinned revision.

## Controls

| Gesture / page | Encoders 1–4 |
| --- | --- |
| Shift | Drum type / choke group / unused / unused |
| Page 1 | Style / tune / decay / tone |
| Page 2 | Attack / voice character / body character / drive where available |

Press any main encoder to switch all four between the two pages. Both rows
remain visible and the active row is emphasized. Shift exposes the same config
row from either page. Choke groups are NONE and 1–3; pads sharing a numbered
group cut each other off. Repeated hits on the same pad use the SDK's short
crossfade instead of piling up voices.

The kit has 24 independent pads, with these drum types: kick, snare, clap, rim,
low/mid/high tom, closed/open hat, cymbal, ride, cowbell, claves, maracas,
low/mid/high conga and tambourine. The default mapping is in `models.h`.

Style selects the available 606, 707, 808 and 909 families for that type. For
example, kick offers all four; ride offers 707/909, and congas offer 808.
Each family's part of the knob travel has equal width. Single-family types
show their fixed family. Tune has an exact zero centre and a ±12-semitone
range. The decay caption is milliseconds; final ranges will follow the DSP.
Other values are amounts from 0 to 100 without a percent sign.

Page 2 labels follow the selected drum: kick has sweep/body, snare has
snappy/body, clap has spread/noise, and hats have metal/open. Simple percussion
leaves the eighth position blank. The SDK stores each used control at full
7-bit precision: twelve eight-control types and six seven-control types fit
its existing prefix codec. Stock owns pad persistence, step-lock offsets,
publication and choke routing; there is no sidecar project format.

## Validate and preview

With Python 3.11+, NumPy 2.3.5 and Clang with WebAssembly linking installed:

```sh
python ../open-xy-x0x-sdk/sdk.py test --machine . --cxx clang++ --host-cxx c++
python tests/sdk_integration.py --sdk ../open-xy-x0x-sdk --cxx c++
python ../open-xy-x0x-sdk/sdk.py preview --machine . --out ../open-xy-x0x-sdk/local/x0x-preview-001 --cxx clang++
python ../open-xy-x0x-sdk/tools/serve_harness.py --preview ../open-xy-x0x-sdk/local/x0x-preview-001
```

On macOS, pass `/opt/homebrew/opt/llvm/bin/clang++` as `--cxx` when Apple's
Clang has no WebAssembly linker. Use fresh preview output folders.

The browser shows the shared C++ display and one model/control set. Its Model
slider corresponds to Shift + black. It does **not** simulate the device's
independent pads or project storage. Editing a second-row control selects that
row. The additional SDK integration test covers actual per-pad editor routing,
all four page presses, eight-track isolation, config from either page, choke,
pad-copy data, and all 128 values of every used control through stock's four
preset words. These checks use the pinned SDK's existing stock-boundary doubles.

## Hardware validation

Build into a fresh folder outside every checkout using the SDK's
[build guide](https://github.com/etcetc000/open-xy/blob/sdk/drum-synthesis/docs/build.md).
A controls-only development package can test pad selection, Shift config,
both control pages, copy/paste, project save/reload and preset save/reload.
Installation/device diagnostics require a separate user instruction. Do not
replace an existing custom collection without preserving its layout and machines.

Before publishing the finished sound engine, follow
[hardware profiling](https://github.com/etcetc000/open-xy/blob/sdk/drum-synthesis/docs/profiling.md)
and collect revision-bound CPU evidence. A host check or successful build
proves neither hardware acceptance nor measured CPU cost.

Machine source, artwork and tests live here. SDK/platform changes belong in a
separate SDK PR against `staging`. No firmware or stock samples belong in this
repository. The source is covered by [LICENSE](LICENSE).
