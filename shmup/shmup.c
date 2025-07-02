#include <raylib.h>
#include "game.h"
#include "assets.h"

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
    CloseWindow();
    return 0;
}
