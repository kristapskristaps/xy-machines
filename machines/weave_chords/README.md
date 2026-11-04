# Choir

Five-note chord voicings, eleven banks and a dedicated root output.

Standalone extraction of Weave’s `chords` model from optimized revision `957afdbac6f2ef9916991366e24517f1483dc0e4`. It preserves that model’s audio path, output gain, clock and existing stock envelope behavior. The original combined Weave engine is retired; its source remains in Git history. This machine has its own saved ID `weave_chords`.

Four controls map directly to the selected Weave model’s Harmonic, Timbre, Morph and Output. The model selector and the inapplicable Strike/Mode/Seed controls are omitted. No other synthesis model is selectable.

| Control | Role |
| --- | --- |
| Chord | Chord-bank selection and blending across the eleven banks. |
| Voicing | Five-oscillator inversion/voicing. |
| Colour | Prepared organ and wavetable colour. |
| Root | Crossfade from the complete voicing to the root-only auxiliary. |

The original retained vector animation reacts to notes and controls, settles when idle, and respects reduced motion. It illustrates the mechanism; no firmware or stock artwork is included.

Run `./xy test weave_chords --native` or `./xy preview weave_chords --watch` from the collection root. Hardware CPU qualification and normal-playback acceptance for this new ID remain pending. Earlier Weave measurements are historical evidence only.

DSP derives from Plaits by Émilie Gillet and the attributed integer port under MIT. See LICENSE and vendor/VENDOR.md.

The fourth control is **Root**: Crossfade from the complete chord to its root-only alternate. It is not volume. The inherited selector is main-only below about 43%, blends around 43–57%, and is alternate-only above about 57%. Compare the two endpoints to hear the difference.
