import {MODELS,FAMILIES,LAYOUT,KEYS,available,family,stepFamily,defaultPad,controlNames,word} from './models.mjs';
import {PreviewDisplay,SvgDisplay,NoteActivity} from '/sdk/browser/display.mjs';
import {shownControls} from '/sdk/browser/controls.mjs';

const $ = id => document.getElementById(id);
const pads = LAYOUT.map((_, i) => defaultPad(i));
let selected = 0, page = 0, configLocked = false, shifted = false;
let module, display, scene, context, processor, outputGain, analyser, starting;
const waveform = new Float32Array(64), activity = new NoteActivity();
const padButtons = [], encoders = [], familyButtons = [], timers = new Set();
const reduced = matchMedia('(prefers-reduced-motion: reduce)');
let beatTimer = null, nextTime = 0, step = 0, heldLevel = 0;
const status = value => { $('status').textContent = value; };
const fail = error => { document.body.classList.add('error'); status(error.message || String(error)); };
const send = message => processor?.port.postMessage(message);
const isConfig = () => configLocked || shifted;
function later(callback, delay) { const timer = setTimeout(() => { timers.delete(timer); callback(); }, delay); timers.add(timer); }

async function enableAudio() {
  if (starting) return starting;
  if (context) { await context.resume(); return; }
  starting = (async () => {
    if (!module) throw new Error('Sound engine is still loading');
    const audio = new AudioContext({sampleRate:44100});
    await audio.resume();
    await audio.audioWorklet.addModule('/x0x/processor.mjs');
    const node = new AudioWorkletNode(audio, 'x0x-drums', {numberOfInputs:0,numberOfOutputs:1,outputChannelCount:[1],processorOptions:{module,pads}});
    const gain = audio.createGain(), meter = audio.createAnalyser(); meter.fftSize = 64;
    gain.gain.value = Number($('volume').value); node.connect(gain).connect(meter).connect(audio.destination);
    await new Promise((resolve, reject) => {
      const timeout = setTimeout(() => reject(new Error('Audio engine did not start')), 5000);
      node.port.onmessage = ({data}) => {
        if (data.ready) { clearTimeout(timeout); resolve(); }
        if (data.error) { clearTimeout(timeout); reject(new Error(data.error)); fail(new Error(data.error)); }
      };
    });
    context = audio; processor = node; outputGain = gain; analyser = meter;
    await audio.resume(); status('Audio on · all 47 voices ready');
  })();
  try { await starting; } finally { starting = null; }
}
function flash(index) {
  activity.noteOn(53 + index); padButtons[index].classList.add('hit');
  later(() => { padButtons[index].classList.remove('hit'); activity.noteOff(53 + index); }, 85);
}
async function hit(index = selected) {
  try { await enableAudio(); send({type:'hit',index}); flash(index); } catch (error) { fail(error); }
}
function select(index) { selected = index; update(); }
function publish() { send({type:'pad',index:selected,pad:pads[selected]}); update(); }
function styleStep(delta, audition = true) { stepFamily(pads[selected], delta); publish(); if (audition) hit(); }
function adjust(column, delta) {
  if (!delta) return;
  const pad = pads[selected];
  if (isConfig()) {
    if (column === 0) pad.model = Math.max(0, Math.min(MODELS.length - 1, pad.model + delta));
    if (column === 1) pad.choke = Math.max(0, Math.min(3, pad.choke + delta));
  } else {
    const slot = page * 4 + column;
    if (!controlNames(pad.model)[slot]) return;
    if (slot === 0) { styleStep(delta); return; }
    pad.raw[slot] = Math.max(0, Math.min(127, pad.raw[slot] + delta));
  }
  publish();
}
function switchPage(value = 1 - page) { page = value; update(); }
function update() {
  const pad = pads[selected], names = controlNames(pad.model), choices = available(pad.model), chosen = family(pad);
  $('selection').textContent = `PAD ${String(selected + 1).padStart(2,'0')} / ${MODELS[pad.model][0].toUpperCase()}`;
  $('type').value = String(pad.model); $('choke').value = String(pad.choke);
  $('config').setAttribute('aria-pressed', String(isConfig()));
  for (let p = 0; p < 2; ++p) $(`page-${p}`).setAttribute('aria-pressed', String(page === p));
  familyButtons.forEach((button, i) => { button.disabled = !choices.includes(i); button.setAttribute('aria-pressed', String(chosen === i)); });
  $('previous-family').disabled = chosen === choices[0]; $('next-family').disabled = chosen === choices.at(-1);
  padButtons.forEach((button, i) => {
    button.classList.toggle('selected', i === selected); button.setAttribute('aria-pressed', String(i === selected));
    button.querySelector('.pad-name').textContent = MODELS[pads[i].model][0];
    button.querySelector('.pad-family').textContent = FAMILIES[family(pads[i])];
  });
  encoders.forEach((encoder, column) => {
    const slot = page * 4 + column;
    const title = isConfig() ? ['machine','choke','',''][column] : names[slot];
    const raw = isConfig() ? (column === 0 ? Math.round(pad.model * 127 / 17) : pad.choke * 42) : pad.raw[slot];
    encoder.label.textContent = title || '—'; encoder.knob.disabled = !title; encoder.range.disabled = !title;
    encoder.knob.setAttribute('aria-label', `${title || 'Unused'} encoder; click to change page`);
    encoder.knob.style.setProperty('--angle', `${-135 + raw * 270 / 127}deg`);
    encoder.range.max = String(isConfig() ? (column === 0 ? 17 : column === 1 ? 3 : 127) : slot === 0 ? choices.length - 1 : 127);
    encoder.range.value = String(isConfig() ? (column === 0 ? pad.model : column === 1 ? pad.choke : 0) : slot === 0 ? choices.indexOf(chosen) : raw);
  });
}

