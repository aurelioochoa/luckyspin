# Game Design Document: Lucky Spin Casino
## An Incremental Gambling Game for the Linux Terminal

---

## 1. Overview

### 1.1 Concept
**Lucky Spin Casino** is a terminal-based incremental game with a gambling theme. Players start with a small bankroll and work their way up by playing various casino games, purchasing automated money generators (passive income), and unlocking upgrades that multiply their earnings over time.

### 1.2 Genre
- **Primary**: Incremental / Idle Game
- **Secondary**: Casino / Gambling Simulation

### 1.3 Platform
- Linux Terminal (C++)
- ANSI color support required

### 1.4 Target Audience
- C++ learners looking for a fun project to study
- Fans of incremental/idle games
- Terminal enthusiasts

---

## 2. Core Gameplay Loop

```
┌─────────────────────────────────────────────────────────┐
│                                                         │
│   EARN  ──►  UPGRADE  ──►  AUTOMATE  ──►  PRESTIGE     │
│     │                                          │        │
│     └──────────────────────────────────────────┘        │
│                                                         │
└─────────────────────────────────────────────────────────┘
```

1. **EARN**: Play casino games to win money (active gameplay)
2. **UPGRADE**: Purchase upgrades to improve win rates and payouts
3. **AUTOMATE**: Buy passive income generators that earn money over time
4. **PRESTIGE**: Reset progress for permanent multipliers (late game)

---

## 3. Game Features

### 3.1 Current Features (Implemented) ✅

| Feature | Description | Status |
|---------|-------------|--------|
| Wheel Spin | 50% chance, 2x payout | ✅ Done |
| Slot Machine | ~17% chance, 5x payout | ✅ Done |
| Adjustable Bets | Player can change bet amount | ✅ Done |
| Statistics | Track wins, losses, peak balance | ✅ Done |
| Colored UI | ANSI colors for visual appeal | ✅ Done |
| ASCII Symbols | Terminal-compatible slot symbols | ✅ Done |

### 3.2 Planned Features (To Implement) 📋

#### Phase 1: Passive Income System (Core Incremental Mechanic)
| Feature | Description | Priority |
|---------|-------------|----------|
| Auto-Clicker | Automatically plays wheel spin every X seconds | HIGH |
| Slot Bot | Automatically plays slots every X seconds | HIGH |
| Interest Rate | Earn % interest on balance over time | HIGH |
| Passive Income Display | Show $/sec earnings on main screen | HIGH |

#### Phase 2: Upgrade System
| Feature | Description | Priority |
|---------|-------------|----------|
| Better Odds | Increase win chance (50% → 55% → 60%...) | HIGH |
| Higher Payouts | Increase multipliers (2x → 2.5x → 3x...) | HIGH |
| Faster Automation | Reduce time between auto-plays | MEDIUM |
| Lucky Charm | Small chance for bonus multiplier | MEDIUM |

#### Phase 3: New Games
| Feature | Description | Priority |
|---------|-------------|----------|
| Blackjack | Simple 21 card game | MEDIUM |
| Roulette | Bet on numbers/colors | MEDIUM |
| Dice Roll | Various betting options | LOW |
| Poker (Simplified) | Video poker style | LOW |

#### Phase 4: Prestige System
| Feature | Description | Priority |
|---------|-------------|----------|
| Casino Stars | Prestige currency earned on reset | LOW |
| Permanent Upgrades | Spend stars on lasting bonuses | LOW |
| Achievements | Unlock bonuses for milestones | LOW |

#### Phase 5: Quality of Life
| Feature | Description | Priority |
|---------|-------------|----------|
| Save/Load Game | Persist progress to file | HIGH |
| Auto-Save | Save every X seconds | MEDIUM |
| Settings Menu | Configure game options | LOW |
| Sound Effects | Terminal beeps for events | LOW |

---

## 4. Economy Design

### 4.1 Currency
| Currency | Purpose | How to Earn |
|----------|---------|-------------|
| **Dollars ($)** | Primary currency | Win games, passive income |
| **Casino Stars** | Prestige currency | Reset with high balance |

### 4.2 Passive Income Sources

