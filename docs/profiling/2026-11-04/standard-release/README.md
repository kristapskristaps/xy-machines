# Standard SDK submission measurements

Source: `16ee2c863de8552ea9769f83ca869e4b5010c1ba`, with per-machine v1.1.0 tags.
SDK: upstream staging `b0244f06db40e388d2de3e7d6fb3f832a32a0a52`.
The full firmware was rebuilt from authenticated stock OS 1.2.1, transferred
with backup/readback, written to boot by the user, and confirmed normally booted
on saved/stopped LAB BOOT with USB audio off. A complete standard capture
received all 211 expected reports without protocol errors. Reports here use
the upstream schema and need no resident-collection schema extension.

The user explicitly accepted the above-3.5% recommendations for this initial
submission. Acidline, Mando and Vader retain those warnings; Halo and Rebound
are below the recommendation. These are callback/control-sweep measurements,
not total-project CPU or a full manual QA acceptance. Each machine's QA.md
remains the checklist for a complete device pass on the tagged commit.

X0X and Westfold are excluded from this submission: their newer local versions
were not the versions measured in the preceding qualification attempt. X0X's
newer eight-voice and DSP work is retained locally on its compatible SDK; the
Westfold compact-core redesign is local and has no new hardware CPU record.
The five Weave spin-offs remain excluded pending their startup investigation.

Raw captures, device metadata, backups and firmware packages remain outside Git.
