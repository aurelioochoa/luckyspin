# Game Design Document: Lucky Spin Casino
## An Incremental Gambling Game for the Linux Terminal and Browser

---

## 1. Overview

### 1.1 Concept
**Lucky Spin Casino** is an incremental/idle game with a gambling theme. Players start with a small bankroll and work their way up by playing five casino games, purchasing automated money generators (passive income), unlocking upgrades, and prestiging for permanent multipliers.

### 1.2 Genre
- **Primary**: Incremental / Idle Game
- **Secondary**: Casino / Gambling Simulation

### 1.3 Platform
- Linux terminal (C++11, ANSI colors): `make run`
- Local web GUI served by the same binary: `make gui` → `http://127.0.0.1:<port>/`

Both frontends share one headless rules engine (`src/game_actions.cpp`, `src/core/`), so the odds, payouts and progression are identical no matter how you play.

### 1.4 Target Audience
- C++ learners looking for a fun project to study
- Fans of incremental/idle games
- Terminal enthusiasts

---

## 2. Core Gameplay Loop

```
┌──────────────────────────────────────────────────────────────┐
│                                                              │
│   EARN  ──►  UPGRADE  ──►  AUTOMATE  ──►  PRESTIGE         │
│     │             │            │                │           │
│     │   (odds &   │   (passive │    (+10%/level,│           │
│     └── payouts)──┘    income) ─┘    stars shop)└─────────┐ │
│                                                            │ │
│   Random events (Happy Hour / Hot Streak / Money Rain)     │ │
│   punctuate active play every few minutes.                 │ │
└────────────────────────────────────────────────────────────┘─┘
```

1. **EARN**: Play casino games to win money (active gameplay)
2. **UPGRADE**: Purchase upgrades that improve odds, payouts and income
3. **AUTOMATE**: Buy passive income generators that earn money over time
4. **PRESTIGE**: Reset progress for permanent multipliers and spendable stars

---

## 3. Features

### 3.1 Games ✅

| Game | Base Odds | Base Payout | Notes |
|------|-----------|-------------|-------|
| Spin the Wheel | 50% | 2x | Upgrades raise chance to 60%, payout to 3x |
| Slot Machine | ~16.7% | 5x | Natural triples can land without the roll |
| Blackjack | skill | 2x (natural 2.5x) | Real 6-deck shoe, dealer stands on 17+, push returns bet |
| Roulette | 48.6% outside / 2.7% straight | 2x / 35x | European wheel (0 loses outside bets); Lucky 7s bonus on 7/17/27 |
| Plinko | binomial walk | risk tables | Low ~0.97x EV, Medium ~0.94x, High ~0.87x; edge buckets pay jackpots up to 35x |

All wager rounds route through shared headless actions, so both frontends settle identically. Wins can trigger **Lucky Charm** (doubling) and **Hot Streak** (event x1.5).

### 3.2 Progression Systems ✅

| System | Description |
|--------|-------------|
| Generators | 11 tiers of passive income with synergy bonuses |
| Bulk Buy | x1 / x10 / x25 / x100 / MAX purchase modes |
| Upgrades | 12 upgrade lines with capped levels |
| Prestige | Reset at $100K lifetime (doubles each time); +10% income per level |
| Prestige Stars | Earned on prestige; spent on four permanent perk lines |
| Achievements | 9 achievements, each worth +1% permanent income |
| Milestones | Six thresholds unlocking mechanics (e.g. banking) |
| Specialization | Pick one of six casino specialties: +50% payout there |
| Synergies | Ownership thresholds (10/25/50/100) plus higher-tier boosts |
| Interest | Post-first-prestige passive % on your bankroll |
| Random Events | Happy Hour (2x income), Hot Streak (1.5x payouts), Money Rain (instant cash) |
| Persistence | Atomic saves with checksum, `.bak` backup, offline earnings |

### 3.3 Backlog (not implemented)

- Auto-clicker / slot bot automation
- Multiple save slots
- Daily bonuses, mini-games, leaderboards
- Sound effects, themes/skins

---

## 4. Economy Design

