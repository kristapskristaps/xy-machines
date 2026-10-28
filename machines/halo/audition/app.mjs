const $=id=>document.getElementById(id);
const definition=await (await fetch('definition.json')).json();
const values=definition.parameters.map(p=>p.default),divisions=['1/32','1/32.','1/16','1/16.','1/8','1/8.','1/4','1/2'];
let context,node,module,bypass=false,level=0,playing=false,activePreset='Soft Space';
const status=text=>$('status').textContent=text;
const sendControls=()=>node?.port.postMessage({type:'controls',values});
function master(){node?.port.postMessage({type:'master',send:+$('send').value,monitor:+$('monitor').value,bpm:+$('bpm').value,bypass});}
function formatted(i){return i===5?divisions[Math.min(7,Math.floor(values[i]*8))]:i===6?(values[i]>.5?'HELD':'OPEN'):Math.round(values[i]*100)+'%';}
function update(){
  definition.parameters.forEach((p,i)=>{
    const element=$('param-'+p.id);if(i!==6)element.value=values[i];else{element.setAttribute('aria-pressed',String(values[i]>.5));element.textContent=values[i]>.5?'Release':'Hold';}
    $('value-'+p.id).textContent=formatted(i);$('dial-'+p.id).style.setProperty('--amount',values[i]);
  });
  for(const name of ['distance','bloom','echo','shimmer'])$('screen'+name[0].toUpperCase()+name.slice(1)).textContent=String(Math.round(values[definition.parameters.findIndex(p=>p.id===name)]*100)).padStart(2,'0');
  $('presetName').textContent=activePreset.toUpperCase();$('displayTime').textContent=formatted(5);
  for(const b of $('presets').children)b.setAttribute('aria-pressed',String(b.dataset.name===activePreset));
  sendControls();
}
definition.parameters.forEach((p,i)=>{
  const cell=document.createElement('div');cell.className='control';
  const label=document.createElement('label');label.htmlFor='param-'+p.id;label.innerHTML=`<span class="index">${String(i+1).padStart(2,'0')}</span>${p.label}`;
  const dial=document.createElement('div');dial.className='dial';dial.id='dial-'+p.id;dial.setAttribute('aria-hidden','true');
  const out=document.createElement('output');out.id='value-'+p.id;out.htmlFor='param-'+p.id;
  const control=document.createElement(i===6?'button':'input');control.id='param-'+p.id;
  if(i===6){control.className='hold';control.setAttribute('aria-label','Hold cloud');control.onclick=()=>{values[i]=values[i]>.5?0:1;activePreset='Your cloud';update();};}
  else{control.type='range';control.min=0;control.max=1;control.step=.001;control.oninput=()=>{values[i]=+control.value;activePreset='Your cloud';update();};}
  cell.append(label,dial,out,control);$('controls').append(cell);
});
const descriptions=['Warm room / faint halo','Repeats into a cloud','Long tail / upper light'];
Object.entries(definition.presets).forEach(([name,preset],i)=>{
  const button=document.createElement('button');button.dataset.name=name;button.innerHTML=name+`<small>${descriptions[i]}</small>`;
  button.onclick=()=>{definition.parameters.forEach((p,i)=>values[i]=preset[p.id]);activePreset=name;update();};$('presets').append(button);
});
update();
try{module=await WebAssembly.compile(await (await fetch('halo.wasm')).arrayBuffer());$('audio').disabled=false;$('audio').textContent='Enable audio';status('Ready · enable audio to play');}
catch(error){status('DSP could not load: '+error.message);}
async function enable(){
  if(!context){
    context=new AudioContext({sampleRate:44100});
    if(context.sampleRate!==44100)throw new Error('This audition requires a 44.1 kHz AudioContext');
    await context.audioWorklet.addModule('worklet.mjs');
    node=new AudioWorkletNode(context,'halo-audition',{numberOfInputs:0,numberOfOutputs:1,outputChannelCount:[2],processorOptions:{module}});
    node.onprocessorerror=()=>status('Audio processor stopped. Reload to restart.');
    node.port.onmessage=event=>{if(event.data.type==='meter'){level=event.data.peak;if(playing&&!event.data.playing)status('Phrase ended · listening to the tail');playing=event.data.playing;$('peak').textContent=level>0?Math.max(-120,20*Math.log10(level)).toFixed(1)+' dB':'−∞ dB';}};
    node.connect(context.destination);sendControls();master();node.port.postMessage({type:'source',value:$('source').value});
    for(const id of ['play','stop','clear'])$(id).disabled=false;
  }
  await context.resume();$('audio').textContent='Mute audio';$('lamp').classList.add('on');status('Audio on · shared Halo C++ DSP · 44,100 Hz');
}
$('audio').onclick=async()=>{try{if(context?.state==='running'){await context.suspend();$('audio').textContent='Enable audio';$('lamp').classList.remove('on');status('Audio muted');}else await enable();}catch(error){status(error.message);}};
$('play').onclick=()=>{if($('source').value==='file'&&!fileLoaded){status('Load an audio file first');return;}node?.port.postMessage({type:'play'});status($('source').value==='file'?'Playing your file · tail continues afterward':'Playing 16 notes · tail continues afterward');};
$('stop').onclick=()=>{node?.port.postMessage({type:'stop'});status('Input stopped · listening to the tail');};
$('clear').onclick=()=>{node?.port.postMessage({type:'reset'});status('Tail cleared');};
$('source').onchange=()=>{node?.port.postMessage({type:'source',value:$('source').value});$('play').textContent=$('source').value==='file'?'Play audio':'Play phrase';if($('source').value==='file'&&!fileLoaded)status('Choose Load audio to use your own file');};
for(const id of ['send','monitor'])$(id).oninput=()=>{$(id+'Value').textContent=Math.round(+$(id).value*100)+'%';master();};
$('bpm').onchange=()=>{$('bpm').value=Math.max(40,Math.min(240,+$('bpm').value||120));master();};
$('bypass').onclick=()=>{bypass=!bypass;$('bypass').setAttribute('aria-pressed',String(bypass));$('bypass').textContent=bypass?'A/B · dry only':'A/B · effect on';master();};
let fileLoaded=false;
$('load').onclick=()=>$('file').click();
$('file').onchange=async()=>{const file=$('file').files[0];if(!file)return;try{await enable();const audio=await context.decodeAudioData(await file.arrayBuffer());const left=audio.getChannelData(0).slice(),right=audio.getChannelData(Math.min(1,audio.numberOfChannels-1)).slice();node.port.postMessage({type:'file',left,right},[left.buffer,right.buffer]);fileLoaded=true;$('source').value='file';$('play').textContent='Play audio';$('fileinfo').textContent=`${file.name} · ${audio.duration.toFixed(1)} seconds · local only`;status('Audio loaded · press Play phrase');}catch(error){status('Could not decode audio: '+error.message);}};
const letters=['a','w','s','e','d','f','t','g','y','h','u','j','k'],black={1:10,3:24.3,6:53,8:67.3,10:81.5},held=new Set();
function note(index,on){if(on&&held.has(index)||!on&&!held.has(index))return;if(on)held.add(index);else held.delete(index);$('key-'+index)?.classList.toggle('down',on);node?.port.postMessage({type:'note',note:60+index,on});}
letters.forEach((letter,i)=>{const b=document.createElement('button');b.id='key-'+i;b.textContent=letter.toUpperCase();b.setAttribute('aria-label','Play '+['C','C sharp','D','D sharp','E','F','F sharp','G','G sharp','A','A sharp','B','C octave'][i]);if(i in black){b.className='black';b.style.left=black[i]+'%';}b.onpointerdown=async event=>{event.preventDefault();b.setPointerCapture(event.pointerId);try{if(context?.state!=='running')await enable();note(i,true);}catch(error){status(error.message);}};b.onpointerup=b.onpointercancel=()=>note(i,false);$('keyboard').append(b);});
window.addEventListener('keydown',event=>{if(event.target instanceof HTMLInputElement||event.target instanceof HTMLSelectElement)return;if(event.key==='Escape'){$('stop').click();for(const i of [...held])note(i,false);return;}const i=letters.indexOf(event.key.toLowerCase());if(i>=0&&!event.repeat&&context?.state==='running'){event.preventDefault();note(i,true);}});
window.addEventListener('keyup',event=>{const i=letters.indexOf(event.key.toLowerCase());if(i>=0)note(i,false);});
window.addEventListener('blur',()=>{for(const i of [...held])note(i,false);});
const ns='http://www.w3.org/2000/svg',rings=[],particles=[];
for(let i=0;i<9;i++){const ring=document.createElementNS(ns,'ellipse');ring.setAttribute('stroke-width',i===4?'1.1':'.55');$('halos').append(ring);rings.push(ring);}
for(let i=0;i<16;i++){const dot=document.createElementNS(ns,'circle');dot.setAttribute('r',i%3===0?'1.2':'.7');$('particles').append(dot);particles.push(dot);}
const reduced=matchMedia('(prefers-reduced-motion: reduce)');let smooth=0;
function draw(time){smooth+=(level-smooth)*.06;const phase=reduced.matches?0:time*.00025;const energy=Math.min(1,smooth*7);const distance=values[0],bloom=values[1],echo=values[2],shimmer=values[3];
 rings.forEach((ring,i)=>{const spread=i/8;ring.setAttribute('cx',240+Math.sin(phase+spread*3)*energy*4);ring.setAttribute('cy',124+Math.cos(phase*.7+i)*energy*3);ring.setAttribute('rx',42+spread*(115+distance*40));ring.setAttribute('ry',14+spread*(29+bloom*17));ring.setAttribute('transform',`rotate(${Math.sin(phase*.6+i*.2)*(3+energy*6)} 240 124)`);ring.setAttribute('opacity',(.18+(1-spread)*.45+energy*.1).toFixed(3));});
 particles.forEach((dot,i)=>{const t=reduced.matches?i/16:(i/16+time*.00004)%1;dot.setAttribute('cx',173+i*8+Math.sin(i*2+phase)*15);dot.setAttribute('cy',150-t*100);dot.setAttribute('opacity',(shimmer*(.15+energy*.85)*Math.sin(t*Math.PI)).toFixed(3));});
 let path='';for(let i=0;i<80;i++){const x=30+i*5.3,y=183-Math.sin(i*.33-phase*3)*Math.exp(-i/40)*(3+energy*14)*echo;path+=(i?'L':'M')+x.toFixed(1)+' '+y.toFixed(1);}$('echoTrace').setAttribute('d',path);$('displayMode').textContent=values[6]>.5?'HOLD':context?.state!=='running'?'IDLE':playing||held.size?'PLAY':'TAIL';requestAnimationFrame(draw);}
requestAnimationFrame(draw);
