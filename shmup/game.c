// Disable deprecation warnings for MSVC
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "game.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Mathematical constants
#ifndef PI
#define PI 3.14159265359f
#endif
#define TWO_PI 6.28318530718f

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

// Enhanced movement pattern calculations
static Vector2 CalculateMovementPattern(Enemy* enemy, float delta) {
    Vector2 new_pos = enemy->position;
    
    switch (enemy->pattern) {
        case PATTERN_STRAIGHT: {
            // Simple straight line movement
            Vector2 target = (enemy->state == ENTERING) ? enemy->formation_pos : enemy->attack_start;
            float dx = target.x - enemy->position.x;
            float dy = target.y - enemy->position.y;
            float distance = sqrtf(dx * dx + dy * dy);
            
            if (distance > 0) {
                float speed = (enemy->state == ENTERING) ? ENEMY_FORMATION_SPEED : ENEMY_ATTACK_SPEED;
                new_pos.x += (dx / distance) * speed * delta;
                new_pos.y += (dy / distance) * speed * delta;
            }
            break;
        }
        
        case PATTERN_ARC: {
            // Arc movement - curved path
            enemy->pattern_progress += delta * 0.5f;
            if (enemy->pattern_progress > 1.0f) enemy->pattern_progress = 1.0f;
            
            Vector2 start = enemy->entry_start;
            Vector2 target = (enemy->state == ENTERING) ? enemy->formation_pos : enemy->attack_start;
            
            // Create arc with control point
            Vector2 control = {
                (start.x + target.x) / 2.0f + enemy->pattern_param * 100.0f,
                (start.y + target.y) / 2.0f - 50.0f
            };
            
            float t = enemy->pattern_progress;
            float inv_t = 1.0f - t;
            
            new_pos.x = inv_t * inv_t * start.x + 2 * inv_t * t * control.x + t * t * target.x;
            new_pos.y = inv_t * inv_t * start.y + 2 * inv_t * t * control.y + t * t * target.y;
            break;
        }
        
        case PATTERN_SPIRAL: {
            // Spiral movement
            enemy->pattern_progress += delta * 2.0f;
            float radius = 50.0f * (1.0f - enemy->pattern_progress * 0.5f);
            float angle = enemy->pattern_progress * TWO_PI * 2.0f;
            
            Vector2 center = (enemy->state == ENTERING) ? enemy->formation_pos : enemy->attack_start;
            new_pos.x = center.x + cosf(angle) * radius;
            new_pos.y = center.y + sinf(angle) * radius + enemy->pattern_progress * 100.0f;
            break;
        }
        
        case PATTERN_SWIRL: {
            // Mirrored swirl pattern
            enemy->pattern_progress += delta * 1.5f;
            float radius = 40.0f + sinf(enemy->pattern_progress * PI) * 20.0f;
            float angle = enemy->pattern_progress * TWO_PI + enemy->pattern_param * PI;
            
            Vector2 center = (enemy->state == ENTERING) ? enemy->formation_pos : enemy->attack_start;
            new_pos.x = center.x + cosf(angle) * radius * enemy->pattern_param;
            new_pos.y = center.y + sinf(angle) * radius * 0.5f + enemy->pattern_progress * 80.0f;
            break;
        }
        
        case PATTERN_LOOP: {
            // Loop pattern for diving attacks
            enemy->pattern_progress += delta * 1.0f;
            if (enemy->pattern_progress > 1.0f) enemy->pattern_progress = 1.0f;
            
            float loop_radius = 60.0f;
            float angle = enemy->pattern_progress * TWO_PI;
            
            Vector2 loop_center = {
                enemy->attack_start.x + (enemy->position.x - enemy->attack_start.x) * 0.5f,
                enemy->attack_start.y + 100.0f
            };
            
            new_pos.x = loop_center.x + cosf(angle) * loop_radius;
            new_pos.y = loop_center.y + sinf(angle) * loop_radius + enemy->pattern_progress * 150.0f;
            break;
        }
        
        case PATTERN_BEAM: {
            // Straight beam attack for boss
            enemy->pattern_progress += delta * 0.8f;
            Vector2 target = {enemy->attack_start.x, SCREEN_HEIGHT + 100};
            
            float t = enemy->pattern_progress;
            new_pos.x = enemy->attack_start.x * (1.0f - t) + target.x * t;
            new_pos.y = enemy->attack_start.y * (1.0f - t) + target.y * t;
            break;
        }
        
        case PATTERN_CURVE: {
            // Curved attack pattern
            enemy->pattern_progress += delta * 1.2f;
            float curve_strength = sinf(enemy->pattern_progress * PI * 2.0f) * 80.0f;
            
            Vector2 target = {enemy->attack_start.x, SCREEN_HEIGHT + 50};
            float t = enemy->pattern_progress;
            
            new_pos.x = enemy->attack_start.x * (1.0f - t) + target.x * t + curve_strength * enemy->pattern_param;
            new_pos.y = enemy->attack_start.y * (1.0f - t) + target.y * t;
            break;
        }
    }
    
    return new_pos;
}

// Function to update tractor beam mechanics
static void UpdateTractorBeam(GameState* gameState, Enemy* boss, float delta) {
    if (!boss->tractor_active) return;
    
    // Calculate tractor beam center
    boss->tractor_center = boss->position;
    boss->tractor_angle += delta * 2.0f;
    
    // Check if player is in tractor beam range
    Vector2 player_center = {
        gameState->player.rect.x + gameState->player.rect.width / 2.0f,
        gameState->player.rect.y + gameState->player.rect.height / 2.0f
    };
    
    float dist_to_player = sqrtf(
        powf(player_center.x - boss->tractor_center.x, 2) +
        powf(player_center.y - boss->tractor_center.y, 2)
    );
    
    if (dist_to_player <= TRACTOR_BEAM_RANGE) {
        // Player is caught in tractor beam
        gameState->player.captured = true;
        
        // Pull player towards beam center
        Vector2 pull_direction = {
            boss->tractor_center.x - player_center.x,
            boss->tractor_center.y - player_center.y
        };
        
        float pull_strength = TRACTOR_BEAM_STRENGTH * delta;
        if (dist_to_player > 0) {
            pull_direction.x = (pull_direction.x / dist_to_player) * pull_strength;
            pull_direction.y = (pull_direction.y / dist_to_player) * pull_strength;
            
            gameState->player.rect.x += pull_direction.x;
            gameState->player.rect.y += pull_direction.y;
        }
        
        // Keep player within screen bounds even when captured
        if (gameState->player.rect.x < 0) gameState->player.rect.x = 0;
        if (gameState->player.rect.x + gameState->player.rect.width > SCREEN_WIDTH) 
            gameState->player.rect.x = SCREEN_WIDTH - gameState->player.rect.width;
        if (gameState->player.rect.y < 0) gameState->player.rect.y = 0;
        if (gameState->player.rect.y + gameState->player.rect.height > SCREEN_HEIGHT) 
            gameState->player.rect.y = SCREEN_HEIGHT - gameState->player.rect.height;
    } else {
        gameState->player.captured = false;
    }
}

