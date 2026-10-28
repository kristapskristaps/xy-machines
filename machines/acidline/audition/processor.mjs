import {validatePattern, stepFrames} from './patterns.mjs';

export class AcidlineProcessor extends AudioWorkletProcessor {
  constructor(options) {
    super();
    const {module, parameters, pattern} = options.processorOptions;
    this.api = new WebAssembly.Instance(module).exports;
    if (sampleRate !== 44100 || this.api.opxy_block_frames() !== 64) throw new Error('Acidline requires 44.1 kHz / 64 frames');
    let next = Number(this.api.__heap_base.value);
    this.parameterAddress = next; next += 32;
    this.pitchAddress = next; next += 256;
    this.outputAddress = next; next += 256;
    if (next > this.api.memory.buffer.byteLength) throw new Error('Preview memory is full');
    this.parameters = new Int32Array(this.api.memory.buffer, this.parameterAddress, 8);
    this.pitch = new Uint32Array(this.api.memory.buffer, this.pitchAddress, 64);
    this.output = new Int32Array(this.api.memory.buffer, this.outputAddress, 64);
    this.voice = this.api.opxy_voice_state(0);
    this.api.opxy_initialize(this.voice);
    this.base = parameters;
    this.pattern = validatePattern(pattern);
    this.bpm = 132; this.gateLength = .60;
    this.playing = false; this.frame = 0; this.nextStep = 0; this.step = -1;
    this.gateOff = -1; this.cursor = 64; this.gain = 0; this.targetGain = 0;
    this.previousTie = false; this.previousGate = false; this.stepAccent = false; this.legato = false;
    this.currentPitch=0;this.targetPitch=0;this.glide=1-Math.exp(-1/(sampleRate*.060));
    this.port.onmessage = ({data}) => {
      try {
        if (data.type === 'parameters') {
          if (!Array.isArray(data.values) || data.values.length !== 8 || data.values.some(x => !Number.isInteger(x) || x < 0 || x > 0x7fffffff)) throw new Error('Invalid controls');
          this.base = data.values;
        }
        if (data.type === 'pattern') this.pattern = validatePattern(data.value);
        if (data.type === 'tempo') {
          if (!Number.isFinite(data.bpm) || data.bpm < 60 || data.bpm > 200) throw new Error('Tempo must be 60–200 BPM');
          this.bpm = data.bpm;
        }
        if (data.type === 'gate') this.gateLength = Math.max(.2, Math.min(.95, data.value));
        if (data.type === 'start') {
          this.playing = true; this.nextStep = this.frame; this.step = -1;
          this.previousTie = this.previousGate = false;
        }
        if (data.type === 'stop') {
          this.playing = false; this.targetGain = 0; this.gateOff = -1;
          this.previousTie = this.previousGate = false;
          this.port.postMessage({step: -1, playing: false});
        }
      } catch (error) { this.port.postMessage({error: error.message}); }
    };
  }
  beginStep() {
    this.step = (this.step + 1) & 15;
    const step = this.pattern[this.step];
    const following = this.pattern[(this.step + 1) & 15];
    const duration = stepFrames(this.bpm, sampleRate);
    this.nextStep += duration;
    this.legato = Boolean(step.gate && this.previousGate && this.previousTie);
    this.stepAccent = step.accent;
    this.parameters.set(this.base);

    if (step.gate) {
      if (!this.legato) this.api.opxy_start(this.voice);
      this.targetPitch=Math.round(440 * 2 ** ((step.note - 69) / 12) / sampleRate * 0x100000000);
      if(!this.legato)this.currentPitch=this.targetPitch;
      this.targetGain = step.accent ? .95 : .76;
      this.gateOff = step.slide && following.gate ? -1 : this.frame + duration * this.gateLength;
    } else {this.targetGain = 0; this.gateOff = -1;}
    this.previousGate = step.gate;
    this.previousTie = step.slide;
    this.port.postMessage({step: this.step, playing: true, accent: Boolean(step.accent), legato: this.legato});
  }
  process(_inputs, outputs) {
    const channels = outputs[0];
    if (!channels?.length) return true;
    for (let i=0; i<channels[0].length; i++) {
      if (this.cursor === 64) {
        if (this.playing && this.frame >= this.nextStep) this.beginStep();
        if (this.gateOff >= 0 && this.frame >= this.gateOff) {this.targetGain=0;this.gateOff=-1;}
        this.parameters.set(this.base);
        // Browser-only controller simulation. The device supplies these pitch
        // increments itself; the C++ voice does no portamento or velocity shaping.
        for(let j=0;j<64;j++){
          if(this.legato)this.currentPitch+=(this.targetPitch-this.currentPitch)*this.glide;
          this.pitch[j]=Math.round(this.currentPitch);
        }
        if (this.playing || this.gain > .00001) this.api.opxy_render(this.voice, this.outputAddress, this.parameterAddress, this.pitchAddress);
        else this.output.fill(0);
        this.cursor = 0;
      }
      this.gain += (this.targetGain-this.gain) * (this.targetGain > this.gain ? .025 : .008);
      const sample = this.output[this.cursor++] / 2147483648 * this.gain * 1.5;
      for (let c=0;c<channels.length;c++) channels[c][i]=sample;
      this.frame++;
    }
    return true;
  }
}
registerProcessor('acidline',AcidlineProcessor);