### 4.1 Currency
| Currency | Purpose | How to Earn |
|----------|---------|-------------|
| **Dollars ($)** | Primary currency | Win games, passive income, interest, events |
| **Casino Stars ★** | Prestige currency | `5 + prestigeLevel + $1M steps of totalLifetime` |

### 4.2 Passive Income Generators

```
Name              Base $/sec   Base Cost    Cost Mult
Tip Jar           0.1          $10          1.15
Penny Slots       0.5          $50          1.15
Auto-Wheel        2            $200         1.18
Card Shark        5            $500         1.17
VIP Table         18           $1,500       1.18
High Roller       60           $5,000       1.19
Casino Floor      280          $20,000      1.20
Casino Empire     1,600        $100,000     1.21
Casino Chain      10,000       $600,000     1.22
Offshore Casino   60,000       $4,000,000   1.23
Money Printer     400,000      $25,000,000  1.24
```

Effective income per unit = `baseIncome x synergyMultiplier`, where synergies add
+10/+15/+20/+25% at 10/25/50/100 owned and each higher tier adds +1% per unit owned.
Totals scale with Income Boost, prestige multiplier, achievement bonus and the
Passive Boost perk.

### 4.3 Upgrade Cost Curve (example: Lucky Wheel)

| Level | Win Chance | Cost |
|-------|------------|------|
| 0 | 50% | - |
| 1 | 52% | $100 |
| 2 | 54% | $250 |
| 3 | 56% | $625 |
| 4 | 58% | $1,563 |
| 5 (MAX) | 60% | $3,907 |

### 4.4 Prestige Formulas

```
Requirement(next) = $100,000 * 2 ^ prestigeLevel
Stars earned      = 5 + prestigeLevel + floor(totalLifetime / $1M)
Income bonus      = +10% per prestige level (multiplicative stack)
```

### 4.5 Random Events

Fired by the income tick every 180–480 seconds of play:

| Event | Effect | Duration |
|-------|--------|----------|
| Happy Hour | Passive income x2 | 60s |
| Hot Streak | All game payouts x1.5 | 60s |
| Money Rain | Instant grant ≈ 30s of passive income (min $25) | instant |

---

## 5. UI/UX Design

### 5.1 Terminal Main Screen

```
╔══════════════════════════════════════════════════╗
║     🎰 LUCKY SPIN CASINO 🎰                      ║
║     ★ ★ ☆ ☆ ☆ ☆ ☆ ☆ ☆ ☆                         ║
╠══════════════════════════════════════════════════╣
║  💰 Balance: $1,234.56  🔥 Income: +$12.50/sec   ║
║  ...stats row...                                 ║
╚══════════════════════════════════════════════════╝

  GAMES
  ▶ 🎡 Spin the Wheel    (50% chance, 2.0x)
    🎰 Play Slots        (17% chance, 5.0x)
    🃏 Blackjack         (payout: 2.0x)
    🎲 Roulette          (up to 35x payout)
    🎯 Plinko            (risk levels, 35x edge)

  SHOP
    🏪 Generator Shop    (passive income)
    ⬆ Upgrade Shop       (improve odds)

  OPTIONS
    💵 Change Bet ...
```

WASD/arrow navigation with Enter to select; digits jump to entries; Q quits.
Games animate inline (wheel dots, slowing reels, card deals, Plinko ball path).

### 5.2 Web GUI

Single-page app embedded in the binary (`web/index.html`, served at `/`):

- **Header**: live balance, passive income, prestige badge, event banner
- **Games tab**: animated wheel, slot reels, roulette board with history,
  canvas Plinko that replays the server's exact ball path, blackjack table
  with hole-card hiding
- **Generators / Upgrades tabs**: live pricing for all bulk-buy modes
- **Prestige tab**: progress bar, star balance, perk shop
- **Awards tab**: achievements grid and run statistics

Polls `/api/state` once per second; actions POST to `/api/action`. The page
is vanilla HTML/CSS/JS with zero external dependencies.

---

## 6. Technical Architecture

### 6.1 Layering

