# Gameplay Improvements (Incremental Design)

> **Status: fully implemented.** All four phases below are live as of 2026:
> Phase A (bulk buy), Phase B (achievement rewards, prestige stars + shop),
> Phase C (milestones, specialization incl. Plinko Pro, synergies, interest),
> and Phase D (random events: Happy Hour / Hot Streak / Money Rain). Plinko
> itself was added as a fifth game. The rules live in headless code
> (`src/game_actions.cpp`, `src/core/`) shared by both the terminal UI and
> the web GUI; see `README.md` for player-facing docs.

This document proposes gameplay improvements for **Lucky Spin Casino**, with a focus on strengthening long-term progression, decision-making, and pacing typical of incremental/idle games.

## Goals

- **Make progress feel inevitable but earned**: player choices and planning matter, but the game doesn’t stall.
- **Create “chapters” of progression**: early game teaches, midgame expands options, late game adds long-term systems.
- **Increase meaningful decisions**: avoid “buy everything always” by adding specialization, milestones, and spend choices.
- **Keep active play relevant**: periodic bursts and events keep the casino games compelling.
- **Preserve the project’s simplicity**: implement improvements as incremental additions to the existing architecture.

## Current Systems (Baseline)

From `docs/` and `gambling_game.cpp`, the game already includes:

- **Primary currency**: `money` (`double`)
- **Passive income**: `vector<Generator> generators`
- **Upgrades**: `vector<Upgrade> upgrades` with `UpgradeType`
- **Prestige**: `prestigeLevel`, `prestigeMultiplier`, requirement based on `lifetimeEarnings`
- **Achievements**: boolean flags + UI popups
- **Persistence**: `saveGame()`, `loadGame()` and offline earnings
- **Real-time UI loop**: `updatePassiveIncome()` called frequently

### Key hook points

- **Passive tick**: `updatePassiveIncome()`
- **Passive total**: `getTotalPassiveIncome()` (applies `getIncomeMultiplier()` and `prestigeMultiplier`)
- **Generator buy**: `purchaseGenerator(int index)`
- **Upgrade buy**: `purchaseUpgrade(int index)`
- **Prestige reset**: `performPrestige()`
- **Achievement checks**: `checkAchievements()`
- **Save/load**: `saveGame()`, `loadGame()`

## High-Impact Improvements (Recommended)

### 1) Bulk Buy + Buy Max (Core Idle UX)

**Why**: Reduces repetitive input and makes late-game scaling playable.

**Player-facing features**

- Toggle purchase quantity: `x1 / x10 / x25 / x100 / MAX`
- Show preview: total cost, total income gain for the selected amount

**Implementation sketch**

- Add shop state: `int buyQuantityMode` or enum.
- Refactor `purchaseGenerator(int index)` into:
  - `purchaseGenerator(int index, int amount)`
  - or `purchaseGeneratorMax(int index)`
- Compute bulk cost using a geometric series based on the generator’s cost formula.

**Hook points**

- `displayGeneratorShop()` (UI and input)
- `purchaseGenerator(...)` (purchase logic)

**Balance notes**

- Bulk buy does not change economy; it improves pacing and reduces friction.

### 2) Achievements Give Rewards (Not Just Notifications)

**Why**: Achievements become a real progression layer (common in NGU Idle / Farmer Against Potatoes Idle).

**Reward options**

- **Global income bonus**: +1% per achievement
- **Mode bonuses**: e.g. wheel chance +0.5%, slots payout +0.05x
- **QoL bonuses**: offline cap increases, unlock bulk tiers, cheaper generators

**Implementation sketch**

- Add `int achievementPoints` or `double achievementMultiplier`.
- When an achievement is unlocked, apply a permanent reward.
- Apply rewards in:
  - `getTotalPassiveIncome()` (income)
  - payout calculations (active play)

**Hook points**

- `checkAchievements()`
- `showAchievement()` (optional to show reward text)
- `saveGame()` / `loadGame()` to persist reward state

**Balance notes**

- Keep each achievement’s reward small; the sum becomes meaningful.

### 3) Prestige as Spendable Currency (“Stars”) + Prestige Shop

**Why**: Turns prestige into a strategic long-term system. This mirrors “meta progression” in many incremental games.

