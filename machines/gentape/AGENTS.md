# Gen Tape

Follow the root AGENTS.md and docs/development.md. Keep DSP, tests and render tools
inside this machine. Use the public opxy API and the pinned SDK without changes.
Run `./xy test gentape --native --cpu`; effects use the machine-local WAV renderer,
not `./xy preview`. See README.md for quality and combined-setting benchmarks.
Keep audio renders in ignored local/. Never claim measured CPU or hardware
acceptance from host results. Update QA.md when device-observable behavior changes.
