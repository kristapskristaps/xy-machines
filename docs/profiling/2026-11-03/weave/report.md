# weave: hardware CPU measurement

Machine commit: `6cf46e53d30cb4e3b4aa37b2476bb1c07107a367`. Benchmark: `opxy-synth-c8-sweep-v1`. Scope: whole voice, including oscillator/filter.

Heaviest of the midpoint run, 185 measured settings (each control through its positions with the others at their defaults) and every control at its heaviest position, from one capture: **16.619%** of the stock 580,352-cycle block budget. Midpoint run: 6.323%. Every control at its heaviest position: 6.322%. Heaviest single setting: Model modal.

| Setting | Mean block budget |
| --- | ---: |
| midpoint (stock) | 6.323% |
| Model shape | 6.365% |
| Model fm | 9.397% |
| Model noise | 7.407% |
| Model particle | 6.851% |
| Model kick | 5.010% |
| Model snare | 4.894% |
| Model hihat | 5.389% |
| Model grain | 8.412% |
| Model chord | 12.186% |
| Model swarm | 8.479% |
| Model wavetable | 15.108% |
| Model analog | 8.498% |
| Model modal | 16.619% |
| Model string | 11.566% |
| Model six op | 11.406% |
| Harmonic 0.0% | 6.339% |
| Harmonic 12.5% | 6.365% |
| Harmonic 25.0% | 6.360% |
| Harmonic 37.5% | 6.357% |
| Harmonic 50.0% | 6.331% |
| Harmonic 62.5% | 6.377% |
| Harmonic 75.0% | 6.334% |
| Harmonic 87.5% | 6.377% |
| Harmonic 100.0% | 6.352% |
| Timbre 0.0% | 6.235% |
| Timbre 12.5% | 6.279% |
| Timbre 25.0% | 6.314% |
| Timbre 37.5% | 6.300% |
| Timbre 50.0% | 6.361% |
| Timbre 62.5% | 6.334% |
| Timbre 75.0% | 6.397% |
| Timbre 87.5% | 6.377% |
| Timbre 100.0% | 6.441% |
| Morph 0.0% | 6.311% |
| Morph 12.5% | 6.351% |
| Morph 25.0% | 6.316% |
| Morph 37.5% | 6.365% |
| Morph 50.0% | 6.326% |
| Morph 62.5% | 6.363% |
| Morph 75.0% | 6.302% |
| Morph 87.5% | 6.346% |
| Morph 100.0% | 6.270% |
| Output 0.0% | 6.365% |
| Output 12.5% | 6.323% |
| Output 25.0% | 6.363% |
| Output 37.5% | 6.326% |
| Output 50.0% | 8.281% |
| Output 62.5% | 7.433% |
| Output 75.0% | 7.442% |
| Output 87.5% | 7.431% |
| Output 100.0% | 7.443% |
| Strike stock | 6.322% |
| Strike soft | 6.365% |
| Strike mid | 6.320% |
| Strike hard | 6.363% |
| Mode normal | 6.325% |
| Mode hold | 6.360% |
| Seed free | 6.325% |
| Seed 1 | 6.361% |
| Seed 2 | 6.326% |
| Seed 3 | 6.365% |
| Seed 4 | 6.323% |
| Seed 5 | 6.366% |
| Seed 6 | 6.322% |
| Seed 7 | 6.365% |
| Seed 8 | 6.323% |
| Seed 9 | 6.363% |
| Seed 10 | 6.325% |
| Seed 11 | 6.361% |
| Seed 12 | 6.326% |
| Seed 13 | 6.365% |
| Seed 14 | 6.325% |
| Seed 15 | 6.365% |
| Seed 16 | 6.322% |
| Seed 17 | 6.366% |
| Seed 18 | 6.325% |
| Seed 19 | 6.361% |
| Seed 20 | 6.328% |
| Seed 21 | 6.361% |
| Seed 22 | 6.325% |
| Seed 23 | 6.365% |
| Seed 24 | 6.325% |
| Seed 25 | 6.365% |
| Seed 26 | 6.322% |
| Seed 27 | 6.366% |
| Seed 28 | 6.323% |
| Seed 29 | 6.365% |
| Seed 30 | 6.325% |
| Seed 31 | 6.360% |
| Seed 32 | 6.326% |
| Seed 33 | 6.365% |
| Seed 34 | 6.325% |
| Seed 35 | 6.365% |
| Seed 36 | 6.323% |
| Seed 37 | 6.366% |
| Seed 38 | 6.320% |
| Seed 39 | 6.366% |
| Seed 40 | 6.325% |
| Seed 41 | 6.361% |
| Seed 42 | 6.326% |
| Seed 43 | 6.363% |
| Seed 44 | 6.326% |
| Seed 45 | 6.363% |
| Seed 46 | 6.326% |
| Seed 47 | 6.365% |
| Seed 48 | 6.322% |
| Seed 49 | 6.366% |
| Seed 50 | 6.323% |
| Seed 51 | 6.361% |
| Seed 52 | 6.326% |
| Seed 53 | 6.363% |
| Seed 54 | 6.328% |
| Seed 55 | 6.363% |
| Seed 56 | 6.323% |
| Seed 57 | 6.363% |
| Seed 58 | 6.322% |
| Seed 59 | 6.363% |
| Seed 60 | 6.323% |
| Seed 61 | 6.365% |
| Seed 62 | 6.325% |
| Seed 63 | 6.360% |
| Seed 64 | 6.326% |
| Seed 65 | 6.363% |
| Seed 66 | 6.325% |
| Seed 67 | 6.365% |
| Seed 68 | 6.323% |
| Seed 69 | 6.365% |
| Seed 70 | 6.322% |
| Seed 71 | 6.365% |
| Seed 72 | 6.323% |
| Seed 73 | 6.363% |
| Seed 74 | 6.326% |
| Seed 75 | 6.360% |
| Seed 76 | 6.325% |
| Seed 77 | 6.365% |
| Seed 78 | 6.325% |
| Seed 79 | 6.365% |
| Seed 80 | 6.322% |
| Seed 81 | 6.365% |
| Seed 82 | 6.322% |
| Seed 83 | 6.361% |
| Seed 84 | 6.325% |
| Seed 85 | 6.361% |
| Seed 86 | 6.326% |
| Seed 87 | 6.363% |
| Seed 88 | 6.325% |
| Seed 89 | 6.365% |
| Seed 90 | 6.325% |
| Seed 91 | 6.365% |
| Seed 92 | 6.323% |
| Seed 93 | 6.363% |
| Seed 94 | 6.326% |
| Seed 95 | 6.361% |
| Seed 96 | 6.328% |
| Seed 97 | 6.360% |
| Seed 98 | 6.326% |
| Seed 99 | 6.365% |
| Seed 100 | 6.323% |
| Seed 101 | 6.363% |
| Seed 102 | 6.323% |
| Seed 103 | 6.365% |
| Seed 104 | 6.322% |
| Seed 105 | 6.361% |
| Seed 106 | 6.331% |
| Seed 107 | 6.358% |
| Seed 108 | 6.326% |
| Seed 109 | 6.363% |
| Seed 110 | 6.322% |
| Seed 111 | 6.365% |
| Seed 112 | 6.325% |
| Seed 113 | 6.365% |
| Seed 114 | 6.323% |
| Seed 115 | 6.363% |
| Seed 116 | 6.326% |
| Seed 117 | 6.360% |
| Seed 118 | 6.326% |
| Seed 119 | 6.363% |
| Seed 120 | 6.325% |
| Seed 121 | 6.366% |
| Seed 122 | 6.323% |
| Seed 123 | 6.365% |
| Seed 124 | 6.322% |
| Seed 125 | 6.365% |
| Seed 126 | 6.325% |
| Seed 127 | 6.361% |
| combined (as read from the reports): Model modal, Harmonic 62.5%, Timbre 100.0%, Morph 37.5%, Output 50.0%, Strike soft, Mode hold, Seed 5 | 6.322% |

Recommended: at most 3.5%. **Warning: 16.619% of the block budget at Model modal is above the recommended 3.5%; make it lighter where you can (docs/profiling.md).**

This is a synthetic measurement on the USB-MIDI command endpoint, not total device CPU, maximum polyphony or other combinations of control settings. Graphics are excluded. Callback tests exclude sample preparation, allocation, stock mixing and voice processing; they include stopwatch-call overhead and may include interrupt time. See docs/profiling.md for the versioned workloads.

Firmware SHA-256: `0b5b9570f04d0028d3c19a9a373ff91a12a6f288cd18179ab931b198f52c0809`.

Compile report SHA-256: `5f77500f98e749456ce7ecf9613091a6621bc4773335e0f778b72360c19c01e5`.

Attribution uses the user-confirmed installed package, expected case order and exact report count; the device does not attest its firmware hash. Raw captures remain private because they contain device identity.

Loaded collection SHA-256: `cf7c9d41c55dfc25782181bca4e2d92d0936ad28fcd01e5aa40cc8badc4bf004`. Firmware SHA-256 above identifies its resident kernel.
