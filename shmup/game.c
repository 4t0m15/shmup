#include "game.h"
#include <math.h>
#include <stdlib.h>

// Helper function to check collision between circle and rectangle
static bool CheckCollisionCircleRec(Vector2 center, float radius, Rectangle rec) {
    float dx = center.x - (rec.x + rec.width / 2.0f);
    float dy = center.y - (rec.y + rec.height / 2.0f);
    
    float closest_x = center.x;
    float closest_y = center.y;
    
    if (center.x < rec.x) closest_x = rec.x;
    else if (center.x > rec.x + rec.width) closest_x = rec.x + rec.width;
    
    if (center.y < rec.y) closest_y = rec.y;
    else if (center.y > rec.y + rec.height) closest_y = rec.y + rec.height;
    
    dx = center.x - closest_x;
    dy = center.y - closest_y;
    
    return (dx * dx + dy * dy) <= (radius * radius);
}

// Function to check player-enemy collisions
static bool CheckPlayerEnemyCollisions(GameState* gameState) {
    Rectangle player_rect = gameState->player.rect;
    
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!gameState->enemies[i].active) continue;
        
        Rectangle enemy_rect = {
            gameState->enemies[i].position.x - ENEMY_SIZE / 2.0f,
            gameState->enemies[i].position.y - ENEMY_SIZE / 2.0f,
            ENEMY_SIZE,
            ENEMY_SIZE
        };
        
        if (CheckCollisionRecs(player_rect, enemy_rect)) {
            return true; // Collision detected
        }
    }
    
    return false;
}

// Function to update enemy behavior
static void UpdateEnemies(GameState* gameState, float delta) {
    static float formation_sway_timer = 0.0f;
    formation_sway_timer += delta;
    
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy* enemy = &gameState->enemies[i];
        
        if (!enemy->active) continue;
        
        enemy->timer -= delta;
        
        switch (enemy->state) {
            case INACTIVE:
                // Do nothing
                break;
                
            case ENTERING:
                if (enemy->timer <= 0.0f) {
                    // Move towards formation position
                    float move_speed = ENEMY_FORMATION_SPEED * delta;
                    float dx = enemy->formation_pos.x - enemy->position.x;
                    float dy = enemy->formation_pos.y - enemy->position.y;
                    float distance = sqrtf(dx * dx + dy * dy);
                    
                    if (distance < 5.0f) {
                        // Reached formation position
                        enemy->position = enemy->formation_pos;
                        enemy->state = FORMATION;
                        enemy->timer = 2.0f + (i % 5) * 0.5f; // Time before first potential attack
                    } else {
                        // Move towards formation
                        enemy->position.x += (dx / distance) * move_speed;
                        enemy->position.y += (dy / distance) * move_speed;
                    }
                }
                break;
                
            case FORMATION:
                // Sway in formation with a gentle sine wave
                enemy->position.x = enemy->formation_pos.x + sinf(formation_sway_timer * 1.5f + i * 0.3f) * ENEMY_SWAY_AMPLITUDE;
                enemy->position.y = enemy->formation_pos.y + cosf(formation_sway_timer * 0.8f + i * 0.2f) * 8.0f;
                
                // Randomly decide to attack
                if (enemy->timer <= 0.0f && (rand() % 100) < ENEMY_ATTACK_CHANCE) {
                    enemy->state = ATTACKING;
                    enemy->timer = 0.0f;
                }
                break;
                
            case ATTACKING:
                // Dive towards player with curved path (Galaga-style)
                enemy->timer += delta;
                
                float attack_speed = ENEMY_ATTACK_SPEED * delta;
                float curve_factor = sinf(enemy->timer * 3.0f) * 30.0f;
                
                // Move towards player with some curve
                float target_x = gameState->player.rect.x + gameState->player.rect.width / 2.0f;
                float target_y = gameState->player.rect.y + gameState->player.rect.height / 2.0f;
                
                float dx = target_x - enemy->position.x;
                float dy = target_y - enemy->position.y;
                float distance = sqrtf(dx * dx + dy * dy);
                
                if (distance > 0) {
                    enemy->position.x += (dx / distance) * attack_speed + curve_factor * delta;
                    enemy->position.y += (dy / distance) * attack_speed;
                }
                
                // Check if enemy has moved past the player or off screen
                if (enemy->position.y > SCREEN_HEIGHT + 50 || 
                    enemy->position.x < -50 || enemy->position.x > SCREEN_WIDTH + 50) {
                    // Return to formation
                    enemy->state = ENTERING;
                    enemy->position.x = enemy->formation_pos.x;
                    enemy->position.y = -50.0f;
                    enemy->timer = 0.5f; // Delay before moving
                }
                break;
        }
    }
}

