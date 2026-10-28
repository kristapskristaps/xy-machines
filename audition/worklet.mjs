class HaloProcessor extends AudioWorkletProcessor {
  constructor(options) {
    super();this.fx=new WebAssembly.Instance(options.processorOptions.module,{}).exports;
    this.left=new Int32Array(this.fx.memory.buffer,this.fx.halo_left(),64);
    this.right=new Int32Array(this.fx.memory.buffer,this.fx.halo_right(),64);
    this.params=new Int32Array(this.fx.memory.buffer,this.fx.halo_parameters(),8);
    this.settings=new Float64Array(8);this.send=.6;this.monitor=.65;this.bpm=120;this.bypass=false;
    this.voices=Array.from({length:12},()=>({on:false,held:false,note:0,phase:0,age:0,env:0}));
    this.inputPair=new Float64Array(2);this.dryL=new Float64Array(64);this.dryR=new Float64Array(64);this.source='keys';this.frame=0;this.until=0;this.next=0;this.phrase=-1;this.file=null;this.fileAt=0;this.fileOn=false;this.meter=0;this.random=37;
    this.fx.halo_reset();
    this.port.onmessage=event=>this.message(event.data);
  }
  message(m) {
    if(m.type==='controls'){this.settings.set(m.values);for(let i=0;i<8;i++)this.params[i]=Math.round(Math.max(0,Math.min(1,m.values[i]))*2147483647);}
    if(m.type==='master'){this.send=m.send;this.monitor=m.monitor;this.bpm=m.bpm;this.bypass=m.bypass;}
    if(m.type==='source'){this.stop();this.source=m.value;}
    if(m.type==='note'){if(m.on)this.note(m.note,true);else for(const v of this.voices)if(v.note===m.note)v.held=false;}
    if(m.type==='play'){this.stop();if(this.source==='file'&&this.file){this.fileAt=0;this.fileOn=true;}else{this.phrase=0;this.next=this.frame;}}
    if(m.type==='stop')this.stop();
    if(m.type==='reset')this.fx.halo_reset();
    if(m.type==='file'){this.stop();this.file=[m.left,m.right];this.fileAt=0;this.source='file';}
  }
  stop(){this.phrase=-1;this.fileOn=false;for(const v of this.voices)v.held=false;}
  note(note,held=false) {
    let v=this.voices.find(v=>!v.on);if(!v)v=this.voices.reduce((a,b)=>a.age>b.age?a:b);
    Object.assign(v,{on:true,held,note,phase:0,age:0,env:0});
  }
  input() {
    if(this.phrase>=0 && this.frame>=this.next) {
      const notes=[48,55,60,63,50,57,60,65,53,60,65,68,55,62,65,70];
      if(this.phrase>=16)this.phrase=-1;
      else{this.note(notes[this.phrase++]);this.next+=Math.round(sampleRate*30/this.bpm);}
    }
    if(this.source==='file') {
      if(this.fileOn && this.fileAt<this.file[0].length){this.inputPair[0]=this.file[0][this.fileAt];this.inputPair[1]=this.file[1][this.fileAt++];return;}
      this.fileOn=false;this.inputPair[0]=this.inputPair[1]=0;return;
    }
    let value=0;
    for(const v of this.voices)if(v.on) {
      const t=v.age/sampleRate,hz=440*2**((v.note-69)/12);v.phase+=hz/sampleRate;
      const a=2*Math.PI*v.phase;
      if(this.source==='drums') {
        this.random=(Math.imul(this.random,1664525)+1013904223)|0;
        value+=.17*Math.sin(2*Math.PI*(58*t+3*(1-Math.exp(-t*25))))*Math.exp(-t*8)+.028*(this.random/2147483648)*Math.exp(-t*32);
        if(t>1.2)v.on=false;
      }else{
        const attack=Math.min(1,t/.004);const decay=v.held?1:Math.exp(-t*(this.source==='pluck'?3.8:2.1));
        if(!v.held && t>2.8){v.on=false;continue;}
        v.env+=(attack*decay-v.env)*.008;
        value+=v.env*(Math.sin(a)*.075+Math.sin(a*2.003)*.023*Math.exp(-t*2)+Math.sin(a*4.01)*.009*Math.exp(-t*6));
      }
      v.age++;
    }
    this.inputPair[0]=this.inputPair[1]=value;
  }
  process(_inputs,outputs) {
    const out=outputs[0];if(!out||out.length<2)return true;
    let peak=0;
    for(let base=0;base<out[0].length;base+=64) {
      for(let i=0;i<64;i++){
        this.input();const l=this.inputPair[0],r=this.inputPair[1];this.dryL[i]=l;this.dryR[i]=r;
        this.left[i]=Math.max(-2147483647,Math.min(2147483647,Math.round(l*this.send*2147483647)));
        this.right[i]=Math.max(-2147483647,Math.min(2147483647,Math.round(r*this.send*2147483647)));this.frame++;
      }
      this.fx.halo_process(Math.round(this.bpm*10),Math.floor(this.frame/sampleRate*this.bpm/60*96),this.phrase>=0||this.fileOn?1:0);
      const dry=1-this.send*(1-this.settings[7]);
      for(let i=0;i<64;i++){
        const l=this.bypass?this.dryL[i]:this.dryL[i]*dry+this.left[i]/2147483648;
        const r=this.bypass?this.dryR[i]:this.dryR[i]*dry+this.right[i]/2147483648;
        out[0][base+i]=Math.max(-1,Math.min(1,l*this.monitor));out[1][base+i]=Math.max(-1,Math.min(1,r*this.monitor));
        peak=Math.max(peak,Math.abs(l),Math.abs(r));
      }
    }
    if(++this.meter>=32){this.meter=0;this.port.postMessage({type:'meter',peak,playing:this.phrase>=0||this.fileOn});}
    return true;
  }
}
registerProcessor('halo-audition',HaloProcessor);
