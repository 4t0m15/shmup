#include <raylib.h>
#include "game.h"

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "shmup");
    SetTargetFPS(60);

    Player player;
    InitGame(&player);

    // Main game loop
    while (!WindowShouldClose())
    {
        float delta = GetFrameTime(); // Get time since last frame
        UpdateGame(&player, delta);   // Pass delta time

        BeginDrawing();
        DrawGame(&player);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}