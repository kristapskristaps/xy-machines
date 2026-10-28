import {PreviewDisplay,SvgDisplay,headerSvg} from './display.mjs';
import {shownControls} from './controls.mjs';

export class MachineScreen {
  constructor(root,machine,module,asset,metadata){
    this.root=root;this.machine=machine;this.display=new PreviewDisplay(module);
    this.renderer=new SvgDisplay(asset,metadata);this.values=machine.definition.parameters.map(p=>p.default);
    this.shown=[0,1,2,3];this.serial=0;this.released=0;this.held=0;this.previous=0;this.live=true;
    this.svg=document.createElementNS('http://www.w3.org/2000/svg','svg');
    this.svg.setAttribute('viewBox','0 0 480 220');this.svg.setAttribute('role','img');
    this.svg.setAttribute('aria-label',machine.name+' reconstructed screen');
    this.header=document.createElementNS('http://www.w3.org/2000/svg','g');
    this.svg.append(this.header,this.renderer.root);root.replaceChildren(this.svg);
    this.motion=matchMedia('(prefers-reduced-motion: reduce)');
    this.draw=this.draw.bind(this);this.request=requestAnimationFrame(this.draw);
  }
  static async open(root,machine){
    const [bytes,asset,metadata]=await Promise.all([fetch(`/visuals/${machine.id}.wasm`).then(r=>r.arrayBuffer()),
      fetch(`/visuals/${machine.id}.svg`).then(r=>r.text()),fetch(`/visuals/${machine.id}.json`).then(r=>r.json())]);
    return new MachineScreen(root,machine,await WebAssembly.compile(bytes),asset,metadata);
  }
  parameters(values,edited=-1){
    this.values=[...values];
    if(edited>=0){
      if(this.machine.definition.press==='page'){
        const secondary=edited>=4;this.shown=[0,1,2,3];
        this.machine.definition.parameters.forEach((p,i)=>{if((i>=4)===secondary)this.shown[p.control??i%4]=i;});
      }else this.shown[this.machine.definition.parameters[edited].control??edited%4]=edited;
    }
  }
  gate(on){if(on&&!this.held)this.serial=(this.serial+1)>>>0;if(!on&&this.held)this.released=(this.released+1)>>>0;this.held=on?1:0;}
  strike(){this.serial=(this.serial+1)>>>0;}
  draw(time){
    if(!this.live)return;
    try{
      const elapsed=this.previous?Math.min(102,Math.round((time-this.previous)*1.024)):0;this.previous=time;
      const words=[...this.values.map(v=>Math.round(v*0x7fffffff)),...Array(8-this.values.length).fill(0)];
      const frame=this.display.tick(shownControls(words,this.shown),elapsed,this.serial,0,this.motion.matches||this.reduced,
        {held_notes:this.held,note_off_serial:this.released});
      this.renderer.apply(frame);
      const parameters=this.machine.definition.parameters;
      const titles=this.shown.map(i=>parameters[i]?.label.toLowerCase()??'');
      const values=this.shown.map(i=>{const p=parameters[i];return p?.mapping.kind==='enum'?p.mapping.labels[Math.min(p.mapping.labels.length-1,Math.floor((this.values[i]??0)*p.mapping.labels.length))]:String(Math.round((this.values[i]??0)*100));});
      this.header.innerHTML=headerSvg(titles,this.renderer.header.map((v,i)=>v||values[i]));
      this.request=requestAnimationFrame(this.draw);
    }catch(error){this.root.dataset.error=error.message;this.live=false;console.error(error);}
  }
  dispose(){this.live=false;cancelAnimationFrame(this.request);}
}
