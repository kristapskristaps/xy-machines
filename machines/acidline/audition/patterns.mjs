export const names = ['Warehouse 16', 'Liquid Chrome', 'Broken Circuit'];
const make = rows => rows.map(([note, gate = true, accent = false, slide = false]) => ({note, gate, accent, slide}));
export const patterns = [
  make([[36,1,1],[36],[43,1,0,1],[48],[36,0],[39,1,1],[36],[36,1,0,1],[48,1,1],[46],[43,0],[39,1,1],[36],[43,1,0,1],[39],[34,1,1]]),
  make([[36,1,1,1],[48,1,0,1],[43],[39],[36,0],[36,1,1],[46,1,0,1],[43],[36,1,1,1],[39],[43],[48,1,0,1],[46],[43,1,1],[39,0],[34,1,0,1]]),
  make([[36,1,1],[48,0],[36],[37,1,1,1],[43],[36,0],[46,1,1],[39,1,0,1],[36],[48,1,1],[34],[36,0],[43,1,0,1],[46],[39,1,1],[36,0]])
];
export function validatePattern(value) {
  if (!Array.isArray(value) || value.length !== 16) throw new Error('A pattern needs exactly 16 steps');
  return value.map(step => {
    if (!step || !Number.isInteger(step.note) || step.note < 24 || step.note > 72) throw new Error('Note is out of range');
    return {note: step.note, gate: Boolean(step.gate), accent: Boolean(step.accent), slide: Boolean(step.slide)};
  });
}
export function stepFrames(bpm, sampleRate = 44100) {
  return sampleRate * 60 / (bpm * 4);
}
