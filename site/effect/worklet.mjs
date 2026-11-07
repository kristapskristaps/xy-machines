// The native effect processes 64-frame stereo Q31 blocks; Web Audio supplies floats.
class XYEffect extends AudioWorkletProcessor {
  constructor({processorOptions: options}) {
    super();
    this.api = new WebAssembly.Instance(options.module, {}).exports;
    this.left = new Int32Array(this.api.memory.buffer, this.api.xy_left(), 64);
    this.right = new Int32Array(this.api.memory.buffer, this.api.xy_right(), 64);
    this.parameters = new Int32Array(this.api.memory.buffer, this.api.xy_parameters(), 8);
    this.dryIndex = options.dryIndex;
    this.dryLevel = options.dryLevel ?? 1;
    this.send = .7; this.bypass = false; this.bpm = 120; this.playing = false;
    this.frames = 0; this.reports = 0; this.active = true;
    this.api.xy_reset();
    this.port.onmessage = ({data}) => this.message(data);
  }
  message(m) {
    if (m.type === 'controls') this.parameters.set(m.values);
    if (m.type === 'master') { this.send = m.send; this.bypass = m.bypass; this.bpm = m.bpm; this.playing = m.playing; }
    if (m.type === 'reset') { this.api.xy_reset(); this.frames = 0; }
    if (m.type === 'dispose') this.active = false;
  }
  process(inputs, outputs) {
    if (!this.active) return false;
    const output = outputs[0], input = inputs[0];
    if (!output?.[0]) return true;
    let peak = 0, wetPeak = 0;
    const sourceDry = this.dryIndex === undefined ? this.dryLevel : this.parameters[this.dryIndex] / 2147483647;
    const dry = 1 - this.send * (1 - sourceDry);
    for (let offset = 0; offset < output[0].length; offset += 64) {
      for (let i = 0; i < 64; i++) {
        const l = input?.[0]?.[offset+i] ?? 0, r = input?.[1]?.[offset+i] ?? l;
        this.left[i] = Math.round(Math.max(-1, Math.min(1, l * this.send)) * 2147483647);
        this.right[i] = Math.round(Math.max(-1, Math.min(1, r * this.send)) * 2147483647);
      }
      this.frames += 64;
      this.api.xy_process(Math.round(this.bpm * 10), Math.floor(this.frames * this.bpm * 96 / (sampleRate * 60)), this.playing ? 1 : 0);
      for (let i = 0; i < 64 && offset+i < output[0].length; i++) {
        const l = input?.[0]?.[offset+i] ?? 0, r = input?.[1]?.[offset+i] ?? l;
        const wetL = this.left[i] / 2147483648, wetR = this.right[i] / 2147483648;
        wetPeak = Math.max(wetPeak, Math.abs(wetL), Math.abs(wetR));
        output[0][offset+i] = this.bypass ? l : Math.max(-1, Math.min(1, l * dry + wetL));
        output[1][offset+i] = this.bypass ? r : Math.max(-1, Math.min(1, r * dry + wetR));
        peak = Math.max(peak, Math.abs(output[0][offset+i]), Math.abs(output[1][offset+i]));
      }
    }
    if (++this.reports === 24) { this.reports = 0; this.port.postMessage({type:'meter', peak, wetPeak}); }
    return true;
  }
}
registerProcessor('xy-effect', XYEffect);