// Function to update enemy shooting
static void UpdateEnemyShooting(GameState* gameState, Enemy* enemy, float delta) {
    if (!enemy->shooting) return;
    
    enemy->shoot_timer -= delta;
    
    if (enemy->shoot_timer <= 0.0f) {
        // Find an inactive enemy bullet
        for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
            if (!gameState->enemy_bullets[i].active) {
                gameState->enemy_bullets[i].active = true;
                gameState->enemy_bullets[i].position = enemy->position;
                
                // Aim towards player
                Vector2 player_center = {
                    gameState->player.rect.x + gameState->player.rect.width / 2.0f,
                    gameState->player.rect.y + gameState->player.rect.height / 2.0f
                };
                
                Vector2 direction = {
                    player_center.x - enemy->position.x,
                    player_center.y - enemy->position.y
                };
                
                float distance = sqrtf(direction.x * direction.x + direction.y * direction.y);
                if (distance > 0) {
                    gameState->enemy_bullets[i].velocity.x = (direction.x / distance) * ENEMY_BULLET_SPEED;
                    gameState->enemy_bullets[i].velocity.y = (direction.y / distance) * ENEMY_BULLET_SPEED;
                } else {
                    gameState->enemy_bullets[i].velocity.x = 0;
                    gameState->enemy_bullets[i].velocity.y = ENEMY_BULLET_SPEED;
                }
                
                // Set next shot timer
                enemy->shoot_timer = 1.0f + (rand() % 200) / 100.0f; // 1-3 seconds
                break;
            }
        }
    }
}

// Function to check player-enemy collisions
static bool CheckPlayerEnemyCollisions(GameState* gameState) {
    Rectangle player_rect = gameState->player.rect;
    
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!gameState->enemies[i].active) continue;
        
        float enemy_size = (gameState->enemies[i].type == BOSS) ? BOSS_SIZE : ENEMY_SIZE;
        Rectangle enemy_rect = {
            gameState->enemies[i].position.x - enemy_size / 2.0f,
            gameState->enemies[i].position.y - enemy_size / 2.0f,
            enemy_size,
            enemy_size
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
                
            case ENTERING: {
                // Use movement patterns for entering formation
                Vector2 new_pos = CalculateMovementPattern(enemy, delta);
                enemy->position = new_pos;
                
                // Check if reached formation position
                float dx = enemy->formation_pos.x - enemy->position.x;
                float dy = enemy->formation_pos.y - enemy->position.y;
                float distance = sqrtf(dx * dx + dy * dy);
                
                if (distance < 10.0f || enemy->pattern_progress >= 1.0f) {
                    enemy->position = enemy->formation_pos;
                    enemy->state = FORMATION;
                    enemy->timer = 2.0f + (i % 5) * 0.5f;
                    enemy->pattern_progress = 0.0f;
                    
                    // Some enemies can shoot from formation
                    if (enemy->type == BOSS || (rand() % 3 == 0)) {
                        enemy->shooting = true;
                        enemy->shoot_timer = 1.0f + (rand() % 200) / 100.0f;
                    }
                }
                break;
            }
                
            case FORMATION:
                // Sway in formation with a gentle sine wave
                enemy->position.x = enemy->formation_pos.x + sinf(formation_sway_timer * 1.5f + i * 0.3f) * ENEMY_SWAY_AMPLITUDE;
                enemy->position.y = enemy->formation_pos.y + cosf(formation_sway_timer * 0.8f + i * 0.2f) * 8.0f;
                
                // Update shooting
                UpdateEnemyShooting(gameState, enemy, delta);
                
                // Randomly decide to attack
                if (enemy->timer <= 0.0f) {
                    float attack_chance = (enemy->type == BOSS) ? 15 : ENEMY_ATTACK_CHANCE;
                    if ((rand() % 100) < attack_chance) {
                        enemy->state = (enemy->type == BOSS) ? SPECIAL_ATTACK : ATTACKING;
                        enemy->timer = 0.0f;
                        enemy->pattern_progress = 0.0f;
                        enemy->attack_start = enemy->position;
                        
                        // Assign attack pattern
                        if (enemy->type == BOSS) {
                            enemy->pattern = PATTERN_BEAM;
                            enemy->tractor_active = true;
                        } else {
                            MovementPattern patterns[] = {PATTERN_LOOP, PATTERN_CURVE, PATTERN_SPIRAL};
                            enemy->pattern = patterns[rand() % 3];
                        }
                        
                        // Set pattern parameter for variation
                        enemy->pattern_param = (rand() % 100) / 50.0f - 1.0f; // -1 to 1
                    } else {
                        enemy->timer = 1.0f + (rand() % 300) / 100.0f; // 1-4 seconds
                    }
                }
                break;
                
            case ATTACKING: {
                // Use movement patterns for attacking
                Vector2 new_pos = CalculateMovementPattern(enemy, delta);
                enemy->position = new_pos;
                
                // Update shooting during attack
                UpdateEnemyShooting(gameState, enemy, delta);
                
                // Check if attack is complete
                if (enemy->pattern_progress >= 1.0f || 
                    enemy->position.y > SCREEN_HEIGHT + 50 || 
                    enemy->position.x < -50 || enemy->position.x > SCREEN_WIDTH + 50) {
                    
                    // Return to formation
                    enemy->state = RETURNING;
                    enemy->pattern = PATTERN_STRAIGHT;
                    enemy->pattern_progress = 0.0f;
                    enemy->timer = 0.0f;
                    enemy->shooting = false;
                    
                    // Reset position for return
                    enemy->position.x = enemy->formation_pos.x;
                    enemy->position.y = -50.0f;
                    enemy->entry_start = enemy->position;
                }
                break;
            }
            
            case SPECIAL_ATTACK: {
                // Boss special attack with tractor beam
                UpdateTractorBeam(gameState, enemy, delta);
                
                // Move in beam pattern
                Vector2 new_pos = CalculateMovementPattern(enemy, delta);
                enemy->position = new_pos;
                
                // Update shooting during special attack
                UpdateEnemyShooting(gameState, enemy, delta);
                
                // End special attack after some time
                if (enemy->pattern_progress >= 1.0f) {
                    enemy->state = RETURNING;
                    enemy->pattern = PATTERN_STRAIGHT;
                    enemy->pattern_progress = 0.0f;
                    enemy->timer = 0.0f;
                    enemy->tractor_active = false;
                    enemy->shooting = false;
                    gameState->player.captured = false;
                    
                    // Reset position for return
                    enemy->position.x = enemy->formation_pos.x;
                    enemy->position.y = -50.0f;
                    enemy->entry_start = enemy->position;
                }
                break;
            }
            
            case RETURNING: {
                // Return to formation using straight movement
                Vector2 new_pos = CalculateMovementPattern(enemy, delta);
                enemy->position = new_pos;
                
                // Check if reached formation position
                float dx = enemy->formation_pos.x - enemy->position.x;
                float dy = enemy->formation_pos.y - enemy->position.y;
                float distance = sqrtf(dx * dx + dy * dy);
                
                if (distance < 10.0f) {
                    enemy->position = enemy->formation_pos;
                    enemy->state = FORMATION;
                    enemy->timer = 2.0f + (i % 5) * 0.5f;
                    enemy->pattern_progress = 0.0f;
                    
                    // Resume shooting
                    if (enemy->type == BOSS || (rand() % 3 == 0)) {
                        enemy->shooting = true;
                        enemy->shoot_timer = 1.0f + (rand() % 200) / 100.0f;
                    }
                }
                break;
            }
        }
    }
}

