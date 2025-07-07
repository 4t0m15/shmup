#include "game.h"
#include <stdio.h>
#include <math.h>

// Function to draw a bullet with realistic appearance
void DrawBullet(Vector2 position, Color color, bool is_player_bullet) {
    if (is_player_bullet) {
        // Player bullets: elongated with bright tip
        float bullet_length = 12.0f;
        float bullet_width = 4.0f;
        
        // Draw main body (rectangle)
        DrawRectangle(
            (int)(position.x - bullet_width / 2.0f),
            (int)(position.y - bullet_length / 2.0f),
            (int)bullet_width,
            (int)bullet_length,
            color
        );
        
        // Draw bright tip (smaller rectangle)
        DrawRectangle(
            (int)(position.x - bullet_width / 4.0f),
            (int)(position.y - bullet_length / 2.0f),
            (int)(bullet_width / 2.0f),
            (int)(bullet_length / 3.0f),
            WHITE
        );
        
        // Draw subtle glow effect
        DrawCircle((int)position.x, (int)position.y, bullet_width, (Color){color.r, color.g, color.b, 60});
    } else {
        // Enemy bullets: different shape, more menacing
        float bullet_length = 10.0f;
        float bullet_width = 3.0f;
        
        // Draw main body (rectangle)
        DrawRectangle(
            (int)(position.x - bullet_width / 2.0f),
            (int)(position.y - bullet_length / 2.0f),
            (int)bullet_width,
            (int)bullet_length,
            color
        );
        
        // Draw darker tip
        DrawRectangle(
            (int)(position.x - bullet_width / 4.0f),
            (int)(position.y + bullet_length / 4.0f),
            (int)(bullet_width / 2.0f),
            (int)(bullet_length / 3.0f),
            (Color){color.r / 2, color.g / 2, color.b / 2, 255}
        );
        
        // Draw subtle red glow for enemy bullets
        DrawCircle((int)position.x, (int)position.y, bullet_width + 1, (Color){255, 100, 100, 40});
    }
}

// Draw scrolling background
void DrawBackground(const GameState* gameState) {
    for (int y = -SCREEN_HEIGHT; y < SCREEN_HEIGHT * 2; y += 40) {
        int scroll_y = (int)(y + gameState->backgroundScrollY) % SCREEN_HEIGHT;
        DrawLine(0, scroll_y, SCREEN_WIDTH, scroll_y, DARKGRAY);
    }
}

// Draw player
void DrawPlayer(const GameState* gameState) {
    DrawRectangleRec(gameState->player.rect, gameState->player.color);
    
    // Draw captured ship if dual fighter
    if (gameState->player.has_captured_ship) {
        DrawRectangle(
            (int)gameState->player.captured_ship_offset.x,
            (int)gameState->player.captured_ship_offset.y,
            PLAYER_SIZE,
            PLAYER_SIZE,
            SKYBLUE
        );
    }
}

// Draw enemies
void DrawEnemies(const GameState* gameState) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (gameState->enemies[i].active) {
            Color enemy_color = GREEN;
            float enemy_size = ENEMY_SIZE;
            
            switch (gameState->enemies[i].type) {
                case BOSS:
                    enemy_color = PURPLE;
                    enemy_size = BOSS_SIZE;
                    break;
                case ESCORT:
                    enemy_color = ORANGE;
                    break;
                case FLAGSHIP:
                    enemy_color = GOLD;
                    enemy_size = FLAGSHIP_SIZE;
                    break;
                case HOSTILE_SHIP:
                    enemy_color = MAROON;
                    break;
                default:
                    enemy_color = GREEN;
                    break;
            }
            
            // Draw morphing effect
            if (gameState->enemies[i].state == MORPHING) {
                float morph_progress = 1.0f - (gameState->enemies[i].morph_timer / MORPH_DURATION);
                enemy_color.a = (unsigned char)(255 * (0.5f + 0.5f * sinf(morph_progress * PI * 4.0f)));
            }
            
            DrawCircle(
                (int)gameState->enemies[i].position.x,
                (int)gameState->enemies[i].position.y,
                enemy_size / 2.0f,
                enemy_color
            );
            
            // Draw tractor beam
            if (gameState->enemies[i].tractor_active) {
                DrawCircleLines(
                    (int)gameState->enemies[i].tractor_center.x,
                    (int)gameState->enemies[i].tractor_center.y,
                    TRACTOR_BEAM_RANGE,
                    YELLOW
                );
            }
        }
    }
}

// Draw bullets
void DrawBullets(const GameState* gameState) {
    // Draw player bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (gameState->bullets[i].active) {
            DrawBullet(gameState->bullets[i].position, YELLOW, true);
        }
    }
    
    // Draw enemy bullets
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (gameState->enemy_bullets[i].active) {
            DrawBullet(gameState->enemy_bullets[i].position, RED, false);
        }
    }
}

// Draw UI elements
void DrawUI(const GameState* gameState) {
    // Draw main UI
    DrawText(TextFormat("Score: %d", gameState->score), 10, 10, 20, WHITE);
    DrawText(TextFormat("High Score: %d", gameState->high_score), 10, 35, 20, WHITE);
    DrawText(TextFormat("Lives: %d", gameState->player.lives), 10, 60, 20, WHITE);
    DrawText(TextFormat("Wave: %d", gameState->wave_number), 10, 85, 20, WHITE);
    
    // Draw controls and debug information in the bottom right
    int controls_x = SCREEN_WIDTH - 200;
    int controls_y = SCREEN_HEIGHT - 120;
    DrawText("Controls:", controls_x, controls_y, 18, LIGHTGRAY);
    DrawText("WASD/Arrows: Move", controls_x, controls_y + 20, 16, GRAY);
    DrawText("Space/Z: Shoot", controls_x, controls_y + 40, 16, GRAY);
    
    // Draw seed information only if FPS is enabled (debug mode)
    if (gameState->menu.show_fps) {
        DrawText(TextFormat("Seed: %u", gameState->random_seed), controls_x, controls_y + 70, 16, YELLOW);
    }
    
    // Draw score popups
    for (int i = 0; i < 10; i++) {
        if (gameState->score_popups[i].active) {
            char score_text[16];
            sprintf(score_text, "%d", gameState->score_popups[i].score);
            DrawText(score_text, 
                (int)gameState->score_popups[i].position.x,
                (int)gameState->score_popups[i].position.y,
                20, WHITE);
        }
    }
    
    if (gameState->is_bonus_stage) {
        DrawText("BONUS STAGE", SCREEN_WIDTH / 2 - 80, 50, 20, GOLD);
    }
}