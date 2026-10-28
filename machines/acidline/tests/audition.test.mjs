import assert from 'node:assert/strict';
import {readFileSync} from 'node:fs';
import {test} from 'node:test';
import {patterns,validatePattern,stepFrames} from '../audition/patterns.mjs';
let Processor;
globalThis.sampleRate=44100;
globalThis.AudioWorkletProcessor=class {constructor(){this.events=[];this.port={postMessage:data=>this.events.push({...data,frame:this.frame})};}};
globalThis.registerProcessor=(_name,value)=>{Processor=value;};
await import('../audition/processor.mjs');
const wasmPath=process.env.ACIDLINE_WASM;
assert.ok(wasmPath,'Set ACIDLINE_WASM to a built SDK engine.wasm');
const module=await WebAssembly.compile(readFileSync(wasmPath));
const definition=JSON.parse(readFileSync(new URL('../definition.json',import.meta.url)));
const parameters=[...definition.parameters.map(p=>Math.round(definition.presets.Warehouse[p.id]*2147483647)),0,0];
const create=()=>new Processor({processorOptions:{module,parameters,pattern:patterns[0]}});
const output=[new Float32Array(128)];
function message(engine,data){engine.port.onmessage({data});}
function blocks(engine,count){let peak=0;for(let b=0;b<count;b++){engine.process([], [output]);for(const sample of output[0]){assert.ok(Number.isFinite(sample));peak=Math.max(peak,Math.abs(sample));}}return peak;}
test('pattern validation and timing',()=>{
  for(const pattern of patterns){const normalized=validatePattern(pattern);assert.equal(normalized.length,16);assert.ok(normalized.some(s=>!s.gate));assert.ok(normalized.some(s=>s.accent));assert.ok(normalized.some(s=>s.slide));}
  assert.throws(()=>validatePattern([]));assert.throws(()=>validatePattern(Array(16).fill({note:999})));
  assert.equal(stepFrames(120),5512.5);
});
test('audio-thread sequence loops without timing drift and makes sound',()=>{
  const engine=create();message(engine,{type:'start'});
  const peak=blocks(engine,1300);assert.ok(peak>.005 && peak<.8,`Peak ${peak}`);
  const steps=engine.events.filter(e=>e.step>=0);assert.ok(steps.length>=32);
  for(let i=0;i<steps.length;i++){
    assert.equal(steps[i].step,i%16);
    assert.ok(Math.abs(steps[i].frame-i*stepFrames(132))<64,`Step ${i} drifted`);
  }
  assert.ok(steps.some(s=>s.legato));assert.ok(steps.some(s=>s.accent));
});
test('tied notes preserve the C++ envelope while ordinary notes retrigger',()=>{
  const engine=create();const transitions=[];const original=engine.beginStep.bind(engine);
  // Voice's envelope follows phase and four pole words in our owned C++ state.
  const envelope=new Int32Array(engine.api.memory.buffer,engine.voice+20,1);
  engine.beginStep=()=>{const before=envelope[0];original();transitions.push({before,after:envelope[0],tied:engine.legato,gate:engine.pattern[engine.step].gate});};
  message(engine,{type:'start'});blocks(engine,500);
  const tied=transitions.filter(t=>t.tied);assert.ok(tied.length>0);
  for(const t of tied){assert.equal(t.after,t.before);assert.ok(t.after<16384);}
  for(const t of transitions.filter(t=>t.gate&&!t.tied))assert.equal(t.after,16384);
});
test('stop fades out; restart and live pattern changes work',()=>{
  const engine=create();message(engine,{type:'start'});blocks(engine,30);
  message(engine,{type:'stop'});blocks(engine,100);assert.ok(blocks(engine,1)<.00001);
  message(engine,{type:'pattern',value:patterns[1]});message(engine,{type:'tempo',bpm:160});
  message(engine,{type:'start'});assert.ok(blocks(engine,80)>.005);
  const errorsBefore=engine.events.filter(e=>e.error).length;
  message(engine,{type:'tempo',bpm:Infinity});assert.equal(engine.events.filter(e=>e.error).length,errorsBefore+1);
});

test('velocity changes host gain without changing the C++ voice',()=>{
  const soft=create(),loud=create();
  message(soft,{type:'pattern',value:patterns[0].map(s=>({...s,accent:false}))});
  message(loud,{type:'pattern',value:patterns[0].map(s=>({...s,accent:true}))});
  message(soft,{type:'start'});message(loud,{type:'start'});
  const a=[new Float32Array(128)],b=[new Float32Array(128)];let energyA=0,energyB=0;
  for(let block=0;block<100;block++){
    soft.process([],[a]);loud.process([],[b]);
    assert.deepEqual(soft.output,loud.output,'Velocity leaked into the sound engine');
    for(let i=0;i<128;i++){energyA+=a[0][i]**2;energyB+=b[0][i]**2;}
  }
  assert.ok(energyB>energyA*1.5);
});
