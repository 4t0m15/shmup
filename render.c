#include "game.h"
#include <stdio.h>
#include <math.h>

static float render_time = 0.0f;

// Draw enhanced scrolling background with parallax and stars
void DrawBackground(const GameState* gameState) {
    // Draw deep space background
    ClearBackground(BLACK);
    
    // Draw distant stars (slow parallax)
    for (int i = 0; i < 200; i++) {
        int x = (i * 123) % SCREEN_WIDTH;
        int y = (i * 456 + (int)(gameState->backgroundScrollY * 0.1f)) % SCREEN_HEIGHT;
        int brightness = (i * 234) % 100 + 100;
        Color star_color = {brightness, brightness, brightness, 255};
        DrawPixel(x, y, star_color);
    }
    
    // Draw medium stars (medium parallax)
    for (int i = 0; i < 50; i++) {
        int x = (i * 234) % SCREEN_WIDTH;
        int y = (i * 567 + (int)(gameState->backgroundScrollY * 0.3f)) % SCREEN_HEIGHT;
        int brightness = (i * 345) % 80 + 150;
        Color star_color = {brightness, brightness, brightness + 20, 255};
        DrawCircle(x, y, 1, star_color);
    }
    
    // Draw grid lines (fast parallax)
    for (int y = -SCREEN_HEIGHT; y < SCREEN_HEIGHT * 2; y += 40) {
        int scroll_y = (int)(y + gameState->backgroundScrollY) % SCREEN_HEIGHT;
        Color line_color = {0, 50, 100, 100};
        DrawLine(0, scroll_y, SCREEN_WIDTH, scroll_y, line_color);
    }
    
    // Add subtle vertical lines
    for (int x = 0; x < SCREEN_WIDTH; x += 80) {
        Color line_color = {0, 30, 60, 50};
        DrawLine(x, 0, x, SCREEN_HEIGHT, line_color);
    }
    
    // Draw nebula effect in background
    render_time += 0.016f; // Approximate 60 FPS
    for (int i = 0; i < 20; i++) {
        float x = (float)(i * 150 % SCREEN_WIDTH);
        float y = (float)(i * 200 % SCREEN_HEIGHT) + sinf(render_time + i) * 20.0f;
        float radius = 30.0f + sinf(render_time * 0.5f + i) * 10.0f;
        Color nebula_color = {20, 0, 40, 30};
        DrawCircleGradient((int)x, (int)y, radius, BLANK, nebula_color);
    }
}

// Draw enhanced player with visual effects
void DrawPlayer(const GameState* gameState) {
    const Player* player = &gameState->player;
    
    // Draw ship shadow
    DrawRectangle(
        (int)(player->rect.x + 2),
        (int)(player->rect.y + 2),
        (int)player->rect.width,
        (int)player->rect.height,
        (Color){0, 0, 0, 100}
    );
    
    // Draw main ship with enhanced visuals
    Color ship_color = player->color;
    
    // Add pulsing effect when low on health
    if (player->lives <= 1) {
        float pulse = 0.7f + 0.3f * sinf(render_time * 8.0f);
        ship_color.r = (unsigned char)(ship_color.r * pulse);
        ship_color.g = (unsigned char)(ship_color.g * pulse);
        ship_color.b = (unsigned char)(ship_color.b * pulse);
    }
    
    // Draw ship body
    DrawRectangleRec(player->rect, ship_color);
    
    // Draw ship details
    DrawRectangle(
        (int)(player->rect.x + 5),
        (int)(player->rect.y + 5),
        (int)(player->rect.width - 10),
        (int)(player->rect.height - 10),
        WHITE
    );
    
    // Draw engine glow
    DrawRectangle(
        (int)(player->rect.x + player->rect.width / 2 - 3),
        (int)(player->rect.y + player->rect.height),
        6,
        8,
        (Color){100, 200, 255, 150}
    );
    
    // Draw captured ship if dual fighter
    if (player->has_captured_ship) {
        Vector2 ship_pos = player->captured_ship_offset;
        
        // Draw captured ship shadow
        DrawRectangle(
            (int)(ship_pos.x + 2),
            (int)(ship_pos.y + 2),
            PLAYER_SIZE,
            PLAYER_SIZE,
            (Color){0, 0, 0, 100}
        );
        
        // Draw captured ship
        DrawRectangle(
            (int)ship_pos.x,
            (int)ship_pos.y,
            PLAYER_SIZE,
            PLAYER_SIZE,
            SKYBLUE
        );
        
        // Draw captured ship details
        DrawRectangle(
            (int)(ship_pos.x + 5),
            (int)(ship_pos.y + 5),
            PLAYER_SIZE - 10,
            PLAYER_SIZE - 10,
            WHITE
        );
        
        // Draw captured ship engine glow
        DrawRectangle(
            (int)(ship_pos.x + PLAYER_SIZE / 2 - 3),
            (int)(ship_pos.y + PLAYER_SIZE),
            6,
            8,
            (Color){100, 255, 200, 150}
        );
        
        // Draw connection beam between ships
        Vector2 main_center = {
            player->rect.x + player->rect.width / 2,
            player->rect.y + player->rect.height / 2
        };
        Vector2 captured_center = {
            ship_pos.x + PLAYER_SIZE / 2,
            ship_pos.y + PLAYER_SIZE / 2
        };
        
        Color beam_color = {100, 255, 200, (unsigned char)(100 + 50 * sinf(render_time * 5.0f))};
        DrawLineEx(main_center, captured_center, 2.0f, beam_color);
    }
}

