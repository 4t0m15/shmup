# WipShmup - "Work in Progress Shoot 'em Up"

A shoot 'em up game built in Rust using ggez.

## Features

- **Difficulty Selection**: Four difficulties: Goober, Standard, Ultra-Violence, Not when, how.
- **Player Movement**: wasd/arrow keys (I prefer arrow keys).
- **Gameplay**: Fire bullets with the spacebar. Hold the spacebar (with Laser power-up) to charge and release to fire a laser that ista-kills enemies.
- **Power-Ups**:
   - **Rapid Fire**: Shoot much faster for a limited time.
   - **Triple Shot**: Fire three bullets at once.
   - **Shield**: Temporary invincibility.
   - **Laser**: Charge and fire a devastating laser beam.
- **Enemy Types**:
   - **Normal (Red)**: Standard enemy, 10 points.
   - **Fast (Green)**: Moves quickly, 10 points.
   - **Big (Magenta)**: Large and tough, 30 points.
   - **Zenith (White)**: Aggressive, "grabs" the player, 50 points.
- **Bosses**:
   - **Destroyer (Red)**: 1000 points.
   - **Carrier (Purple)**: 1500 points.
   - **Behemoth (Blue)**: 2000 points.
- **Combo System**: Kill enemies quickly to build combos and increase your score multiplier.
- **Statistics**: Tracks high score, games played, total time spent, enemies killed (by type), bosses defeated and max combo,
- **Game Over & Restart**: Lose all lives to trigger an explosion and be forced to restart.
- **Fullscreen & Scaling**: Automatically detects and uses your monitor's resolution and adapts enemy size/difficulty.

## Controls

- **WASD / Arrow Keys**: Move
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
2. **Choose Difficulty**
3. **Survive**: Dodge enemies, collect power-ups, and defeat bosses
4. **Score**: Build combos for higher multipliers and aim for a high score
5. **Game Over**: Play again, or be productive, its up to you.

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