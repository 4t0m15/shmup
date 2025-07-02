# Assets Directory

This directory contains the game assets. Place your resource files here:

## Required Files:
- `background.png` - Background texture for scrolling (recommended: 800x450 or larger)
- `player.png` - Player sprite texture (recommended: 30x30 or larger)
- `shoot.wav` - Sound effect for shooting
- `bgm.ogg` - Background music (optional)

## Fallback Behavior:
If any asset files are missing, the game will create procedural alternatives:
- **Background**: Blue gradient texture
- **Player**: Red square texture
- **Shoot Sound**: Generated square wave beep
- **Music**: Silent (no music)

## Asset Guidelines:
- Use PNG format for images (supports transparency)
- Use WAV format for short sound effects
- Use OGG format for music (smaller file size)
- Keep image dimensions reasonable for performance

## Example Asset Sources:
- Create simple sprites using free tools like GIMP, Paint.NET, or online pixel art editors
- Find free game assets on sites like OpenGameArt.org, Kenney.nl, or Freesound.org
- Generate simple textures procedurally using noise generators

The game will automatically detect and load any assets you place in this directory!