// Draw enhanced enemies with visual effects
void DrawEnemies(const GameState* gameState) {
    float pulse_factor = 0.5f + 0.5f * sinf(render_time * 3.0f);
    
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (gameState->enemies[i].active) {
            DrawEnhancedEnemy(&gameState->enemies[i], pulse_factor);
        }
    }
}

// Draw enhanced bullets with trails (replaced with advanced bullets)
void DrawBullets(const GameState* gameState) {
    float trail_alpha = 0.8f + 0.2f * sinf(render_time * 5.0f);
    
    // Draw player bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (gameState->bullets[i].active) {
            DrawEnhancedBullet(gameState->bullets[i].position, YELLOW, true, trail_alpha);
        }
    }
    
    // Draw enemy bullets
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (gameState->enemy_bullets[i].active) {
            DrawEnhancedBullet(gameState->enemy_bullets[i].position, RED, false, trail_alpha);
        }
    }
}

// Draw enhanced UI with visual effects
void DrawUI(const GameState* gameState) {
    // Draw UI background panels
    DrawRectangle(5, 5, 200, 110, (Color){0, 0, 50, 100});
    DrawRectangleLines(5, 5, 200, 110, (Color){100, 150, 255, 150});
    
    // Draw main UI with enhanced styling
    Color ui_color = WHITE;
    if (gameState->player.lives <= 1) {
        ui_color = (Color){255, (unsigned char)(100 + 155 * sinf(render_time * 8.0f)), 100, 255};
    }
    
    DrawText(TextFormat("Score: %d", gameState->score), 15, 15, 20, YELLOW);
    DrawText(TextFormat("High Score: %d", gameState->high_score), 15, 40, 20, WHITE);
    DrawText(TextFormat("Lives: %d", gameState->player.lives), 15, 65, 20, ui_color);
    DrawText(TextFormat("Wave: %d", gameState->wave_number), 15, 90, 20, WHITE);
    
    // Draw lives indicator with ship icons
    for (int i = 0; i < gameState->player.lives && i < 5; i++) {
        int life_x = 150 + i * 15;
        int life_y = 70;
        DrawRectangle(life_x, life_y, 8, 8, BLUE);
        DrawRectangle(life_x + 1, life_y + 1, 6, 6, WHITE);
    }
    
    // Draw power-up indicators
    if (gameState->player.has_captured_ship) {
        DrawRectangle(SCREEN_WIDTH - 120, 10, 110, 30, (Color){0, 100, 0, 100});
        DrawText("DUAL FIGHTER", SCREEN_WIDTH - 115, 18, 12, GREEN);
        
        // Pulsing effect
        Color glow_color = {0, 255, 0, (unsigned char)(100 + 100 * sinf(render_time * 4.0f))};
        DrawRectangleLines(SCREEN_WIDTH - 120, 10, 110, 30, glow_color);
    }
    
    // Draw controls and debug information in the bottom right
    int controls_x = SCREEN_WIDTH - 200;
    int controls_y = SCREEN_HEIGHT - 120;
    
    // Controls background
    DrawRectangle(controls_x - 5, controls_y - 5, 195, 110, (Color){0, 0, 50, 80});
    DrawRectangleLines(controls_x - 5, controls_y - 5, 195, 110, (Color){100, 150, 255, 100});
    
    DrawText("Controls:", controls_x, controls_y, 18, LIGHTGRAY);
    DrawText("WASD/Arrows: Move", controls_x, controls_y + 20, 16, GRAY);
    DrawText("Space/Z: Shoot", controls_x, controls_y + 40, 16, GRAY);
    DrawText("P/ESC: Pause", controls_x, controls_y + 60, 16, GRAY);
    
    // Draw seed information only if FPS is enabled (debug mode)
    if (gameState->menu.show_fps) {
        DrawText(TextFormat("Seed: %u", gameState->random_seed), controls_x, controls_y + 80, 16, YELLOW);
    }
    
    // Draw enhanced score popups with effects
    for (int i = 0; i < 10; i++) {
        if (gameState->score_popups[i].active) {
            char score_text[16];
            sprintf(score_text, "%d", gameState->score_popups[i].score);
            
            // Add glow effect to score popups
            Vector2 pos = gameState->score_popups[i].position;
            float alpha = gameState->score_popups[i].timer / 2.0f;
            
            Color glow_color = {255, 255, 0, (unsigned char)(alpha * 100)};
            DrawText(score_text, (int)pos.x + 1, (int)pos.y + 1, 20, glow_color);
            DrawText(score_text, (int)pos.x, (int)pos.y, 20, WHITE);
        }
    }
    
    // Draw bonus stage indicator with effects
    if (gameState->is_bonus_stage) {
        float bonus_pulse = 0.8f + 0.2f * sinf(render_time * 6.0f);
        Color bonus_color = {255, (unsigned char)(215 * bonus_pulse), 0, 255};
        
        // Draw bonus stage background
        DrawRectangle(SCREEN_WIDTH / 2 - 100, 45, 200, 30, (Color){100, 50, 0, 150});
        DrawText("BONUS STAGE", SCREEN_WIDTH / 2 - 80, 50, 20, bonus_color);
        
        // Draw bonus stage progress
        if (gameState->bonus_stage_total_enemies > 0) {
            float progress = (float)gameState->bonus_stage_enemies_hit / gameState->bonus_stage_total_enemies;
            DrawRectangle(SCREEN_WIDTH / 2 - 100, 80, (int)(200 * progress), 5, GOLD);
            DrawRectangleLines(SCREEN_WIDTH / 2 - 100, 80, 200, 5, WHITE);
        }
    }
    
    // Draw wave transition effect
    static float wave_announce_timer = 0.0f;
    if (gameState->wave_timer < 2.0f) {
        wave_announce_timer = 2.0f;
    }
    
    if (wave_announce_timer > 0.0f) {
        wave_announce_timer -= 0.016f;
        float alpha = wave_announce_timer / 2.0f;
        
        Color wave_color = {255, 255, 255, (unsigned char)(alpha * 255)};
        char wave_text[32];
        sprintf(wave_text, "WAVE %d", gameState->wave_number);
        
        DrawText(wave_text, SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 40, 40, wave_color);
    }
}

