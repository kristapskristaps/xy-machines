# Tandem

Two coupled virtual-analog oscillators with sync and detune.

Originally extracted from Weave’s `analog` model at revision `957afdbac6f2ef9916991366e24517f1483dc0e4`. It now prepares two independently tuned periodic components in bounded 16-point tiles and reads them directly at 44.1 kHz. The original detuning, PWM, shape, sync and gain maps define the components. Their edges use frequency-aware phase taps rather than four running BLEP oscillators. This intentionally changes the edge character and attack. The saved machine ID and stock envelope behavior remain unchanged. The retired combined Weave source remains in Git history.

Four controls map directly to the selected Weave model’s Harmonic, Timbre, Morph and Output. The model selector and the inapplicable Strike/Mode/Seed controls are omitted. No other synthesis model is selectable.

| Control | Role |
| --- | --- |
| Harmonic | Detuning between the oscillator branches, with flat regions around musical intervals. |
| Timbre | Oscillator synchronization amount. |
| Morph | Wave shape and pulse width. |
| Output | Full-range crossfade between the main oscillator mix and phase-aligned sync alternate. |

Both endpoints are prepared together in about 23 ms, followed by a 5.8 ms timbre crossfade. Harmonic, Timbre, Morph and pitch changes finish the current bounded preparation before starting the latest target. Output blends the ready endpoints directly, ramps over one 64-frame block and never rebuilds the waves. Its entire travel is active; the alternate root now shares the main root’s polarity. Playback never returns to the old expensive renderer. Pitch changes advance the oscillator every sample. Newly initialized voices begin with a quiet simple waveform while the first requested wave is prepared. These attack and modulation differences need a listening pass before release.

The original retained vector animation reacts to notes and controls, settles when idle, and respects reduced motion. It illustrates the mechanism; no firmware or stock artwork is included.

Run `./xy test tandem --native` or `./xy preview tandem --watch` from the collection root. Hardware CPU qualification and normal-playback acceptance for this new ID remain pending. Earlier Weave measurements are historical evidence only.

DSP derives from Plaits by Émilie Gillet and the attributed integer port under MIT. See LICENSE and vendor/VENDOR.md.

## Factory presets

Four native OP-XY presets ship with this machine: **Init**, **Warm analog**, **Octave pair**, **Crossed wires**. They use the same engine settings as the browser patches. `default.json` is Init; other presets have their own files. Stock AMP/filter/LFO settings start from the SDK’s public preset template.
