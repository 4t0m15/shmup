# Galactic Shmup - Enhanced Edition

## 🚀 Overview

An advanced shoot-em-up game built with raylib featuring sophisticated AI, weapon systems, visual effects, and progression mechanics. This enhanced edition transforms the classic arcade experience with modern game development techniques.

## ✨ Enhanced Features

### 🎯 Advanced Weapon System
- **8 Unique Weapons** with distinct behaviors:
  - Single Shot (Basic projectile)
  - Double Shot (Parallel bullets)
  - Triple Shot (Spread pattern)
  - Spread Shot (Wide fan)
  - Rapid Fire (High rate of fire)
  - Laser (Penetrating beam)
  - Homing Missiles (Target-seeking)
  - Plasma Cannon (Explosive area damage)
- **5-Level Upgrade System** per weapon
- **Dynamic Unlocking** based on progression
- **Weapon Switching** with Q/E keys

### 🏆 Achievement System
- **33 Achievements** across multiple categories:
  - Combat (kills, accuracy, combos)
  - Progression (waves, scores)
  - Special (rescues, speed runs, perfect runs)
  - Time-based (total play time)
- **4 Tier System**: Bronze, Silver, Gold, Platinum
- **Real-time Notifications** with visual effects
- **Persistent Progress** tracking and statistics
- **Completion Rewards** and unlockables

### 🎨 Advanced Visual Effects
- **Post-Processing Shaders**:
  - Bloom (glowing bright objects)
  - Chromatic Aberration (color separation)
  - Screen Distortion (wave/ripple effects)
  - Energy Fields (animated backgrounds)
  - Gaussian Blur (smooth blurring)
- **Dynamic Effect Triggers** based on gameplay events
- **Enhanced Particle System** (500+ particles)
- **Screen Shake** and flash effects

### 🤖 Enhanced AI Systems
- **7 Advanced Behaviors**:
  - Formation Flying
  - Aggressive Attack
  - Flanking Maneuver
  - Swarm Behavior
  - Evasive Maneuver
  - Coordinated Attack
  - Defensive Formation
- **Predictive Targeting** with player movement analysis
- **Adaptive Difficulty** scaling based on player skill
- **Squad Coordination** and group tactics

### 🎵 Procedural Audio
- **Generated Sound Effects** for all game events
- **Dynamic Music System** with contextual tracks
- **3D Positional Audio** support
- **Adaptive Volume** controls

### ⚖️ Advanced Balance System
- **Real-time Difficulty Adjustment** based on:
  - Player accuracy
  - Kill rate
  - Survival time
  - Performance metrics
- **Combo System** with multipliers
- **Power-up Integration** with temporary abilities
- **Quality of Life Features**:
  - Auto-pause on focus loss
  - Near-miss detection
  - Screen edge warnings
  - Hit indicators

## 🎮 Controls

### Basic Controls
- **Movement**: Arrow Keys or WASD
- **Shoot**: Space or Z
- **Pause**: P or ESC

### Enhanced Controls
- **Q/E**: Switch weapons
- **U**: Upgrade current weapon
- **Tab**: Show achievement progress (when implemented)

### Menu Navigation
- **Arrow Keys/WASD**: Navigate menus
- **Enter/Space**: Select option
- **ESC**: Back/Cancel

## 🏗️ Technical Architecture

### Core Systems
- **Modular Design** with separate systems
- **Component-based Architecture** for game objects
- **Event-driven Communication** between systems
- **Memory-efficient Particle Pooling**
- **Spatial Optimization** for collision detection

### File Structure
```
wipshmup/
├── shmup.c           # Main entry point
├── game.c            # Core game logic
├── player.c          # Player mechanics
├── enemy.c           # Enemy spawning and behavior
├── enemy_ai.c        # Advanced AI systems
├── weapons.c         # Weapon system (NEW)
├── achievements.c    # Achievement tracking (NEW)
├── shaders.c         # Visual effects (NEW)
├── effects.c         # Particle systems
├── audio.c           # Audio management
├── balance.c         # Difficulty and progression
├── collision.c       # Physics and collision
├── score.c           # Scoring system
├── menu.c            # UI and menus
├── render.c          # Enhanced rendering
├── utils.c           # Utility functions
└── game.h            # Master header file
```

## 🛠️ Building

### Prerequisites
- **raylib 5.0+**
- **C99 compatible compiler**
- **OpenGL 3.3+ support** (for shaders)

### macOS (Recommended)
```bash
# Install dependencies
brew install raylib

# Build
make
# or
./build.sh

# Run
./galactic_shmup
```

