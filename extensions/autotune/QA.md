# Auto Tune device check

Record the SDK and extension revisions, firmware SHA-256, source and settings.
Use headphones for the microphone check.

1. Open a test project with an instrument pattern playing. With Aux 5 monitoring
   off, check that the pattern sounds normal.
2. On Aux 5 M1, select the mic and click the dark encoder. Sing a steady note
   slightly sharp and then flat. Listen for both to settle on the nearest
   chromatic pitch. Check a phrase with consonants and silence.
3. Select the 3.5 mm audio input. Monitor a 432 Hz tone and check for 440 Hz,
   then a 451 Hz tone and check for 440 Hz. Play left-only and right-only tones
   to check channel routing.
4. Hold Shift on Aux 5 M1. Check Tilt Mic, Tune, Retune and Amount in encoder
   order. Choose C major, Hard and 100%. A 313 Hz line tone should move to
   E4 (329.6 Hz). Turn Tune to Off or Amount to 0% for an immediate A/B.
   Set Brain to C major, then choose Auto and compare it with manual C major.
   Release Shift and check that the normal encoder actions return.
5. Select the headset mic and repeat the vocal check. Toggle monitoring off
   and back on, then switch between the three analog sources.
   Check for clicks and any replay of sound from the previous source.
6. Raise the Aux 5 FX send on M3 and verify the corrected note reaches the FX.
   Check that the instrument pattern's pitch stays the same.
7. With monitoring disabled, sample the line input from the stock sampler.
   Verify that a 432 Hz input remains at 432 Hz in the raw recording.
8. Run the test with a busy project playing and check for audio dropouts.
   Record device CPU measurements separately if diagnostic support is available.
