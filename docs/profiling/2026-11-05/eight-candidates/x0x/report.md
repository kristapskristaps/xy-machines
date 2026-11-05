# x0x: hardware CPU measurement

Machine commit: `c104047d4fb15f45ca258aa7a8b6a2006b84b3cf`. Benchmark: `opxy-drum-callback-v1`. Scope: the kit's voices through the SDK's drum adapter, not stock's drum renderer.

Heaviest single voice, from one capture: **5.344%** of the stock 580,352-cycle block budget (model 0 at 127). Heaviest at the middle: 4.894%. Whole kit: its own keys 26.813%, same-key retriggers 6.810%, full polyphony of the heaviest voice 40.822%.

| Run | Mean block budget |
| --- | ---: |
| the kit's keys, 8 voices | 26.813% |
| model 0 at 64 | 4.793% |
| model 0 at 0 | 1.122% |
| model 0 at 127 | 5.344% |
| model 1 at 64 | 4.894% |
| model 1 at 0 | 1.045% |
| model 1 at 127 | 4.825% |
| model 2 at 64 | 4.077% |
| model 2 at 0 | 1.027% |
| model 2 at 127 | 4.128% |
| model 3 at 64 | 2.707% |
| model 3 at 0 | 0.404% |
| model 3 at 127 | 4.764% |
| model 4 at 64 | 4.831% |
| model 4 at 0 | 1.111% |
| model 4 at 127 | 4.880% |
| model 5 at 64 | 4.834% |
| model 5 at 0 | 0.911% |
| model 5 at 127 | 4.889% |
| model 6 at 64 | 4.837% |
| model 6 at 0 | 1.106% |
| model 6 at 127 | 4.895% |
| model 7 at 64 | 1.865% |
| model 7 at 0 | 0.394% |
| model 7 at 127 | 2.585% |
| model 8 at 64 | 3.172% |
| model 8 at 0 | 1.022% |
| model 8 at 127 | 4.350% |
| model 9 at 64 | 3.803% |
| model 9 at 0 | 4.120% |
| model 9 at 127 | 4.263% |
| model 10 at 64 | 4.230% |
| model 10 at 0 | 1.988% |
| model 10 at 127 | 4.303% |
| model 11 at 64 | 4.413% |
| model 11 at 0 | 0.885% |
| model 11 at 127 | 4.514% |
| model 12 at 64 | 3.926% |
| model 12 at 0 | 0.362% |
| model 12 at 127 | 4.077% |
| model 13 at 64 | 3.301% |
| model 13 at 0 | 0.421% |
| model 13 at 127 | 3.388% |
| model 14 at 64 | 4.405% |
| model 14 at 0 | 0.829% |
| model 14 at 127 | 4.501% |
| model 15 at 64 | 4.407% |
| model 15 at 0 | 0.816% |
| model 15 at 127 | 4.501% |
| model 16 at 64 | 4.410% |
| model 16 at 0 | 0.810% |
| model 16 at 127 | 4.508% |
| model 17 at 64 | 3.540% |
| model 17 at 0 | 0.658% |
| model 17 at 127 | 2.194% |
| same-key retriggers, the heaviest model | 6.810% |
| the heaviest model, 8 voices | 40.822% |

Recommended: at most 3.5%. **Warning: 5.344% of the block budget for model 0 at 127 is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, stock's drum renderer, its amplitude envelope or mixing, or settings between the levels. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `9ef33641571f208415fe88344a1147b223a85b6a34e5bcf7faff192d0498f5b5`.

Compile report SHA-256: `54c0ffc152bd8655e9c4c57102129c9f44faac8ce360ac66693bbc8e8ccd2dcd`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