for (let i = 0; i < 24; ++i) {
  const button = document.createElement('button'); button.className = 'pad'; button.id = `pad-${i + 1}`;
  button.innerHTML = `<span class="pad-number">${String(i + 1).padStart(2,'0')}</span><span class="pad-name"></span><span class="pad-family"></span><kbd>${KEYS[i]}</kbd>`;
  button.addEventListener('click', () => { select(i); hit(i); }); $('pads').append(button); padButtons.push(button);
}
MODELS.forEach(([name], i) => { const option = document.createElement('option'); option.value = i; option.textContent = name; $('type').append(option); });
$('type').addEventListener('change', () => { pads[selected].model = Number($('type').value); publish(); hit(); });
$('choke').addEventListener('change', () => { pads[selected].choke = Number($('choke').value); publish(); });
FAMILIES.forEach((name, i) => {
  const button = document.createElement('button'); button.textContent = name; button.id = `style-${name}`;
  button.addEventListener('click', () => { pads[selected].raw[0] = 16 + 32 * i; publish(); hit(); });
  button.addEventListener('keydown', event => { if (event.key === 'ArrowLeft' || event.key === 'ArrowRight') { event.preventDefault(); styleStep(event.key === 'ArrowRight' ? 1 : -1); } });
  $('families').append(button); familyButtons.push(button);
});
$('previous-family').addEventListener('click', () => styleStep(-1)); $('next-family').addEventListener('click', () => styleStep(1));
for (let column = 0; column < 4; ++column) {
  const item = document.createElement('div'); item.className = 'encoder';
  item.innerHTML = `<div class="encoder-label"></div><button class="dial" type="button"></button><output></output><input type="range" min="0" max="127" step="1" aria-label="Encoder ${column + 1} value">`;
  $('encoders').append(item);
  const encoder = {label:item.querySelector('.encoder-label'),knob:item.querySelector('.dial'),range:item.querySelector('input'),output:item.querySelector('output')};
  encoders.push(encoder); let drag = null;
  encoder.knob.addEventListener('pointerdown', event => { if (event.button !== 0) return; event.preventDefault(); encoder.knob.setPointerCapture(event.pointerId); drag = {id:event.pointerId,y:event.clientY,start:event.clientY,moved:false}; });
  encoder.knob.addEventListener('pointermove', event => {
    if (!drag || event.pointerId !== drag.id) return;
    const discrete = isConfig() || page === 0 && column === 0, distance = discrete ? 9 : 2;
    const delta = Math.trunc((drag.y - event.clientY) / distance);
    if (delta) { drag.y -= delta * distance; drag.moved = true; adjust(column, delta); }
  });
  encoder.knob.addEventListener('pointerup', event => { if (!drag) return; const click = !drag.moved && Math.abs(event.clientY - drag.start) < 4; drag = null; if (encoder.knob.hasPointerCapture(event.pointerId)) encoder.knob.releasePointerCapture(event.pointerId); if (click) switchPage(); });
  for (const type of ['pointercancel','lostpointercapture']) encoder.knob.addEventListener(type, () => { drag = null; });
  encoder.knob.addEventListener('wheel', event => { event.preventDefault(); adjust(column, -Math.sign(event.deltaY)); }, {passive:false});
  encoder.knob.addEventListener('keydown', event => { const delta = {ArrowUp:1,ArrowRight:1,ArrowDown:-1,ArrowLeft:-1}[event.key]; if (delta) { event.preventDefault(); adjust(column, delta); } });
  encoder.knob.addEventListener('click', event => { if (event.detail === 0) switchPage(); });
  encoder.range.addEventListener('input', () => {
    const pad = pads[selected], value = Number(encoder.range.value);
    if (isConfig()) { if (column === 0) pad.model = value; if (column === 1) pad.choke = value; }
    else if (page === 0 && column === 0) { pad.raw[0] = 16 + 32 * available(pad.model)[value]; }
    else pad.raw[page * 4 + column] = value;
    publish();
  });
}
$('page-0').addEventListener('click', () => switchPage(0)); $('page-1').addEventListener('click', () => switchPage(1));
$('config').addEventListener('click', () => { configLocked = !configLocked; update(); });
$('hit').addEventListener('click', () => hit());
$('volume').addEventListener('input', () => { if (outputGain) outputGain.gain.setTargetAtTime(Number($('volume').value), context.currentTime, .01); });
$('tempo').addEventListener('input', () => { $('tempo-value').textContent = $('tempo').value; });
function stopBeat() { clearInterval(beatTimer); beatTimer = null; send({type:'clear'}); for (const timer of timers) clearTimeout(timer); timers.clear(); activity.panic(); padButtons.forEach(button => button.classList.remove('hit')); $('beat').setAttribute('aria-pressed','false'); $('beat').textContent = 'Play beat'; }
function scheduleBeat() {
  while (nextTime < context.currentTime + .12) {
    const notes = [7]; if ([0,6,8].includes(step)) notes.push(0); if (step === 4 || step === 12) notes.push(1); if (step === 14) notes.push(8);
    for (const index of notes) { send({type:'hit',index,when:nextTime}); later(() => flash(index), Math.max(0,(nextTime - context.currentTime)*1000)); }
    nextTime += 60 / Number($('tempo').value) / 4; step = (step + 1) % 16;
  }
}
$('beat').addEventListener('click', async () => {
  if (beatTimer) { stopBeat(); return; }
  try { await enableAudio(); nextTime = context.currentTime + .06; step = 0; beatTimer = setInterval(scheduleBeat,25); scheduleBeat(); $('beat').setAttribute('aria-pressed','true'); $('beat').textContent = 'Pause beat'; } catch (error) { fail(error); }
});
$('stop').addEventListener('click', () => { stopBeat(); send({type:'panic'}); });
addEventListener('keydown', event => {
  if (event.metaKey || event.ctrlKey || event.altKey) return;
  if (event.key === 'Shift') { shifted = true; update(); }
  if (event.repeat || /INPUT|SELECT|TEXTAREA/.test(event.target.tagName)) return;
  const index = KEYS.indexOf(event.key.toLowerCase());
  if (index >= 0) { event.preventDefault(); select(index); hit(index); }
  if (event.code === 'Space' && event.target.tagName !== 'BUTTON') { event.preventDefault(); hit(); }
});
addEventListener('keyup', event => { if (event.key === 'Shift') { shifted = false; update(); } });
addEventListener('blur', () => { shifted = false; update(); });

