#ifndef ASSETS_H
#define ASSETS_H

#include <raylib.h>

// Structure to hold all game assets
typedef struct Assets {
    // Background texture
    Texture2D backgroundTexture;
    // Enemy texture
    Texture2D enemyTexture;
    // Shoot sound
    Sound shootSound;
} Assets;

#endif // ASSETS_H