```
┌────────────────────────────────────────────────────────────────┐
│ PASSIVE INCOME GENERATORS                                      │
├────────────────────────────────────────────────────────────────┤
│ Name              │ Base $/sec │ Base Cost │ Cost Multiplier  │
├───────────────────┼────────────┼───────────┼──────────────────┤
│ Tip Jar           │ 0.1        │ $10       │ 1.15x            │
│ Penny Slots       │ 0.5        │ $50       │ 1.15x            │
│ Auto-Wheel        │ 2.0        │ $200      │ 1.20x            │
│ Slot Bot          │ 5.0        │ $500      │ 1.20x            │
│ VIP Table         │ 20.0       │ $2,000    │ 1.25x            │
│ High Roller Room  │ 100.0      │ $10,000   │ 1.30x            │
│ Casino Floor      │ 500.0      │ $50,000   │ 1.35x            │
│ Casino Chain      │ 2,500.0    │ $250,000  │ 1.40x            │
└───────────────────┴────────────┴───────────┴──────────────────┘
```

### 4.3 Upgrade Costs (Example: Win Rate)

| Level | Win Rate | Cost |
|-------|----------|------|
| 1 | 50% | Base |
| 2 | 52% | $500 |
| 3 | 54% | $2,000 |
| 4 | 56% | $8,000 |
| 5 | 58% | $32,000 |
| MAX | 60% | - |

### 4.4 Prestige Formula

```
Stars Earned = floor(sqrt(lifetime_earnings / 1,000,000))
```

Each star provides +1% to all income.

---

## 5. UI/UX Design

### 5.1 Main Screen Layout

```
==================================================
*****************************
*   LUCKY SPIN CASINO   *
*****************************
==================================================

--------------------------------------------------
  Balance: $1,234.56    Bet: $10.00    Round: 42
  Wins: 25  Losses: 17  Peak: $1,500.00
  
  PASSIVE INCOME: $12.50/sec
--------------------------------------------------

  GAMES:
    [1] Spin the Wheel  (50% chance, 2x payout)
    [2] Play Slots      (17% chance, 5x payout)

  UPGRADES:
    [3] Upgrade Shop
    [4] Passive Income Generators

  OPTIONS:
    [5] Change Bet Amount
    [6] Save Game
    [7] Quit Game

  Select option (1-7): 
```

### 5.2 Upgrade Shop Layout

```
==================================================
          UPGRADE SHOP
==================================================

  YOUR BALANCE: $1,234.56

  WHEEL UPGRADES:
    [1] Win Rate Lv.2     52% chance     $500
    [2] Payout Lv.1       2.2x payout    $750

  SLOT UPGRADES:
    [3] Win Rate Lv.1     18% chance     $1,000
    [4] Payout Lv.1       5.5x payout    $1,500

  GLOBAL UPGRADES:
    [5] Lucky Charm Lv.1  5% bonus roll  $2,000

  [0] Back to Main Menu

  Select upgrade (0-5): 
```

### 5.3 Passive Income Shop Layout

```
==================================================
       PASSIVE INCOME GENERATORS
==================================================

  YOUR BALANCE: $1,234.56
  CURRENT INCOME: $12.50/sec

  GENERATORS:                    OWNED    COST      $/SEC
    [1] Tip Jar                  5        $18       +0.1
    [2] Penny Slots              3        $76       +0.5
    [3] Auto-Wheel               1        $240      +2.0
    [4] Slot Bot                 0        $500      +5.0
    [5] VIP Table                0        $2,000    +20.0
    [6] High Roller Room         0        $10,000   +100.0

  [0] Back to Main Menu

  Select generator to buy (0-6): 
```

---

## 6. Technical Architecture

### 6.1 Class Structure

```cpp
// Main game class
class GamblingGame {
    // Economy
    double money;
    double lifetimeEarnings;
    int prestigeStars;
    
    // Statistics
    int totalWins, totalLosses;
    double peakMoney;
    
    // Generators (passive income)
    vector<Generator> generators;
    
    // Upgrades
    map<string, int> upgradeLevels;
    
    // Timing
    time_t lastUpdate;
};

// Passive income generator
class Generator {
    string name;
    double baseIncome;
    double baseCost;
    double costMultiplier;
    int owned;
};

// Upgrade definition
class Upgrade {
    string name;
    string description;
    vector<double> costs;
    vector<double> values;
    int currentLevel;
    int maxLevel;
};
```

### 6.2 Game Loop (with Passive Income)

