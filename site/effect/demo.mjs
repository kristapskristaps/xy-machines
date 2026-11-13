// Original, synthesized four-bar loops: no external audio or sample downloads.
export function demoLoop(context, mode, bpm) {
  const rate = context.sampleRate, beat = 60 / bpm, length = Math.round(16 * beat * rate);
  const buffer = context.createBuffer(2, length, rate);
  let noise = 31;
  for (let i = 0; i < length; i++) {
    const t = i / rate, step = Math.floor(t / (beat / 2)), age = t % (beat / 2);
    const root = [130.8128, 155.5635, 103.8262, 116.5409][Math.floor(t / (beat * 4)) % 4];
    const note = [1, 1.5, 2, 2.3784, 1.5, 2, 1.1892, 1.5][step % 8];
    const env = Math.min(1, age / .004) * Math.exp(-age * 11);
    const phase = 2 * Math.PI * root * note * age;
    const pluck = env * (.17 * Math.sin(phase) + .05 * Math.sin(2.002 * phase) + .025 * Math.sin(3 * phase));
    const kickAge = t % beat, snareAge = (t + beat) % (2 * beat);
    noise = (Math.imul(noise, 1664525) + 1013904223) | 0;
    const hiss = noise / 2147483648;
    const drums = .22 * Math.sin(2 * Math.PI * (48 * kickAge + 2 * (1 - Math.exp(-35 * kickAge)))) * Math.exp(-20 * kickAge)
      + .08 * hiss * Math.exp(-25 * snareAge) + .025 * hiss * Math.exp(-90 * age);
    const padEnv = Math.sin(Math.PI * (t % (4 * beat)) / (4 * beat)) ** 2;
    for (let side = 0; side < 2; side++) {
      const detune = side ? 1.002 : .998;
      const pad = padEnv * (.07 * Math.sin(2 * Math.PI * root * detune * t) + .04 * Math.sin(2 * Math.PI * root * 1.5 * detune * t) + .03 * Math.sin(2 * Math.PI * root * 2.3784 * detune * t));
      buffer.getChannelData(side)[i] = mode === 'drums' ? drums : mode === 'pads' ? pad : pluck + drums * .65;
    }
  }
  return buffer;
}
