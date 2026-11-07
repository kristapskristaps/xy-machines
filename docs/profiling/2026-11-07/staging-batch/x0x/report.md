# x0x: hardware CPU measurement

Machine commit: `1fa86cf9f4f2a610050283244c594c928333b57d`. Benchmark: `opxy-drum-callback-v1`. Scope: the kit's voices through the SDK's drum adapter, not stock's drum renderer.

Heaviest single voice, each the median of 3 captures: **3.989%** of the stock 580,352-cycle block budget (model 8 at 127). Heaviest at the middle: 3.838%. Whole kit: its own keys 17.678%, same-key retriggers 5.086%, full polyphony of the heaviest voice 30.384%.

| Run | Mean block budget (median across captures) | Worst observed block |
| --- | ---: | ---: |
| the kit's keys, 8 voices | 17.678% | 24.643% |
| model 0 at 64 | 3.293% | 6.073% |
| model 0 at 0 | 0.798% | 4.958% |
| model 0 at 127 | 3.824% | 6.856% |
| model 1 at 64 | 3.528% | 6.128% |
| model 1 at 0 | 0.763% | 4.407% |
| model 1 at 127 | 3.725% | 6.261% |
| model 2 at 64 | 2.878% | 6.085% |
| model 2 at 0 | 0.937% | 4.472% |
| model 2 at 127 | 2.898% | 5.402% |
| model 3 at 64 | 1.988% | 5.045% |
| model 3 at 0 | 0.381% | 4.814% |
| model 3 at 127 | 3.450% | 7.224% |
| model 4 at 64 | 3.182% | 5.989% |
| model 4 at 0 | 0.763% | 4.356% |
| model 4 at 127 | 3.230% | 6.207% |
| model 5 at 64 | 3.183% | 5.718% |
| model 5 at 0 | 0.882% | 4.962% |
| model 5 at 127 | 3.236% | 5.942% |
| model 6 at 64 | 3.186% | 5.997% |
| model 6 at 0 | 0.757% | 4.517% |
| model 6 at 127 | 3.244% | 5.806% |
| model 7 at 64 | 1.697% | 4.506% |
| model 7 at 0 | 0.380% | 9.952% |
| model 7 at 127 | 2.406% | 6.564% |
| model 8 at 64 | 2.821% | 5.516% |
| model 8 at 0 | 0.925% | 8.399% |
| model 8 at 127 | 3.989% | 7.707% |
| model 9 at 64 | 3.210% | 5.794% |
| model 9 at 0 | 3.973% | 8.812% |
| model 9 at 127 | 3.915% | 7.094% |
| model 10 at 64 | 3.838% | 6.264% |
| model 10 at 0 | 1.767% | 5.563% |
| model 10 at 127 | 3.949% | 6.493% |
| model 11 at 64 | 3.374% | 5.960% |
| model 11 at 0 | 0.800% | 5.667% |
| model 11 at 127 | 3.406% | 6.094% |
| model 12 at 64 | 2.542% | 5.164% |
| model 12 at 0 | 0.317% | 3.687% |
| model 12 at 127 | 2.669% | 5.231% |
| model 13 at 64 | 1.955% | 4.491% |
| model 13 at 0 | 0.331% | 3.441% |
| model 13 at 127 | 2.020% | 4.511% |
| model 14 at 64 | 2.782% | 5.496% |
| model 14 at 0 | 0.604% | 5.232% |
| model 14 at 127 | 2.902% | 5.487% |
| model 15 at 64 | 2.708% | 4.627% |
| model 15 at 0 | 0.587% | 4.788% |
| model 15 at 127 | 2.789% | 4.868% |
| model 16 at 64 | 2.690% | 5.272% |
| model 16 at 0 | 0.575% | 4.271% |
| model 16 at 127 | 2.768% | 6.435% |
| model 17 at 64 | 3.148% | 5.730% |
| model 17 at 0 | 0.600% | 4.550% |
| model 17 at 127 | 1.851% | 5.522% |
| same-key retriggers, the heaviest model | 5.086% | 10.213% |
| the heaviest model, 8 voices | 30.384% | 35.660% |

Worst observed block: **35.660%** (the heaviest model, 8 voices), the maximum across every measured setting/workload and every supplied capture. Peaks are never averaged or reduced to a median. This is an observed spike, not a proven worst-case bound for untested settings or live projects.

Recommended mean and worst observed block peak: each at most 3.5%. Exceeding the recommendation is a warning, not a catalog rejection. **Warning: Mean 3.989% of the block budget for model 8 at 127 is above the recommended 3.5%; make it lighter where you can (docs/profiling.md); Worst observed block peak 35.660% of the block budget across all measured workloads is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, stock's drum renderer, its amplitude envelope or mixing, or settings between the levels. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `2f2776fcf107a24791c1f0b0310c0eb9b39979f0aa0aee6bf3e828e70eebb516`.

Compile report SHA-256: `80c17039dc3b10b42b790a94c3fd05a43dbeed91c37e925d3c69e2e6024652c4`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