// Function to check bullet-enemy collisions
static void CheckBulletEnemyCollisions(GameState* gameState) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!gameState->bullets[i].active) continue;
        
        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (!gameState->enemies[j].active) continue;
            
            Rectangle enemy_rect = {
                gameState->enemies[j].position.x - ENEMY_SIZE / 2.0f,
                gameState->enemies[j].position.y - ENEMY_SIZE / 2.0f,
                ENEMY_SIZE,
                ENEMY_SIZE
            };
            
            if (CheckCollisionCircleRec(gameState->bullets[i].position, BULLET_SIZE, enemy_rect)) {
                // Hit! Deactivate both bullet and enemy
                gameState->bullets[i].active = false;
                gameState->enemies[j].active = false;
                break;
            }
        }
    }
}

// Function to spawn a new wave of enemies
static void SpawnEnemyWave(GameState* gameState) {
    // Check if any enemies are still active
    bool any_active = false;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (gameState->enemies[i].active) {
            any_active = true;
            break;
        }
    }
    
    // If no enemies are active, spawn a new wave
    if (!any_active) {
        int enemies_per_row = MAX_ENEMIES / 2;
        float spacing = 60.0f;
        float formation_width = (enemies_per_row - 1) * spacing;
        float start_x = (SCREEN_WIDTH - formation_width) / 2.0f;

        for (int i = 0; i < MAX_ENEMIES; i++) {
            int row = i / enemies_per_row;
            int col = i % enemies_per_row;

            gameState->enemies[i].formation_pos.x = start_x + col * spacing;
            gameState->enemies[i].formation_pos.y = 100.0f + row * spacing;
            
            // Start off-screen at the top
            gameState->enemies[i].position.x = gameState->enemies[i].formation_pos.x;
            gameState->enemies[i].position.y = -50.0f - row * spacing;

            gameState->enemies[i].state = ENTERING;
            gameState->enemies[i].timer = i * 0.15f; // Slightly faster entry for subsequent waves
            gameState->enemies[i].active = true;
        }
    }
}

void InitGame(GameState* gameState) {
    // Initialize player in the center of the screen
    gameState->player.rect.x = SCREEN_WIDTH / 2.0f - PLAYER_SIZE / 2.0f;
    gameState->player.rect.y = SCREEN_HEIGHT - 80.0f; // Move player closer to bottom
    gameState->player.rect.width = PLAYER_SIZE;
    gameState->player.rect.height = PLAYER_SIZE;
    gameState->player.color = BLUE;
    
    // Initialize background scroll position
    gameState->backgroundScrollY = 0.0f;
    
    // Initialize bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        gameState->bullets[i].active = false;
    }
    
    // Initialize shoot cooldown
    gameState->shootCooldown = 0.0f;

    // Initialize enemies in a grid formation
    int enemies_per_row = MAX_ENEMIES / 2;
    float spacing = 60.0f;
    float formation_width = (enemies_per_row - 1) * spacing;
    float start_x = (SCREEN_WIDTH - formation_width) / 2.0f;

    for (int i = 0; i < MAX_ENEMIES; i++) {
        int row = i / enemies_per_row;
        int col = i % enemies_per_row;

        gameState->enemies[i].formation_pos.x = start_x + col * spacing;
        gameState->enemies[i].formation_pos.y = 100.0f + row * spacing;
        
        // Start off-screen at the top
        gameState->enemies[i].position.x = gameState->enemies[i].formation_pos.x;
        gameState->enemies[i].position.y = -50.0f - row * spacing;

        gameState->enemies[i].state = ENTERING;
        gameState->enemies[i].timer = i * 0.2f; // Stagger entry
        gameState->enemies[i].active = true;
    }
}