```
┌──────────────────────────────┐   ┌──────────────────────────────┐
│ Terminal frontend            │   │ Web frontend                 │
│ game_display / game_games /  │   │ web/index.html (embedded)    │
│ game_shops / game_loop       │   │ src/web/{server,api}.cpp     │
└──────────────┬───────────────┘   └──────────────┬───────────────┘
               │  render result structs            │ JSON over HTTP
               ▼                                   ▼
        ┌─────────────────────────────────────────────┐
        │ Headless engine                             │
        │ game_actions.cpp  (round/shop/blackjack)    │
        │ gambling_game.h   (state machine + members) │
┌───────┴─────────────────────────────────────────────┴───────┐
│ Pure core (unit-tested, no terminal/clock/state)            │
│ core/economy.h  core/plinko.h  core/roulette.h  core/deck.h │
│ core/rng.h  core/money.h  core/checksum.h  core/save_path.h │
└─────────────────────────────────────────────────────────────┘
```

Rules exist exactly once (headless actions); frontends only animate and draw.
Achievements/milestones/events print for the terminal *and* queue for the web
GUI's toast system (drained exactly once).

### 6.2 Game Loop (terminal)

```cpp
while (!quit) {
    updatePassiveIncome();      // delta-time income, interest, events
    displayStatus();
    char c = getchWithTimeout(1000);   // non-blocking input, 1s refresh
    // ... navigation / dispatchMenuAction() ...
    autoSave();                 // every 30s
}
saveGame();
```

The web server ticks the same `updatePassiveIncome()` on every poll, so the
idle economy runs identically in both frontends.

### 6.3 Save File Format

Key/value text with an integrity trailer:

```
LUCKY_SPIN_SAVE_V1
timestamp=1755000000
money=1234.56
gen_0=5
upg_0=2
prestigeLevel=1
specializationId=3
milestone_2=1
checksum=<16-hex FNV-1a of everything above>
```

Writes go to a temp file, then `rename()` into place (atomic); the previous
save is kept as `.bak`. Loading verifies the checksum (warn-only), falls back
to the backup, and applies capped offline earnings.

---

## 7. Implementation History

| Sprint | Scope | Status |
|--------|-------|--------|
| 1 | Delta-time passive income, generators, generator shop | ✅ |
| 2 | Upgrade class + shop, odds/payout upgrades | ✅ |
| 3 | Save/load, auto-save, offline earnings | ✅ |
| 4 | Blackjack, roulette, prestige, achievements | ✅ |
| 5 | Game-specific upgrades, flavor descriptions | ✅ |
| 6 | Real-time UI, Unicode symbols and decorations | ✅ |
| 7 | Marquee animation, WASD/arrow menu navigation | ✅ |
| 8 | Hardening: safe parsing, atomic saves + checksum, bias-free RNG, finite shoe, testable banner | ✅ |
| 9 | Pure core extraction + 20k-check test suite (incl. ASan) | ✅ |
| 10 | Plinko game, random events (Phase D), late-game generators | ✅ |
| 11 | Headless action layer shared by two frontends | ✅ |
| 12 | Embedded web GUI (HTTP server, JSON API, SPA) | ✅ |

---

## 8. Learning Objectives (C++)

| Concept | Where Used |
|---------|------------|
| Classes & OOP | GamblingGame, Generator, Upgrade, cards::Shoe |
| Vectors | Generators, upgrades, hands, notification queue |
| File I/O | Atomic save/load with checksum verification |
| Time Functions | Delta-time income, offline earnings, events |
| Pointers/References | Result structs, const-correct accessors |
| RAII | Terminal state restore, temp-file cleanup |
| Enum dispatch | MenuAction table, BlackjackPhase state machine |
| Templates/Generic style | Minimal dependency-free test harness |
| POSIX sockets | Embedded HTTP server for the web GUI |
| Sanitizers/tooling | ASan/UBSan clean test suite |

---

## 9. Notes

### Balance Considerations
- Early game: active play dominates until the first generators land
- Mid game: passive overtakes active; upgrades meaningfully shift odds
- Late game: prestige perks and synergies compound; events reward bursts
- Every game keeps a house edge except where upgrades intentionally flip it

---

*Document Version: 2.0*
*Last Updated: August 2026*
