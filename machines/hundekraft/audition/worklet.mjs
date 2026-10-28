// Local test source and stereo send audition for the compiled C++ effects.
class EffectAudition extends AudioWorkletProcessor {
  constructor({processorOptions}) {
    super();
    this.api=new WebAssembly.Instance(processorOptions.module,{}).exports;
    const memory=this.api.memory.buffer;
    this.left=new Int32Array(memory,this.api.xy_left(),64);
    this.right=new Int32Array(memory,this.api.xy_right(),64);
    this.parameters=new Int32Array(memory,this.api.xy_parameters(),8);
    this.normalized=new Float64Array(8);
    this.dryIndex=processorOptions.dryIndex;
    this.dryLeft=new Float32Array(64);this.dryRight=new Float32Array(64);
    this.send=.65;this.monitor=.45;this.tempo=120;this.bypass=false;
    this.mode='keys';this.playing=false;this.position=0;this.total=0;this.report=0;this.active=true;
    this.file=null;this.noise=0x12345;
    this.api.xy_reset();
    this.port.onmessage=event=>{
      const m=event.data;
      switch(m.type){
        case 'controls':
          this.normalized.set(m.values);
          for(let i=0;i<8;i++)this.parameters[i]=Math.round(Math.max(0,Math.min(1,m.values[i]))*2147483647);
          break;
        case 'master':this.send=m.send;this.monitor=m.monitor;this.tempo=m.bpm;this.bypass=m.bypass;break;
        case 'source':this.mode=m.value;this.playing=false;break;
        case 'file':this.file=[m.left,m.right];this.mode='file';this.position=0;this.playing=false;break;
        case 'play':this.position=0;this.playing=true;break;
        case 'stop':this.playing=false;break;
        case 'reset':this.api.xy_reset();break;
        case 'dispose':this.active=false;this.playing=false;break;
      }
    };
  }
  source(channel) {
    if(!this.playing)return 0;
    if(this.mode==='file'){
      if(!this.file||this.position>=this.file[0].length){this.playing=false;return 0;}
      return this.file[channel][this.position];
    }
    const seconds=this.position/sampleRate,beat=30/this.tempo;
    const step=Math.floor(seconds/beat);
    if(step>=16){this.playing=false;return 0;}
    const interval=[0,7,12,15,19,15,12,7][step%8];
    const frequency=130.81278265*2**(interval/12);
    const age=seconds-step*beat;
    if(this.mode==='drums'){
      this.noise=(Math.imul(this.noise,1103515245)+12345)|0;
      return .18*Math.sin(2*Math.PI*(52*age+2*(1-Math.exp(-32*age))))*Math.exp(-24*age)
        +.035*(this.noise/2147483648)*Math.exp(-45*age);
    }
    const phase=2*Math.PI*frequency*age;
    const envelope=Math.min(1,age/.005)*Math.exp(-(this.mode==='pluck'?9:4)*age);
    return envelope*(.13*Math.sin(phase)+.035*Math.sin(phase*2.002)+.015*Math.sin(phase*3.001));
  }
  process(_input,output) {
    if(!this.active)return false;
    const stereo=output[0];if(!stereo||stereo.length!==2)return true;
    let peak=0,wetPeak=0;
    for(let offset=0;offset<stereo[0].length;offset+=64){
      for(let i=0;i<64;i++){
        const l=this.source(0),r=this.mode==='file'?this.source(1):l;
        this.dryLeft[i]=l;this.dryRight[i]=r;
        this.left[i]=Math.max(-2147483648,Math.min(2147483647,Math.round(l*this.send*2147483647)));
        this.right[i]=Math.max(-2147483648,Math.min(2147483647,Math.round(r*this.send*2147483647)));
        if(this.playing)this.position++;
        this.total++;
      }
      this.api.xy_process(Math.round(this.tempo*10),Math.floor(this.total*this.tempo*96/(sampleRate*60)),this.playing?1:0);
      const dry=1-this.send*(1-this.normalized[this.dryIndex]);
      for(let i=0;i<64;i++){
        wetPeak=Math.max(wetPeak,Math.abs(this.left[i]/2147483648),Math.abs(this.right[i]/2147483648));
        const l=this.bypass?this.dryLeft[i]:this.dryLeft[i]*dry+this.left[i]/2147483648;
        const r=this.bypass?this.dryRight[i]:this.dryRight[i]*dry+this.right[i]/2147483648;
        stereo[0][offset+i]=Math.max(-1,Math.min(1,l*this.monitor));
        stereo[1][offset+i]=Math.max(-1,Math.min(1,r*this.monitor));
        peak=Math.max(peak,Math.abs(l),Math.abs(r));
      }
    }
    if(++this.report===24){this.report=0;this.port.postMessage({type:'meter',peak,wetPeak,playing:this.playing});}
    return true;
  }
}
registerProcessor('hundekraft-effect',EffectAudition);
