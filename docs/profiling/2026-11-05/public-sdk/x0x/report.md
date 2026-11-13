# x0x: hardware CPU measurement

Machine commit: `d8246183f45d556a176f1b37c9b8dd29ae2d5ee0`. Benchmark: `opxy-drum-callback-v1`. Scope: the kit's voices through the SDK's drum adapter, not stock's drum renderer.

Heaviest single voice, from one capture: **4.201%** of the stock 580,352-cycle block budget (model 9 at 0). Heaviest at the middle: 3.787%. Whole kit: its own keys 17.801%, same-key retriggers 7.608%, full polyphony of the heaviest voice 32.232%.

| Run | Mean block budget |
| --- | ---: |
| the kit's keys, 8 voices | 17.801% |
| model 0 at 64 | 3.310% |
| model 0 at 0 | 0.803% |
| model 0 at 127 | 3.830% |
| model 1 at 64 | 3.540% |
| model 1 at 0 | 0.763% |
| model 1 at 127 | 3.742% |
| model 2 at 64 | 2.872% |
| model 2 at 0 | 0.938% |
| model 2 at 127 | 2.899% |
| model 3 at 64 | 2.022% |
| model 3 at 0 | 0.381% |
| model 3 at 127 | 3.503% |
| model 4 at 64 | 3.200% |
| model 4 at 0 | 0.768% |
| model 4 at 127 | 3.249% |
| model 5 at 64 | 3.201% |
| model 5 at 0 | 0.882% |
| model 5 at 127 | 3.253% |
| model 6 at 64 | 3.201% |
| model 6 at 0 | 0.760% |
| model 6 at 127 | 3.261% |
| model 7 at 64 | 1.704% |
| model 7 at 0 | 0.386% |
| model 7 at 127 | 2.376% |
| model 8 at 64 | 2.829% |
| model 8 at 0 | 0.974% |
| model 8 at 127 | 3.931% |
| model 9 at 64 | 3.215% |
| model 9 at 0 | 4.201% |
| model 9 at 127 | 3.874% |
| model 10 at 64 | 3.787% |
| model 10 at 0 | 1.769% |
| model 10 at 127 | 3.903% |
| model 11 at 64 | 3.401% |
| model 11 at 0 | 0.798% |
| model 11 at 127 | 3.426% |
| model 12 at 64 | 2.458% |
| model 12 at 0 | 0.317% |
| model 12 at 127 | 2.586% |
| model 13 at 64 | 1.967% |
| model 13 at 0 | 0.333% |
| model 13 at 127 | 2.028% |
| model 14 at 64 | 2.625% |
| model 14 at 0 | 0.589% |
| model 14 at 127 | 2.710% |
| model 15 at 64 | 2.835% |
| model 15 at 0 | 0.604% |
| model 15 at 127 | 2.916% |
| model 16 at 64 | 2.679% |
| model 16 at 0 | 0.575% |
| model 16 at 127 | 2.765% |
| model 17 at 64 | 3.157% |
| model 17 at 0 | 0.601% |
| model 17 at 127 | 1.854% |
| same-key retriggers, the heaviest model | 7.608% |
| the heaviest model, 8 voices | 32.232% |

Recommended: at most 3.5%. **Warning: 4.201% of the block budget for model 9 at 0 is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, stock's drum renderer, its amplitude envelope or mixing, or settings between the levels. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `d9cece5592a8bda9c47fec8f03b7b0b4d08c99eec0417119a92a88eb9eaaf339`.

Compile report SHA-256: `49ed43f91f3ab3d8917ce43c2959f5752ecedb6512471274f801a7c4b6ce60d8`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
