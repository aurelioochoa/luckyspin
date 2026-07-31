# 🎰 Lucky Spin Casino

A terminal-based incremental gambling game written in C++ for Linux.

![C++11](https://img.shields.io/badge/C%2B%2B-11-blue.svg)
![Platform](https://img.shields.io/badge/Platform-Linux-green.svg)
![License](https://img.shields.io/badge/License-MIT-yellow.svg)

## 📖 Overview

**Lucky Spin Casino** is an incremental/idle game with a casino theme. Start with $100, play various casino games, purchase passive income generators, unlock upgrades, and work your way to becoming a casino tycoon!

### Features

#### 🎮 Core Gameplay
- 🎡 **Spin the Wheel** - 50% chance, 2x payout
- 🎰 **Slot Machine** - Match symbols for jackpots
- 🃏 **Blackjack** - Classic 21 card game
- 🎲 **Roulette** - Bet on colors, numbers, odd/even

#### 💰 Progression Systems
- 🏪 **Generator Shop** - Buy passive income sources with bulk buy (x1/x10/x25/x100/MAX)
- ⬆️ **Upgrade Shop** - Improve odds and payouts
- ⭐ **Prestige System** - Reset for permanent multipliers and prestige stars
- 🏆 **Achievements** - Unlock rewards (+1% income per achievement)
- 💾 **Auto-Save** - Progress saved automatically every 30 seconds

#### 🌟 Advanced Features (Phase B)
- ⭐ **Prestige Stars** - Spendable currency earned from prestige
- 🛒 **Prestige Shop** - Permanent perks (starting money, generator discounts, passive boost, offline cap)
- 📈 **Achievement Rewards** - Each achievement grants +1% permanent income bonus

#### 🎯 Strategic Depth (Phase C)
- 🏆 **Milestones** - Unlock new mechanics at progression thresholds
- 🎯 **Specialization** - Choose a casino specialty for +50% payout bonus
- 🔗 **Generator Synergies** - Threshold bonuses and tier boosts
- 🏦 **Interest System** - Earn passive income from your bankroll (unlocked via milestone)

## 🚀 Quick Start

### Prerequisites

- **GCC/G++** with C++11 support
- **Linux** terminal with ANSI color support
- **Noto fonts** (recommended for Unicode symbols)

### Build & Run

```bash
# Clone the repository
git clone https://github.com/yourusername/lucky-spin-casino.git
cd lucky-spin-casino

# Build the game
make

# Run the game
make run
```

### Install as Command

```bash
# Install to ~/.local/bin (no sudo required)
make install

# Now you can run from anywhere
lucky-spin

# To uninstall
make uninstall
```

## 🎮 Controls

| Key | Action |
|-----|--------|
| **W / ↑** | Navigate up |
| **S / ↓** | Navigate down |
| **Enter / Space / D** | Select option |
| **E** | Change buy mode (in shops) |
| **Q** | Quick exit / Cancel |
| **0-9** | Quick select menu items |

## 📦 Project Structure

```
lucky-spin-casino/
├── gambling_game.cpp    # Main game source code
├── Makefile             # Build configuration
├── README.md            # This file
├── LICENSE              # MIT license
├── docs/
│   ├── GDD.md           # Game Design Document
│   └── GAMEPLAY_IMPROVEMENTS.md
├── src/
│   ├── colors.h         # ANSI color definitions
│   ├── unicode.h        # Unicode codepoint constants
│   ├── symbols.h        # Game-facing symbol sets (Symbols/Cards/UI)
│   ├── utils.h          # Terminal I/O, display-width helpers
│   └── core/            # Pure game logic (unit-testable)
├── tests/               # Unit tests
└── test_unicode.cpp     # Unicode compatibility test
```

## 🔧 Make Targets

| Command | Description |
|---------|-------------|
| `make` | Build the game |
| `make run` | Build and run |
| `make clean` | Remove build files |
| `make install` | Install as `lucky-spin` command |
| `make uninstall` | Remove installed command |
| `make help` | Show all available targets |

### Font Installation

For proper Unicode symbol display:

```bash
# Auto-detect distro and install
make install-fonts

# Or manually for your distro:
make install-fonts-arch     # Arch Linux
make install-fonts-debian   # Debian/Ubuntu
make install-fonts-fedora   # Fedora/RHEL

# Check if fonts are installed
make check-fonts
```

## 🎯 Gameplay

### Core Loop

```
EARN → UPGRADE → AUTOMATE → PRESTIGE → SPECIALIZE
  ↑________________________________________________↓
```

1. **Earn** - Play casino games to win money
2. **Upgrade** - Purchase upgrades to improve win rates and payouts
3. **Automate** - Buy generators for passive income (with bulk buy support)
4. **Prestige** - Reset for permanent multipliers and prestige stars
5. **Specialize** - Choose your casino specialty and unlock milestones
6. **Optimize** - Spend prestige stars on permanent perks
7. **Synergize** - Stack generators for exponential growth

### Generators (Passive Income)

| Generator | Icon | Base Income | Base Cost | Description |
|-----------|------|-------------|-----------|-------------|
| Tip Jar | 🫙 | $0.1/sec | $10 | A humble jar collecting spare change |
| Penny Slots | 🎰 | $0.5/sec | $50 | Old-school one-armed bandits |
| Auto-Wheel | 🎡 | $2/sec | $200 | A mechanical wheel that spins itself |
| Card Shark | 🃏 | $5/sec | $500 | A skilled dealer who finds an edge |
| VIP Table | 👑 | $15/sec | $1,500 | Exclusive high-stakes table |
| High Roller | 💎 | $50/sec | $5,000 | Wealthy patron who tips big |
| Casino Floor | 🏛️ | $200/sec | $20,000 | Entire floor generating revenue |
| Casino Empire | 🌍 | $1,000/sec | $100,000 | Global chain printing money 24/7 |

**Synergy Bonuses:**
- **Threshold Bonuses**: +10% at 10 owned, +15% at 25, +20% at 50, +25% at 100
- **Tier Bonuses**: Each higher-tier generator boosts all lower tiers by +1% per owned

### Upgrades

- **Wheel Chance** - Increase wheel win probability (up to 75%)
- **Wheel Payout** - Multiply wheel winnings (up to 3x)
- **Slots Chance** - Increase slots win probability (up to 35%)
- **Slots Payout** - Multiply slots winnings (up to 7x)
- **Blackjack Payout** - Better blackjack payouts (up to 2x)
- **Income Boost** - Multiply all passive income (up to 2x)

### Prestige System

When you've accumulated enough lifetime earnings, you can **prestige** to:
- **Reset Progress**: Money, generators, and upgrades reset
- **Gain Multiplier**: +10% permanent income multiplier per prestige
- **Earn Stars**: Base 5 stars + 1 per prestige level + 1 per $1M lifetime earnings
- **Unlock Milestones**: First prestige unlocks the interest/banking system

#### Prestige Shop (Spend Stars)

| Perk | Max Level | Effect |
|------|-----------|--------|
| Starting Money | 10 | Start with +$50 per level after prestige |
| Generator Discount | 10 | All generators cost -5% per level |
| Offline Cap | 5 | Offline earnings cap +2 hours per level |
| Passive Boost | 20 | Permanent +5% passive income per level |

### Specialization Builds

Choose your casino specialty for strategic bonuses:

| Specialization | Icon | Bonus |
|----------------|------|-------|
| Wheel Tycoon | 🎡 | +50% payout on Wheel spins |
| Slot Mogul | 🎰 | +50% payout on Slots |
| Card Shark | 🃏 | +50% payout on Blackjack |
| Roulette Baron | 🎲 | +50% payout on Roulette |

**Strategy**: Focus on one game type for maximum efficiency!

### Milestones

Unlock new mechanics as you progress:

1. **Generator Collector** - Own 10 total generators
2. **Income Milestone** - Reach $10/sec passive income
3. **Banking Unlocked** - First prestige (unlocks interest system!)
4. **Generator Hoarder** - Own 50 total generators
5. **Millionaire Club** - $1M total lifetime earnings
6. **Prestige Veteran** - Reach prestige level 3

## 💡 Progression Tips

### Early Game (First Hour)
1. **Play actively** - Casino games are your main income source
2. **Buy first generators** - Start with Tip Jar and Penny Slots
3. **Upgrade odds** - Invest in Wheel Chance and Slots Chance early
4. **Use bulk buy** - Press `E` in Generator Shop to toggle buy modes

### Mid Game (Building Passive Income)
1. **Focus on generators** - Passive income will overtake active play
2. **Hit thresholds** - Buy 10+ of each generator for synergy bonuses
3. **Unlock achievements** - Each one gives +1% permanent income
4. **Save for prestige** - Aim for first prestige around $100k lifetime earnings

### Late Game (Prestige & Beyond)
1. **Prestige strategically** - Wait until you have good lifetime earnings for more stars
2. **Invest in perks** - Passive Boost and Generator Discount are powerful
3. **Choose specialization** - Pick one game type and focus on it (+50% bonus!)
4. **Stack synergies** - Higher-tier generators boost all lower tiers
5. **Use interest** - Let your bankroll compound with the interest system

### Pro Tips
- **Offline earnings** cap at 24 hours (+ perk bonuses)
- **Achievements persist** through prestige - their bonuses stack forever
- **Specialization** can be changed anytime - experiment to find your style
- **Milestones** unlock automatically - just keep playing!
- **Synergies multiply** - 100 Tip Jars + 10 Casino Empires = massive boost!

## 🖥️ Screenshots

```
╔══════════════════════════════════════════════════╗
║     🎰 LUCKY SPIN CASINO 🎰                      ║
║     ★ ★ ★ ★ ★ ★ ★ ★ ★ ★                         ║
╠══════════════════════════════════════════════════╣
║  💰 Balance: $1,234.56  🔥 Income: +$12.50/sec   ║
║  🎯 Bet: $10.00  ⭐ P2 (1.2x)  🎯 Wheel Tycoon   ║
║  ✓ Wins: 45  ✗ Losses: 12  🏆 Peak: $5,432.10   ║
╚══════════════════════════════════════════════════╝

  GAMES
  ▶ 🎡 Spin the Wheel    (65% chance, 2.5x)
    🎰 Play Slots        (25% chance, 6.0x)
    🃏 Blackjack         (payout: 1.8x)
    🎲 Roulette          (up to 35x payout)

  SHOP
    🏪 Generator Shop    (passive income)
    ⬆ Upgrade Shop       (improve odds)

  OPTIONS
    💵 Change Bet        (current: $10)
    ⭐ Prestige          (level 2)
    🎯 Specialization    (Wheel Tycoon)
    🏆 Achievements      
```

## 🐛 Troubleshooting

### Symbols not displaying correctly?

1. Install Noto fonts: `make install-fonts`
2. Use a terminal with Unicode support (e.g., GNOME Terminal, Konsole, Alacritty)
3. Run the Unicode test: `make test-unicode`

### Game not saving?

- Save file location: `${XDG_DATA_HOME:-~/.local/share}/luckyspin/lucky_spin_save.txt`
- A `.bak` copy of the previous save is kept alongside it
- Game auto-saves every 30 seconds
- Manual save: Select "Save Game" from menu
- Legacy saves in the current directory are migrated automatically on first run

### Colors not showing?

- Ensure your terminal supports ANSI colors
- Try setting `TERM=xterm-256color`

## 📄 License

This project is open source and available under the [MIT License](LICENSE).

## 🤝 Contributing

Contributions are welcome! Feel free to:
- Report bugs
- Suggest features
- Submit pull requests

## 📚 Documentation

- **[docs/GDD.md](docs/GDD.md)** - Game Design Document with technical details
- **[docs/GAMEPLAY_IMPROVEMENTS.md](docs/GAMEPLAY_IMPROVEMENTS.md)** - Roadmap and improvement phases

## 🎮 Menu Structure

```
Main Menu
├── GAMES
│   ├── 🎡 Spin the Wheel
│   ├── 🎰 Play Slots
│   ├── 🃏 Blackjack
│   └── 🎲 Roulette
├── SHOP
│   ├── 🏪 Generator Shop (with bulk buy: x1/x10/x25/x100/MAX)
│   └── ⬆ Upgrade Shop
└── OPTIONS
    ├── 💵 Change Bet
    ├── ⭐ Prestige
    │   ├── [P] Prestige Now
    │   └── [S] Prestige Shop (spend stars)
    ├── 🎯 Specialization (choose your build)
    ├── 🏆 Achievements (view progress)
    ├── 💾 Save Game
    ├── 🔄 Restart Game
    └── 🚪 Quit Game
```

## 🎯 Achievement List

| Achievement | Requirement | Reward |
|-------------|-------------|--------|
| First Win | Win your first game | +1% income |
| High Roller | Win $1,000 in a single bet | +1% income |
| Millionaire | Reach $1,000,000 | +1% income |
| Lucky Streak | Win 10 games in a row | +1% income |
| Generator Tycoon | Own 10 of any generator | +1% income |
| Fully Upgraded | Max out all upgrades | +1% income |
| First Prestige | Prestige for the first time | +1% income |
| Prestige Master | Reach prestige level 5 | +1% income |

**Total Possible**: 8 achievements = +8% permanent income bonus

---

**Have fun and gamble responsibly!** 🎲🍀
