# Source provenance

Source: https://hundekraft.github.io/op1-workshop/source/hundekraft-3.0.3-source.tar.gz
SHA-256: 2f745e3858f9fabf8e653ecfe3511679b47f795ff09f13e9bd5590e26995b1d7

The DSP is ported from `synth_sdk/plugins/reverb`. Original notices follow.

reverb.c

/* "reverb" - fixed-point port of the Mutable Instruments Elements reverb (elements/dsp/fx/reverb.h):
 * Griesinger topology from the Dattorro paper, 4 input allpasses (the first one smeared by an LFO) into a loop
 * of 2 x (2 allpasses + low-pass + delay), long delay modulated by a slow LFO. Delay lengths scaled from
 * 32 kHz to 44.1 kHz so the room keeps its size. Mono: both loop outputs are summed.
 *   knob 0 (blue)  : time       decay, 0.35 .. 0.98 loop gain (Elements: 0.35 .. 0.95)
 *   knob 1 (green) : tone       loop low-pass 0.3 (dark) .. 0.95 (bright) (Elements: 0.7 .. 0.9)
 *   knob 2 (white) : diffusion  allpass gain 0.5 .. 0.75 (Elements: 0.55 .. 0.7)
 *   knob 3 (red)   : mix        dry .. wet          (green / red swapped in op1r 24)
 * Memory: 32 Ki int16 (64 KB, 16-bit like Elements) in this FX position's scratch buffer.
 * Original code (c) 2014 Emilie Gillet, MIT license. */

Private development only. Hardware acceptance and author clarification of the OP-XY firmware exception remain pending.
