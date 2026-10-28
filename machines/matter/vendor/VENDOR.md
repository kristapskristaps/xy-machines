# Matter DSP provenance

Elements by Émilie Gillet, MIT: https://github.com/pichenettes/eurorack/tree/f4cfa7697095a28d292bff758beb51deb49b5a86/elements
Pinned source commit: `f4cfa7697095a28d292bff758beb51deb49b5a86`. Relevant sources: dsp/resonator.cc, dsp/exciter.cc, dsp/tube.cc, and resources.cc. stmlib reference revision: `e3bd7c9cc00e4364166f9905c0509b6ffd0535ec`.

resources.h preserves Elements' noise and strike samples, modal stiffness and other response tables. These are Mutable Instruments resources distributed under MIT, not Teenage Engineering stock content.

numeric.h adapts high-precision fixed-point routines from https://github.com/DMDaudio/plaits-on-models/tree/03899cc1d02656e0048e7d07df14e4001621fc39/engines at `03899cc1d02656e0048e7d07df14e4001621fc39`, with SDK multiply primitives and portable defined arithmetic. Both licences are preserved in LICENSE.

The adapted multiply follows the SDK's saturating Q31 contract. Coefficient
mantissas are positive, and resonator/bow states are bounded below full scale,
so the upstream wrapping `INT_MIN * INT_MIN` corner is unreachable in this
engine. Removing that special branch also avoids a condition-register allocation
failure in the pinned Blackfin compiler. Independent coefficient and audio-bound
tests cover the engine's actual operand range.

engine.cpp is a fixed-point adaptation of the modal resonator, eight bowed feedback modes and six exciter families. Changes include Q31 damping coefficient precision, 44.1 kHz timing, 16-bit bowed delay storage, bounded state/summing and output limiting, resource leases and a single selected exciter. It omits the original simultaneous exciter mixer and additional resonator/effect modes. Independent double-precision tests check coefficients and pickup recurrence. The XY display and integration are new code.

VENDOR.json records hashes of adapted vendor files. No firmware or stock artwork is included.
