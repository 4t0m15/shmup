#include <raylib.h>
#include "game.h"
#include <stdio.h>

// Helper function to load a texture from the "Resources" directory
static Texture2D LoadTextureFromResources(const char* filename) {
    char path[512];
    snprintf(path, sizeof(path), "Resources/%s", filename);
    
    if (FileExists(path)) {
        Texture2D texture = LoadTexture(path);
        if (texture.id != 0) {
            return texture;
        }
    }
    
    return (Texture2D){0};
}

// Helper function to load a sound from the "Resources" directory
static Sound LoadSoundFromResources(const char* filename) {
    char path[512];
    snprintf(path, sizeof(path), "Resources/%s", filename);
    
    if (FileExists(path)) {
        Sound sound = LoadSound(path);
        if (sound.frameCount > 0) {
            return sound;
        }
    }
    
    return (Sound){0};
}

// Inline asset loading function
static void LoadAssets(Assets* assets) {
    if (assets == NULL) {
        return;
    }
    
    if (!IsAudioDeviceReady()) {
        InitAudioDevice();
    }
    
    assets->backgroundTexture = LoadTextureFromResources("underwater-fantasy-preview.png");
    assets->enemyTexture = LoadTextureFromResources("enemy.png");
    assets->shootSound = LoadSoundFromResources("shoot.ogg");
}

// Inline asset unloading function
static void UnloadAssets(Assets* assets) {
    if (assets == NULL) {
        return;
    }
    
    if (assets->backgroundTexture.id != 0) UnloadTexture(assets->backgroundTexture);
    if (assets->enemyTexture.id != 0) UnloadTexture(assets->enemyTexture);
    if (assets->shootSound.frameCount > 0) UnloadSound(assets->shootSound);
}

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "shmup");
    SetTargetFPS(60);

    Assets assets;
    LoadAssets(&assets);

    GameState gameState;
    InitGame(&gameState); 

    // Main game loop
    while (!WindowShouldClose())
    {
        float delta = GetFrameTime(); // Get time since last frame
        UpdateGame(&gameState, delta, &assets);   // Pass delta time and assets

        BeginDrawing();
        DrawGame(&gameState, &assets);
        EndDrawing();
    }

    UnloadAssets(&assets);
    
    // Close audio device if it was initialized
    if (IsAudioDeviceReady()) {
        CloseAudioDevice();
    }
    
    CloseWindow();
    return 0;
}
