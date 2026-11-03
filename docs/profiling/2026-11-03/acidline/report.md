# acidline: hardware CPU measurement

Machine commit: `6cf46e53d30cb4e3b4aa37b2476bb1c07107a367`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 66 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **9.323%** of the stock 580,352-cycle block budget. Midpoint run: 8.611%. Every control at its heaviest position: 7.344%. Heaviest single setting: Wave Square.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 8.611% |
| Cutoff 0.0% | 8.740% |
| Cutoff 12.5% | 8.634% |
| Cutoff 25.0% | 8.702% |
| Cutoff 37.5% | 8.548% |
| Cutoff 50.0% | 8.661% |
| Cutoff 62.5% | 8.540% |
| Cutoff 75.0% | 8.688% |
| Cutoff 87.5% | 8.605% |
| Cutoff 100.0% | 8.734% |
| Reso 0.0% | 8.589% |
| Reso 12.5% | 8.708% |
| Reso 25.0% | 8.562% |
| Reso 37.5% | 8.704% |
| Reso 50.0% | 8.589% |
| Reso 62.5% | 8.707% |
| Reso 75.0% | 8.559% |
| Reso 87.5% | 8.685% |
| Reso 100.0% | 8.595% |
| Distort 0.0% | 7.352% |
| Distort 12.5% | 8.580% |
| Distort 25.0% | 8.701% |
| Distort 37.5% | 8.582% |
| Distort 50.0% | 8.704% |
| Distort 62.5% | 8.586% |
| Distort 75.0% | 8.710% |
| Distort 87.5% | 8.594% |
| Distort 100.0% | 8.719% |
| Env Amt 0.0% | 8.583% |
| Env Amt 12.5% | 8.702% |
| Env Amt 25.0% | 8.580% |
| Env Amt 37.5% | 8.702% |
| Env Amt 50.0% | 8.580% |
| Env Amt 62.5% | 8.701% |
| Env Amt 75.0% | 8.580% |
| Env Amt 87.5% | 8.701% |
| Env Amt 100.0% | 8.580% |
| Decay 0.0% | 8.702% |
| Decay 12.5% | 8.582% |
| Decay 25.0% | 8.702% |
| Decay 37.5% | 8.580% |
| Decay 50.0% | 8.708% |
| Decay 62.5% | 8.582% |
| Decay 75.0% | 8.711% |
| Decay 87.5% | 8.582% |
| Decay 100.0% | 8.693% |
| Wave Saw | 8.580% |
| Wave Triangle | 8.540% |
| Wave Square | 9.323% |
| Sub 0.0% | 8.701% |
| Sub 12.5% | 8.879% |
| Sub 25.0% | 8.998% |
| Sub 37.5% | 8.878% |
| Sub 50.0% | 8.998% |
| Sub 62.5% | 8.879% |
| Sub 75.0% | 8.998% |
| Sub 87.5% | 8.878% |
| Sub 100.0% | 8.998% |
| Damp 0.0% | 8.580% |
| Damp 12.5% | 8.780% |
| Damp 25.0% | 8.672% |
| Damp 37.5% | 8.780% |
| Damp 50.0% | 8.670% |
| Damp 62.5% | 8.780% |
| Damp 75.0% | 8.670% |
| Damp 87.5% | 8.780% |
| Damp 100.0% | 8.672% |
| combined (as read from the reports): Cutoff 0.0%, Reso 12.5%, Distort 100.0%, Env Amt 12.5%, Decay 75.0%, Wave Square, Sub 25.0%, Damp 12.5% | 7.344% |

Recommended: at most 3.5%. **Warning: 9.323% of the block budget at Wave Square is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `0b5b9570f04d0028d3c19a9a373ff91a12a6f288cd18179ab931b198f52c0809`.

Compile report SHA-256: `1c5f96cf1aeb65402e7347026e33c9878052d39c904035d3b46c3cda59a5e862`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.

Loaded collection SHA-256: `7a454a4b044dcf31753eb414d3c32c01013d0b0a60cae6784803116db5400aef`. Firmware SHA-256 above identifies its resident kernel.
