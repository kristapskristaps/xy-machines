// Author original device presets using the SDK's version-4 preset template.
import {readFileSync, writeFileSync, mkdirSync} from 'node:fs';
import {fileURLToPath, pathToFileURL} from 'node:url';
import {resolve} from 'node:path';

const root = fileURLToPath(new URL('..', import.meta.url));
const sdk = process.argv[2] ? resolve(process.argv[2]) : fileURLToPath(new URL('../../../../open-xy/', import.meta.url));
const {presetFiles} = await import(pathToFileURL(resolve(sdk, 'builder/core.mjs')));
const definition = JSON.parse(readFileSync(resolve(root, 'definition.json'), 'utf8'));
const machine = JSON.parse(readFileSync(resolve(root, 'machine.json'), 'utf8'));
const descriptor = {id: definition.id, kind: 'synth', presetDefaults: machine.presetDefaults,
  catalog: JSON.stringify({type: definition.id, default_params: definition.parameters.map(p => Math.round(p.default * 32767))})};
const initial = Object.values(presetFiles(descriptor))[0];
const presets = {};
function sound(values) {
  const result = structuredClone(initial);
  result.engine.params = definition.parameters.map(p => Math.round(values[p.id] * 32767));
  result.engine.playmode = 'mono';
  result.engine['portamento.amount'] = 4096;
  result.engine.volume = 32767;
  result.octave = -2;
  return result;
}
presets.default = sound(Object.fromEntries(definition.parameters.map(p => [p.id, p.default])));
for (const [name, values] of Object.entries(definition.presets)) {
  const preset = sound(values);
  if (name === 'Short Fuse') {
    preset.envelope.amp.decay = 8192;
    preset.engine['portamento.amount'] = 0;
  }
  if (name === 'Seismic') preset.envelope.amp.decay = 23000;
  presets[name.toLowerCase().replaceAll(' ', '-')] = preset;
}
mkdirSync(resolve(root, 'presets'), {recursive: true});
for (const [path, preset] of Object.entries(presetFiles({...descriptor, presets}))) {
  const name = path.split('/').at(-1);
  writeFileSync(resolve(root, 'presets', name), JSON.stringify(preset, null, 2) + '\n');
}
console.log(`Generated ${Object.keys(presets).length} mono Vader presets with native AMP and glide settings.`);
