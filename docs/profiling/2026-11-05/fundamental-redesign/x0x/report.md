# x0x: hardware CPU measurement

Machine commit: `d07d8c9677b692ca044b01ed8d9018c05e9064a8`. Benchmark: `opxy-drum-callback-v1`. Scope: the kit's voices through the SDK's drum adapter, not stock's drum renderer.

Heaviest single voice, from one capture: **4.347%** of the stock 580,352-cycle block budget (model 0 at 127). Heaviest at the middle: 4.276%. Whole kit: its own keys 20.505%, same-key retriggers 6.006%, full polyphony of the heaviest voice 33.199%.

| Run | Mean block budget |
| --- | ---: |
| the kit's keys, 8 voices | 20.505% |
| model 0 at 64 | 3.958% |
| model 0 at 0 | 0.934% |
| model 0 at 127 | 4.347% |
| model 1 at 64 | 4.276% |
| model 1 at 0 | 0.916% |
| model 1 at 127 | 3.920% |
| model 2 at 64 | 2.928% |
| model 2 at 0 | 1.007% |
| model 2 at 127 | 2.966% |
| model 3 at 64 | 2.324% |
| model 3 at 0 | 0.398% |
| model 3 at 127 | 4.039% |
| model 4 at 64 | 3.594% |
| model 4 at 0 | 0.848% |
| model 4 at 127 | 3.650% |
| model 5 at 64 | 3.595% |
| model 5 at 0 | 0.897% |
| model 5 at 127 | 3.658% |
| model 6 at 64 | 3.600% |
| model 6 at 0 | 0.841% |
| model 6 at 127 | 3.664% |
| model 7 at 64 | 1.918% |
| model 7 at 0 | 0.385% |
| model 7 at 127 | 2.553% |
| model 8 at 64 | 3.207% |
| model 8 at 0 | 0.969% |
| model 8 at 127 | 4.237% |
| model 9 at 64 | 3.749% |
| model 9 at 0 | 4.259% |
| model 9 at 127 | 4.207% |
| model 10 at 64 | 4.059% |
| model 10 at 0 | 1.900% |
| model 10 at 127 | 4.219% |
| model 11 at 64 | 3.705% |
| model 11 at 0 | 0.833% |
| model 11 at 127 | 3.677% |
| model 12 at 64 | 3.128% |
| model 12 at 0 | 0.339% |
| model 12 at 127 | 3.258% |
| model 13 at 64 | 1.984% |
| model 13 at 0 | 0.334% |
| model 13 at 127 | 2.049% |
| model 14 at 64 | 2.583% |
| model 14 at 0 | 0.581% |
| model 14 at 127 | 2.646% |
| model 15 at 64 | 2.586% |
| model 15 at 0 | 0.569% |
| model 15 at 127 | 2.654% |
| model 16 at 64 | 2.586% |
| model 16 at 0 | 0.563% |
| model 16 at 127 | 2.655% |
| model 17 at 64 | 3.458% |
| model 17 at 0 | 0.633% |
| model 17 at 127 | 2.184% |
| same-key retriggers, the heaviest model | 6.006% |
| the heaviest model, 8 voices | 33.199% |

Recommended: at most 3.5%. **Warning: 4.347% of the block budget for model 0 at 127 is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, stock's drum renderer, its amplitude envelope or mixing, or settings between the levels. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `881167fb726a3020d0e4ed00591776bfeb554a842ef79b91e30ed9141e61eb7d`.

Compile report SHA-256: `a917ed58dd9f46a77628bb5655e0eafea31d512be54aae32fe0743203367d1c0`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.
