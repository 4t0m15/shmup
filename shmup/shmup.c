#include <raylib.h>
#include "game.h"

int main()
{
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "shmup");
    SetTargetFPS(999);

    Player player;
    InitGame(&player);

    // Main game loop
    while (!WindowShouldClose())
    {
        UpdateGame(&player);

        BeginDrawing();
        DrawGame(&player);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}