// Function to update enemy bullets
static void UpdateEnemyBullets(GameState* gameState, float delta) {
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!gameState->enemy_bullets[i].active) continue;
        
        gameState->enemy_bullets[i].position.x += gameState->enemy_bullets[i].velocity.x * delta;
        gameState->enemy_bullets[i].position.y += gameState->enemy_bullets[i].velocity.y * delta;
        
        // Deactivate bullet if it goes off screen
        if (gameState->enemy_bullets[i].position.y > SCREEN_HEIGHT + 50 ||
            gameState->enemy_bullets[i].position.x < -50 ||
            gameState->enemy_bullets[i].position.x > SCREEN_WIDTH + 50 ||
            gameState->enemy_bullets[i].position.y < -50) {
            gameState->enemy_bullets[i].active = false;
        }
        
        // Check collision with player
        if (CheckCollisionCircleRec(gameState->enemy_bullets[i].position, BULLET_SIZE, gameState->player.rect)) {
            gameState->enemy_bullets[i].active = false;
            
            // Only reduce lives if not captured (captured players are protected)
            if (!gameState->player.captured) {
                gameState->player.lives--;
                gameState->player.color = (Color){255, 100, 100, 255};
                
                // Check for game over
                if (gameState->player.lives <= 0) {
                    HandleGameOver(gameState);
                } else {
                    // Reset player position after hit
                    gameState->player.rect.x = SCREEN_WIDTH / 2.0f - PLAYER_SIZE / 2.0f;
                    gameState->player.rect.y = SCREEN_HEIGHT - 80.0f;
                }
            }
        }
    }
}

