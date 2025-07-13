# WipShmup - Space Shoot 'Em Up

A shoot 'em up game built in Rust using the ggez game framework.

## Features

- **Difficulty Selection**: Four difficulties: Goober, Standard, Ultra-Violence, Not when, how.
- **Player Movement**: Smooth WASD or arrow key movement.
- **Gameplay**: Fire bullets with Spacebar. Hold Spacebar (with Laser power-up) to charge and unleash a powerful laser.
- **Power-Ups**:
   - **Rapid Fire**: Shoot much faster for a limited time.
   - **Triple Shot**: Fire three bullets at once.
   - **Shield**: Temporary invincibility.
   - **Laser**: Charge and fire a devastating laser beam.
- **Enemy Types**:
   - **Normal (Red)**: Standard enemy, 10 points.
   - **Fast (Green)**: Moves quickly, 10 points.
   - **Big (Magenta)**: Large and tough, 30 points.
   - **Zenith (White)**: Aggressive, can grab the player, 50 points.
- **Bosses**:
   - **Destroyer (Red)**: 1000 points.
   - **Carrier (Purple)**: 1500 points.
   - **Behemoth (Blue)**: 2000 points.
- **Combo System**: Kill enemies quickly to build combos and increase your score multiplier.
- **Statistics**: Tracks high score, games played, total play time, enemies killed (by type), bosses defeated, max combo, and more.
- **Game Over & Restart**: Lose all lives to trigger a dramatic explosion and game over. Press R to restart.
- **Fullscreen & Scaling**: Automatically detects and uses your monitor's resolution.

## Controls

- **WASD / Arrow Keys**: Move your ship
- **Spacebar**: Shoot bullets
- **Hold Spacebar (with Laser power-up)**: Charge and fire laser
- **R**: Restart after game over
- **ESC**: Return to menu (when game is over or in submenus)
- **Mouse**: Navigate and click menu buttons
- **Enter/Return**: Select menu options
- **Up/Down**: Navigate menu options

## Menus & Screens

- **Main Menu**: Play, Statistics, Controls, About, Quit
- **Difficulty Select**: Choose from four difficulties, each with unique enemy behavior
- **Statistics**: View your high score, play time, kill counts, combos, and more
- **Controls**: Quick reference for all controls
- **About**: Game rules, enemy/boss point values, and version info

## How to Play

1. **Build and Run**:
   Navigate to the "src" folder and run "cargo run"

2. **Choose Difficulty**: Select your challenge level
3. **Survive**: Dodge enemies, collect power-ups, and defeat bosses
4. **Score**: Build combos for higher multipliers and maximize your score
5. **Game Over**: Try to beat your high score and stats!

## "Tech specs"

- Built with Rust and ggez
- Fullscreen, auto-scaling to your monitor
- Modular codebase (entities, effects, weapons, menus, stats)
- Save files for high score and statistics in `saves/`

## Dependencies

- `ggez`: Game framework
- `glam`: Vector math
- `rand`: Random number generation
- `serde`, `serde_json`: For saving/loading stats

Thanks for reading!