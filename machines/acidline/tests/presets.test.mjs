import test from 'node:test';
import assert from 'node:assert/strict';
import {readFileSync,readdirSync} from 'node:fs';
const root=new URL('../',import.meta.url);
const definition=JSON.parse(readFileSync(new URL('definition.json',root)));
test('original six slots keep their indices; sub and damp fill the second page',()=>{
  assert.equal(definition.press,'page');
  assert.deepEqual(definition.parameters.map(p=>p.id),['cutoff','resonance','drive','env','decay','wave','sub','damp']);
  assert.deepEqual(definition.parameters.slice(4).map(p=>p.control),[0,1,2,3]);
  assert.deepEqual(definition.parameters[5].mapping.labels,['Saw','Triangle','Square']);
});
test('every factory preset selects independent stock mono mode and preserves its sound controls',()=>{
  const folder=new URL('presets/',root), files=readdirSync(folder).filter(f=>f.endsWith('.json'));
  assert.equal(files.length,6);
  for(const file of files){
    const p=JSON.parse(readFileSync(new URL(file,folder)));
    assert.equal(p.engine.playmode,'mono',file);
    assert.equal(p.engine.params.length,8,file);
    const name=file==='default.json'?null:Object.keys(definition.presets).find(n=>n.toLowerCase().replaceAll(' ','-')+'.json'===file);
    const values=name?definition.presets[name]:Object.fromEntries(definition.parameters.map(p=>[p.id,p.default]));
    assert.deepEqual(p.engine.params,definition.parameters.map(p=>Math.round(values[p.id]*32767)),file);
  }
});
