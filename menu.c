#include "game.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

// =============================================================================
// FORWARD DECLARATIONS
// =============================================================================

static void DrawMainMenu(const GameState* gameState);
static void DrawOptionsMenu(const GameState* gameState);
static void DrawCreditsMenu(const GameState* gameState);
static void DrawInstructions(const GameState* gameState);

// =============================================================================
// MENU INPUT HANDLING (defined first to avoid forward references)
// =============================================================================

// Handle menu input
void HandleMenuInput(GameState* gameState) {
    if (!gameState) return;
    
    MenuSystem* menu = &gameState->menu;
    
    // Navigation
    if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W)) {
        PlayGameSound(&gameState->audio, GAME_SOUND_MENU_MOVE, 1.0f);
        menu->selected_option--;
        if (menu->selected_option < 0) {
            switch (menu->current_menu) {
                case MAIN_MENU:
                    menu->selected_option = 4; // 5 options (0-4)
                    break;
                case OPTIONS_MENU:
                    menu->selected_option = 5; // 6 options (0-5)
                    break;
                case CREDITS_MENU:
                    menu->selected_option = 0; // 1 option (0)
                    break;
            }
        }
    }
    
    if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S)) {
        PlayGameSound(&gameState->audio, GAME_SOUND_MENU_MOVE, 1.0f);
        menu->selected_option++;
        switch (menu->current_menu) {
            case MAIN_MENU:
                if (menu->selected_option > 4) menu->selected_option = 0;
                break;
            case OPTIONS_MENU:
                if (menu->selected_option > 5) menu->selected_option = 0;
                break;
            case CREDITS_MENU:
                if (menu->selected_option > 0) menu->selected_option = 0;
                break;
        }
    }
    
    // Selection
    if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
        PlayGameSound(&gameState->audio, GAME_SOUND_MENU_SELECT, 1.0f);
        switch (menu->current_menu) {
            case MAIN_MENU:
                switch (menu->selected_option) {
                    case 0: // Start Game
                        InitGame(gameState);
                        gameState->screen_state = PLAYING;
                        break;
                    case 1: // Instructions
                        menu->show_instructions = true;
                        menu->instruction_timer = 0.0f;
                        break;
                    case 2: // Options
                        menu->current_menu = OPTIONS_MENU;
                        menu->selected_option = 0;
                        break;
                    case 3: // Credits
                        menu->current_menu = CREDITS_MENU;
                        menu->selected_option = 0;
                        break;
                    case 4: // Quit
                        // Exit game (handled in main loop)
                        break;
                }
                break;
                
            case OPTIONS_MENU:
                switch (menu->selected_option) {
                    case 0: // Music Volume
                        // Handled by left/right keys
                        break;
                    case 1: // SFX Volume
                        // Handled by left/right keys
                        break;
                    case 2: // Difficulty
                        // Handled by left/right keys
                        break;
                    case 3: // Show FPS
                        menu->show_fps = !menu->show_fps;
                        PlayGameSound(&gameState->audio, GAME_SOUND_MENU_SELECT, 0.8f);
                        break;
                    case 4: // Reset High Score
                        gameState->high_score = 0;
                        SaveHighScore(gameState);
                        PlayGameSound(&gameState->audio, GAME_SOUND_MENU_SELECT, 0.8f);
                        break;
                    case 5: // Back
                        menu->current_menu = MAIN_MENU;
                        menu->selected_option = 0;
                        break;
                }
                break;
                
            case CREDITS_MENU:
                switch (menu->selected_option) {
                    case 0: // Back
                        menu->current_menu = MAIN_MENU;
                        menu->selected_option = 0;
                        break;
                }
                break;
        }
    }
    
    // Back/Escape
    if (IsKeyPressed(KEY_ESCAPE)) {
        switch (menu->current_menu) {
            case MAIN_MENU:
                // Could quit game here
                break;
            case OPTIONS_MENU:
            case CREDITS_MENU:
                menu->current_menu = MAIN_MENU;
                menu->selected_option = 0;
                break;
        }
    }
    
    // Options menu special handling
    if (menu->current_menu == OPTIONS_MENU) {
        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A)) {
            PlayGameSound(&gameState->audio, GAME_SOUND_MENU_MOVE, 0.7f);
            switch (menu->selected_option) {
                case 0: // Music Volume
                    menu->music_volume -= 0.1f;
                    if (menu->music_volume < 0.0f) menu->music_volume = 0.0f;
                    SetGameMusicVolume(&gameState->audio, menu->music_volume);
                    break;
                case 1: // SFX Volume
                    menu->sfx_volume -= 0.1f;
                    if (menu->sfx_volume < 0.0f) menu->sfx_volume = 0.0f;
                    SetGameSFXVolume(&gameState->audio, menu->sfx_volume);
                    break;
                case 2: // Difficulty
                    menu->difficulty--;
                    if (menu->difficulty < 0) menu->difficulty = 2;
                    break;
            }
        }
        
        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D)) {
            PlayGameSound(&gameState->audio, GAME_SOUND_MENU_MOVE, 0.7f);
            switch (menu->selected_option) {
                case 0: // Music Volume
                    menu->music_volume += 0.1f;
                    if (menu->music_volume > 1.0f) menu->music_volume = 1.0f;
                    SetGameMusicVolume(&gameState->audio, menu->music_volume);
                    break;
                case 1: // SFX Volume
                    menu->sfx_volume += 0.1f;
                    if (menu->sfx_volume > 1.0f) menu->sfx_volume = 1.0f;
                    SetGameSFXVolume(&gameState->audio, menu->sfx_volume);
                    break;
                case 2: // Difficulty
                    menu->difficulty++;
                    if (menu->difficulty > 2) menu->difficulty = 0;
                    break;
            }
        }
    }
}

