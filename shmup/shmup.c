#include <raylib.h>
#include "game.h"

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "shmup");
    SetTargetFPS(60);

    GameState gameState;
    InitGame(&gameState); 

    // Main game loop
    while (!WindowShouldClose())
    {
        float delta = GetFrameTime(); // Get time since last frame
        UpdateGame(&gameState, delta);

        BeginDrawing();
        DrawGame(&gameState);
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
