# Build Instructions for Galactic Shmup

## Prerequisites

### macOS (Recommended)
1. Install Xcode Command Line Tools:
   ```bash
   xcode-select --install
   ```

2. Install Homebrew (if not already installed):
   ```bash
   /bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"
   ```

3. Install raylib:
   ```bash
   brew install raylib
   ```

### Linux (Ubuntu/Debian)
```bash
sudo apt update
sudo apt install build-essential git cmake
sudo apt install libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev libglu1-mesa-dev libxcursor-dev libxinerama-dev

# Install raylib
git clone https://github.com/raysan5/raylib.git
cd raylib/src
make PLATFORM=PLATFORM_DESKTOP
sudo make install PLATFORM=PLATFORM_DESKTOP
```

### Windows
1. Install Visual Studio with C++ support
2. Download raylib from: https://github.com/raysan5/raylib/releases
3. Use the provided `.vcxproj` file to build in Visual Studio

## Building the Game

### Option 1: Using the Build Script (macOS/Linux)
```bash
cd shmup
chmod +x build.sh
./build.sh
```

### Option 2: Using Make (macOS/Linux)
```bash
cd shmup
make
```

### Option 3: Manual Compilation (macOS/Linux)
```bash
cd shmup
clang -Wall -Wextra -std=c99 -O2 \
    $(pkg-config --cflags raylib) \
    shmup.c game.c player.c enemy.c enemy_ai.c collision.c score.c menu.c render.c utils.c \
    $(pkg-config --libs raylib) \
    -o galactic_shmup
```

### Option 4: Visual Studio (Windows)
1. Open `shmup.sln` in Visual Studio
2. Make sure raylib paths are correctly configured in project properties
3. Build Solution (Ctrl+Shift+B)

## Running the Game

### macOS/Linux
```bash
./galactic_shmup
```

### Windows
Run the executable from the Debug or Release folder, or run from Visual Studio.

## Controls

- **Movement**: Arrow Keys or WASD
- **Shoot**: Space or Z
- **Pause**: P or ESC
- **Menu Navigation**: Arrow Keys/WASD, Enter/Space to select

## Game Features

- **7 AI Behaviors**: Formation Flying, Aggressive Attack, Flanking Maneuver, Swarm Behavior, Evasive Maneuver, Coordinated Attack, Defensive Formation
- **Enemy Morphing**: Normal enemies can transform into more powerful Flagship enemies
- **Captured Ship Mechanics**: Rescue captured ships for dual fighter capability
- **Wave Progression**: 5 normal stages → 1 boss stage → 1 bonus stage (repeating)
- **Dynamic Difficulty**: Aggression scaling based on wave number
- **Scoring System**: Multiple scoring mechanics with extends at 20,000 and 70,000 points

## Troubleshooting

### Common Issues

1. **"raylib not found" error**:
   - Make sure raylib is properly installed
   - Try: `brew reinstall raylib` (macOS) or reinstall raylib (Linux)

2. **Compilation errors about missing headers**:
   - Verify raylib installation: `pkg-config --exists raylib`
   - Check if raylib is in your system's include path

3. **Permission denied when running build script**:
   ```bash
   chmod +x build.sh
   ```

4. **"shmup is a directory" error**:
   - The build script automatically handles this by naming the executable `galactic_shmup`

### Build Verification
To verify a successful build:
```bash
./galactic_shmup --help  # Should start the game (no help flag implemented)
```

The game should open a window and display the main menu.

## Development

### File Structure
- `shmup.c` - Main entry point
- `game.c` - Core game logic and state management
- `player.c` - Player movement and shooting
- `enemy.c` - Enemy spawning and basic behavior
- `enemy_ai.c` - Advanced AI behaviors
- `collision.c` - Collision detection systems
- `score.c` - Scoring and high score management
- `menu.c` - Menu system and UI
- `render.c` - Rendering functions
- `utils.c` - Utility functions and movement patterns
- `game.h` - Header with all declarations and constants

### Clean Build
```bash
make clean  # or rm -f galactic_shmup
make        # or ./build.sh
```

### Debug Build
```bash
make debug
```

This builds with debug symbols and additional compiler flags for debugging.