async function load() {
  const manifest = await (await fetch('/preview/manifest.json')).json();
  const responses = await Promise.all([fetch('/preview/engine.wasm'),fetch('/preview/panel.svg')]);
  if (responses.some(response => !response.ok)) throw new Error('Preview assets are unavailable');
  const [bytes, artwork] = await Promise.all(responses.map(response => response.arrayBuffer()));
  const digest = async buffer => [...new Uint8Array(await crypto.subtle.digest('SHA-256',buffer))].map(v => v.toString(16).padStart(2,'0')).join('');
  if (await digest(bytes) !== manifest.wasm_sha256 || await digest(artwork) !== manifest.graphics.asset_sha256) throw new Error('Preview identity mismatch');
  module = await WebAssembly.compile(bytes); display = new PreviewDisplay(module); scene = new SvgDisplay(new TextDecoder().decode(artwork),manifest.graphics);
  $('screen-svg').append(scene.root); update(); status('Full engine ready · click a pad to hear it');
  let last = performance.now();
  function draw(now) {
    try {
      const pad = pads[selected], shown = [0,1,2,3].map(i => i + page * 4);
      scene.apply(display.tick(shownControls(pad.raw.map(word),shown),Math.round(Math.min(100,now-last)*1024/1000),activity.note_on_serial,0,reduced.matches,activity,pad.model,Number(isConfig()),[pad.model,pad.choke,0,0]));
      encoders.forEach((encoder, i) => { encoder.output.textContent = scene.header[i] || '—'; });
      if (analyser) { analyser.getFloatTimeDomainData(waveform); let level = 0; for (const sample of waveform) level = Math.max(level,Math.abs(sample)); heldLevel = Math.max(level * 4, heldLevel * .91); $('level').value = heldLevel; }
      last = now; requestAnimationFrame(draw);
    } catch (error) { fail(error); }
  }
  requestAnimationFrame(draw);
}
update(); load().catch(fail);
