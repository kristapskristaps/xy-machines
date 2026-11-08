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
9. Hold Shift on Aux 5 M1 and press M1. Check Harmony, Blend, Compress and
   De-ess in encoder order. Press M1 again to return to Tune and Tilt Mic.
   Release Shift on the effects page, then hold it again to check that the
   overlay opens on Tune and Tilt Mic.
10. With Brain in C major, set Third Up and Blend to 20%. Sing D and E to
    check the harmony moves from F to G. Try Third Down and Double. Check
    the lead stays clear on breaths and consonants and that a source change
    clears the previous voice. Repeat with Tune Off.
    Try Thirds, Triad, Octaves and Choir with higher Blend. On C4, Choir
    should add C3, E4 and G4. Confirm Hard tuning still snaps quickly when
    moving across notes and that the transitions sound clean.
11. Turn harmony off and compare the two dynamics controls independently
    on quiet vowels, loud vowels and sibilants. Then use both together with
    Hard tuning and harmony while a busy project plays. Time the complete
    vocal chain on the device.
