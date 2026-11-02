# Weave DSP provenance

Original Plaits by Émilie Gillet, MIT: https://github.com/pichenettes/eurorack

Integer source: https://github.com/DMDaudio/plaits-on-models/tree/03899cc1d02656e0048e7d07df14e4001621fc39/engines
at commit `03899cc1d02656e0048e7d07df14e4001621fc39`, based on gdeo607/digi1_mods. The fifteen-model selection and factory FM banks come from that port. See LICENSE and the original source headers for all notices.

Changes for Open-XY: portable C++11 types and namespace; SDK fractional multiply helpers replacing EMAC assembly (including saturating MIN × MIN instead of its original modular overflow); defined unsigned wrapping for signed shifts and oscillator phase; bounded wavetable coordinate smoothing; corrected sine-LFO peak overflow. The fixture bank retains the original 60 deterministic renders from the unmodified integer source. Forty-one original/corrected cases remain bit-exact; nineteen changed cases pin the bounded-quality adaptation. One all-maximum FM fixture corrects the original exponential overflow. The XY adaptation uses four particles (with total impulse density preserved), four spread swarm oscillators, twelve modal partials and one internal string per external voice. Chords retains five-note voicings and FM retains all six operators and 96 patches. Wavetable Hermite interpolation uses direct Q31 products with at most four Q8 table units of error, independently swept against the original polynomial. The limiter and FM soft clip use a bounded reciprocal divide, checked against exact division for every divisor in their domain and randomized numerators. macro_division.h is new adaptation code, not an upstream file. An independent 65,536-point polynomial sweep verifies that fix. Regression tests cover the corrected cases separately.

The surrounding engine owns per-voice state, maps XY controls, runs the DSP at 48 kHz and resamples to 44.1 kHz with a fixed-point 24-tap polyphase converter. Visuals, integration and resampler are new code. No Teenage Engineering firmware or stock artwork is included.

VENDOR.json records SHA-256 hashes of the adapted vendor files, rather than claiming they are unmodified upstream copies.

Secondary controls added by Open-XY: zero-compatible stock/soft/mid/hard
excitation presets for drums, Modal and String; held Swarm density and held
six-op gate. Excitation work is at block/note rate, except selecting the shift
in String's existing pluck-noise generator. Swarm's held envelope reuses its
existing block-rate work. The surrounding engine optionally reseeds the
existing RNG at note/model triggers; it does not reset ringing DSP history.
All sixty existing vendor fixtures retain their previous expected output with
the new slots at zero. No LPG or additional audio processing stage is added.

Block interpolation uses literal signed division for source lengths 32 and 16,
the six-op renderer's doubled length 64, and internal 8/4-sample chunks.
Internal 12-sample chunks use a Q31 reciprocal with exact remainder correction;
other supported lengths retain ordinary division. This preserves truncation
toward zero for negative ramps and all existing render fixtures. The block
helper is adaptation code in `macro_division.h`; its tests cover lengths 1–64,
full-range signed edges and randomized numerators.
Single-operator FM rendering remains inlined at its routing-specific call
sites, so source/add choices do not become branches inside every sample.

Coefficient normalization uses a 32-entry De Bruijn lookup with the same
leading-zero results, including the original zero case. Modal's reciprocal
seed reuses the existing division table and one refinement, followed by an
exact remainder correction. Tests compare all 32,768 possible seed divisors
against integer division; the filter coefficients and render arithmetic retain
their previous values.