// Function to check bullet-enemy collisions
static void CheckBulletEnemyCollisions(GameState* gameState) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!gameState->bullets[i].active) continue;
        
        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (!gameState->enemies[j].active) continue;
            
            float enemy_size = (gameState->enemies[j].type == BOSS) ? BOSS_SIZE : ENEMY_SIZE;
            Rectangle enemy_rect = {
                gameState->enemies[j].position.x - enemy_size / 2.0f,
                gameState->enemies[j].position.y - enemy_size / 2.0f,
                enemy_size,
                enemy_size
            };
            
            if (CheckCollisionCircleRec(gameState->bullets[i].position, BULLET_SIZE, enemy_rect)) {
                // Hit! Deactivate bullet
                gameState->bullets[i].active = false;
                
                // Reduce enemy health
                gameState->enemies[j].health--;
                
                // Destroy enemy if health reaches 0
                if (gameState->enemies[j].health <= 0) {
                    HandleEnemyDestroy(gameState, j, gameState->enemies[j].position);
                }
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
        gameState->wave_number++;
        
        // Check if this should be a bonus stage (every 3rd wave)
        if (gameState->wave_number % 3 == 0 && gameState->wave_number > 0) {
            SpawnBonusStage(gameState);
            return;
        }
        
        // Determine if this is a boss wave
        bool is_boss_wave = (gameState->wave_number % gameState->boss_wave_interval == 0);
        
        if (is_boss_wave) {
            // Spawn boss with escorts
            int boss_index = MAX_ENEMIES / 2;
            int escort_group_id = gameState->wave_number; // Unique group ID
            
            gameState->enemies[boss_index].type = BOSS;
            gameState->enemies[boss_index].health = 5;
            gameState->enemies[boss_index].formation_pos.x = SCREEN_WIDTH / 2.0f;
            gameState->enemies[boss_index].formation_pos.y = 80.0f;
            gameState->enemies[boss_index].position.x = SCREEN_WIDTH / 2.0f;
            gameState->enemies[boss_index].position.y = -BOSS_SIZE;
            gameState->enemies[boss_index].entry_start = gameState->enemies[boss_index].position;
            gameState->enemies[boss_index].state = ENTERING;
            gameState->enemies[boss_index].pattern = PATTERN_SPIRAL;
            gameState->enemies[boss_index].pattern_progress = 0.0f;
            gameState->enemies[boss_index].pattern_param = 1.0f;
            gameState->enemies[boss_index].timer = 1.0f;
            gameState->enemies[boss_index].active = true;
            gameState->enemies[boss_index].shooting = false;
            gameState->enemies[boss_index].tractor_active = false;
            gameState->enemies[boss_index].is_escort_in_combo = false;
            gameState->enemies[boss_index].escort_group_id = escort_group_id;
            
            // Spawn escorts
            for (int i = 0; i < 6; i++) {
                if (i == boss_index) continue;
                
                gameState->enemies[i].type = ESCORT;
                gameState->enemies[i].health = 1;
                gameState->enemies[i].formation_pos.x = SCREEN_WIDTH / 2.0f + (i - 3) * 80.0f;
                gameState->enemies[i].formation_pos.y = 150.0f;
                gameState->enemies[i].position.x = gameState->enemies[i].formation_pos.x;
                gameState->enemies[i].position.y = -ENEMY_SIZE - i * 30;
                gameState->enemies[i].entry_start = gameState->enemies[i].position;
                gameState->enemies[i].state = ENTERING;
                gameState->enemies[i].is_escort_in_combo = true;
                gameState->enemies[i].escort_group_id = escort_group_id;
                
                // Vary entry patterns
                MovementPattern entry_patterns[] = {PATTERN_ARC, PATTERN_SWIRL, PATTERN_SPIRAL};
                gameState->enemies[i].pattern = entry_patterns[i % 3];
                gameState->enemies[i].pattern_progress = 0.0f;
                gameState->enemies[i].pattern_param = (i % 2 == 0) ? 1.0f : -1.0f;
                gameState->enemies[i].timer = i * 0.3f;
                gameState->enemies[i].active = true;
                gameState->enemies[i].shooting = false;
                gameState->enemies[i].tractor_active = false;
            }
        } else {
            // Spawn normal wave
            int enemies_per_row = MAX_ENEMIES / 2;
            float spacing = 60.0f;
            float formation_width = (enemies_per_row - 1) * spacing;
            float start_x = (SCREEN_WIDTH - formation_width) / 2.0f;

            for (int i = 0; i < MAX_ENEMIES; i++) {
                int row = i / enemies_per_row;
                int col = i % enemies_per_row;

                gameState->enemies[i].type = NORMAL;
                gameState->enemies[i].health = 1;
                gameState->enemies[i].formation_pos.x = start_x + col * spacing;
                gameState->enemies[i].formation_pos.y = 100.0f + row * spacing;
                gameState->enemies[i].position.x = gameState->enemies[i].formation_pos.x;
                gameState->enemies[i].position.y = -50.0f - row * spacing;
                gameState->enemies[i].entry_start = gameState->enemies[i].position;
                gameState->enemies[i].state = ENTERING;
                gameState->enemies[i].is_escort_in_combo = false;
                gameState->enemies[i].escort_group_id = 0;
                
                // Vary entry patterns for choreographed effect
                MovementPattern entry_patterns[] = {PATTERN_ARC, PATTERN_SWIRL, PATTERN_SPIRAL};
                gameState->enemies[i].pattern = entry_patterns[i % 3];
                gameState->enemies[i].pattern_progress = 0.0f;
                gameState->enemies[i].pattern_param = (i % 2 == 0) ? 1.0f : -1.0f;
                gameState->enemies[i].timer = i * 0.15f;
                gameState->enemies[i].active = true;
                gameState->enemies[i].shooting = false;
                gameState->enemies[i].tractor_active = false;
            }
        }
    }
}

void InitGame(GameState* gameState) {
    // Initialize player in the center of the screen
    gameState->player.rect.x = SCREEN_WIDTH / 2.0f - PLAYER_SIZE / 2.0f;
    gameState->player.rect.y = SCREEN_HEIGHT - 80.0f;
    gameState->player.rect.width = PLAYER_SIZE;
    gameState->player.rect.height = PLAYER_SIZE;
    gameState->player.color = BLUE;
    gameState->player.captured = false;
    gameState->player.dual_fire = false;
    gameState->player.lives = STARTING_LIVES;
    gameState->player.extend_1_awarded = false;
    gameState->player.extend_2_awarded = false;
    
    // Initialize game state variables
    gameState->wave_number = 0;
    gameState->wave_timer = 0.0f;
    gameState->boss_wave_interval = 4; // Boss every 4th wave
    gameState->backgroundScrollY = 0.0f;
    gameState->shootCooldown = 0.0f;
    
    // Initialize bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        gameState->bullets[i].active = false;
    }
    
    // Initialize enemy bullets
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        gameState->enemy_bullets[i].active = false;
    }
    
    // Initialize enemies
    for (int i = 0; i < MAX_ENEMIES; i++) {
        gameState->enemies[i].active = false;
        gameState->enemies[i].state = INACTIVE;
        gameState->enemies[i].health = 1;
        gameState->enemies[i].shooting = false;
        gameState->enemies[i].tractor_active = false;
        gameState->enemies[i].is_escort_in_combo = false;
        gameState->enemies[i].escort_group_id = 0;
        gameState->enemies[i].timer = 0.0f;
        gameState->enemies[i].pattern_progress = 0.0f;
        gameState->enemies[i].pattern_param = 0.0f;
        gameState->enemies[i].shoot_timer = 0.0f;
        gameState->enemies[i].tractor_angle = 0.0f;
    }
    
    // Initialize scoring system
    gameState->score = 0;
    gameState->high_score = 0;
    for (int i = 0; i < 10; i++) {
        gameState->score_popups[i].active = false;
    }
    
    // Initialize bonus stage tracking
    gameState->is_bonus_stage = false;
    gameState->bonus_stage_enemies_hit = 0;
    gameState->bonus_stage_total_enemies = 0;
    gameState->bonus_stage_timer = 0.0f;
    
    // Initialize combo tracking
    gameState->boss_escort_combo_active = false;
    gameState->boss_escort_combo_count = 0;
    gameState->combo_timer = 0.0f;
    
    // Initialize game state management
    gameState->screen_state = PLAYING;
    gameState->game_over_timer = 0.0f;
    
    // Load high score from file
    LoadHighScore(gameState);
}

