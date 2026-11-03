# vader: hardware CPU measurement

Machine commit: `6cf46e53d30cb4e3b4aa37b2476bb1c07107a367`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 67 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **3.755%** of the stock 580,352-cycle block budget. Midpoint run: 2.927%. Every control at its heaviest position: 2.142%. Heaviest single setting: Drive 100.0%.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 2.927% |
| Punch 0.0% | 2.153% |
| Punch 12.5% | 2.156% |
| Punch 25.0% | 2.159% |
| Punch 37.5% | 2.158% |
| Punch 50.0% | 2.161% |
| Punch 62.5% | 2.159% |
| Punch 75.0% | 2.162% |
| Punch 87.5% | 2.159% |
| Punch 100.0% | 2.162% |
| Fall 0.0% | 2.142% |
| Fall 12.5% | 2.150% |
| Fall 25.0% | 2.153% |
| Fall 37.5% | 2.164% |
| Fall 50.0% | 2.174% |
| Fall 62.5% | 2.199% |
| Fall 75.0% | 2.222% |
| Fall 87.5% | 2.269% |
| Fall 100.0% | 2.335% |
| Drive 0.0% | 2.159% |
| Drive 12.5% | 3.252% |
| Drive 25.0% | 3.568% |
| Drive 37.5% | 3.754% |
| Drive 50.0% | 3.684% |
| Drive 62.5% | 3.731% |
| Drive 75.0% | 3.688% |
| Drive 87.5% | 3.656% |
| Drive 100.0% | 3.755% |
| Shape round | 2.150% |
| Shape tube | 2.193% |
| Shape diode | 2.200% |
| Shape fold | 2.170% |
| Click 0.0% | 2.156% |
| Click 12.5% | 2.161% |
| Click 25.0% | 2.158% |
| Click 37.5% | 2.161% |
| Click 50.0% | 2.159% |
| Click 62.5% | 2.161% |
| Click 75.0% | 2.159% |
| Click 87.5% | 2.162% |
| Click 100.0% | 2.158% |
| Sub 0.0% | 2.158% |
| Sub 12.5% | 2.155% |
| Sub 25.0% | 2.158% |
| Sub 37.5% | 2.156% |
| Sub 50.0% | 2.158% |
| Sub 62.5% | 2.156% |
| Sub 75.0% | 2.159% |
| Sub 87.5% | 2.155% |
| Sub 100.0% | 2.158% |
| Tone 0.0% | 2.156% |
| Tone 12.5% | 2.158% |
| Tone 25.0% | 2.179% |
| Tone 37.5% | 2.158% |
| Tone 50.0% | 2.156% |
| Tone 62.5% | 2.158% |
| Tone 75.0% | 2.156% |
| Tone 87.5% | 2.173% |
| Tone 100.0% | 2.155% |
| Boost 0.0% | 2.141% |
| Boost 12.5% | 2.138% |
| Boost 25.0% | 2.141% |
| Boost 37.5% | 2.138% |
| Boost 50.0% | 2.141% |
| Boost 62.5% | 2.138% |
| Boost 75.0% | 2.139% |
| Boost 87.5% | 2.138% |
| Boost 100.0% | 2.159% |
| combined (as read from the reports): Punch 75.0%, Fall 100.0%, Drive 100.0%, Shape diode, Click 87.5%, Sub 75.0%, Tone 25.0%, Boost 100.0% | 2.142% |

Recommended: at most 3.5%. **Warning: 3.755% of the block budget at Drive 100.0% is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `0b5b9570f04d0028d3c19a9a373ff91a12a6f288cd18179ab931b198f52c0809`.

Compile report SHA-256: `e9a672835f954189e99b4b9146501991b3a470c29a8dcce6751df33b502b8bce`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.

Loaded collection SHA-256: `9aa09831641a09f8765d82d636c50f6b8338106af90f03f987987178cc4f3f88`. Firmware SHA-256 above identifies its resident kernel.