// =============================================================================
// MENU SYSTEM FUNCTIONS
// =============================================================================

// Initialize menu system
void InitMenu(MenuSystem* menu) {
    if (!menu) return;
    
    menu->current_menu = MAIN_MENU;
    menu->selected_option = 0;
    menu->transition_timer = 0.0f;
    menu->show_instructions = false;
    menu->instruction_timer = 0.0f;
    
    // Default options
    menu->music_volume = 0.7f;
    menu->sfx_volume = 0.8f;
    menu->difficulty = 1; // Normal
    menu->show_fps = false;
}

// Update menu system
void UpdateMenu(GameState* gameState, float delta) {
    if (!gameState) return;
    
    MenuSystem* menu = &gameState->menu;
    
    menu->transition_timer += delta;
    
    if (menu->show_instructions) {
        menu->instruction_timer += delta;
        
        // Hide instructions after 10 seconds or if player presses any key
        if (menu->instruction_timer > 10.0f || 
            IsKeyPressed(KEY_SPACE) || 
            IsKeyPressed(KEY_ENTER) || 
            IsKeyPressed(KEY_ESCAPE)) {
            menu->show_instructions = false;
            menu->instruction_timer = 0.0f;
        }
        return;
    }
    
    HandleMenuInput(gameState);
}

// =============================================================================
// MENU DRAWING FUNCTIONS
// =============================================================================

// Draw menu system
void DrawMenu(const GameState* gameState) {
    if (!gameState) return;
    
    const MenuSystem* menu = &gameState->menu;
    
    ClearBackground(BLACK);
    
    // Draw starfield background
    for (int i = 0; i < 100; i++) {
        int x = (i * 123) % SCREEN_WIDTH;
        int y = (i * 456 + (int)(menu->transition_timer * 20)) % SCREEN_HEIGHT;
        int brightness = (i * 234) % 128 + 127;
        DrawPixel(x, y, (Color){brightness, brightness, brightness, 255});
    }
    
    if (menu->show_instructions) {
        DrawInstructions(gameState);
        return;
    }
    
    // Draw title
    int title_y = 50;
    DrawText("GALACTIC SHMUP", SCREEN_WIDTH/2 - 140, title_y, 40, WHITE);
    
    // Draw subtitle with animation
    float subtitle_alpha = (sinf(menu->transition_timer * 2.0f) + 1.0f) / 2.0f;
    Color subtitle_color = {255, 255, 0, (unsigned char)(subtitle_alpha * 255)};
    DrawText("Enhanced Edition", SCREEN_WIDTH/2 - 80, title_y + 45, 20, subtitle_color);
    
    // Draw high score
    DrawText(TextFormat("High Score: %d", gameState->high_score), 
             SCREEN_WIDTH/2 - 80, title_y + 80, 20, YELLOW);
    
    switch (menu->current_menu) {
        case MAIN_MENU:
            DrawMainMenu(gameState);
            break;
        case OPTIONS_MENU:
            DrawOptionsMenu(gameState);
            break;
        case CREDITS_MENU:
            DrawCreditsMenu(gameState);
            break;
    }
    
    // Draw footer
    DrawText("Use Arrow Keys/WASD to navigate, ENTER/SPACE to select", 
             SCREEN_WIDTH/2 - 200, SCREEN_HEIGHT - 30, 16, GRAY);
}