void UpdateGame(GameState* gameState, float delta) {
    switch (gameState->screen_state) {
        case PLAYING:
            // Update score popups
            UpdateScorePopups(gameState, delta);
            
            // Update bonus stage if active
            if (gameState->is_bonus_stage) {
                UpdateBonusStage(gameState, delta);
                
                // In bonus stage, enemies don't shoot or collide with player
                // And they follow predictable patterns
                for (int i = 0; i < MAX_ENEMIES; i++) {
                    if (gameState->enemies[i].active) {
                        gameState->enemies[i].position.x += 50.0f * delta;
                        gameState->enemies[i].position.y += sinf(gameState->enemies[i].timer) * 30.0f * delta;
                        gameState->enemies[i].timer += delta;
                        
                        // Remove enemies that go off screen
                        if (gameState->enemies[i].position.x > SCREEN_WIDTH + 50) {
                            gameState->enemies[i].active = false;
                        }
                    }
                }
            }
            
            // Player movement (reduced if captured by tractor beam)
            float move_reduction = gameState->player.captured ? 0.3f : 1.0f;
            float moveAmount = PLAYER_SPEED * 200.0f * delta * move_reduction;
            
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
            
            // Handle shooting (dual fire if rescued)
            if (IsKeyDown(KEY_SPACE) && gameState->shootCooldown <= 0.0f) {
                // Find inactive bullets to use
                int bullets_fired = 0;
                int max_bullets = gameState->player.dual_fire ? 2 : 1;
                
                for (int i = 0; i < MAX_BULLETS && bullets_fired < max_bullets; i++) {
                    if (!gameState->bullets[i].active) {
                        gameState->bullets[i].active = true;
                        gameState->bullets[i].position.x = gameState->player.rect.x + gameState->player.rect.width / 2.0f;
                        gameState->bullets[i].position.y = gameState->player.rect.y;
                        
                        // Spread bullets if dual fire
                        if (gameState->player.dual_fire) {
                            gameState->bullets[i].position.x += (bullets_fired == 0) ? -10.0f : 10.0f;
                        }
                        
                        bullets_fired++;
                    }
                }
                
                if (bullets_fired > 0) {
                    gameState->shootCooldown = 0.15f; // Slightly faster firing for dual fire
                }
            }
            
            // Debug: Test game over screen (remove in final version)
            if (IsKeyPressed(KEY_ESCAPE)) {
                HandleGameOver(gameState);
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
            
            // Skip normal enemy updates during bonus stage
            if (!gameState->is_bonus_stage) {
                // Update enemies
                UpdateEnemies(gameState, delta);
                
                // Update enemy bullets
                UpdateEnemyBullets(gameState, delta);
                
                // Check player-enemy collisions
                bool player_hit = CheckPlayerEnemyCollisions(gameState);
                if (player_hit && !gameState->player.captured) {
                    // Player loses a life
                    gameState->player.lives--;
                    gameState->player.color = (Color){255, 100, 100, 255};
                    
                    // Reset player position
                    gameState->player.rect.x = SCREEN_WIDTH / 2.0f - PLAYER_SIZE / 2.0f;
                    gameState->player.rect.y = SCREEN_HEIGHT - 80.0f;
                    
                    // Game over check
                    if (gameState->player.lives <= 0) {
                        HandleGameOver(gameState);
                    }
                } else if (!gameState->player.captured) {
                    gameState->player.color = BLUE;
                }
                
                // Special color when captured
                if (gameState->player.captured) {
                    gameState->player.color = (Color){255, 255, 100, 255}; // Yellow when captured
                }
                
                // Spawn new wave if needed
                SpawnEnemyWave(gameState);
            }
            
            // Check collisions (works in both normal and bonus stages)
            CheckBulletEnemyCollisions(gameState);
            
            // Update background scrolling
            gameState->backgroundScrollY += BACKGROUND_SCROLL_SPEED * delta;
            if (gameState->backgroundScrollY >= SCREEN_HEIGHT) {
                gameState->backgroundScrollY = 0.0f;
            }
            break;
            
        case GAME_OVER:
            UpdateGameOver(gameState, delta);
            break;
    }
}

void DrawGame(const GameState* gameState) {
    switch (gameState->screen_state) {
        case PLAYING:
            ClearBackground(DARKBLUE);
            
            // Draw simple scrolling background pattern
            for (int y = -50; y < SCREEN_HEIGHT + 50; y += 50) {
                int offset_y = (int)(y + gameState->backgroundScrollY) % 50;
                DrawLine(0, offset_y, SCREEN_WIDTH, offset_y, (Color){0, 50, 100, 100});
            }
            
            // Draw tractor beams
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (gameState->enemies[i].active && gameState->enemies[i].tractor_active) {
                    Vector2 beam_center = gameState->enemies[i].tractor_center;
                    
                    // Draw rotating tractor beam effect
                    for (int j = 0; j < 8; j++) {
                        float angle = gameState->enemies[i].tractor_angle + j * (TWO_PI / 8.0f);
                        Vector2 beam_end = {
                            beam_center.x + cosf(angle) * TRACTOR_BEAM_RANGE,
                            beam_center.y + sinf(angle) * TRACTOR_BEAM_RANGE
                        };
                        
                        DrawLineEx(beam_center, beam_end, 2.0f, (Color){255, 255, 0, 60});
                    }
                    
                    // Draw beam circle
                    DrawCircleLines((int)beam_center.x, (int)beam_center.y, TRACTOR_BEAM_RANGE, (Color){255, 255, 0, 120});
                }
            }
            
            // Draw enemies
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (gameState->enemies[i].active) {
                    Color enemy_color = RED;
                    float enemy_size = ENEMY_SIZE;
                    
                    // Different colors and sizes based on type and state
                    switch (gameState->enemies[i].type) {
                        case NORMAL:
                            enemy_color = RED;
                            enemy_size = ENEMY_SIZE;
                            break;
                        case BOSS:
                            enemy_color = (Color){139, 0, 139, 255}; // Dark magenta
                            enemy_size = BOSS_SIZE;
                            break;
                        case ESCORT:
                            enemy_color = ORANGE;
                            enemy_size = ENEMY_SIZE;
                            break;
                    }
                    
                    // Modify color based on state
                    switch (gameState->enemies[i].state) {
                        case ENTERING: 
                            enemy_color = (Color){enemy_color.r, enemy_color.g, enemy_color.b, 180}; 
                            break;
                        case FORMATION: 
                            // Keep normal color
                            break;
                        case ATTACKING: 
                        case SPECIAL_ATTACK:
                            enemy_color = (Color){255, enemy_color.g / 2, enemy_color.b / 2, 255}; 
                            break;
                        case RETURNING:
                            enemy_color = (Color){enemy_color.r / 2, enemy_color.g, enemy_color.b, 200}; 
                            break;
                        default: 
                            break;
                    }
                    
                    // Draw enemy
                    DrawRectangle(
                        (int)(gameState->enemies[i].position.x - enemy_size / 2.0f),
                        (int)(gameState->enemies[i].position.y - enemy_size / 2.0f),
                        (int)enemy_size,
                        (int)enemy_size,
                        enemy_color
                    );
                    
                    // Draw health for boss
                    if (gameState->enemies[i].type == BOSS && gameState->enemies[i].health > 1) {
                        for (int h = 0; h < gameState->enemies[i].health; h++) {
                            DrawRectangle(
                                (int)(gameState->enemies[i].position.x - enemy_size / 2.0f + h * 8),
                                (int)(gameState->enemies[i].position.y - enemy_size / 2.0f - 10),
                                6, 4, GREEN
                            );
                        }
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
            
            // Draw enemy bullets (only in normal stages)
            if (!gameState->is_bonus_stage) {
                for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
                    if (gameState->enemy_bullets[i].active) {
                        DrawCircle((int)gameState->enemy_bullets[i].position.x, 
                                  (int)gameState->enemy_bullets[i].position.y, 
                                  BULLET_SIZE, RED);
                    }
                }
            }
            
            // Draw player
            DrawRectangleRec(gameState->player.rect, gameState->player.color);
            
            // Draw dual fire indicator
            if (gameState->player.dual_fire) {
                DrawCircle((int)(gameState->player.rect.x + gameState->player.rect.width / 2.0f - 15), 
                          (int)(gameState->player.rect.y + gameState->player.rect.height / 2.0f), 
                          3, YELLOW);
                DrawCircle((int)(gameState->player.rect.x + gameState->player.rect.width / 2.0f + 15), 
                          (int)(gameState->player.rect.y + gameState->player.rect.height / 2.0f), 
                          3, YELLOW);
            }
            
            // Draw score popups
            for (int i = 0; i < 10; i++) {
                if (gameState->score_popups[i].active) {
                    Color popup_color = YELLOW;
                    char score_text[32];
                    
                    if (gameState->score_popups[i].score == -1) {
                        const char* extra_life_text = "EXTRA LIFE";
                        int text_len = strlen(extra_life_text);
                        if (text_len < 31) {
                            for (int j = 0; j < text_len; j++) {
                                score_text[j] = extra_life_text[j];
                            }
                            score_text[text_len] = '\0';
                        } else {
                            score_text[0] = '\0';
                        }
                        popup_color = GREEN;
                    } else {
                        snprintf(score_text, sizeof(score_text), "%d", gameState->score_popups[i].score);
                    }
                    
                    // Fade out over time
                    float alpha = gameState->score_popups[i].timer / 2.0f;
                    popup_color.a = (unsigned char)(255 * alpha);
                    
                    DrawText(score_text, 
                            (int)gameState->score_popups[i].position.x, 
                            (int)gameState->score_popups[i].position.y, 
                            16, popup_color);
                }
            }
            
            // Draw UI
            DrawText("SCORE", 10, 10, 16, WHITE);
            DrawText(TextFormat("%08d", gameState->score), 10, 30, 16, YELLOW);
            
            DrawText("HIGH SCORE", 10, 55, 16, WHITE);
            DrawText(TextFormat("%08d", gameState->high_score), 10, 75, 16, YELLOW);
            
            DrawText("LIVES", 10, 100, 16, WHITE);
            for (int i = 0; i < gameState->player.lives; i++) {
                DrawRectangle(10 + i * 20, 120, 15, 15, BLUE);
            }
            
            DrawText(TextFormat("Wave: %d", gameState->wave_number), 10, 145, 16, LIGHTGRAY);
            
            // Special UI for bonus stage
            if (gameState->is_bonus_stage) {
                DrawText("BONUS STAGE", SCREEN_WIDTH / 2 - 80, 50, 20, GOLD);
                DrawText(TextFormat("Time: %.1f", gameState->bonus_stage_timer), SCREEN_WIDTH / 2 - 50, 75, 16, YELLOW);
                DrawText(TextFormat("Hits: %d/%d", gameState->bonus_stage_enemies_hit, gameState->bonus_stage_total_enemies), 
                        SCREEN_WIDTH / 2 - 50, 95, 16, YELLOW);
            }
            
            if (gameState->player.captured) {
                DrawText("CAPTURED BY TRACTOR BEAM!", SCREEN_WIDTH / 2 - 150, SCREEN_HEIGHT / 2, 20, YELLOW);
            }
            
            if (gameState->player.dual_fire) {
                DrawText("DUAL FIRE", SCREEN_WIDTH - 120, 120, 16, YELLOW);
            }
            
            // Count active enemies for display
            int active_enemies = 0;
            int boss_count = 0;
            
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (gameState->enemies[i].active) {
                    active_enemies++;
                    if (gameState->enemies[i].type == BOSS) boss_count++;
                }
            }
            
            DrawText(TextFormat("Enemies: %d %s", active_enemies, boss_count > 0 ? "(BOSS WAVE)" : ""), 
                     10, 170, 16, LIGHTGRAY);
            
            // Draw controls
            DrawText("Arrow keys: Move | SPACE: Shoot", 10, SCREEN_HEIGHT - 30, 14, LIGHTGRAY);
            
            // Draw FPS counter
            DrawFPS(SCREEN_WIDTH - 100, 10);
            break;
            
        case GAME_OVER:
            DrawGameOver(gameState);
            break;
    }
}

