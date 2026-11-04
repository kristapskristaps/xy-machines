# Crease

A slope oscillator pressed through a prepared shaper and wavefolder.

Standalone extraction of Weave’s `shaper` model from optimized revision `957afdbac6f2ef9916991366e24517f1483dc0e4`. It preserves that model’s audio path, output gain, clock and existing stock envelope behavior. The original combined Weave engine is retired; its source remains in Git history. This machine has its own saved ID `weave_shaper`.

Four controls map directly to the selected Weave model’s Harmonic, Timbre, Morph and Output. The model selector and the inapplicable Strike/Mode/Seed controls are omitted. No other synthesis model is selectable.

| Control | Role |
| --- | --- |
| Harmonic | Waveshaper curve. |
| Timbre | Drive into the composed shaper/folder transfer. |
| Morph | Overtone shaping. |
| Alt mix | Crossfade toward the sine auxiliary. |

The original retained vector animation reacts to notes and controls, settles when idle, and respects reduced motion. It illustrates the mechanism; no firmware or stock artwork is included.

Run `./xy test weave_shaper --native` or `./xy preview weave_shaper --watch` from the collection root. Hardware CPU qualification and normal-playback acceptance for this new ID remain pending. Earlier Weave measurements are historical evidence only.

DSP derives from Plaits by Émilie Gillet and the attributed integer port under MIT. See LICENSE and vendor/VENDOR.md.

The fourth control is **Alt mix**: Crossfade between the main waveshaper and its alternate transfer/sine output. It is not volume. The inherited selector is main-only below about 43%, blends around 43–57%, and is alternate-only above about 57%. Compare the two endpoints to hear the difference.