**Player-facing features**

- On prestige, gain `prestigeStars` based on run performance
- Spend stars on permanent perks

**Example prestige shop perks**

- Start with more money
- Generators cost -X%
- Offline earnings cap +X%
- Permanent unlocks (bulk buy tiers, event unlocks)

**Implementation sketch**

- Add `int prestigeStars` and `vector<int> prestigePerks` (levels).
- Add `displayPrestigeShop()` menu.
- In `performPrestige()`, calculate and award stars.

**Hook points**

- `performPrestige()`
- `displayPrestigeMenu()` (add option to open prestige shop)
- `saveGame()` / `loadGame()`

**Balance notes**

- Ensure prestige perk scaling doesn’t trivialize early game too quickly.

## Medium-Impact Improvements (Adds Depth)

### 4) Milestones That Change Rules (Progress “Chapters”)

**Why**: Milestones create punctuated progression and unlock new mechanics (Cookie Clicker style).

**Milestone triggers**

- Total generators owned
- Passive income thresholds
- `prestigeLevel`
- `totalLifetimeEarnings`

**Example unlocks**

- Bulk buy tiers
- Interest system
- Golden events system
- Upgrade tiers / new generator tiers

**Hook points**

- `updatePassiveIncome()`
- `purchaseGenerator()` / `purchaseUpgrade()`
- `performPrestige()`

### 5) Interest / Bank System

**Why**: Adds a “bankroll management” passive layer that fits the casino theme.

**Implementation**

- Compute interest as `money * rate * dt` and add in `updatePassiveIncome()`
- Make rate upgradeable (new `UpgradeType`) or milestone unlocked

**Hook points**

- `updatePassiveIncome()`
- upgrades initialization (add new upgrade)

**Balance notes**

- Add caps or softcaps to avoid runaway exponential growth early.

### 6) Specialization (“Builds”)

**Why**: Prevents “buy everything always”; gives players identity and strategy.

**Example specializations**

- Wheel Tycoon
- Slot Mogul
- Card Shark
- Roulette Baron

**Hook points**

- Add menu item + state: `specializationId`
- Apply multipliers in game payout functions
- Persist in `saveGame()` / `loadGame()`

## High-Excitement Improvements (Short-Term Bursts)

### 7) Golden Events / Timed Buffs

**Why**: Creates spikes of excitement and encourages short active sessions.

**Examples**

- “High Roller Rush” (temporary payout boost)
- “Lucky Dealer” (next win doubled)

**Hook points**

- Main loop near `updatePassiveIncome()`
- Apply effects in payout logic and/or `getTotalPassiveIncome()`

## Generator Depth Improvements

### 8) Generator Synergies (Cookie Clicker Style)

**Why**: Adds depth without adding too many new objects.

**Examples**

- Each higher-tier generator boosts the tier below
- Threshold bonuses at 10/25/50/100 owned

**Hook points**

- `getTotalPassiveIncome()` (compute effective per-generator income)

## Roadmap (Suggested Implementation Order)

### Phase A (QoL + immediate pacing)

1. Bulk buy + buy max (generators)
2. Bulk buy (upgrades) (optional)

### Phase B (Progression layers)

3. Achievement rewards
4. Prestige stars + prestige shop

### Phase C (Depth + replayability)

5. Milestones
6. Specialization builds
7. Synergies

### Phase D (Active-play excitement)

8. Golden events/timed buffs
9. Interest/bank system (or earlier if you prefer more idle emphasis)

## Balancing Guidelines

- **Early game**: active play dominates until the player buys 1–3 generators.
- **Mid game**: passive starts to outperform active; upgrades meaningfully affect odds/payouts.
- **Late game**: prestige and meta systems dominate; active play is for bursts/events.

Recommended guardrails:

- Keep permanent bonuses mostly additive early (small multipliers).
- Prefer “softcaps” (diminishing returns) over hard caps.
- Maintain the ability to recover from bad luck via passive income.

## Open Questions (for final design decisions)

- Should the late game be mostly idle (Cookie Clicker) or keep active gambling always relevant?
- Should achievements be permanent across prestiges? (typical answer: yes)
- Do you want extremely large numbers (beyond trillions), or should softcaps slow growth earlier?
