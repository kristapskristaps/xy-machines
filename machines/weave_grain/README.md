# Sift

Paired grainlet formants and a Z-oscillator auxiliary.

Standalone extraction of Weave’s `grain` model from optimized revision `957afdbac6f2ef9916991366e24517f1483dc0e4`. It preserves that model’s audio path, output gain, clock and existing stock envelope behavior. The original Weave and its saved ID remain available. This machine has its own saved ID `weave_grain`.

Four controls map directly to the selected Weave model’s Harmonic, Timbre, Morph and Output. The model selector and the inapplicable Strike/Mode/Seed controls are omitted. No other synthesis model is selectable.

| Control | Role |
| --- | --- |
| Harmonic | The relationship between the two formant frequencies and the auxiliary mode. |
| Timbre | Formant frequency. |
| Morph | Grainlet/formant shape and bleed. |
| Output | Crossfade between the paired grainlets and Z-oscillator auxiliary. |

The original retained vector animation reacts to notes and controls, settles when idle, and respects reduced motion. It illustrates the mechanism; no firmware or stock artwork is included.

Run `./xy test weave_grain --native` or `./xy preview weave_grain --watch` from the collection root. Hardware CPU qualification and normal-playback acceptance for this new ID remain pending. Earlier Weave measurements are historical evidence only.

DSP derives from Plaits by Émilie Gillet and the attributed integer port under MIT. See LICENSE and vendor/VENDOR.md.
