#include <raylib.h>
#include <stdio.h>
#include "../shmup/assets.h"

int main() {
    printf("=== Asset Loading Test ===\n");
    
    // Initialize minimal raylib context
    InitWindow(400, 300, "Asset Test");
    
    if (!IsAudioDeviceReady()) {
        InitAudioDevice();
    }
    
    // Test asset loading - manually load as the functions no longer exist
    Assets assets = {0};
    
    // Try loading textures and sounds from Resources directory
    if (FileExists("Resources/underwater-fantasy-preview.png")) {
        assets.backgroundTexture = LoadTexture("Resources/underwater-fantasy-preview.png");
    }
    
    if (FileExists("Resources/enemy.png")) {
        assets.enemyTexture = LoadTexture("Resources/enemy.png");
    }
    
    if (FileExists("Resources/shoot.ogg")) {
        assets.shootSound = LoadSound("Resources/shoot.ogg");
    }
    
    // Test individual assets
    printf("\nAsset Status:\n");
    printf("- Background texture: %s (ID: %u)\n", 
           assets.backgroundTexture.id != 0 ? "LOADED" : "MISSING", 
           assets.backgroundTexture.id);
    printf("- Enemy texture: %s (ID: %u)\n", 
           assets.enemyTexture.id != 0 ? "LOADED" : "MISSING", 
           assets.enemyTexture.id);
    printf("- Shoot sound: %s (Frames: %u)\n", 
           assets.shootSound.frameCount > 0 ? "LOADED" : "MISSING", 
           assets.shootSound.frameCount);
    
    // Test sound playback
    if (assets.shootSound.frameCount > 0) {
        printf("\nTesting sound playback...\n");
        PlaySound(assets.shootSound);
        // Wait a bit for sound to play
        WaitTime(0.5f);
    }
    
    // Clean up
    if (assets.backgroundTexture.id != 0) UnloadTexture(assets.backgroundTexture);
    if (assets.enemyTexture.id != 0) UnloadTexture(assets.enemyTexture);
    if (assets.shootSound.frameCount > 0) UnloadSound(assets.shootSound);
    
    if (IsAudioDeviceReady()) {
        CloseAudioDevice();
    }
    
    CloseWindow();
    
    printf("=== Test Complete ===\n");
    return 0;
}