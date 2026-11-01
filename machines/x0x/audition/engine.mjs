import {validatePad, word} from './models.mjs';

// Audition host only. Every sound sample comes from the compiled C++ machine.
export class DrumAudition {
  constructor(module, rate, pads, firstFrame = 0) {
    this.api = new WebAssembly.Instance(module).exports;
    const api = this.api;
    if (api.opxy_preview_abi() !== 1 || api.opxy_sample_rate() !== rate || api.opxy_block_frames() !== 64 ||
        api.opxy_voice_capacity() !== 8) throw new Error('Unsupported drum preview');
    this.rate = rate; this.frame = firstFrame; this.clock = 0;
    this.pads = Array.from({length:24}, (_, i) => this.copyPad(pads[i]));
    let next = Number(api.__heap_base.value);
    const allocate = words => { const at = next; next += words * 4; return at; };
    this.voices = Array.from({length:8}, (_, index) => {
      const state = api.opxy_voice_state(index), pitchAddress = allocate(64), outputAddress = allocate(64);
      const parameterAddress = allocate(8);
      const voice = {state, pitchAddress, outputAddress, parameterAddress, pad:0, active:false, age:0,
        cursor:64, fade:0, silent:0, blocks:0,
        output:new Int32Array(api.memory.buffer, outputAddress, 64),
        parameters:new Int32Array(api.memory.buffer, parameterAddress, 8)};
      new Uint32Array(api.memory.buffer, pitchAddress, 64).fill(25479940);
      api.opxy_initialize(state);
      return voice;
    });
    if (next > api.memory.buffer.byteLength) throw new Error('Audition memory exceeded');
    this.gain = 0.11 / api.opxy_output_peak_q31();
    this.queuePads = new Uint8Array(128); this.queueFrames = new Float64Array(128);
    this.read = 0; this.write = 0;
  }
  copyPad(pad) { validatePad(pad); return {model:pad.model, raw:pad.raw.slice(), choke:pad.choke}; }
  setPad(index, pad) {
    if (!Number.isInteger(index) || index < 0 || index >= 24) throw new Error('Invalid pad');
    this.pads[index] = this.copyPad(pad);
  }
  hit(index) {
    if (!Number.isInteger(index) || index < 0 || index >= 24) return;
    const pad = this.pads[index];
    for (const voice of this.voices) if (voice.active &&
        (voice.pad === index || (pad.choke && this.pads[voice.pad].choke === pad.choke))) voice.fade = 128;
    let chosen = null;
    for (const voice of this.voices) {
      if (!voice.active) { chosen = voice; break; }
      if (!chosen || (voice.fade && !chosen.fade) || (Boolean(voice.fade) === Boolean(chosen.fade) && voice.age < chosen.age)) chosen = voice;
    }
    this.api.opxy_destroy(chosen.state); this.api.opxy_initialize(chosen.state); this.api.opxy_start(chosen.state);
    chosen.pad = index; chosen.active = true; chosen.cursor = 64; chosen.fade = 0;
    chosen.age = ++this.clock; chosen.silent = 0; chosen.blocks = 0;
  }
  schedule(index, when) {
    if (!Number.isInteger(index) || index < 0 || index >= 24 || !Number.isFinite(when)) throw new Error('Invalid hit');
    const frame = Math.round(when * this.rate);
    if (frame <= this.frame) { this.hit(index); return; }
    const next = (this.write + 1) & 127;
    if (next === this.read) throw new Error('Audition queue is full');
    this.queuePads[this.write] = index; this.queueFrames[this.write] = frame; this.write = next;
  }
  clearQueue() { this.read = this.write = 0; }
  panic() {
    this.clearQueue();
    for (const voice of this.voices) { if (voice.active) this.api.opxy_destroy(voice.state); voice.active = false; }
  }
  render(output) {
    for (let i = 0; i < output.length; ++i, ++this.frame) {
      while (this.read !== this.write && this.queueFrames[this.read] <= this.frame) {
        this.hit(this.queuePads[this.read]); this.read = (this.read + 1) & 127;
      }
      let mixed = 0;
      for (const voice of this.voices) {
        if (!voice.active) continue;
        if (voice.cursor === 64) {
          const pad = this.pads[voice.pad];
          for (let s = 0; s < 8; ++s) voice.parameters[s] = word(pad.raw[s]);
          this.api.opxy_preview_model(pad.model);
          this.api.opxy_render(voice.state, voice.outputAddress, voice.parameterAddress, voice.pitchAddress);
          let silent = true;
          for (let j = 0; j < 64; ++j) if (voice.output[j]) { silent = false; break; }
          voice.silent = silent ? voice.silent + 1 : 0;
          if (voice.silent >= 2 || ++voice.blocks > 6000) {
            this.api.opxy_destroy(voice.state); voice.active = false; continue;
          }
          voice.cursor = 0;
        }
        let sample = voice.output[voice.cursor++] * this.gain;
        if (voice.fade) {
          sample *= (voice.fade - 1) / 128;
          if (!--voice.fade) { this.api.opxy_destroy(voice.state); voice.active = false; }
        }
        mixed += sample;
      }
      output[i] = mixed;
    }
  }
}