// Scoring functions implementation
void AddScore(GameState* gameState, int points, Vector2 position) {
    gameState->score += points;
    
    // Update high score if necessary
    if (gameState->score > gameState->high_score) {
        gameState->high_score = gameState->score;
        SaveHighScore(gameState);
    }
    
    // Create score popup
    for (int i = 0; i < 10; i++) {
        if (!gameState->score_popups[i].active) {
            gameState->score_popups[i].position = position;
            gameState->score_popups[i].score = points;
            gameState->score_popups[i].timer = 2.0f;
            gameState->score_popups[i].active = true;
            break;
        }
    }
    
    // Check for extends
    CheckForExtends(gameState);
}

void CheckForExtends(GameState* gameState) {
    // First extend at 20,000 points
    if (!gameState->player.extend_1_awarded && gameState->score >= FIRST_EXTEND_SCORE) {
        gameState->player.extend_1_awarded = true;
        if (gameState->player.lives < MAX_LIVES) {
            gameState->player.lives++;
            // Add popup for extra life
            Vector2 center = {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
            AddScore(gameState, 0, center); // This will just create a popup slot
            // Override the popup to show "EXTRA LIFE"
            for (int i = 0; i < 10; i++) {
                if (gameState->score_popups[i].active && gameState->score_popups[i].score == 0) {
                    gameState->score_popups[i].score = -1; // Special code for extra life
                    break;
                }
            }
        }
    }
    
    // Second extend at 70,000 points
    if (!gameState->player.extend_2_awarded && gameState->score >= SECOND_EXTEND_SCORE) {
        gameState->player.extend_2_awarded = true;
        if (gameState->player.lives < MAX_LIVES) {
            gameState->player.lives++;
            // Add popup for extra life
            Vector2 center = {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
            AddScore(gameState, 0, center); // This will just create a popup slot
            // Override the popup to show "EXTRA LIFE"
            for (int i = 0; i < 10; i++) {
                if (gameState->score_popups[i].active && gameState->score_popups[i].score == 0) {
                    gameState->score_popups[i].score = -1; // Special code for extra life
                    break;
                }
            }
        }
    }
}

int CalculateEnemyScore(const Enemy* enemy) {
    switch (enemy->type) {
        case NORMAL: // Bee (Zako)
            return (enemy->state == ATTACKING || enemy->state == SPECIAL_ATTACK) ? 
                   SCORE_BEE_DIVE : SCORE_BEE_FORMATION;
        case ESCORT: // Butterfly (Goei)
            return (enemy->state == ATTACKING || enemy->state == SPECIAL_ATTACK) ? 
                   SCORE_BUTTERFLY_DIVE : SCORE_BUTTERFLY_FORMATION;
        case BOSS: // Boss Galaga
            return (enemy->state == ATTACKING || enemy->state == SPECIAL_ATTACK) ? 
                   SCORE_BOSS_DIVE : SCORE_BOSS_FORMATION;
        default:
            return 50;
    }
}

void HandleEnemyDestroy(GameState* gameState, int enemy_index, Vector2 position) {
    Enemy* enemy = &gameState->enemies[enemy_index];
    int base_score = CalculateEnemyScore(enemy);
    
    // Check for boss + escort combo
    if (enemy->type == BOSS && enemy->is_escort_in_combo) {
        // Check if all escorts in the combo were destroyed
        bool all_escorts_destroyed = true;
        
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (gameState->enemies[i].active && 
                gameState->enemies[i].escort_group_id == enemy->escort_group_id &&
                gameState->enemies[i].type == ESCORT) {
                all_escorts_destroyed = false;
                break;
            }
        }
        
        if (all_escorts_destroyed) {
            // Award combo bonus
            AddScore(gameState, SCORE_BOSS_ESCORT_COMBO, position);
        } else {
            AddScore(gameState, base_score, position);
        }
    } else if (enemy->type == ESCORT && enemy->is_escort_in_combo) {
        // Check if this completes a boss+escort combo
        bool boss_still_alive = false;
        bool other_escorts_alive = false;
        
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (gameState->enemies[i].active && 
                gameState->enemies[i].escort_group_id == enemy->escort_group_id) {
                if (gameState->enemies[i].type == BOSS) {
                    boss_still_alive = true;
                } else if (gameState->enemies[i].type == ESCORT && i != enemy_index) {
                    other_escorts_alive = true;
                }
            }
        }
        
        if (!other_escorts_alive && boss_still_alive) {
            // This was the last escort - mark boss for combo potential
            for (int i = 0; i < MAX_ENEMIES; i++) {
                if (gameState->enemies[i].active && 
                    gameState->enemies[i].escort_group_id == enemy->escort_group_id &&
                    gameState->enemies[i].type == BOSS) {
                    gameState->enemies[i].is_escort_in_combo = true;
                    break;
                }
            }
        }
        
        AddScore(gameState, base_score, position);
    } else {
        // Normal scoring
        AddScore(gameState, base_score, position);
    }
    
    // Handle captured ship rescue
    if (enemy->type == BOSS && enemy->tractor_active && gameState->player.captured) {
        AddScore(gameState, SCORE_CAPTURED_SHIP_RESCUE, position);
        gameState->player.dual_fire = true;
        gameState->player.captured = false;
    }
    
    // Handle bonus stage scoring
    if (gameState->is_bonus_stage) {
        gameState->bonus_stage_enemies_hit++;
    }
    
    // Deactivate enemy
    enemy->active = false;
    enemy->tractor_active = false;
}

