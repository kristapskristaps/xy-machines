export const FAMILIES = ['606', '707', '808', '909'];
export const MODELS = [
  ['kick',15],['snare',15],['clap',14],['rim',14],['tom lo',15],['tom mid',14],
  ['tom hi',15],['cl.hat',15],['op.hat',15],['cymbal',15],['ride',10],
  ['cowbell',6],['claves',4],['maracas',4],['conga l',4],['conga m',4],['conga h',4],['tamb',2],
];
export const LAYOUT = [0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,0,1,7,8,2,9];
export const KEYS = ['1','2','3','4','5','6','q','w','e','r','t','y','a','s','d','f','g','h','z','x','c','v','b','n'];
export const available = model => FAMILIES.map((_, i) => i).filter(i => MODELS[model][1] & (1 << i));
export function family(pad) {
  const requested = Math.floor(pad.raw[0] / 32), choices = available(pad.model);
  return choices.find(i => i >= requested) ?? choices.at(-1);
}
export function stepFamily(pad, delta) {
  const choices = available(pad.model), index = choices.indexOf(family(pad));
  const next = choices[Math.max(0, Math.min(choices.length - 1, index + delta))];
  pad.raw[0] = 16 + 32 * next;
  return next;
}
export function defaultPad(index) {
  const model = LAYOUT[index];
  const pad = {model, raw:[80,64,48,64,0,64,64,0], choke:0};
  if (model === 10) pad.raw[0] = 112;
  pad.raw[0] = 16 + 32 * family(pad);
  return pad;
}
export function validatePad(pad) {
  if (!pad || !Number.isInteger(pad.model) || pad.model < 0 || pad.model >= MODELS.length ||
      !Number.isInteger(pad.choke) || pad.choke < 0 || pad.choke > 3 ||
      !Array.isArray(pad.raw) || pad.raw.length !== 8 ||
      pad.raw.some(v => !Number.isInteger(v) || v < 0 || v > 127)) throw new Error('Invalid pad settings');
  return pad;
}
export const word = raw => raw === 127 ? 0x7fffffff : raw * 0x1000000;
export function controlNames(model) {
  const names = ['style','tune','decay','tone','attack','sweep','body','drive'];
  if (model === 1) names[5] = 'snappy';
  if (model === 2) { names[5] = 'spread'; names[6] = 'noise'; }
  if (model === 7 || model === 8) { names[5] = 'metal'; names[6] = 'open'; }
  if (model === 9 || model === 10) { names[5] = 'metal'; names[6] = 'bell'; }
  if (model === 11) names[5] = 'metal';
  if (model === 3 || model === 12) names[5] = 'click';
  if (model === 13 || model === 17) names[5] = 'noise';
  if (available(model).length === 1) names[0] = '';
  if (model >= 12) names[7] = '';
  return names;
}
