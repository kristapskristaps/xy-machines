# Tandem

Two coupled virtual-analog oscillators with sync and detune.

Standalone extraction of Weave’s `analog` model from optimized revision `957afdbac6f2ef9916991366e24517f1483dc0e4`. It preserves that model’s audio path, output gain, clock and existing stock envelope behavior. The original combined Weave engine is retired; its source remains in Git history. This machine has its own saved ID `tandem`.

Four controls map directly to the selected Weave model’s Harmonic, Timbre, Morph and Output. The model selector and the inapplicable Strike/Mode/Seed controls are omitted. No other synthesis model is selectable.

| Control | Role |
| --- | --- |
| Harmonic | Detuning between the oscillator branches, with flat regions around musical intervals. |
| Timbre | Oscillator synchronization amount. |
| Morph | Wave shape and pulse width. |
| Output | Crossfade between the main oscillator mix and auxiliary sync output. |

The original retained vector animation reacts to notes and controls, settles when idle, and respects reduced motion. It illustrates the mechanism; no firmware or stock artwork is included.

Run `./xy test tandem --native` or `./xy preview tandem --watch` from the collection root. Hardware CPU qualification and normal-playback acceptance for this new ID remain pending. Earlier Weave measurements are historical evidence only.

DSP derives from Plaits by Émilie Gillet and the attributed integer port under MIT. See LICENSE and vendor/VENDOR.md.
