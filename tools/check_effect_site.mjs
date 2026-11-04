// Run the shipped worklet against every real effect payload, without a browser.
import fs from 'node:fs';
import path from 'node:path';
import assert from 'node:assert/strict';
import {pathToFileURL} from 'node:url';
import {createHash} from 'node:crypto';
const root = path.resolve(process.argv[2]);
const machines = JSON.parse(fs.readFileSync(path.join(root, 'machines.json'))).machines.filter(m => m.kind === 'effect');
let Processor;
globalThis.sampleRate = 44100;
globalThis.AudioWorkletProcessor = class { constructor() { this.port = {postMessage(){}}; } };
globalThis.registerProcessor = (_name, constructor) => { Processor = constructor; };
for (const machine of machines) {
  const folder = path.join(root, machine.player), manifest = JSON.parse(fs.readFileSync(path.join(folder, 'manifest.json')));
  const bytes = fs.readFileSync(path.join(folder, 'effect.wasm'));
  assert.equal(createHash('sha256').update(bytes).digest('hex'), manifest.wasm_sha256);
  const module = await WebAssembly.compile(bytes);
  assert.deepEqual(WebAssembly.Module.imports(module), []);
  await import(pathToFileURL(path.join(folder, 'worklet.mjs')));
  const processor = new Processor({processorOptions:{module, dryIndex:manifest.dryIndex}});
  const values = manifest.definition.parameters.map(p => Math.round(p.default * 2147483647));
  while (values.length < 8) values.push(0);
  processor.message({type:'controls', values});
  processor.message({type:'master', send:.7, bpm:120, bypass:false, playing:true});
  const input = [[new Float32Array(128), new Float32Array(128)]];
  const output = [[new Float32Array(128), new Float32Array(128)]];
  let wetEnergy = 0;
  for (let block = 0; block < 2200; block++) {
    const capture = manifest.definition.parameters.findIndex(p => p.id === 'capture');
    if (capture >= 0 && block === 500) { values[capture] = 2147483647; processor.message({type:'controls', values}); }
    for (let i = 0; i < 128; i++) {
      const t = (block * 128 + i) / 44100;
      input[0][0][i] = .16 * Math.sin(2 * Math.PI * 220 * t) * Math.exp(-8 * (t % .25));
      input[0][1][i] = .11 * Math.sin(2 * Math.PI * 330 * t) * Math.exp(-8 * (t % .25));
    }
    assert.equal(processor.process(input, output), true);
    for (const channel of output[0]) for (const sample of channel) { assert(Number.isFinite(sample), machine.id); assert(Math.abs(sample) <= 1, machine.id); }
    for (let i = 0; i < 64; i++) wetEnergy += (processor.left[i]/2147483648)**2 + (processor.right[i]/2147483648)**2;
  }
  assert(wetEnergy > .000001, machine.id + ' has no wet signal');
  processor.message({type:'master', send:.7, bpm:120, bypass:true, playing:true});
  processor.process(input, output);
  assert.deepEqual(output, input, machine.id + ' stereo dry comparison');
  processor.process([[input[0][0]]], output);
  assert.deepEqual(output[0][1], input[0][0], machine.id + ' mono file duplication');
  processor.message({type:'reset'});
  processor.message({type:'master', send:0, bpm:120, bypass:false, playing:false});
  processor.process(input, output);
  assert.deepEqual(output, input, machine.id + ' zero-send dry path');
  for (let block = 0; block < 500; block++) processor.process([[]], output);
  for (const channel of output[0]) for (const sample of channel) assert.equal(sample, 0, machine.id + ' clear tail');
  processor.message({type:'dispose'}); assert.equal(processor.process(input, output), false);
  console.log('PASS:', machine.id, 'wet signal, finite stereo, exact dry comparison, mono input, zero send, clear tail and disposal');
}
assert(machines.length > 0, 'No effect auditions exported');
const {demoLoop} = await import(pathToFileURL(path.join(root, machines[0].player, 'demo.mjs')));
const context = {sampleRate:44100, createBuffer(channels, frames) { const data = Array.from({length:channels}, () => new Float32Array(frames)); return {length:frames, getChannelData:i => data[i]}; }};
for (const mode of ['groove','drums','pads']) {
  const buffer = demoLoop(context, mode, 120);
  assert.equal(buffer.length, 352800);
  let energy = 0;
  for (let channel = 0; channel < 2; channel++) for (const value of buffer.getChannelData(channel)) { assert(Number.isFinite(value) && Math.abs(value) < 1); energy += value*value; }
  assert(energy > 1);
}
console.log('PASS: three original stereo demo loops');