void UpdateScorePopups(GameState* gameState, float delta) {
    for (int i = 0; i < 10; i++) {
        if (gameState->score_popups[i].active) {
            gameState->score_popups[i].timer -= delta;
            gameState->score_popups[i].position.y -= 30.0f * delta; // Float upward
            
            if (gameState->score_popups[i].timer <= 0.0f) {
                gameState->score_popups[i].active = false;
            }
        }
    }
}

void SpawnBonusStage(GameState* gameState) {
    gameState->is_bonus_stage = true;
    gameState->bonus_stage_enemies_hit = 0;
    gameState->bonus_stage_total_enemies = 0;
    gameState->bonus_stage_timer = 10.0f; // 10 seconds for bonus stage
    
    // Spawn enemies in predictable patterns (no shooting, no collision)
    for (int i = 0; i < MAX_ENEMIES; i++) {
        gameState->enemies[i].active = true;
        gameState->enemies[i].type = NORMAL;
        gameState->enemies[i].health = 1;
        gameState->enemies[i].state = ATTACKING;
        gameState->enemies[i].pattern = PATTERN_ARC;
        gameState->enemies[i].pattern_progress = 0.0f;
        gameState->enemies[i].pattern_param = (i % 2 == 0) ? 1.0f : -1.0f;
        gameState->enemies[i].shooting = false;
        gameState->enemies[i].tractor_active = false;
        gameState->enemies[i].is_escort_in_combo = false;
        gameState->enemies[i].escort_group_id = 0;
        
        // Set starting positions for predictable movement
        gameState->enemies[i].position.x = -50.0f + (i * 30.0f);
        gameState->enemies[i].position.y = 50.0f + (i % 3) * 40.0f;
        gameState->enemies[i].attack_start = gameState->enemies[i].position;
        gameState->enemies[i].timer = i * 0.2f;
        
        gameState->bonus_stage_total_enemies++;
    }
}

