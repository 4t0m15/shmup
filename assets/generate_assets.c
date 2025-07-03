#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main() {
    InitWindow(800, 600, "Asset Generator");
    InitAudioDevice();
    
    printf("Generating basic game assets...\n");
    
    // Generate background texture (800x450 underwater-style gradient)
    Image bgImage = GenImageGradientRadial(800, 450, 0.0f, DARKBLUE, BLUE);
    ExportImage(bgImage, "underwater-fantasy-preview.png");
    UnloadImage(bgImage);
    printf("Generated underwater-fantasy-preview.png\n");
    
    // Generate enemy texture (25x25 simple enemy)
    Image enemyImage = GenImageColor(25, 25, BLANK);
    ImageDrawRectangle(&enemyImage, 8, 8, 9, 9, RED);       // Main body
    ImageDrawRectangle(&enemyImage, 6, 6, 13, 13, MAROON);  // Outer shell
    ImageDrawRectangle(&enemyImage, 10, 10, 5, 5, ORANGE); // Center
    ExportImage(enemyImage, "enemy.png");
    UnloadImage(enemyImage);
    printf("Generated enemy.png\n");
    
    // Generate a simple beep sound (shoot.ogg)
    Wave wave = { 0 };
    wave.frameCount = 4410;  // 0.1 seconds at 44100Hz
    wave.sampleRate = 44100;
    wave.sampleSize = 16;
    wave.channels = 1;
    wave.data = malloc(wave.frameCount * sizeof(short));
    
    if (wave.data) {
        short* samples = (short*)wave.data;
        for (int i = 0; i < wave.frameCount; i++) {
            float time = (float)i / wave.sampleRate;
            float amplitude = 0.3f * sinf(2.0f * 3.14159f * 800.0f * time);
            samples[i] = (short)(amplitude * 32767);
        }
        
        ExportWave(wave, "shoot.ogg");
        UnloadWave(wave);
        printf("Generated shoot.ogg\n");
    }
    
    CloseAudioDevice();
    CloseWindow();
    
    printf("Asset generation complete!\n");
    return 0;
}