// --- MAIN GAME RENDERING WITH SHADER SYSTEM ---

// Draw the main game playing state, wrapped with shader system and advanced bullets
void DrawGamePlaying(const GameState* gameState) {
    if (!gameState) return;

    // Begin shader post-processing
    BeginGameShaderMode((ShaderSystem*)&gameState->shaders);

    // Apply screen shake offset
    if (gameState->effects.screen_shake_duration > 0.0f) {
        // Apply screen shake by offsetting the camera
        BeginMode2D((Camera2D){
            .offset = {SCREEN_WIDTH/2.0f, SCREEN_HEIGHT/2.0f},
            .target = {SCREEN_WIDTH/2.0f + gameState->effects.screen_offset.x,
                      SCREEN_HEIGHT/2.0f + gameState->effects.screen_offset.y},
            .rotation = 0.0f,
            .zoom = 1.0f
        });
    }

    // Draw background
    DrawBackground(gameState);

    // Draw game objects
    DrawPlayer(gameState);
    // --- Use advanced bullets instead of old bullets ---
    DrawAdvancedBullets(gameState);
    DrawEnemies(gameState);
    DrawPowerUps(&gameState->powerups);

    // Draw particle effects
    DrawParticleSystem(&gameState->effects);

    // End screen shake effect
    if (gameState->effects.screen_shake_duration > 0.0f) {
        EndMode2D();
    }

    // End shader post-processing and apply effects
    EndGameShaderMode((ShaderSystem*)&gameState->shaders);

    // Draw UI (unaffected by shaders)
    DrawUI(gameState);

    // Draw Quality of Life UI elements
    DrawQoLUI(&gameState->qol, gameState);

    // Draw bonus stage indicator
    if (gameState->is_bonus_stage) {
        DrawText("BONUS STAGE", SCREEN_WIDTH / 2 - 80, 50, 20, GOLD);
    }

    // Draw pause overlay
    if (gameState->is_paused) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0, 0, 0, 128});
        DrawText("PAUSED", SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 20, 40, WHITE);
        DrawText("Press P or ESC to resume", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 30, 20, WHITE);
    }

    // Draw FPS if enabled
    if (gameState->menu.show_fps) {
        DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, GREEN);
        DrawText(TextFormat("Difficulty: %.1f", gameState->balance.difficulty_multiplier), 10, 35, 16, YELLOW);
        DrawText(TextFormat("Combo: x%.1f (%d hits)", gameState->balance.combo_multiplier, gameState->balance.consecutive_hits), 10, 55, 16, ORANGE);
        DrawText(TextFormat("Skill: %.2f", gameState->balance.player_skill_rating), 10, 75, 16, PURPLE);
        // Draw shader debug UI
        DrawShaderDebugUI(&gameState->shaders);
    }
}