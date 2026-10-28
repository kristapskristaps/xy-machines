import fs from 'node:fs';
import assert from 'node:assert/strict';
const [folder,expected]=process.argv.slice(2);if(!folder||!expected)throw Error('usage: node check_audition.mjs <audition> <host-hash>');
const module=await WebAssembly.compile(fs.readFileSync(folder+'/halo.wasm'));
assert.deepEqual(WebAssembly.Module.imports(module),[]);
const fx=new WebAssembly.Instance(module,{}).exports,l=new Int32Array(fx.memory.buffer,fx.halo_left(),64),r=new Int32Array(fx.memory.buffer,fx.halo_right(),64),p=new Int32Array(fx.memory.buffer,fx.halo_parameters(),8);
let phase=0,rng=21,hash=2166136261>>>0;
for(let block=0;block<2400;block++){
 if(!(block%73))for(let i=0;i<8;i++){rng=(Math.imul(rng,1664525)+1013904223)>>>0;p[i]=rng>>>1;}
 if(block===1200)fx.halo_reset();
 for(let i=0;i<64;i++){phase=(phase+23192823)>>>0;l[i]=Math.trunc((phase|0)/4);r[i]=-l[i];}
 fx.halo_process(block%3?1200:600,block,1);
 for(let i=0;i<64;i++){hash=Math.imul(hash^l[i],16777619)>>>0;hash=Math.imul(hash^r[i],16777619)>>>0;}
}
assert.equal(hash,Number(expected));console.log('PASS: host / WebAssembly parity:',hash);
let Processor;globalThis.sampleRate=44100;globalThis.AudioWorkletProcessor=class{constructor(){this.port={postMessage(){},onmessage:null};}};globalThis.registerProcessor=(_name,p)=>Processor=p;
await import(new URL('../audition/worklet.mjs',import.meta.url));
const processor=new Processor({processorOptions:{module}}),out=[[new Float32Array(128),new Float32Array(128)]];
processor.message({type:'controls',values:[.65,.73,.66,.1,.38,.68,0,1]});processor.message({type:'play'});
let energy=0;
for(let block=0;block<3600;block++){
 processor.process([],out);
 for(const channel of out[0])for(const value of channel){assert(Number.isFinite(value));assert(Math.abs(value)<=1);energy+=value*value;}
}
assert(energy>1);processor.message({type:'stop'});processor.message({type:'reset'});
for(const v of processor.voices)v.on=false;
processor.process([],out);for(const channel of out[0])for(const value of channel)assert.equal(value,0);
console.log('PASS: full browser worklet phrase, finite stereo output and tail reset; energy',energy.toFixed(2));
// An uploaded stereo file must retain every dry sample, including sample zero
// of each 64-frame slice, when Send is zero.
const fileL=Float32Array.from({length:128},(_,i)=>i/2048),fileR=Float32Array.from(fileL,x=>-x);
processor.message({type:'master',send:0,monitor:1,bpm:120,bypass:false});
processor.message({type:'file',left:fileL,right:fileR});processor.message({type:'play'});processor.process([],out);
for(let i=0;i<128;i++){assert(out[0][0][i]===fileL[i]);assert(out[0][1][i]===fileR[i]);}
console.log('PASS: local stereo file playback and exact zero-send dry signal');
