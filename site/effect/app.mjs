import {createControls} from './controls.mjs';
import {demoLoop} from './demo.mjs';
const $ = id => document.getElementById(id);
const status = text => { $('state').textContent = text; };
let manifest, controls, values, context, node, gain, source, uploaded, starting, initializing;
let playing = false, bypass = false, fileGeneration = 0, disposed = false;
const sourceBuffers = new Map();
function master() {
  node?.port.postMessage({type:'master', send:+$('send').value, bpm:+$('tempo').value, bypass, playing});
  $('send-value').textContent = Math.round(+$('send').value * 100) + '%';
  $('volume-value').textContent = Math.round(+$('volume').value * 100) + '%';
  if (gain) gain.gain.setTargetAtTime(+$('volume').value, context.currentTime, .015);
}
function update() {
  node?.port.postMessage({type:'controls', values:controls.q31(values)});
  for (const p of controls.parameters) {
    $('parameter-' + p.id).value = p.mapping.kind === 'enum' ? controls.physical(values)[p.id] : values[p.id];
    $('value-' + p.id).textContent = controls.displayValue(p.id, values);
  }
}
function stopInput() {
  if (source) { source.onended = null; source.stop(); source.disconnect(); source = null; }
  playing = false; master();
  $('play').textContent = 'Play loop';
}
async function audio() {
  if (initializing) return initializing;
  if (!context) {
    context = new AudioContext({sampleRate:44100});
    initializing = (async () => {
      if (context.sampleRate !== 44100) throw Error('This preview requires 44.1 kHz audio.');
      const [bytes] = await Promise.all([fetch('./effect.wasm').then(r => { if (!r.ok) throw Error('Could not load the effect.'); return r.arrayBuffer(); }), context.audioWorklet.addModule('./worklet.mjs')]);
      const digest = [...new Uint8Array(await crypto.subtle.digest('SHA-256', bytes))].map(x => x.toString(16).padStart(2, '0')).join('');
      if (digest !== manifest.wasm_sha256) throw Error('The effect download is incomplete. Reload to try again.');
      const module = await WebAssembly.compile(bytes);
      if (disposed) return;
      node = new AudioWorkletNode(context, 'xy-effect', {numberOfInputs:1, numberOfOutputs:1, outputChannelCount:[2], channelCount:2, channelCountMode:'explicit', processorOptions:{module, dryIndex:manifest.dryIndex}});
      gain = context.createGain(); gain.gain.value = +$('volume').value;
      node.connect(gain).connect(context.destination);
      node.port.onmessage = ({data}) => {
        if (data.type !== 'meter' || context.state !== 'running') return;
        $('meter').value = data.peak;
        $('wet-state').textContent = data.wetPeak > .00001 ? 'Effect signal active' : playing ? 'Listening…' : 'Input stopped';
      };
      node.onprocessorerror = () => { stopInput(); context.suspend(); status('Audio stopped. Reload the player to try again.'); };
      update(); master();
      $('stop').disabled = false; $('clear').disabled = false;
    })();
    try { await initializing; }
    catch (error) { await context.close(); context = null; throw error; }
    finally { initializing = null; }
  }
  if (!disposed) await context.resume();
}
async function play() {
  if (starting) return;
  starting = true; $('play').disabled = true;
  try {
    status('Starting audio…'); await audio();
    if (disposed) return;
    const mode = $('source').value, bpm = +$('tempo').value;
    let buffer = uploaded;
    if (mode !== 'file') {
      const key = mode + bpm;
      if (!sourceBuffers.has(key)) { sourceBuffers.clear(); sourceBuffers.set(key, demoLoop(context, mode, bpm)); }
      buffer = sourceBuffers.get(key);
    }
    if (!buffer) throw Error('Load an audio file first.');
    stopInput();
    source = context.createBufferSource(); source.buffer = buffer; source.loop = $('loop').checked;
    source.connect(node); playing = true; master();
    source.onended = () => { source?.disconnect(); source = null; playing = false; master(); $('play').textContent = 'Play loop'; status('Input finished · tail continues'); };
    source.start(); $('play').textContent = 'Restart loop'; status(mode === 'file' ? 'Playing your audio' : 'Playing demo loop');
  } catch (error) { status(error.message); }
  finally { starting = false; $('play').disabled = false; }
}
function renderControls() {
  for (const p of controls.parameters) {
    const cell = document.createElement('div'); cell.className = 'control';
    const label = document.createElement('label'); label.htmlFor = 'parameter-' + p.id;
    const name = document.createElement('span'); name.textContent = p.label;
    const output = document.createElement('output'); output.id = 'value-' + p.id; label.append(name, output);
    let input;
    if (p.mapping.kind === 'enum') {
      input = document.createElement('select');
      p.mapping.labels.forEach((text, i) => { const option = document.createElement('option'); option.value = i; option.textContent = text; input.append(option); });
    } else { input = document.createElement('input'); input.type = 'range'; input.min = 0; input.max = 1; input.step = .001; }
    input.id = 'parameter-' + p.id;
    input.addEventListener('input', () => {
      values[p.id] = p.mapping.kind === 'enum' ? (+input.value + .5) / p.mapping.labels.length : +input.value;
      update();
    });
    cell.append(label, input); $('controls').append(cell);
  }
  for (const [name, preset] of Object.entries(controls.presets ?? {})) {
    const button = document.createElement('button'); button.textContent = name;
    button.onclick = () => { values = {...preset}; update(); }; $('presets').append(button);
  }
  update();
}
$('play').onclick = play;
$('stop').onclick = () => { stopInput(); status('Input stopped · tail continues'); };
$('clear').onclick = async () => { stopInput(); node?.port.postMessage({type:'reset'}); await context?.suspend(); $('meter').value = 0; $('wet-state').textContent = 'Input stopped'; status('Tail cleared'); };
$('bypass').onclick = () => { bypass = !bypass; $('bypass').setAttribute('aria-pressed', String(bypass)); $('bypass').textContent = bypass ? 'Compare · original audio' : 'Compare · effect on'; master(); };
$('reset').onclick = () => { values = controls.defaults(); update(); };
$('loop').onchange = () => { if (source) source.loop = $('loop').checked; };
$('source').onchange = () => { stopInput(); status('Source changed · press Play loop'); };
$('tempo').onchange = () => {
  const number = +$('tempo').value; $('tempo').value = Number.isFinite(number) ? Math.round(Math.max(40, Math.min(240, number || 120))) : 120;
  const restart = playing && $('source').value !== 'file'; master(); if (restart) play();
};
for (const id of ['send', 'volume']) $(id).oninput = master;
$('file').onchange = async () => {
  const token = ++fileGeneration, file = $('file').files[0]; if (!file) return;
  try {
    if (file.size > 64 * 1024 * 1024) throw Error('Choose an audio file smaller than 64 MB.');
    status('Opening audio…'); await audio();
    const decoded = await context.decodeAudioData(await file.arrayBuffer());
    if (token !== fileGeneration || disposed) return;
    stopInput(); uploaded = decoded; $('source').querySelector('[value="file"]').disabled = false; $('source').value = 'file';
    $('file-info').textContent = `${file.name} · ${decoded.duration.toFixed(1)} seconds · stays in your browser`;
    status('Your audio is ready · press Play loop');
  } catch (error) { if (token === fileGeneration) status('Could not open audio: ' + error.message); }
};
window.addEventListener('pagehide', () => { disposed = true; stopInput(); node?.port.postMessage({type:'dispose'}); node?.disconnect(); context?.close(); });
window.addEventListener('keydown', event => { if (event.key === 'Escape') { stopInput(); node?.port.postMessage({type:'reset'}); } });
try {
  const response = await fetch('./manifest.json'); if (!response.ok) throw Error('Could not load this effect.');
  manifest = await response.json(); controls = createControls(manifest.definition); values = controls.defaults(); renderControls(); $('play').disabled = false; $('reset').disabled = false;
} catch (error) { status(error.message); }
