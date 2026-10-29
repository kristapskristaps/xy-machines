// Use the SDK's public preset template and validation; never copy factory sounds.
import {readFileSync, writeFileSync, mkdirSync} from 'node:fs';
import {resolve, dirname} from 'node:path';
import {pathToFileURL} from 'node:url';
const [sdk, descriptors, output] = process.argv.slice(2);
const {presetFiles} = await import(pathToFileURL(resolve(sdk, 'builder/core.mjs')));
for (const machine of JSON.parse(readFileSync(descriptors, 'utf8'))) {
  for (const [name, preset] of Object.entries(presetFiles(machine))) {
    const path = resolve(output, name);
    mkdirSync(dirname(path), {recursive: true});
    writeFileSync(path, JSON.stringify(preset, null, 2) + '\n', {flag: 'wx'});
  }
}