void UpdateGame(GameState* gameState, float delta, const Assets* assets) {
    // Move player with arrow keys (frame-rate independent)
    float moveAmount = PLAYER_SPEED * 200.0f * delta; // 200 is a tuning factor for speed
    if (IsKeyDown(KEY_LEFT)) gameState->player.rect.x -= moveAmount;
    if (IsKeyDown(KEY_RIGHT)) gameState->player.rect.x += moveAmount;
    if (IsKeyDown(KEY_UP)) gameState->player.rect.y -= moveAmount;
    if (IsKeyDown(KEY_DOWN)) gameState->player.rect.y += moveAmount;

    // Prevent player from leaving the screen
    if (gameState->player.rect.x < 0) gameState->player.rect.x = 0;
    if (gameState->player.rect.x + gameState->player.rect.width > SCREEN_WIDTH) 
        gameState->player.rect.x = SCREEN_WIDTH - gameState->player.rect.width;
    if (gameState->player.rect.y < 0) gameState->player.rect.y = 0;
    if (gameState->player.rect.y + gameState->player.rect.height > SCREEN_HEIGHT) 
        gameState->player.rect.y = SCREEN_HEIGHT - gameState->player.rect.height;
    
    // Update shoot cooldown
    if (gameState->shootCooldown > 0.0f) {
        gameState->shootCooldown -= delta;
    }
    
    // Handle shooting
    if (IsKeyDown(KEY_SPACE) && gameState->shootCooldown <= 0.0f) {
        // Find an inactive bullet to use
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!gameState->bullets[i].active) {
                gameState->bullets[i].active = true;
                gameState->bullets[i].position.x = gameState->player.rect.x + gameState->player.rect.width / 2.0f;
                gameState->bullets[i].position.y = gameState->player.rect.y;
                gameState->shootCooldown = 0.2f; // 200ms cooldown
                
                // Play shoot sound inline
                if (assets != NULL && assets->shootSound.frameCount > 0) {
                    PlaySound(assets->shootSound);
                }
                break;
            }
        }
    }
    
    // Update bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (gameState->bullets[i].active) {
            gameState->bullets[i].position.y -= BULLET_SPEED * delta;
            
            // Deactivate bullet if it goes off screen
            if (gameState->bullets[i].position.y < -BULLET_SIZE) {
                gameState->bullets[i].active = false;
            }
        }
    }
    
    // Update enemies
    UpdateEnemies(gameState, delta);
    
    // Check collisions
    CheckBulletEnemyCollisions(gameState);
    
    // Check player-enemy collisions (for now just detect, could reset game later)
    bool player_hit = CheckPlayerEnemyCollisions(gameState);
    if (player_hit) {
        // For now, just change player color to indicate hit
        // In a full game, you might reset the level or reduce lives
        gameState->player.color = (Color){255, 100, 100, 255}; // Light red when hit
    } else {
        gameState->player.color = BLUE; // Normal color
    }
    
    // Spawn new wave if needed
    SpawnEnemyWave(gameState);
    
    // Update background scrolling
    gameState->backgroundScrollY += BACKGROUND_SCROLL_SPEED * delta;
    
    // Reset background position when it scrolls off screen
    // This assumes the background texture height is used for seamless scrolling
    if (gameState->backgroundScrollY >= SCREEN_HEIGHT) {
        gameState->backgroundScrollY = 0.0f;
    }
}

void DrawGame(const GameState* gameState, const Assets* assets) {
    ClearBackground(RAYWHITE);
    
    // Draw scrolling background
    // Draw the background texture twice to create seamless scrolling
    DrawTextureEx(assets->backgroundTexture, 
                  (Vector2){0, gameState->backgroundScrollY - SCREEN_HEIGHT}, 
                  0.0f, 1.0f, WHITE);
    DrawTextureEx(assets->backgroundTexture, 
                  (Vector2){0, gameState->backgroundScrollY}, 
                  0.0f, 1.0f, WHITE);
    
    // Draw enemies
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (gameState->enemies[i].active) {
            Color enemy_color = RED;
            
            // Different colors based on state
            switch (gameState->enemies[i].state) {
                case ENTERING: enemy_color = ORANGE; break;
                case FORMATION: enemy_color = RED; break;
                case ATTACKING: enemy_color = (Color){139, 0, 0, 255}; break; // Dark red
                default: enemy_color = RED; break;
            }
            
            if (assets->enemyTexture.id != 0) {
                // Draw using enemy texture
                Vector2 position = {
                    gameState->enemies[i].position.x - (float)assets->enemyTexture.width / 2.0f,
                    gameState->enemies[i].position.y - (float)assets->enemyTexture.height / 2.0f
                };
                
                // Scale texture to fit enemy size
                float scale = (float)ENEMY_SIZE / (float)assets->enemyTexture.width;
                DrawTextureEx(assets->enemyTexture, position, 0.0f, scale, enemy_color);
            } else {
                // Fallback to rectangle
                DrawRectangle(
                    (int)(gameState->enemies[i].position.x - ENEMY_SIZE / 2.0f),
                    (int)(gameState->enemies[i].position.y - ENEMY_SIZE / 2.0f),
                    ENEMY_SIZE,
                    ENEMY_SIZE,
                    enemy_color
                );
            }
        }
    }
    
    // Draw bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (gameState->bullets[i].active) {
            DrawCircle((int)gameState->bullets[i].position.x, 
                      (int)gameState->bullets[i].position.y, 
                      BULLET_SIZE, YELLOW);
        }
    }
    
    // Draw player
    DrawRectangleRec(gameState->player.rect, gameState->player.color);
    
    // Draw UI
    DrawText("Use arrow keys to move. Press SPACE to shoot.", 10, 10, 20, DARKGRAY);
    
    // Count active enemies for display
    int active_enemies = 0;
    int entering_enemies = 0;
    int formation_enemies = 0;
    int attacking_enemies = 0;
    
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (gameState->enemies[i].active) {
            active_enemies++;
            switch (gameState->enemies[i].state) {
                case ENTERING: entering_enemies++; break;
                case FORMATION: formation_enemies++; break;
                case ATTACKING: attacking_enemies++; break;
                default: break;
            }
        }
    }
    
    DrawText(TextFormat("Enemies: %d (E:%d F:%d A:%d)", active_enemies, entering_enemies, formation_enemies, attacking_enemies), 10, 35, 20, DARKGRAY);
    
    // Draw FPS counter in the top-right corner
    DrawFPS(SCREEN_WIDTH - 100, 10);
}
