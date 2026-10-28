import {MachineScreen} from './visual-host.mjs';
const $=id=>document.getElementById(id);
const machines=await(await fetch('/machines.json')).json();
let current,values=[],token=0;
for(const m of machines){const option=document.createElement('option');option.value=m.id;option.textContent=m.name+' · '+m.kind;$('machine').append(option);}
async function choose(id){
  const own=++token;const machine=machines.find(m=>m.id===id);$('machine').value=id;
  current?.dispose();values=machine.definition.parameters.map(p=>p.default);$('controls').replaceChildren();
  for(const [i,p] of machine.definition.parameters.entries()){
    const cell=document.createElement('div');cell.className='control';const label=document.createElement('label');label.htmlFor='param-'+p.id;
    const title=document.createElement('span');title.textContent=p.label;const out=document.createElement('output');
    const input=document.createElement('input');input.id=label.htmlFor;input.type='range';input.min='0';input.max='1';input.step='.001';input.value=values[i];
    const show=()=>{out.textContent=p.mapping.kind==='enum'?p.mapping.labels[Math.min(p.mapping.labels.length-1,Math.floor(values[i]*p.mapping.labels.length))]:Math.round(values[i]*100);};
    show();input.oninput=()=>{values[i]=+input.value;show();current?.parameters(values,i);};label.append(title,out);cell.append(label,input);$('controls').append(cell);
  }
  $('screen-note').textContent=machine.kind==='synth'?'Native synth display · Hold note and turn the controls.':machine.kind==='sample'?'Display study · The waveform and grains are illustrations; the XY currently uses its stock sampler screen.':'Display study · The SDK currently keeps the stock effect screen. Motion illustrates the controls and note activity.';
  const screen=await MachineScreen.open(document.createElement('div'),machine);
  if(own!==token){screen.dispose();return;}current=screen;current.reduced=$('motion').value==='reduced';
  current.root=$('screen');current.root.replaceChildren(current.svg);
  $('gate').setAttribute('aria-pressed','false');$('gate').textContent='Hold note';location.hash=id;
}
$('machine').onchange=()=>choose($('machine').value);
$('gate').onclick=()=>{const on=$('gate').getAttribute('aria-pressed')!=='true';$('gate').setAttribute('aria-pressed',String(on));$('gate').textContent=on?'Release note':'Hold note';current?.gate(on);};
$('strike').onclick=()=>current?.strike();
$('motion').onchange=()=>{if(current)current.reduced=$('motion').value==='reduced';};
for(const m of machines){const card=document.createElement('button');card.className='screen-card';card.dataset.machine=m.id;
  const picture=document.createElement('div');picture.className='machine-screen';const title=document.createElement('span');title.textContent=m.name;
  card.append(picture,title);$('screen-grid').append(card);card.onclick=()=>{choose(m.id);$('screen').scrollIntoView({behavior:'instant',block:'center'});};
  const screen=await MachineScreen.open(picture,m);screen.reduced=true;screen.draw(performance.now());screen.dispose();
}
await choose(machines.some(m=>m.id===location.hash.slice(1))?location.hash.slice(1):'hkmbira');
