#include <raylib.h>
#include "game.h"

int main() {
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Galactic Shmup - Enhanced Edition");
    SetTargetFPS(90);

    GameState gameState;
    InitGame(&gameState); 

    // Main game loop
    while (!WindowShouldClose()) {
        float delta = GetFrameTime(); // Get time since last frame
        
        // Check for quit from menu
        if (gameState.screen_state == MENU && 
            gameState.menu.current_menu == MAIN_MENU && 
            gameState.menu.selected_option == 4 && 
            (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE))) {
            break; // Exit game loop
        }
        
        UpdateGame(&gameState, delta);

        BeginDrawing();
        DrawGame(&gameState);
        EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
