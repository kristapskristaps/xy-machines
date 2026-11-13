# x0x: hardware CPU measurement

Machine commit: `1fa86cf9f4f2a610050283244c594c928333b57d`. Benchmark: `opxy-drum-callback-v1`. Scope: the kit's voices through the SDK's drum adapter, not stock's drum renderer.

Heaviest single voice, each the median of 3 captures: **3.989%** of the stock 580,352-cycle block budget (model 8 at 127). Heaviest at the middle: 3.838%. Whole kit: its own keys 17.678%, same-key retriggers 5.086%, full polyphony of the heaviest voice 30.384%.

| Run | Median |
| --- | ---: |
| the kit's keys, 8 voices | 17.678% |
| model 0 at 64 | 3.293% |
| model 0 at 0 | 0.798% |
| model 0 at 127 | 3.824% |
| model 1 at 64 | 3.528% |
| model 1 at 0 | 0.763% |
| model 1 at 127 | 3.725% |
| model 2 at 64 | 2.878% |
| model 2 at 0 | 0.937% |
| model 2 at 127 | 2.898% |
| model 3 at 64 | 1.988% |
| model 3 at 0 | 0.381% |
| model 3 at 127 | 3.450% |
| model 4 at 64 | 3.182% |
| model 4 at 0 | 0.763% |
| model 4 at 127 | 3.230% |
| model 5 at 64 | 3.183% |
| model 5 at 0 | 0.882% |
| model 5 at 127 | 3.236% |
| model 6 at 64 | 3.186% |
| model 6 at 0 | 0.757% |
| model 6 at 127 | 3.244% |
| model 7 at 64 | 1.697% |
| model 7 at 0 | 0.380% |
| model 7 at 127 | 2.406% |
| model 8 at 64 | 2.821% |
| model 8 at 0 | 0.925% |
| model 8 at 127 | 3.989% |
| model 9 at 64 | 3.210% |
| model 9 at 0 | 3.973% |
| model 9 at 127 | 3.915% |
| model 10 at 64 | 3.838% |
| model 10 at 0 | 1.767% |
| model 10 at 127 | 3.949% |
| model 11 at 64 | 3.374% |
| model 11 at 0 | 0.800% |
| model 11 at 127 | 3.406% |
| model 12 at 64 | 2.542% |
| model 12 at 0 | 0.317% |
| model 12 at 127 | 2.669% |
| model 13 at 64 | 1.955% |
| model 13 at 0 | 0.331% |
| model 13 at 127 | 2.020% |
| model 14 at 64 | 2.782% |
| model 14 at 0 | 0.604% |
| model 14 at 127 | 2.902% |
| model 15 at 64 | 2.708% |
| model 15 at 0 | 0.587% |
| model 15 at 127 | 2.789% |
| model 16 at 64 | 2.690% |
| model 16 at 0 | 0.575% |
| model 16 at 127 | 2.768% |
| model 17 at 64 | 3.148% |
| model 17 at 0 | 0.600% |
| model 17 at 127 | 1.851% |
| same-key retriggers, the heaviest model | 5.086% |
| the heaviest model, 8 voices | 30.384% |

Recommended: at most 3.5%. **Warning: 3.989% of the block budget for model 8 at 127 is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, stock's drum renderer, its amplitude envelope or mixing, or settings between the levels. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `2f2776fcf107a24791c1f0b0310c0eb9b39979f0aa0aee6bf3e828e70eebb516`.

Compile report SHA-256: `80c17039dc3b10b42b790a94c3fd05a43dbeed91c37e925d3c69e2e6024652c4`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
