# vader: hardware CPU measurement

Machine commit: `16ee2c863de8552ea9769f83ca869e4b5010c1ba`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 67 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **3.879%** of the stock 580,352-cycle block budget. Midpoint run: 3.011%. Every control at its heaviest position: 2.237%. Heaviest single setting: Drive 37.5%.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 3.011% |
| Punch 0.0% | 2.252% |
| Punch 12.5% | 2.254% |
| Punch 25.0% | 2.260% |
| Punch 37.5% | 2.254% |
| Punch 50.0% | 2.261% |
| Punch 62.5% | 2.255% |
| Punch 75.0% | 2.261% |
| Punch 87.5% | 2.255% |
| Punch 100.0% | 2.261% |
| Fall 0.0% | 2.238% |
| Fall 12.5% | 2.251% |
| Fall 25.0% | 2.249% |
| Fall 37.5% | 2.267% |
| Fall 50.0% | 2.271% |
| Fall 62.5% | 2.300% |
| Fall 75.0% | 2.333% |
| Fall 87.5% | 2.393% |
| Fall 100.0% | 2.443% |
| Drive 0.0% | 2.261% |
| Drive 12.5% | 3.354% |
| Drive 25.0% | 3.752% |
| Drive 37.5% | 3.879% |
| Drive 50.0% | 3.738% |
| Drive 62.5% | 3.790% |
| Drive 75.0% | 3.799% |
| Drive 87.5% | 3.806% |
| Drive 100.0% | 3.818% |
| Shape round | 2.257% |
| Shape tube | 2.274% |
| Shape diode | 2.266% |
| Shape fold | 2.296% |
| Click 0.0% | 2.254% |
| Click 12.5% | 2.263% |
| Click 25.0% | 2.257% |
| Click 37.5% | 2.264% |
| Click 50.0% | 2.257% |
| Click 62.5% | 2.264% |
| Click 75.0% | 2.257% |
| Click 87.5% | 2.264% |
| Click 100.0% | 2.257% |
| Sub 0.0% | 2.260% |
| Sub 12.5% | 2.254% |
| Sub 25.0% | 2.261% |
| Sub 37.5% | 2.254% |
| Sub 50.0% | 2.261% |
| Sub 62.5% | 2.254% |
| Sub 75.0% | 2.260% |
| Sub 87.5% | 2.254% |
| Sub 100.0% | 2.261% |
| Tone 0.0% | 2.243% |
| Tone 12.5% | 2.260% |
| Tone 25.0% | 2.240% |
| Tone 37.5% | 2.245% |
| Tone 50.0% | 2.240% |
| Tone 62.5% | 2.246% |
| Tone 75.0% | 2.240% |
| Tone 87.5% | 2.260% |
| Tone 100.0% | 2.254% |
| Boost 0.0% | 2.260% |
| Boost 12.5% | 2.240% |
| Boost 25.0% | 2.246% |
| Boost 37.5% | 2.240% |
| Boost 50.0% | 2.246% |
| Boost 62.5% | 2.240% |
| Boost 75.0% | 2.246% |
| Boost 87.5% | 2.240% |
| Boost 100.0% | 2.261% |
| combined (as read from the reports): Punch 50.0%, Fall 100.0%, Drive 37.5%, Shape fold, Click 37.5%, Sub 25.0%, Tone 12.5%, Boost 100.0% | 2.237% |

Recommended: at most 3.5%. **Warning: 3.879% of the block budget at Drive 37.5% is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `0b5b9570f04d0028d3c19a9a373ff91a12a6f288cd18179ab931b198f52c0809`.

Compile report SHA-256: `9e2971b041e82946f1cd66ca2e868f1ffbfca7d3147f387d732c4939c05d8d75`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.

Loaded collection SHA-256: `9d484556b54cbdf18ea6ca5e9325bc012ff24ab30a0cffb5272e8eb7f7957aed`. Firmware SHA-256 above identifies its resident kernel.