### Linux (Ubuntu/Debian)
```bash
# Install dependencies
sudo apt update
sudo apt install build-essential git cmake
sudo apt install libasound2-dev libx11-dev libxrandr-dev libxi-dev libgl1-mesa-dev

# Install raylib
git clone https://github.com/raysan5/raylib.git
cd raylib/src && make PLATFORM=PLATFORM_DESKTOP
sudo make install PLATFORM=PLATFORM_DESKTOP

# Build game
cd /path/to/shmup
make
```

### Windows (Visual Studio)
1. Download raylib from releases
2. Open `shmup.sln` in Visual Studio
3. Configure raylib paths in project properties
4. Build Solution (Ctrl+Shift+B)

## 🎯 Gameplay Features

### Wave Progression
- **7-Wave Cycles**: 5 normal → 1 boss → 1 bonus
- **Progressive Difficulty** with enemy morphing
- **Dynamic Spawn Rates** based on player performance
- **Special Events** and bonus objectives

### Enemy Types
- **Normal** (Bee-type, can morph)
- **Escort** (Butterfly-type, aggressive)
- **Boss** (Galaga-type, tractor beam)
- **Flagship** (Morphed enemies, high value)
- **Hostile Ship** (Rescued ships turned enemy)

### Scoring System
- **Base Scores** per enemy type and state
- **Combo Multipliers** for consecutive hits
- **Formation vs Diving** bonus differentiation
- **Perfect Bonus Stages** with high rewards
- **Life Extends** at 20,000 and 70,000 points

### Power-Up System
- **6 Power-Up Types**:
  - Rapid Fire (faster shooting)
  - Shield (temporary invincibility)
  - Spread Shot (multi-directional fire)
  - Slow Motion (bullet-time effect)
  - Extra Life (health restoration)
  - Score Multiplier (bonus points)
- **Weapon Upgrades** and unlocks
- **Visual Enhancement** effects

## 📊 Statistics & Analytics

### Player Metrics
- **Accuracy Tracking** (shots hit/fired ratio)
- **Kill Rate** (enemies per minute)
- **Survival Score** (time alive/deaths)
- **Combo Performance** (max consecutive hits)
- **Wave Progression** (highest wave reached)

### Achievement Categories
- **Combat Mastery** (1 to 5000 kills)
- **Wave Progression** (10 to 100 waves)
- **Score Achievements** (10K to 500K points)
- **Accuracy Challenges** (75% to 100% accuracy)
- **Special Feats** (perfect runs, speed runs)
- **Dedication Rewards** (play time milestones)

## 🔧 Configuration

### Graphics Options
- **Post-Processing Effects** toggle
- **Individual Shader Controls**
- **Particle Density** settings
- **Performance Profiling** display

### Audio Options
- **Master Volume** (0-100%)
- **SFX Volume** (0-100%)
- **Music Volume** (0-100%)
- **3D Audio** positioning

### Gameplay Options
- **Difficulty Selection** (Easy/Normal/Hard)
- **Adaptive Difficulty** toggle
- **Quality of Life** features
- **Debug Information** display

## 🚀 Future Enhancements

### Planned Features
- **Campaign Mode** with story progression
- **Multiplayer Support** (local co-op)
- **Level Editor** for custom waves
- **Mobile Platform** support
- **VR Mode** experimentation

### Community Features
- **Leaderboards** and score sharing
- **Replay System** for epic moments
- **Custom Skins** and ship variants
- **Mod Support** framework

## 🐛 Known Issues

- Shader effects require OpenGL 3.3+
- Some achievements may not trigger in edge cases
- Weapon balance still being tuned
- Memory usage can increase with extended play

## 🤝 Contributing

### Development Setup
1. Fork the repository
2. Create feature branch
3. Follow coding standards
4. Test thoroughly
5. Submit pull request

### Coding Standards
- **C99 Standard** compliance
- **4-space indentation**
- **Descriptive variable names**
- **Comprehensive comments**
- **Error handling** for all functions

## 📚 Documentation

- `ENHANCEMENT_GUIDE.md` - Detailed feature roadmap
- `INTEGRATION_EXAMPLE.md` - Code integration examples
- `BUILD_INSTRUCTIONS.md` - Platform-specific build guides
- `GAME_POLISH_FEATURES.md` - Polish implementation details

## 📄 License

This project is released under the MIT License. See LICENSE file for details.

## 🙏 Acknowledgments

- **raylib** community for the excellent framework
- **Classic Arcade Games** for inspiration
- **Modern Indie Games** for polish reference
- **Open Source Community** for tools and resources

## 📞 Support

For issues, questions, or contributions:
- Create GitHub issues for bugs
- Use discussions for feature requests
- Check documentation for implementation help
- Follow coding standards for contributions

---

**Galactic Shmup Enhanced Edition** - Where classic arcade action meets modern game development excellence.