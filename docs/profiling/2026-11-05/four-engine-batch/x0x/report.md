# x0x: hardware CPU measurement

Machine commit: `b6c7af888eabd7ffb63e22a0c9c1d30074a2db1b`. Benchmark: `opxy-drum-callback-v1`. Scope: the kit's voices through the SDK's drum adapter, not stock's drum renderer.

Heaviest single voice, from one capture: **5.521%** of the stock 580,352-cycle block budget (model 0 at 127). Heaviest at the middle: 5.098%. Whole kit: its own keys 26.952%, same-key retriggers 7.105%, full polyphony of the heaviest voice 42.138%.

| Run | Mean block budget |
| --- | ---: |
| the kit's keys, 8 voices | 26.952% |
| model 0 at 64 | 5.098% |
| model 0 at 0 | 1.173% |
| model 0 at 127 | 5.521% |
| model 1 at 64 | 5.016% |
| model 1 at 0 | 1.048% |
| model 1 at 127 | 4.816% |
| model 2 at 64 | 3.894% |
| model 2 at 0 | 1.019% |
| model 2 at 127 | 3.935% |
| model 3 at 64 | 2.890% |
| model 3 at 0 | 0.404% |
| model 3 at 127 | 5.103% |
| model 4 at 64 | 4.736% |
| model 4 at 0 | 1.099% |
| model 4 at 127 | 4.767% |
| model 5 at 64 | 4.738% |
| model 5 at 0 | 0.905% |
| model 5 at 127 | 4.775% |
| model 6 at 64 | 4.741% |
| model 6 at 0 | 1.094% |
| model 6 at 127 | 4.784% |
| model 7 at 64 | 1.872% |
| model 7 at 0 | 0.363% |
| model 7 at 127 | 2.568% |
| model 8 at 64 | 3.185% |
| model 8 at 0 | 0.969% |
| model 8 at 127 | 4.332% |
| model 9 at 64 | 3.807% |
| model 9 at 0 | 3.694% |
| model 9 at 127 | 4.237% |
| model 10 at 64 | 4.208% |
| model 10 at 0 | 1.976% |
| model 10 at 127 | 4.280% |
| model 11 at 64 | 3.984% |
| model 11 at 0 | 0.879% |
| model 11 at 127 | 4.086% |
| model 12 at 64 | 4.314% |
| model 12 at 0 | 0.377% |
| model 12 at 127 | 4.442% |
| model 13 at 64 | 2.902% |
| model 13 at 0 | 0.395% |
| model 13 at 127 | 2.988% |
| model 14 at 64 | 4.253% |
| model 14 at 0 | 0.818% |
| model 14 at 127 | 4.323% |
| model 15 at 64 | 4.234% |
| model 15 at 0 | 0.801% |
| model 15 at 127 | 4.306% |
| model 16 at 64 | 4.240% |
| model 16 at 0 | 0.793% |
| model 16 at 127 | 4.314% |
| model 17 at 64 | 3.516% |
| model 17 at 0 | 0.656% |
| model 17 at 127 | 2.179% |
| same-key retriggers, the heaviest model | 7.105% |
| the heaviest model, 8 voices | 42.138% |

Recommended: at most 3.5%. **Warning: 5.521% of the block budget for model 0 at 127 is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, stock's drum renderer, its amplitude envelope or mixing, or settings between the levels. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `3a6a97840a7a7e36c7cd64d66e61f330a04cef90eca323afc8e424773c990c3f`.

Compile report SHA-256: `fea427cfd9be83cd524409d00c66d15d512c13e15f7a823c6352a97c1f7804af`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
