import test from 'node:test';
import assert from 'node:assert/strict';
import {readFile} from 'node:fs/promises';
import {MODELS,available,family,stepFamily,defaultPad,validatePad} from '../audition/models.mjs';
import {DrumAudition} from '../audition/engine.mjs';

test('one style detent moves one available family in either direction for every raw value', () => {
  for (let model = 0; model < MODELS.length; ++model) for (let raw = 0; raw < 128; ++raw) {
    const choices = available(model), pad = {...defaultPad(0), model, raw:[raw,64,48,64,0,64,64,0]};
    const index = choices.indexOf(family(pad));
    for (const delta of [-1,1]) {
      const changed = {...pad,raw:pad.raw.slice()}; stepFamily(changed, delta);
      assert.equal(family(changed), choices[Math.max(0,Math.min(choices.length-1,index+delta))]);
      assert.deepEqual(changed.raw.slice(1), pad.raw.slice(1));
    }
  }
});
test('browser model catalog matches the machine family masks', async () => {
  const source = await readFile(new URL('../models.h',import.meta.url),'utf8');
  const mask = source.match(/family_masks\[model_count\]\s*=\s*\{([^}]+)\}/)[1].match(/\d+/g).map(Number);
  assert.deepEqual(MODELS.map(model => model[1]),mask);
  assert.throws(() => validatePad({...defaultPad(0),raw:[0]}),/Invalid/);
});
const wasm = process.env.X0X_PREVIEW_WASM;
if (!wasm) throw new Error('Set X0X_PREVIEW_WASM to a fresh full-engine preview');
const module = await WebAssembly.compile(await readFile(wasm));
const pads = () => Array.from({length:24},(_,i)=>defaultPad(i));
test('all 47 full-engine sounds produce bounded nonzero browser audio', () => {
  const engine = new DrumAudition(module,44100,pads()), output = new Float32Array(128);
  let cases = 0;
  for (let model = 0; model < MODELS.length; ++model) for (const choice of available(model)) {
    engine.panic(); const pad = defaultPad(0); pad.model = model; pad.raw[0] = 16 + 32 * choice;
    engine.setPad(0,pad); engine.hit(0); let energy = 0;
    for (let block = 0; block < 100; ++block) {
      engine.render(output);
      for (const sample of output) { assert.ok(Number.isFinite(sample) && Math.abs(sample) <= .111); energy += sample*sample; }
    }
    assert.ok(energy > 1e-7,`${MODELS[model][0]} family ${choice} is silent`); ++cases;
  }
  assert.equal(cases,47);
});
test('polyphonic pads retain their own model and parameters', () => {
  const combined = new DrumAudition(module,44100,pads());
  const kick = new DrumAudition(module,44100,pads()), snare = new DrumAudition(module,44100,pads());
  combined.hit(0); combined.hit(1); kick.hit(0);
  snare.hit(0); snare.hit(1); snare.api.opxy_destroy(snare.voices[0].state); snare.voices[0].active = false;
  const a = new Float32Array(128), b = new Float32Array(128), c = new Float32Array(128);
  for (let block = 0; block < 50; ++block) {
    combined.render(a); kick.render(b); snare.render(c);
    for (let i = 0; i < 128; ++i) assert.ok(Math.abs(a[i]-b[i]-c[i]) < 2e-8);
  }
});
test('scheduled hits wait for their frame and panic clears pending events', () => {
  const engine = new DrumAudition(module,44100,pads()), out = new Float32Array(128);
  engine.schedule(0,256/44100); engine.render(out); assert.ok(out.every(sample=>sample===0));
  engine.render(out); assert.ok(out.every(sample=>sample===0)); engine.render(out); assert.ok(out.some(sample=>sample!==0));
  engine.schedule(0,1); engine.panic(); engine.render(out); assert.ok(out.every(sample=>sample===0));
});
