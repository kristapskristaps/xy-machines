# Atlas, Tandem, Sift and X0X: local optimization batch

This batch targets a hardware control-sweep result below 4% for each machine,
preferably 3.5% or less. **No hardware CPU result exists for this batch.** Host
estimates and generated-code inspection do not qualify a catalog submission.
The installed seven-machine collection has not been changed by this work.

| Machine | Retained change | Sound checks |
| --- | --- | --- |
| Atlas | Skip settled coordinate updates; fuse oscillator, gain and output mixing into one pass instead of materializing main/auxiliary arrays | 45 frozen legacy scenes retained; 45 long candidate/reference scenes; worst relative RMS difference 0.000128, level ratio 1.000000–1.000002; oscillator phase unchanged |
| Tandem | Choose square, saw/triangle and saw/square correction kernels once per block, removing inactive waveform corrections | Frozen PCM/state tests plus 32,768 blocks through waveform boundaries, sync, pulse width and pitch motion match exactly |
| Sift | Prepare the shared grainlet carrier window once; use bounded Q15 signal operations while retaining the phase clocks, DC filtering and conversion path | 45 frozen legacy scenes retained; 45 × 1,536-block comparisons; worst relative RMS difference 0.000354, level ratio 0.999883–1.000036; oscillator phases unchanged |
| X0X | Specialize synthesized drum models outside the sample loop and separate their mutable working state from read-only block settings | All 47 type/style combinations match prior PCM/state through extremes, automation, retriggers and live type changes; eight-voice capacity and all seven kits retained |

Both outputs in Atlas use the same gain. The fused path combines the gain and
crossfade with at most two signed16 rounding units of difference; near the
clipping rails it preserves the original independent clipping. The original
Q31 waveform interpolation is retained. The generated Blackfin wavetable
kernel is 2,564 bytes versus 2,722 bytes before this batch; the largest single
Atlas voice-object stack frame falls from 356 to 108 bytes. Neither number is a
measured CPU reduction.

Machine preview, host and Blackfin native checks pass for all four. Atlas,
Tandem and Sift also passed undefined-behavior checks on their extended
comparison tests. All 21 shared repository workflow tests pass. The SDK check
retains its three previously observed macOS fixture failures (drum-services
unused constant and two master-processor registration/link tests); the SDK
checkout was not modified. The selected machine checks and verified firmware
build do not fail on those fixtures.

The combined firmware was compiled and reconstructed from clean source at
`bef3373`. It contains all seven machines, all 43 authored presets and the
identical layout with 42 saved ID reservations. Its allocated span is
1,939,560 bytes. The staged bundle contains nine files; only `os.ldr` differs
from the last verified restoration bundle. Presets are excluded from that
transfer bundle. One complete 332-case capture can provide reports for the
whole collection; no separate per-engine bank is required. Firmware, raw
captures, compiler objects and transfer bundles remain outside every checkout.

## Rejected local experiments

- Removing Atlas's 48 kHz conversion produced larger waveform differences at
  high notes; it is not in this batch.
- Atlas's lower-precision cubic interpolation was superseded by pass fusion,
  which has a smaller sound difference and removes actual intermediate work.
- Four/eight-sample X0X envelope ramps were tested but not retained: their host
  timing benefit was inconclusive and they changed transient/pitch envelopes.
  The SDK already implements Q31 multiplication with one Blackfin instruction;
  counting a Q31 operation as a software wide multiply would overstate savings.

The next hardware run must confirm the CPU figures, boot, preset behavior and
control motion. Retain only measured improvements. This document does not claim
that any of the four is already below its hardware CPU target, or that the
catalog workload bounds arbitrary project CPU usage.
