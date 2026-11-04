const cards = [...document.querySelectorAll('.machine-card')];
const buttons = [...document.querySelectorAll('[data-filter]')];
const search = document.querySelector('#search');
let selected = 'all';
function filter() {
  const query = search.value.trim().toLocaleLowerCase();
  let count = 0;
  for (const card of cards) {
    card.hidden = (selected !== 'all' && card.dataset.kind !== selected) || !card.dataset.search.includes(query);
    if (!card.hidden) count++;
  }
  for (const group of document.querySelectorAll('.machine-collection')) {
    const visible = [...group.querySelectorAll('.machine-card')].filter(card => !card.hidden).length;
    group.hidden = visible === 0;
    group.querySelector('[data-group-count]').textContent = `${visible} ${visible === 1 ? 'machine' : 'machines'}`;
  }
  document.querySelector('#result-count').textContent = `${count} ${count === 1 ? 'machine' : 'machines'}`;
  document.querySelector('#empty').hidden = count !== 0;
  for (const button of buttons) button.setAttribute('aria-pressed', String(button.dataset.filter === selected));
}
for (const button of buttons) button.addEventListener('click', () => { selected = button.dataset.filter; filter(); });
search.addEventListener('input', filter);
document.querySelector('#clear-search').addEventListener('click', () => { search.value = ''; selected = 'all'; filter(); search.focus(); });
const dialog = document.querySelector('#audition');
const mount = document.querySelector('#player-mount');
const loading = document.querySelector('#player-loading');
function stop() { mount.replaceChildren(); loading.hidden = true; }
if (typeof dialog.showModal === 'function') {
  for (const link of document.querySelectorAll('[data-play]')) link.addEventListener('click', event => {
    if (event.metaKey || event.ctrlKey || event.shiftKey || event.altKey) return;
    event.preventDefault();
    stop();
    document.querySelector('#audition-title').textContent = link.dataset.name;
    document.querySelector('#audition-guide').href = link.dataset.guide;
    const player = document.createElement('iframe');
    player.title = `${link.dataset.name} instrument audition`;
    player.allow = 'autoplay';
    player.src = link.href;
    loading.hidden = false;
    player.addEventListener('load', () => { loading.hidden = true; }, {once: true});
    mount.append(player);
    dialog.showModal();
  });
  document.querySelector('#close-audition').addEventListener('click', () => dialog.close());
  dialog.addEventListener('close', stop);
  dialog.addEventListener('click', event => { if (event.target === dialog && event.clientY < dialog.getBoundingClientRect().top) dialog.close(); });
}
