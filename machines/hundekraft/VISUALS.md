# Hundekraft display reconstruction

The source artwork is Hundekraft 3.0.3's own vector artwork. The contours are imported, recoloured, packed into the XY's retained drawing format and animated by portable C++. These replace the initial shared gauges. Black, white and grey dominate; the few coloured parts use muted blue, red or orange. This is a reconstruction, with the adaptations below, rather than a claim of pixel-identical OP-1 emulation.

## Synths: integrated native displays

| Machine | Original scene and reconstructed motion |
| --- | --- |
| Wavetable | Wave factory: moving conveyor marks, opposing gears, pumping press, resonance spring and the front card's waveform. The waveform uses the DSP's actual morph function. |
| Taffy | Taffy puller: opposing rollers, live ribbon, compressed spring, strike spark and envelope crank. The ribbon uses the DSP's actual wave-shaping function. |
| PEW | Organ and organist: drawbar-controlled pipe heights, rotating Leslie horn, moving drum slats, rocking organist and rotor arcs. |
| Elg | Moose: growing antlers, breath clouds, muzzle hum arcs, aurora and note-gated blinking. |
| Supersaw | Chainsaw: moving saw/square chain teeth, detuned and sub runs, drive shake, sprocket rotation and rising smoke. |
| FM Bell | Bell: swinging clapper, expanding strike arcs and the original four corner pointers. |
| SID | Original chip illustration and parameter readouts; the original screen is predominantly static. |
| MBIRA | Original kalimba and thumb: four tick meters, alternate labels, tine vibration, spreading sound rings and red bottle-cap rattle. |
| Oxbow | Original animal, horns, birds, bell and breath: cutoff stretches/droops the horns, detune moves the birds, resonance swings the bell, envelope amount expands the breath; stars follow held note count. |
| EVA | Original astronaut: moving meter masks, vowel crosshair, breathing visor fog and radio strip flicker. |
| Arc | Carbon-arc lamp: phase-portrait illustration, carbon outlines, bowing/jittering arc, strike sparks and the original pointers. |
| Mixtur | Original undertone ladder: selected strings, divider beads and formant-response curves. |

Synths with five to eight controls use the SDK 1.4 two-row page header. Both rows remain visible and pressing an encoder switches the whole page. EVA's Octave alternate is on the COM encoder, matching the upstream screen; saved parameter order and audio mapping are unchanged.

## Effects: browser display studies

The public SDK registers synth displays only. Every effect therefore has its portable drawing source and artwork under `visual/`, compiled into the browser audition. Its native machine continues to use the stock FX page. No undocumented screen hook or firmware address is used to bypass that boundary.

| Machine | Original scene and reconstructed motion |
| --- | --- |
| Washer | Washing drum, swinging socks, depth-modulated water, moving rate arcs, foam bubbles and escaped sock. |
| Crush | Original quantizer illustration, input sine, drive rails and stepped output; actual bit count and rate are derived from its DSP control functions. |
| Tape | Original tape echo and serpentine tape path, spinning roller spokes and pinch roller, heads, signal LED and moving registration marks. |
| Warble | Original tape transport, rotating hub spokes and depth/rate waveform. |
| Plate | Upstream rectangle artwork, rectangular plate ripples, tone hatching and predelay waveform. |
| Snowglobe | Original globe and village, swirling snow layers, shimmer, expanding halo, freeze wind-up key and chimney smoke. |
| Father | Original perspective grid and person, distance-scaled face, moving gate posts and dry meter. |
| Reverb | Original Kali artwork: time reveals arms, tone reveals the fire, diffusion changes skull brightness, dry moves the scale and all three needles follow their controls. |
| Clouds | Original buffer line, write head, position bracket, grain ticks and freeze state. |
| HAIL | Original bead loom: moving beads along the actual source thread geometry, envelope arcs, loose pellets, time marker and primary/alternate rings. |
| Stutter | Original projector: turning reel, parameter rings and mode-dependent slice-history illustration. |
| Palme | Original palm soundboard: vibrating strings along the source geometry, metal rings, opening damper wings and breathing speaker. |
| In a room | Original room panel: size/shape floor plan, nodal grid, loop-waveform illustration and moving playhead. |

## Granular: browser display study

The original sample window, pointers, position/spray/size guides and four grain markers are reconstructed. The gallery waveform is explicitly an illustration, not a reading of the assigned sample. A native Granular machine still uses the SDK's stock sampler page; no sample custom-display API exists yet.

## Fidelity and validation

- The display API supplies parameter snapshots, held-note count and note serials. It does not expose per-voice envelopes, oscillator traces, grain positions, sample peaks or effect internals. Where upstream reads those, the reconstruction uses a display-only model driven by controls and note activity. Effect auditions gate this model from the measured wet return. No audio callback was changed to publish unsafe shared state.
- Continuous translation, rotation and uniform scaling use retained nodes. Nonuniform source transforms, such as pipe height or visor fog, use pre-baked geometry keyframes within the native artwork and node limits. Small outlines are resampled, some strokes are coalesced, procedural traces are reduced to bounded segment counts and tiny subpixel lines are omitted. Fonts and native rasterization can differ from the browser.
- To follow XY UX conventions, idle scenes stay still and reduced motion snaps to a static pose. Track activation establishes a baseline without fabricating a strike. These deliberately differ from upstream's continuously running idle effects.
- All 26 displays have host invariant tests for bounds, geometry budgets, activation, track changes, idle and reduced motion, including 1,800 control/activity frames each. Native and WASM builds use the same C++ source. The existing sound and arithmetic checks remain in each machine.
- Device appearance, GUI load and hardware audio CPU measurements remain unverified. Only the 12 synth display registrations are included in the native development module. A separate SDK change is needed to register effect/sample custom displays and to expose any additional safe telemetry.

`tools/visuals.py` regenerates the artwork and C++ from the pinned upstream source, the machine-specific translations in `tools/visual_motion/`, and `tools/visual_runtime.hpp.in`. Every machine's `VISUALS.md` and `visual.json` identify its original artwork and hash. Original Hundekraft notices and licences continue to apply; the SDK's generated header and browser renderer retain the Open-XY MIT notice.
