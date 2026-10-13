/** Live browser regressions. Requires Playwright and a Chromium installation. */
import assert from 'node:assert/strict';
import { spawn } from 'node:child_process';
import { mkdtemp, mkdir, rm } from 'node:fs/promises';
import { tmpdir } from 'node:os';
import { join, resolve } from 'node:path';
import { createServer } from 'node:net';
import { once } from 'node:events';
const {chromium}=await import(process.env.PLAYWRIGHT_MODULE||'playwright');
const temporary=await mkdtemp(join(tmpdir(),'luckyspin-browser-'));
const reservation=createServer();reservation.listen(0,'127.0.0.1');await once(reservation,'listening');
const port=reservation.address().port;await new Promise(r=>reservation.close(r));
const game=spawn(resolve('gambling_game'),['--gui','--no-open','--port',String(port)],{env:{...process.env,XDG_DATA_HOME:temporary}});
let browser;
try{
  const url=await new Promise((resolve,reject)=>{
    let output='';const timer=setTimeout(()=>reject(new Error('Server startup timed out')),10000);
    game.stdout.on('data',data=>{output+=data;const match=output.match(/Serving:\s+(http:\/\/[^\s]+)/);if(match){clearTimeout(timer);resolve(match[1]);}});
    game.on('exit',code=>{clearTimeout(timer);reject(new Error('Server exited '+code));});
  });
  browser=await chromium.launch({headless:true,...(process.env.CHROMIUM_PATH?{executablePath:process.env.CHROMIUM_PATH}:{})});
  const page=await browser.newPage({viewport:{width:1440,height:1100}});
  const errors=[];page.on('pageerror',e=>errors.push(e.message));
  await page.goto(url);await page.waitForFunction(()=>document.querySelector('#connection').textContent.startsWith('Connected'));
  const state=()=>page.evaluate(()=>fetch('/api/state').then(r=>r.json()));
  const tab=key=>page.locator(`[data-tab="${key}"]`).click();
  const table=key=>page.locator(`[data-game="${key}"]`).click();
  const result=id=>page.waitForFunction(id=>document.getElementById(id).textContent.includes('net'),id);
  assert.equal(await page.locator('#balance').textContent(),'$100.00');
  await page.locator('#bet').fill('7');await page.waitForTimeout(1200);
  assert.equal(await page.locator('#bet').inputValue(),'7','poll must preserve a draft bet');
  await page.locator('#bet').press('Tab');await page.waitForFunction(()=>!document.querySelector('#saveBtn').disabled);
  assert.equal((await state()).bet,7);
  await page.locator('#bet').fill('999999');
  assert.equal(await page.locator('#spinWheelBtn').isDisabled(),true);
  assert.match(await page.locator('#betHint').textContent(),/exceeds/);
  await page.locator('#bet').fill('1');await page.locator('#bet').press('Tab');
  await page.waitForFunction(()=>!document.querySelector('#spinWheelBtn').disabled);
  let before=await state();
  await page.evaluate(()=>{document.querySelector('#spinWheelBtn').click();document.querySelector('#spinWheelBtn').click();});
  assert.equal(await page.locator('#spinWheelBtn').isDisabled(),true);
  await result('wheelResult');let after=await state();
  assert.equal(after.round,before.round+1,'double click starts exactly one round');
  assert.match(await page.locator('#wheelResult').textContent(),/Returned/);
  await table('slots');await page.locator('#slotsBtn').click();await result('slotsResult');
  assert.equal(await page.locator('.reel.spinning').count(),0);
  assert.equal(await page.locator('#slotsBtn').isEnabled(),true);
  await table('blackjack');await page.locator('#bjDeal').click();
  await page.waitForFunction(()=>!document.querySelector('#bjStand').disabled);
  assert.equal(await page.locator('#bjDealer .pcard').count(),2,'visible card and hidden hole card');
  assert.equal(await page.locator('#bjDealer .pcard.back').count(),1);
  assert.equal((await state()).blackjack.dealerCards.length,1,'hidden rank stays off the wire');
  await page.locator('#bjStand').click();await page.waitForFunction(()=>!document.querySelector('#bjDeal').disabled);
  assert.equal((await state()).blackjack.phase,'settled');
  await table('roulette');await page.locator('[data-t="5"]').click();await page.locator('#rouletteNum').fill('7');
  await page.locator('#rouletteBtn').click();await result('rouletteResult');
  assert.equal(await page.locator('#rHist span').count(),1);
  await table('plinko');await page.locator('#plinkoBtn').click();
  assert.equal(await page.locator('[data-r="2"]').isDisabled(),true,'risk remains fixed during a drop');
  await result('plinkoResult');assert.equal(await page.locator('#plinkoBtn').isEnabled(),true);
  await tab('upgrades');await page.locator('#specialization').selectOption('5');
  await page.waitForFunction(()=>!document.querySelector('#saveBtn').disabled);
  assert.equal((await state()).specialization,5);
  await tab('generators');const buy=page.locator('#genRows .buybtn').first();
  await buy.click();await page.waitForFunction(()=>document.querySelector('#genRows .name').textContent.includes('×1'));
  assert.equal((await state()).generators[0].owned,1);
  await page.locator('[data-mode="4"]').click();
  assert.match(await page.locator('#genRows .buybtn').first().textContent(),/MAX/);
  await tab('awards');await page.locator('#restartBtn').click();
  assert.equal(await page.locator('#confirmDialog').isVisible(),true);
  assert.equal(await page.locator('#confirmDialog [value="cancel"]').evaluate(el=>el===document.activeElement),true);
  await page.keyboard.press('Escape');assert.equal((await state()).generators[0].owned,1,'cancel preserves progress');
  await page.locator('#restartBtn').click();await page.locator('#confirmProceed').click();
  await page.waitForFunction(()=>document.querySelector('#tab-games').classList.contains('active'));
  after=await state();assert.equal(after.money,100);assert.equal(after.specialization,0);assert.equal(after.generators[0].owned,0);
  // A network error visibly disables bets and recovers on the next poll.
  await page.route('**/api/state',route=>route.abort());await page.waitForFunction(()=>document.querySelector('#connection').classList.contains('offline'));
  assert.equal(await page.locator('#spinWheelBtn').isDisabled(),true);
  await page.unroute('**/api/state');await page.waitForFunction(()=>document.querySelector('#connection').textContent.startsWith('Connected'));
  assert.equal(await page.locator('#spinWheelBtn').isEnabled(),true);
  await page.keyboard.press('5');assert.equal(await page.locator('#game-plinko').isVisible(),true);
  await tab('about');assert.match(await page.locator('#tab-about').textContent(),/MIT/);
  const art=await page.request.get(url+'assets/casino.png');assert.equal(art.status(),200);assert.equal((await art.body()).subarray(1,4).toString(),'PNG');
  await tab('games');await table('wheel');
  const screens=process.env.SCREENSHOT_DIR;
  let capture;
  if(screens){
    await mkdir(screens,{recursive:true});
    capture=await browser.newPage({viewport:{width:1440,height:1100}});await capture.goto(url);
    await capture.waitForFunction(()=>document.querySelector('#connection').textContent.startsWith('Connected'));
    await capture.screenshot({path:join(screens,'gui-desktop.png'),fullPage:true});
  }
  for(const width of [320,390,768,1440]){
    await page.setViewportSize({width,height:900});
    for(const key of ['games','generators','upgrades','prestige','awards','about']){
      await tab(key);
      const fits=await page.evaluate(()=>document.documentElement.scrollWidth<=innerWidth);
      if(!fits)console.log(await page.evaluate(()=>({width:innerWidth,scroll:document.documentElement.scrollWidth,body:document.body.scrollWidth,nodes:[...document.querySelectorAll('body *')].filter(el=>el.getBoundingClientRect().right>innerWidth||el.scrollWidth>el.clientWidth+1).map(el=>({tag:el.tagName,id:el.id,class:el.className,width:el.clientWidth,scroll:el.scrollWidth,right:el.getBoundingClientRect().right,text:el.textContent.slice(0,50)}))}))); 
      assert.equal(fits,true,`${key} must fit at ${width}px`);
    }
    if(screens&&width===390){await capture.setViewportSize({width,height:844});await capture.screenshot({path:join(screens,'gui-mobile.png'),fullPage:true});}
  }
  const reduced=await browser.newPage({reducedMotion:'reduce'});await reduced.goto(url);
  await reduced.waitForFunction(()=>!document.querySelector('#spinWheelBtn').disabled);
  await reduced.locator('#spinWheelBtn').click();await reduced.waitForFunction(()=>document.querySelector('#wheelResult').textContent.includes('net'));
  assert.equal(await reduced.locator('#spinWheelBtn').isEnabled(),true,'reduced motion resolves immediately');
  assert.deepEqual(errors,[],'browser has no uncaught errors');
  console.log('Browser verification passed: five games, bets, shops, specialties, reset, reconnect, keyboard, reduced motion, and four viewport widths.');
}finally{
  if(browser)await browser.close();
  game.kill('SIGTERM');if(game.exitCode===null)await once(game,'exit');
  await rm(temporary,{recursive:true,force:true});
}
