import {MachineScreen} from './visual-host.mjs';
const $=id=>document.getElementById(id);
const machines=await(await fetch('/machines.json')).json();
const effects=machines.filter(m=>m.kind==='effect');
const synths=machines.filter(m=>m.kind==='synth');
const basePort=Number(location.port);
for(const [i,m] of synths.entries()){
  const a=document.createElement('a');a.href=`http://127.0.0.1:${basePort+i+1}/`;a.target='_blank';a.rel='noopener';a.textContent=m.name;
  const small=document.createElement('small');small.textContent='Play · controls · patch export';a.append(small);$('synths').append(a);
}
for(const m of [{id:'dry',name:'Dry reference',kind:'source'},...machines]){
  const cell=document.createElement('div');cell.className='recording';const title=document.createElement('p');title.textContent=`${m.name} · ${m.kind}`;
  const audio=document.createElement('audio');audio.controls=true;audio.preload='none';audio.src=`/audio/${m.id}.wav`;cell.append(title,audio);$('recordings').append(cell);
}
let selected,values=[],context,node,bypass=false,fileLoaded=false,loadedFile,generation=0,workletReady,screen;
const status=text=>$('status').textContent=text;
for(const m of effects){const option=document.createElement('option');option.value=m.id;option.textContent=m.name;$('effect').append(option);}
function controls(){node?.port.postMessage({type:'controls',values:[...values,...Array(8-values.length).fill(0)]});}
function master(){node?.port.postMessage({type:'master',send:+$('send').value,monitor:+$('monitor').value,bpm:+$('bpm').value,bypass});}
function formatted(parameter,value){const mapping=parameter.mapping;if(mapping.kind==='enum')return mapping.labels[Math.min(mapping.labels.length-1,Math.floor(value*mapping.labels.length))];return Math.round(value*100)+'%';}
function renderControls(){
  $('controls').replaceChildren();
  selected.definition.parameters.forEach((p,i)=>{
    const cell=document.createElement('div');cell.className='control';const label=document.createElement('label');label.htmlFor='parameter-'+p.id;
    const name=document.createElement('span');name.textContent=p.label;const out=document.createElement('output');out.textContent=formatted(p,values[i]);label.append(name,out);
    const slider=document.createElement('input');slider.id='parameter-'+p.id;slider.type='range';slider.min=0;slider.max=1;slider.step=.001;slider.value=values[i];
    slider.oninput=()=>{values[i]=+slider.value;out.textContent=formatted(p,values[i]);controls();screen?.parameters(values,i);};cell.append(label,slider);$('controls').append(cell);
  });
}
async function choose(){
  const token=++generation;
  selected=effects.find(m=>m.id===$('effect').value);values=selected.definition.parameters.map(p=>p.default);renderControls();
  screen?.dispose();screen=null;
  const pending=await MachineScreen.open(document.createElement('div'),selected);
  if(token!==generation){pending.dispose();return;}
  screen=pending;screen.root=$('effect-screen');screen.root.replaceChildren(screen.svg);screen.parameters(values);
  if(context){const running=context.state==='running';await buildNode(token);if(running)await context.resume();}
  status(selected.name+' ready');
}
async function buildNode(token=generation){
  const machine=selected;
  const module=await WebAssembly.compile(await(await fetch(`/live/${machine.id}.wasm`)).arrayBuffer());
  await workletReady;if(token!==generation)return;
  if(node){node.port.onmessage=null;node.port.postMessage({type:'dispose'});node.disconnect();}
  node=new AudioWorkletNode(context,'hundekraft-effect',{numberOfInputs:0,numberOfOutputs:1,outputChannelCount:[2],processorOptions:{module,dryIndex:machine.definition.parameters.findIndex(p=>p.id==='dry')}});
  const current=node;
  node.onprocessorerror=()=>{if(node===current)status('Audio stopped. Reload to restart.');};
  node.port.onmessage=event=>{if(node===current&&event.data.type==='meter'){screen?.gate(event.data.wetPeak>.0001);status(`${machine.name} · ${event.data.peak>0?(20*Math.log10(event.data.peak)).toFixed(1)+' dB':'silent'}`);}};
  node.connect(context.destination);controls();master();node.port.postMessage({type:'source',value:$('source').value});
  if(loadedFile)node.port.postMessage({type:'file',...loadedFile});
}
async function enable(){
  if(!context){context=new AudioContext({sampleRate:44100});if(context.sampleRate!==44100)throw Error('44.1 kHz audio is required');workletReady=context.audioWorklet.addModule('/worklet.mjs');await buildNode();}
  await context.resume();$('audio').textContent='Mute audio';for(const id of ['play','stop','reset'])$(id).disabled=false;
}
$('effect').onchange=()=>choose().catch(e=>status(e.message));
$('audio').onclick=async()=>{try{if(context?.state==='running'){await context.suspend();screen?.gate(false);$('audio').textContent='Enable audio';status('Audio muted');}else await enable();}catch(e){status(e.message);}};
$('play').onclick=()=>{if($('source').value==='file'&&!fileLoaded){status('Load an audio file first');return;}node?.port.postMessage({type:'play'});};
$('stop').onclick=()=>node?.port.postMessage({type:'stop'});
$('reset').onclick=()=>node?.port.postMessage({type:'reset'});
$('source').onchange=()=>node?.port.postMessage({type:'source',value:$('source').value});
$('bypass').onclick=()=>{bypass=!bypass;$('bypass').setAttribute('aria-pressed',bypass);$('bypass').textContent=bypass?'A/B · dry only':'A/B · effect on';master();};
for(const id of ['send','monitor','bpm'])$(id).oninput=master;
$('file').onchange=async()=>{try{const file=$('file').files[0];if(!file)return;await enable();const audio=await context.decodeAudioData(await file.arrayBuffer());loadedFile={left:audio.getChannelData(0).slice(),right:audio.getChannelData(Math.min(1,audio.numberOfChannels-1)).slice()};node.port.postMessage({type:'file',...loadedFile});fileLoaded=true;$('source').value='file';$('fileinfo').textContent=file.name+' · '+audio.duration.toFixed(1)+' seconds · local only';}catch(e){status(e.message);}};
window.addEventListener('keydown',e=>{if(e.key==='Escape')node?.port.postMessage({type:'stop'});});
await choose();
window.audition={getState:()=>({machine:selected.id,values:[...values],audio:context?.state??'off'}),setValues:next=>{values=[...next];renderControls();controls();screen?.parameters(values);}};
