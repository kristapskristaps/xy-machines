import {names, patterns, validatePattern} from './patterns.mjs';
const $ = id => document.getElementById(id);
const manifest = await (await fetch('/preview/manifest.json')).json();
const definition = manifest.definition;
let values = Object.fromEntries(definition.parameters.map(p => [p.id,p.default]));
let patternIndex = 0, pattern = validatePattern(patterns[0]);
let context, processor, gain, analyser, scopeData, starting = false, playing = false;
const noteNames = ['C','C♯','D','D♯','E','F','F♯','G','G♯','A','A♯','B'];
const noteName = midi => noteNames[midi%12]+(Math.floor(midi/12)-1);
const hints = {cutoff:'Base frequency',resonance:'Feedback / squelch',drive:'Smooth soft clipping',env:'Filter sweep',decay:'Pluck length',wave:'Three distinct waveforms',sub:'Sine one octave down',damp:'Tame distortion brightness'};
const units = {cutoff:'Hz',resonance:'%',drive:'%',env:'%',decay:'ms',wave:'',sub:'%',damp:'%'};
const readout = (id,v) => id==='wave'?['Saw','Triangle','Square'][Math.min(2,Math.floor(v*3))]:id==='cutoff'?Math.round(45*(12000/45)**v).toLocaleString():id==='decay'?Math.round(55*24**v):Math.round(v*100);
const q31 = () => definition.parameters.map(p => Math.round(values[p.id]*2147483647));
const status = text => {$('status').textContent=text;};
function send(type, extra={}) {processor?.port.postMessage({type,...extra});}
function updateControls() {
  for (const p of definition.parameters) {
    $(p.id).value = values[p.id];
    $('value-'+p.id).textContent=readout(p.id,values[p.id]);
  }
  send('parameters',{values:q31()});
}
for (const p of definition.parameters) {
  const el=document.createElement('div');el.className='control';
  el.innerHTML=`<label for="${p.id}">${p.label}<span>${String(definition.parameters.indexOf(p)+1).padStart(2,'0')}</span></label><div class="readout"><span id="value-${p.id}"></span><span class="unit">${units[p.id]}</span></div><input type="range" id="${p.id}" min="0" max="1" step="0.001" aria-label="${p.label}"><small>${hints[p.id]}</small>`;
  $('controls').append(el);
  $(p.id).addEventListener('input',()=>{values[p.id]=Number($(p.id).value);$('preset').value='';updateControls();});
}
const custom=document.createElement('option');custom.value='';custom.textContent='Custom';$('preset').append(custom);
for (const name of Object.keys(definition.presets)) {const option=document.createElement('option');option.value=name;option.textContent=name;$('preset').append(option);}
$('preset').addEventListener('change',()=>{if(definition.presets[$('preset').value]){values={...definition.presets[$('preset').value]};updateControls();}});
$('preset').value='Warehouse';values={...definition.presets.Warehouse};updateControls();
for (const [i,name] of names.entries()) {const option=document.createElement('option');option.value=String(i);option.textContent=name;$('pattern').append(option);}
function buildSteps() {
  $('steps').replaceChildren();
  for(let i=0;i<16;i++) {
    const el=document.createElement('div');el.className='step';el.id='step-'+i;
    const number=document.createElement('div');number.className='step-number';number.textContent=String(i+1).padStart(2,'0');el.append(number);
    const select=document.createElement('select');select.setAttribute('aria-label',`Step ${i+1} note`);
    for(let note=24;note<=72;note++){const option=document.createElement('option');option.value=String(note);option.textContent=noteName(note);select.append(option);}
    select.value=String(pattern[i].note);select.addEventListener('change',()=>{pattern[i].note=Number(select.value);send('pattern',{value:pattern});});el.append(select);
    for(const [key,label] of [['gate','G'],['accent','A'],['slide','S']]) {
      const button=document.createElement('button');button.textContent=label;button.dataset.action=key;button.setAttribute('aria-label',`Step ${i+1} ${key}`);button.setAttribute('aria-pressed',String(Boolean(pattern[i][key])));
      button.addEventListener('click',()=>{pattern[i][key]=!pattern[i][key];button.setAttribute('aria-pressed',String(pattern[i][key]));el.classList.toggle('off',!pattern[i].gate);send('pattern',{value:pattern});});el.append(button);
    }
    el.classList.toggle('off',!pattern[i].gate);$('steps').append(el);
  }
}
function changePattern(){patternIndex=Number($('pattern').value);pattern=validatePattern(patterns[patternIndex]);buildSteps();send('pattern',{value:pattern});}
$('pattern').addEventListener('change',changePattern);$('reset').addEventListener('click',changePattern);
$('mutate').addEventListener('click',()=>{
  const scale=[0,0,3,5,7,10,12];
  pattern=pattern.map((s,i)=>({...s,note:i===0?36:Math.random()<.35?36+scale[Math.floor(Math.random()*scale.length)]:s.note,gate:i===0||Math.random()>.2,accent:Math.random()<.3,slide:Math.random()<.25}));
  buildSteps();send('pattern',{value:pattern});
});buildSteps();
$('tempo').addEventListener('change',()=>{const bpm=Number($('tempo').value);if(!Number.isFinite(bpm)||bpm<60||bpm>200){$('tempo').value='132';status('Tempo must be between 60 and 200 BPM.');}send('tempo',{bpm:Number($('tempo').value)});});
$('gate-length').addEventListener('input',()=>send('gate',{value:Number($('gate-length').value)}));
$('monitor').addEventListener('input',()=>{if(gain)gain.gain.setTargetAtTime(Number($('monitor').value),context.currentTime,.015);});
async function enable() {
  if(context){await context.resume();return;}
  context=new AudioContext({sampleRate:44100,latencyHint:'interactive'});
  try {
    const wasm=await(await fetch('/preview/engine.wasm')).arrayBuffer();
    const hash=[...new Uint8Array(await crypto.subtle.digest('SHA-256',wasm))].map(x=>x.toString(16).padStart(2,'0')).join('');
    if(hash!==manifest.wasm_sha256)throw new Error('The preview source does not match its manifest.');
    const module=await WebAssembly.compile(wasm);
    await context.audioWorklet.addModule('/acidline/processor.mjs');
    processor=new AudioWorkletNode(context,'acidline',{numberOfInputs:0,numberOfOutputs:1,outputChannelCount:[1],processorOptions:{module,parameters:q31(),pattern}});
    gain=context.createGain();gain.gain.value=Number($('monitor').value);
    analyser=context.createAnalyser();analyser.fftSize=2048;scopeData=new Float32Array(analyser.fftSize);
    processor.connect(gain).connect(analyser).connect(context.destination);
    processor.port.onmessage=({data})=>{
      if(data.error){status(data.error);return;}
      if(Number.isInteger(data.step)) {
        document.querySelectorAll('.step.current').forEach(el=>el.classList.remove('current'));
        if(data.step>=0)$('step-'+data.step).classList.add('current');
        $('activity').textContent=data.step<0?'STOPPED':data.legato?'SLIDE':data.accent?'ACCENT':'STEP '+String(data.step+1).padStart(2,'0');
      }
    };
    processor.onprocessorerror=()=>{playing=false;$('play').textContent='Play';$('play').setAttribute('aria-pressed','false');status('Audio engine stopped. Reload to try again.');};
    await context.resume();
  } catch(error){await context.close();context=undefined;processor=undefined;throw error;}
}
async function toggle() {
  if(starting)return;
  if(playing){playing=false;send('stop');$('play').textContent='Play';$('play').setAttribute('aria-pressed','false');status('Stopped.');return;}
  starting=true;$('play').disabled=true;
  try {
    await enable();send('tempo',{bpm:Number($('tempo').value)});send('gate',{value:Number($('gate-length').value)});send('pattern',{value:pattern});send('parameters',{values:q31()});send('start');
    playing=true;$('play').textContent='Stop';$('play').setAttribute('aria-pressed','true');status('Playing · 16 steps · 44.1 kHz');
  } catch(error){status('Audio unavailable: '+error.message);}
  finally{starting=false;$('play').disabled=false;}
}
$('play').addEventListener('click',toggle);
document.addEventListener('keydown',event=>{
  if(event.code==='Space'&&!/INPUT|SELECT|BUTTON/.test(event.target.tagName)){event.preventDefault();toggle();}
  if(event.code==='Escape'&&playing)toggle();
});
window.addEventListener('pagehide',()=>send('stop'));
const canvas=$('scope'),painter=canvas.getContext('2d');
function draw() {
  const width=canvas.width,height=canvas.height;
  painter.clearRect(0,0,width,height);painter.strokeStyle='#626b66';painter.lineWidth=1;painter.beginPath();painter.moveTo(0,height/2);painter.lineTo(width,height/2);painter.stroke();
  if(analyser){
    analyser.getFloatTimeDomainData(scopeData);painter.strokeStyle='#ec8453';painter.lineWidth=1.5;painter.beginPath();
    for(let i=0;i<width;i++){const x=scopeData[Math.floor(i*scopeData.length/width)];const y=height/2-x*height*1.8;if(i===0)painter.moveTo(i,y);else painter.lineTo(i,y);}painter.stroke();
  }
  requestAnimationFrame(draw);
}draw();