// Draw main menu
static void DrawMainMenu(const GameState* gameState) {
    if (!gameState) return;
    
    const MenuSystem* menu = &gameState->menu;
    int start_y = 180;
    int spacing = 35;
    
    const char* menu_options[] = {
        "Play",
        "Instructions",
        "Options",
        "Credits",
        "Quit"
    };
    
    for (int i = 0; i < 5; i++) {
        Color color = (i == menu->selected_option) ? YELLOW : WHITE;
        
        // Add selection arrow
        if (i == menu->selected_option) {
            DrawText(">", SCREEN_WIDTH/2 - 120, start_y + i * spacing, 20, YELLOW);
        }
        
        DrawText(menu_options[i], SCREEN_WIDTH/2 - 100, start_y + i * spacing, 20, color);
    }
    
    // Draw additional info
    DrawText("Wave: Enhanced AI Edition", SCREEN_WIDTH/2 - 90, start_y + 6 * spacing, 16, GREEN);
    DrawText("Features: Advanced Enemy AI, Morphing, Dual Fighter", 
             SCREEN_WIDTH/2 - 180, start_y + 7 * spacing, 14, LIGHTGRAY);
}

// Draw options menu
static void DrawOptionsMenu(const GameState* gameState) {
    if (!gameState) return;
    
    const MenuSystem* menu = &gameState->menu;
    int start_y = 160;
    int spacing = 35;
    
    const char* difficulty_names[] = {"Easy", "Normal", "Hard"};
    
    // Music Volume
    Color color = (menu->selected_option == 0) ? YELLOW : WHITE;
    if (menu->selected_option == 0) {
        DrawText(">", SCREEN_WIDTH/2 - 150, start_y, 20, YELLOW);
    }
    DrawText("Music Volume:", SCREEN_WIDTH/2 - 130, start_y, 20, color);
    DrawText(TextFormat("%.1f", menu->music_volume), SCREEN_WIDTH/2 + 50, start_y, 20, color);
    
    // SFX Volume
    color = (menu->selected_option == 1) ? YELLOW : WHITE;
    if (menu->selected_option == 1) {
        DrawText(">", SCREEN_WIDTH/2 - 150, start_y + spacing, 20, YELLOW);
    }
    DrawText("SFX Volume:", SCREEN_WIDTH/2 - 130, start_y + spacing, 20, color);
    DrawText(TextFormat("%.1f", menu->sfx_volume), SCREEN_WIDTH/2 + 50, start_y + spacing, 20, color);
    
    // Difficulty
    color = (menu->selected_option == 2) ? YELLOW : WHITE;
    if (menu->selected_option == 2) {
        DrawText(">", SCREEN_WIDTH/2 - 150, start_y + 2 * spacing, 20, YELLOW);
    }
    DrawText("Difficulty:", SCREEN_WIDTH/2 - 130, start_y + 2 * spacing, 20, color);
    DrawText(difficulty_names[menu->difficulty], SCREEN_WIDTH/2 + 50, start_y + 2 * spacing, 20, color);
    
    // Show FPS
    color = (menu->selected_option == 3) ? YELLOW : WHITE;
    if (menu->selected_option == 3) {
        DrawText(">", SCREEN_WIDTH/2 - 150, start_y + 3 * spacing, 20, YELLOW);
    }
    DrawText("Show FPS:", SCREEN_WIDTH/2 - 130, start_y + 3 * spacing, 20, color);
    DrawText(menu->show_fps ? "ON" : "OFF", SCREEN_WIDTH/2 + 50, start_y + 3 * spacing, 20, color);
    
    // Reset High Score
    color = (menu->selected_option == 4) ? YELLOW : WHITE;
    if (menu->selected_option == 4) {
        DrawText(">", SCREEN_WIDTH/2 - 150, start_y + 4 * spacing, 20, YELLOW);
    }
    DrawText("Reset High Score", SCREEN_WIDTH/2 - 130, start_y + 4 * spacing, 20, color);
    
    // Back
    color = (menu->selected_option == 5) ? YELLOW : WHITE;
    if (menu->selected_option == 5) {
        DrawText(">", SCREEN_WIDTH/2 - 150, start_y + 5 * spacing, 20, YELLOW);
    }
    DrawText("Back", SCREEN_WIDTH/2 - 130, start_y + 5 * spacing, 20, color);
    
    // Instructions
    DrawText("Use Left/Right arrows to adjust values", SCREEN_WIDTH/2 - 130, start_y + 7 * spacing, 16, GRAY);
}

