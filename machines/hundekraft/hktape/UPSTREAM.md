# Source provenance

Source: https://hundekraft.github.io/op1-workshop/source/hundekraft-3.0.3-source.tar.gz
SHA-256: 2f745e3858f9fabf8e653ecfe3511679b47f795ff09f13e9bd5590e26995b1d7

The DSP is ported from `synth_sdk/plugins/tape`. Original notices follow.

tape.c

/* "tape" - simple tape delay (custom FX, own screen display/tapeecho.svg: vintage tape echo).
 * Knobs in OP-1 order, matching the screen elements left to right:
 *   knob 0 (blue)  : age       erase head: darker + more saturated repeats (low-pass in the loop)
 *   knob 1 (green) : feedback  record head: 0 .. ~110 % (soft-saturated, self-oscillates safely)
 *   knob 2 (white) : time      play head / tape speed: 30 ms .. 1.45 s, glides like a speed change
 *   knob 3 (red)   : wow & flutter depth, capstan: 0.6 Hz wow up to ~+-1.5 % pitch + 7 Hz flutter up to
 *                    ~+-0.4 % on the read head (squared knob curve: subtle low half, seasick at max)
 *
 * Reuses the built-in delay's scratch buffer for this FX position (synth/drum): 64 Ki int16
 * samples ring buffer (128 KiB of the 172 KiB area).
 */

Private development only. Hardware acceptance and author clarification of the OP-XY firmware exception remain pending.