```cpp
while (running) {
    // Calculate time delta
    time_t now = time(0);
    double deltaSeconds = difftime(now, lastUpdate);
    lastUpdate = now;
    
    // Apply passive income
    money += passiveIncomePerSecond * deltaSeconds;
    Sprint 2: Upgrades
[ ] Implement Upgrade class
[ ] Add upgrade shop menu
[ ] Make win rates/payouts upgradeable
[ ] Add automation speed upgrades
    // Update display
    displayStatus();
    
    // Handle input (non-blocking or with timeout)
    if (inputAvailable()) {
        processInput();
    }
    
    // Small sleep to prevent CPU spinning
    usleep(100000); // 100ms
}
```

### 6.3 Save File Format (JSON-like)

```
LUCKY_SPIN_SAVE_V1
money=1234.56
lifetime=50000.00
stars=5
wins=100
losses=80
gen_tipjar=5
gen_pennyslots=3
gen_autowheel=1
upg_wheelrate=2
upg_wheelpayout=1
upg_slotrate=0
```

---

## 7. Implementation Roadmap

### Sprint 1: Core Incremental Mechanics ✅ COMPLETE
- [x] Add time tracking (delta time calculation)
- [x] Implement Generator class
- [x] Add passive income calculation
- [x] Display $/sec on main screen
- [x] Create generator shop menu

### Sprint 2: Upgrades ✅ COMPLETE
- [x] Implement Upgrade class
- [x] Add upgrade shop menu
- [x] Make win rates/payouts upgradeable
- [x] Add automation speed upgrades

### Sprint 3: Persistence ✅ COMPLETE
- [x] Implement save game function
- [x] Implement load game function
- [x] Add auto-save feature
- [x] Handle offline earnings on load

### Sprint 4: Polish ✅ COMPLETE
- [x] Add more games (Blackjack, Roulette)
- [x] Implement prestige system
- [x] Add achievements
- [x] Balance economy

### Sprint 5: Game-Specific Content & Descriptions ✅ COMPLETE
- [x] Add Blackjack-specific upgrades (card counting, dealer bust chance)
- [x] Add Roulette-specific upgrades (lucky numbers, color streaks)
- [x] Add descriptions to all generators (flavor text explaining what they are)
- [x] Add descriptions to all upgrades (explain the mechanic being improved)
- [x] Update generator/upgrade shops to display descriptions

### Sprint 6: Real-Time UI & Visual Enhancements ✅ COMPLETE
- [x] Implement real-time balance updates (non-blocking input loop)
- [x] Add Unicode symbols for slots (🍒 🍊 🍋 🍇 🔔 7️⃣ 💎 ⭐)
- [x] Add card suit symbols for Blackjack (♠ ♥ ♦ ♣)
- [x] Add visual representation of owned generators on main screen
- [x] Add Unicode decorations for stats (💰 🏆 🎲 📈 etc.)
- [x] Improve box drawing with Unicode characters (─ │ ┌ ┐ └ ┘)

### Sprint 7: Animations & Modern Navigation ✅ COMPLETE
- [x] Implement casino marquee animation (blinking stars around title)
- [x] Add card suit display for blackjack
- [x] Replace number-based menu with arrow/WASD navigation
- [x] Add visual menu cursor/highlight for selected option
- [x] Support both keyboard navigation and direct number input

---

## 8. Learning Objectives (C++)

This project teaches the following C++ concepts:

| Concept | Where Used |
|---------|------------|
| Classes & OOP | GamblingGame, Generator, Upgrade classes |
| Vectors | Storing generators and upgrades |
| Maps | Tracking upgrade levels by name |
| File I/O | Save/Load system |
| Time Functions | Delta time, passive income |
| Pointers/References | Passing objects efficiently |
| Inheritance | Different game types sharing base class |
| Templates | Generic container usage |
| Operator Overloading | Custom number formatting |
| RAII | Resource management |

---

## 9. Notes & Ideas

### Future Ideas (Backlog)
- [ ] Multiple save slots
- [ ] Daily bonuses
- [ ] Mini-games for bonus multipliers
- [ ] Leaderboard (local high scores)
- [ ] Different casino themes/skins
- [ ] Combo system (win streaks)
- [ ] Risk/reward betting modes
- [ ] Tutorial for new players

### Balance Considerations
- Early game should feel rewarding (quick progression)
- Mid game introduces strategic choices (which upgrades?)
- Late game requires patience or prestige resets
- Passive income should eventually exceed active play

---

*Document Version: 1.0*
*Last Updated: December 2024*