// Draw credits menu
static void DrawCreditsMenu(const GameState* gameState) {
    if (!gameState) return;
    
    const MenuSystem* menu = &gameState->menu;
    int start_y = 140;
    int spacing = 25;
    
    DrawText("GALACTIC SHMUP - Enhanced Edition", SCREEN_WIDTH/2 - 160, start_y, 24, WHITE);
    
    DrawText("Programming & Design:", SCREEN_WIDTH/2 - 100, start_y + 2 * spacing, 18, YELLOW);
    DrawText("Arsen Martirosyan", SCREEN_WIDTH/2 - 50, start_y + 3 * spacing, 16, WHITE);
    
    DrawText("Features:", SCREEN_WIDTH/2 - 40, start_y + 5 * spacing, 18, YELLOW);
    DrawText("- Advanced Enemy AI with 7 behavior types", SCREEN_WIDTH/2 - 140, start_y + 6 * spacing, 14, WHITE);
    DrawText("- Enemy morphing and captured ship mechanics", SCREEN_WIDTH/2 - 140, start_y + 7 * spacing, 14, WHITE);
    DrawText("- Predictive AI targeting system", SCREEN_WIDTH/2 - 140, start_y + 8 * spacing, 14, WHITE);
    DrawText("- Coordinated enemy attacks", SCREEN_WIDTH/2 - 140, start_y + 9 * spacing, 14, WHITE);
    DrawText("- Dynamic difficulty scaling", SCREEN_WIDTH/2 - 140, start_y + 10 * spacing, 14, WHITE);
    
    DrawText("Inspired by classic arcade shooters", SCREEN_WIDTH/2 - 120, start_y + 12 * spacing, 16, GRAY);
    
    // Back option
    Color color = (menu->selected_option == 0) ? YELLOW : WHITE;
    if (menu->selected_option == 0) {
        DrawText(">", SCREEN_WIDTH/2 - 50, start_y + 14 * spacing, 20, YELLOW);
    }
    DrawText("Back", SCREEN_WIDTH/2 - 30, start_y + 14 * spacing, 20, color);
}

// Draw instructions
static void DrawInstructions(const GameState* gameState) {
    if (!gameState) return;
    
    const MenuSystem* menu = &gameState->menu;
    int start_y = 60;
    int spacing = 20;
    
    DrawText("INSTRUCTIONS", SCREEN_WIDTH/2 - 80, start_y, 24, WHITE);
    
    DrawText("MOVEMENT:", 50, start_y + 2 * spacing, 18, YELLOW);
    DrawText("Arrow Keys or WASD - Move player", 70, start_y + 3 * spacing, 16, WHITE);
    
    DrawText("COMBAT:", 50, start_y + 5 * spacing, 18, YELLOW);
    DrawText("SPACE - Shoot", 70, start_y + 6 * spacing, 16, WHITE);
    DrawText("Destroy all enemies to advance waves", 70, start_y + 7 * spacing, 16, WHITE);
    
    DrawText("ENEMY TYPES:", 50, start_y + 9 * spacing, 18, YELLOW);
    DrawText("Normal (Bee) - Basic enemy, can morph", 70, start_y + 10 * spacing, 16, WHITE);
    DrawText("Escort (Butterfly) - Faster, more aggressive", 70, start_y + 11 * spacing, 16, WHITE);
    DrawText("Boss (Galaga) - Tractor beam, captures ships", 70, start_y + 12 * spacing, 16, WHITE);
    DrawText("Flagship - Morphed enemy, high value", 70, start_y + 13 * spacing, 16, WHITE);
    
    DrawText("WAVE PROGRESSION:", 50, start_y + 15 * spacing, 18, YELLOW);
    DrawText("- 5 Normal stages, then 1 Boss, then 1 Bonus", 70, start_y + 16 * spacing, 16, WHITE);
    DrawText("- Rescue captured ships for dual fighter", 70, start_y + 17 * spacing, 16, WHITE);
    DrawText("- Enemies use advanced AI behaviors", 70, start_y + 18 * spacing, 16, WHITE);
    DrawText("- Difficulty scales with wave number", 70, start_y + 19 * spacing, 16, WHITE);
    
    // Fade out instruction
    float fade_alpha = 1.0f;
    if (menu->instruction_timer > 8.0f) {
        fade_alpha = 1.0f - (menu->instruction_timer - 8.0f) / 2.0f;
    }
    
    Color fade_color = {255, 255, 255, (unsigned char)(fade_alpha * 255)};
    DrawText("Press any key to continue...", SCREEN_WIDTH/2 - 100, SCREEN_HEIGHT - 40, 16, fade_color);
}