#include "assets.h"
#include <stdio.h>

// Helper function to create a simple colored texture when file doesn't exist
Texture2D CreateFallbackTexture(int width, int height, Color color) {
    Image image = GenImageColor(width, height, color);
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);
    return texture;
}

// Helper function to create a simple gradient background texture
Texture2D CreateBackgroundTexture(int width, int height) {
    Image image = GenImageGradientLinear(width, height, 0, DARKBLUE, BLUE);
    Texture2D texture = LoadTextureFromImage(image);
    UnloadImage(image);
    return texture;
}

void LoadAssets(Assets* assets) {
    // Load background texture with fallback
    if (FileExists("assets/background.png")) {
        assets->backgroundTexture = LoadTexture("assets/background.png");
        printf("Loaded background texture from file\n");
    } else {
        // Create a procedural background if file doesn't exist
        assets->backgroundTexture = CreateBackgroundTexture(800, 450);
        printf("Created procedural background texture\n");
    }
    
    // Load player texture with fallback
    if (FileExists("assets/player.png")) {
        assets->playerTexture = LoadTexture("assets/player.png");
        printf("Loaded player texture from file\n");
    } else {
        // Create a simple colored rectangle for the player
        assets->playerTexture = CreateFallbackTexture(30, 30, RED);
        printf("Created fallback player texture\n");
    }
    
    // Load sounds with fallback
    if (FileExists("assets/shoot.wav")) {
        assets->shootSound = LoadSound("assets/shoot.wav");
        printf("Loaded shoot sound from file\n");
    } else {
        // Create empty sound as fallback (no sound generation for now)
        assets->shootSound = (Sound){0};
        printf("No shoot sound file found - using silent fallback\n");
    }
    
    // Load music with fallback
    if (FileExists("assets/bgm.ogg")) {
        assets->bgm = LoadMusicStream("assets/bgm.ogg");
        printf("Loaded background music from file\n");
    } else {
        // Create empty music stream (silent)
        assets->bgm = (Music){0};
        printf("No background music file found\n");
    }
}

void UnloadAssets(Assets* assets) {
    // Unload background texture
    if (assets->backgroundTexture.id != 0) {
        UnloadTexture(assets->backgroundTexture);
    }
    
    // Unload player texture
    if (assets->playerTexture.id != 0) {
        UnloadTexture(assets->playerTexture);
    }
    
    // Unload sounds
    if (assets->shootSound.frameCount > 0) {
        UnloadSound(assets->shootSound);
    }
    
    // Unload music
    if (assets->bgm.frameCount > 0) {
        UnloadMusicStream(assets->bgm);
    }
}
