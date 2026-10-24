#pragma once
#include <Arduino.h>
const char DASHBOARD_HTML[] PROGMEM = R"HTML(<!doctype html>
<html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>HX711 / Scale bench</title>
<style>
:root{color-scheme:dark;font:15px system-ui,sans-serif;background:#101614;color:#e9f1ec}*{box-sizing:border-box}body{margin:0}main{max-width:1150px;margin:auto;padding:32px 24px}header{display:flex;justify-content:space-between;align-items:center;gap:20px}h1{font-size:27px;letter-spacing:-1px;margin:8px 0}p{color:#9aafa3;line-height:1.6}.eyebrow{font:12px ui-monospace,monospace;color:#a3c3b1;letter-spacing:2px}#status{border:1px solid #3b5144;border-radius:25px;padding:9px 14px;font-size:13px;color:#b4c7bc}.stats{display:grid;grid-template-columns:2fr 1fr 1fr;gap:16px;margin:30px 0 16px}.card{padding:20px;background:#19221d;border:1px solid #2c3b31;border-radius:12px}.label{color:#9aafa3;font-size:12px;text-transform:uppercase;letter-spacing:1px}.value{font:30px ui-monospace,monospace;margin:12px 0 5px}.unit{font-size:12px;color:#9aafa3}.plot{background:#141d18;border:1px solid #2c3b31;border-radius:12px;padding:20px}.plot-head{display:flex;align-items:center;justify-content:space-between;gap:16px}canvas{display:block;width:100%;height:360px;margin-top:16px}button{font:inherit;background:#26382d;color:#e9f1ec;border:1px solid #405d4c;border-radius:7px;padding:10px 14px;cursor:pointer}button:hover{background:#34513f}button:disabled{opacity:.4;cursor:default}.controls{display:flex;gap:10px;flex-wrap:wrap;margin-top:16px}.accent{background:#b8e899;color:#142312;border-color:#b8e899}.foot{display:flex;justify-content:space-between;gap:20px;font-size:12px}#notice{min-height:20px;color:#e8bf7a}#demo{color:#e8bf7a}footer{margin-top:28px;border-top:1px solid #2c3b31;padding-top:18px;color:#7f998a;font:12px ui-monospace,monospace}@media(max-width:650px){main{padding:24px 14px}.stats{grid-template-columns:1fr 1fr}.card:first-child{grid-column:1/-1}header{align-items:start;flex-direction:column}canvas{height:280px}.foot{flex-direction:column;gap:0}.value{font-size:25px}}
</style></head><body><main>
<header><div><div class="eyebrow">SCALE BENCH / 01 <span id="demo"></span></div><h1>See what the cells are doing.</h1><p>Two load cells. One HX711. Live, unfiltered ADC counts.</p></div><div id="status" role="status">Connecting…</div></header>
<section class="stats" aria-label="Sensor readings">
<div class="card"><div class="label" id="reading-label">Raw signal</div><div class="value" id="reading">—</div><div class="unit">ADC counts · uncalibrated</div></div>
<div class="card"><div class="label">Sample rate</div><div class="value" id="rate">—</div><div class="unit">samples / second</div></div>
<div class="card"><div class="label">Noise span</div><div class="value" id="span">—</div><div class="unit">peak to peak · last 5 s</div></div>
</section>
<section class="plot"><div class="plot-head"><strong>Signal over time</strong><span class="unit">Last 60 seconds · auto scale</span></div>
<canvas id="chart" role="img" aria-label="Live HX711 signal in ADC counts over the last 60 seconds"></canvas>
<div class="controls"><button class="accent" id="zero" disabled>Zero here</button><button id="raw">Show raw</button><button id="pause" aria-pressed="false">Pause graph</button><button id="csv" disabled>Export CSV</button></div>
<p id="notice" role="status">Waiting for the first conversion…</p>
<div class="foot"><p>Press each cell gently; the signal should move, then settle back.</p><p>Zero affects this browser only.</p></div></section>
<footer>DOUT → GPIO14 / D5 &nbsp; · &nbsp; SCK → GPIO12 / D6 &nbsp; · &nbsp; CHANNEL A / GAIN 128</footer>
</main><script>
'use strict';
const $ = id => document.getElementById(id);
const canvas = $('chart'), ctx = canvas.getContext('2d');
let samples = [], lastSeq = 0, lastUptime = null, offset = 0, zeroed = false, paused = false, missed = 0;
const number = n => Math.round(n).toLocaleString();
function reset() { samples = []; lastSeq = 0; offset = 0; zeroed = false; missed = 0; }
function draw() {
  const width = canvas.clientWidth, height = canvas.clientHeight, ratio = window.devicePixelRatio || 1;
  canvas.width = width * ratio; canvas.height = height * ratio; ctx.scale(ratio, ratio);
  const left = 78, right = width - 14, top = 15, bottom = height - 32;
  const end = samples.length ? samples[samples.length - 1][1] : 60000;
  const start = end - 60000, points = samples.filter(s => s[1] >= start);
  const values = points.map(s => s[2] - offset);
  let low = values.length ? Math.min(...values) : -100, high = values.length ? Math.max(...values) : 100;
  const pad = Math.max(10, (high - low) * .15); low -= pad; high += pad;
  ctx.font = '11px ui-monospace, monospace'; ctx.lineWidth = 1;
  for (let i = 0; i <= 4; i++) {
    const y = top + (bottom - top) * i / 4;
    ctx.strokeStyle = '#2a3a30'; ctx.beginPath(); ctx.moveTo(left, y); ctx.lineTo(right, y); ctx.stroke();
    ctx.fillStyle = '#9aafa3'; ctx.textAlign = 'right'; ctx.fillText(number(high - (high - low) * i / 4), left - 9, y + 4);
    const x = left + (right - left) * i / 4;
    ctx.textAlign = 'center'; ctx.fillText((i * 15 - 60) + ' s', x, height - 8);
  }
  if (!points.length) { ctx.textAlign = 'center'; ctx.fillStyle = '#9aafa3'; ctx.fillText('Waiting for sensor data', (left + right) / 2, height / 2); return; }
  ctx.strokeStyle = '#b8e899'; ctx.fillStyle = '#b8e899'; ctx.lineWidth = 2; ctx.beginPath();
  points.forEach((s, i) => {
    const x = left + (s[1] - start) / 60000 * (right - left), y = bottom - (s[2] - offset - low) / (high - low) * (bottom - top);
    if (!i || s[0] !== points[i - 1][0] + 1 || s[1] - points[i - 1][1] > 2000) ctx.moveTo(x, y); else ctx.lineTo(x, y);
  }); ctx.stroke();
}
function update(data) {
  if (lastUptime !== null && data.uptime < lastUptime) reset();
  lastUptime = data.uptime;
  $('demo').textContent = data.demo ? '/ SIMULATED DATA' : '';
  for (const s of data.samples) {
    if (s[0] <= lastSeq) continue;
    if (lastSeq && s[0] > lastSeq + 1) missed += s[0] - lastSeq - 1;
    samples.push(s); lastSeq = s[0];
  }
  if (samples.length) {
    const latest = samples[samples.length - 1];
    samples = samples.filter(s => latest[1] - s[1] <= 60000).slice(-6000);
    const recent = samples.filter(s => latest[1] - s[1] <= 5000);
    const elapsed = latest[1] - recent[0][1];
    $('reading').textContent = data.ok ? number(latest[2] - offset) : '—';
    $('rate').textContent = data.ok && elapsed ? ((recent.length - 1) * 1000 / elapsed).toFixed(1) : '—';
    $('span').textContent = data.ok ? number(Math.max(...recent.map(s => s[2])) - Math.min(...recent.map(s => s[2]))) : '—';
    $('notice').textContent = !data.ok ? 'No conversion for 2 seconds. Check HX711 power, ground, DOUT and SCK.' : Math.abs(latest[2]) >= 8388600 ? 'ADC at its limit. Check bridge wiring and excitation.' : missed ? missed + ' samples missed during connection gaps.' : 'Receiving live counts. Add and remove a load to check the response.';
  } else $('notice').textContent = 'No HX711 data yet. Check power, ground, DOUT and SCK.';
  $('status').textContent = data.ok ? (data.demo ? '● Demo signal' : '● Sensor live') : '○ HX711 not ready';
  $('zero').disabled = !data.ok || !samples.length; $('csv').disabled = !samples.length;
  $('reading-label').textContent = zeroed ? 'Signal from zero' : 'Raw signal';
  if (!paused) draw();
}
async function poll() {
  try {
    const response = await fetch('/api/readings', {cache:'no-store', signal:AbortSignal.timeout(3000)});
    if (!response.ok) throw new Error('HTTP ' + response.status);
    update(await response.json());
  } catch (e) {
    $('status').textContent = '○ Connection lost'; $('reading').textContent = '—'; $('rate').textContent = '—'; $('span').textContent = '—'; $('zero').disabled = true;
    $('notice').textContent = 'Reconnect to HX711-Scale Wi-Fi. Retrying automatically…';
  } finally { setTimeout(poll, 250); }
}
$('zero').onclick = () => {
  if (!samples.length) return;
  const tail = samples.slice(-20); offset = tail.reduce((a,s) => a + s[2], 0) / tail.length; zeroed = true;
  $('reading-label').textContent = 'Signal from zero'; $('reading').textContent = number(samples[samples.length-1][2] - offset); if (!paused) draw();
};
$('raw').onclick = () => { offset = 0; zeroed = false; $('reading-label').textContent = 'Raw signal'; if (!paused) draw(); };
$('pause').onclick = () => { paused = !paused; $('pause').textContent = paused ? 'Resume graph' : 'Pause graph'; $('pause').setAttribute('aria-pressed', String(paused)); if (!paused) draw(); };
$('csv').onclick = () => {
  const text = 'sequence,device_ms,raw_counts,zeroed_counts\n' + samples.map(s => s.join(',') + ',' + (s[2] - offset)).join('\n');
  const url = URL.createObjectURL(new Blob([text], {type:'text/csv'}));
  const link = document.createElement('a'); link.href = url; link.download = 'hx711-counts.csv'; link.click(); setTimeout(() => URL.revokeObjectURL(url), 1000);
};
window.addEventListener('resize', () => { if (!paused) draw(); }); draw(); poll();
</script></body></html>)HTML";
