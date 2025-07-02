#ifndef ASSETS_H
#define ASSETS_H

#include <raylib.h>

// Structure to hold all game assets
typedef struct Assets {
    // Background texture
    Texture2D backgroundTexture;
    // Example texture
    Texture2D playerTexture;
    // Example sound
    Sound shootSound;
    // Example music
    Music bgm;
    // Add more assets as needed
} Assets;

// Loads all assets and returns an Assets struct
void LoadAssets(Assets* assets);

// Unloads all assets
void UnloadAssets(Assets* assets);

#endif // ASSETS_H
