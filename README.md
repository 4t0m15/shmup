# WipShmup - A Basic Shoot 'Em Up Game

A simple shoot 'em up game built in Rust using the ggez game framework.

## Features

- **Player Movement**: Control your ship with WASD or arrow keys
- **Shooting**: Press Spacebar to shoot bullets
- **Enemy Spawning**: Enemies spawn from the top of the screen
- **Collision Detection**: Bullets destroy enemies, enemies destroy player
- **Scoring System**: Earn points for each enemy destroyed
- **Game Over**: When you collide with an enemy, game ends with final score
- **Restart**: Press R to restart after game over

## Controls

- **WASD** or **Arrow Keys**: Move the player ship
- **Spacebar**: Shoot bullets
- **R**: Restart the game (when game over)

## How to Play

1. **Build and Run**:
   ```bash
   cargo run
   ```

2. **Objective**: Destroy as many enemies as possible while avoiding collisions
3. **Movement**: Use WASD or arrow keys to move your blue ship
4. **Shooting**: Press Spacebar to fire yellow bullets upward
5. **Enemies**: Red squares spawn from the top and move downward
6. **Scoring**: Each enemy destroyed gives you 10 points
7. **Game Over**: If an enemy hits your ship, the game ends
8. **Restart**: Press R to start a new game

## Game Elements

- **Blue Square**: Your player ship
- **Yellow Circles**: Your bullets
- **Red Squares**: Enemy ships
- **Score Display**: Shows current score in top-left corner

## Technical Details

- Built with Rust and ggez game framework
- 800x600 pixel window
- 60 FPS gameplay
- Collision detection using rectangular bounds
- Smooth movement with delta time

## Dependencies

- `ggez`: Game framework
- `glam`: Vector math library
- `rand`: Random number generation

Enjoy the game! 