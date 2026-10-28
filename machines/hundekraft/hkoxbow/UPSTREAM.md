# Source provenance

Source: https://hundekraft.github.io/op1-workshop/source/hundekraft-3.0.3-source.tar.gz
SHA-256: 2f745e3858f9fabf8e653ecfe3511679b47f795ff09f13e9bd5590e26995b1d7

The DSP is ported from `synth_sdk/plugins/oxbow`. Original notices follow.

oxbow.c

/* Oxbow: zvenson's OberVave, an 8-voice American poly of 1979-84 in the OB-X / OB-8 manner from his FM-1 firmware, as a
 * kennel synth plugin (loaded from oxbow.knl). Per voice: two oscillators (saw, pulse, saw+pulse; OSC 2 with hard
 * sync, X-MOD into OSC 1, the filter envelope on its pitch), noise, drive, a state-variable filter (LP 12, HP 12,
 * BP 12, notch, LP 24, SEM morph), a filter ADSR with RC curves and the stock T2 amp envelope, one LFO, and each voice card's own
 * small tune, cutoff and envelope offsets plus a slow pitch drift ("vintage").
 *
 * blue   (knob 0) CUTOFF
 * green  (knob 1) RESONANCE
 * white  (knob 2) ENV AMOUNT: the filter envelope, bipolar (middle = none)
 * orange (knob 3) DETUNE: OSC 2 against OSC 1, 0 .. +1/2 semitone
 * Pressed (page 2, saved with the sound):
 * blue   WAVE: saw, pulse, both, sync, cross-mod or fifth (word 5)
 * green  FILTER TYPE: LP 12, HP 12, BP 12, NOTCH, LP 24, SEM (word 6)
 * white  DRIVE: drive before the filter (word 7)
 *
 * The OP-1 allocates six voices and provides pitch, glide and the sole amplitude
 * envelope (T2). No internal programs or second amp envelope remain. The original
 * filter envelope and per-card drift remain; normal OP-1 presets hold sound choices.
 * Instrument state (the LFO, the cutoff and resonance slew) advances once per block, from the first
 * voice rendered in it (voices render in ascending order, as EVA does).
 * Synthesis runs at 22.05 kHz with control steps every eight internal samples;
 * pitch/control timing matches the native 44.1 kHz interface. Filter coefficients
 * use the internal sample rate; four-point interpolation reconstructs each output
 * block. Neutral voice constants replace the former program-bank setup. T2 is
 * still the sole amplitude envelope.
 *
 * From OberVave (github.com/zvenson/obervave a2fb3d0, firmware/src/obv_engine.h), the voice code nearly unchanged.
 * Copyright (C) 2026 zvenson (the synth), 2026 Hundekraft (the kennel port). Used in kennel with zvenson's
 * permission (2026-10-15). GPL-3.0-or-later with the OP-1 firmware permission (see LICENSE and NOTICE).
 */

Private development only. Hardware acceptance and author clarification of the OP-XY firmware exception remain pending.
