import {DrumAudition} from './engine.mjs';
class Processor extends AudioWorkletProcessor {
  constructor(options) {
    super();
    const {module, pads} = options.processorOptions;
    this.engine = new DrumAudition(module, sampleRate, pads, currentFrame);
    this.port.onmessage = ({data}) => {
      try {
        if (data.type === 'pad') this.engine.setPad(data.index, data.pad);
        if (data.type === 'hit') data.when === undefined ? this.engine.hit(data.index) : this.engine.schedule(data.index, data.when);
        if (data.type === 'clear') this.engine.clearQueue();
        if (data.type === 'panic') this.engine.panic();
      } catch (error) { this.port.postMessage({error:error.message}); }
    };
    this.port.postMessage({ready:true});
  }
  process(_inputs, outputs) {
    const channels = outputs[0];
    if (channels?.length) { this.engine.render(channels[0]); for (let i = 1; i < channels.length; ++i) channels[i].set(channels[0]); }
    return true;
  }
}
registerProcessor('x0x-drums', Processor);