void UpdateBonusStage(GameState* gameState, float delta) {
    if (!gameState->is_bonus_stage) return;
    
    gameState->bonus_stage_timer -= delta;
    
    // Check if bonus stage is complete
    bool all_enemies_destroyed = true;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (gameState->enemies[i].active) {
            all_enemies_destroyed = false;
            break;
        }
    }
    
    if (all_enemies_destroyed || gameState->bonus_stage_timer <= 0.0f) {
        // Calculate bonus score
        int bonus_score = 0;
        Vector2 center = {SCREEN_WIDTH / 2.0f, SCREEN_HEIGHT / 2.0f};
        
        if (gameState->bonus_stage_enemies_hit >= 40) {
            bonus_score = SCORE_BONUS_STAGE_PERFECT;
        } else if (gameState->bonus_stage_enemies_hit == 39) {
            bonus_score = SCORE_BONUS_STAGE_39;
        } else if (gameState->bonus_stage_enemies_hit == 38) {
            bonus_score = SCORE_BONUS_STAGE_38;
        } else if (gameState->bonus_stage_enemies_hit == 37) {
            bonus_score = SCORE_BONUS_STAGE_37;
        } else if (gameState->bonus_stage_enemies_hit == 36) {
            bonus_score = SCORE_BONUS_STAGE_36;
        } else {
            bonus_score = gameState->bonus_stage_enemies_hit * SCORE_BONUS_STAGE_BASE;
        }
        
        AddScore(gameState, bonus_score, center);
        
        // End bonus stage
        gameState->is_bonus_stage = false;
        
        // Clear any remaining enemies
        for (int i = 0; i < MAX_ENEMIES; i++) {
            gameState->enemies[i].active = false;
        }
    }
}

// High score functions
void LoadHighScore(GameState* gameState) {
    FILE* file = fopen("shmup/highscore.txt", "r");
    if (file != NULL) {
        if (fscanf(file, "%d", &gameState->high_score) != 1) {
            gameState->high_score = 0;
        }
        fclose(file);
    } else {
        gameState->high_score = 0;
    }
}

void SaveHighScore(const GameState* gameState) {
    FILE* file = fopen("shmup/highscore.txt", "w");
    if (file != NULL) {
        fprintf(file, "%d", gameState->high_score);
        fclose(file);
    }
}

// Game state management functions
void HandleGameOver(GameState* gameState) {
    // Update high score if necessary and save it
    if (gameState->score > gameState->high_score) {
        gameState->high_score = gameState->score;
        SaveHighScore(gameState);
    }
    
    // Switch to game over state
    gameState->screen_state = GAME_OVER;
    gameState->game_over_timer = 0.0f;
}

void UpdateGameOver(GameState* gameState, float delta) {
    gameState->game_over_timer += delta;
    
    // Allow restart after 2 seconds
    if (gameState->game_over_timer > 2.0f) {
        if (IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_ENTER)) {
            // Reset game but keep high score
            int saved_high_score = gameState->high_score;
            InitGame(gameState);
            gameState->high_score = saved_high_score;
        }
    }
}

void DrawGameOver(const GameState* gameState) {
    ClearBackground(BLACK);
    
    // Draw a simple starfield background
    for (int i = 0; i < 50; i++) {
        int x = (i * 137) % SCREEN_WIDTH;
        int y = (i * 73) % SCREEN_HEIGHT;
        DrawCircle(x, y, 1, WHITE);
    }
    
    // Game Over title
    const char* game_over_text = "GAME OVER";
    int title_width = MeasureText(game_over_text, 60);
    DrawText(game_over_text, SCREEN_WIDTH / 2 - title_width / 2, SCREEN_HEIGHT / 2 - 120, 60, RED);
    
    // Final score
    const char* final_score_text = TextFormat("FINAL SCORE: %08d", gameState->score);
    int score_width = MeasureText(final_score_text, 30);
    DrawText(final_score_text, SCREEN_WIDTH / 2 - score_width / 2, SCREEN_HEIGHT / 2 - 50, 30, YELLOW);
    
    // High score
    const char* high_score_text = TextFormat("HIGH SCORE: %08d", gameState->high_score);
    int high_score_width = MeasureText(high_score_text, 25);
    Color high_score_color = (gameState->score == gameState->high_score) ? GOLD : WHITE;
    DrawText(high_score_text, SCREEN_WIDTH / 2 - high_score_width / 2, SCREEN_HEIGHT / 2 - 10, 25, high_score_color);
    
    // New high score message
    if (gameState->score == gameState->high_score && gameState->score > 0) {
        const char* new_record_text = "NEW HIGH SCORE!";
        int record_width = MeasureText(new_record_text, 20);
        float pulse = sinf(gameState->game_over_timer * 4.0f) * 0.5f + 0.5f;
        Color pulse_color = (Color){255, (unsigned char)(255 * pulse), 0, 255};
        DrawText(new_record_text, SCREEN_WIDTH / 2 - record_width / 2, SCREEN_HEIGHT / 2 + 20, 20, pulse_color);
    }
    
    // Wave reached
    const char* wave_text = TextFormat("WAVE REACHED: %d", gameState->wave_number);
    int wave_width = MeasureText(wave_text, 20);
    DrawText(wave_text, SCREEN_WIDTH / 2 - wave_width / 2, SCREEN_HEIGHT / 2 + 50, 20, LIGHTGRAY);
    
    // Instructions
    if (gameState->game_over_timer > 2.0f) {
        const char* restart_text = "PRESS SPACE OR ENTER TO RESTART";
        int restart_width = MeasureText(restart_text, 20);
        float alpha = sinf(gameState->game_over_timer * 2.0f) * 0.5f + 0.5f;
        Color blink_color = (Color){255, 255, 255, (unsigned char)(255 * alpha)};
        DrawText(restart_text, SCREEN_WIDTH / 2 - restart_width / 2, SCREEN_HEIGHT / 2 + 100, 20, blink_color);
    } else {
        const char* wait_text = "...";
        int wait_width = MeasureText(wait_text, 30);
        DrawText(wait_text, SCREEN_WIDTH / 2 - wait_width / 2, SCREEN_HEIGHT / 2 + 100, 30, GRAY);
    }
    
    // Controls reminder
    const char* controls_text = "Arrow keys: Move | SPACE: Shoot";
    int controls_width = MeasureText(controls_text, 14);
    DrawText(controls_text, SCREEN_WIDTH / 2 - controls_width / 2, SCREEN_HEIGHT - 40, 14, DARKGRAY);
}
