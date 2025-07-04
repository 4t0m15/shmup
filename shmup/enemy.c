#include "game.h"
#include <math.h>
#include <stdlib.h>

// Function to update morphing mechanics
void UpdateMorphing(GameState* gameState, float delta) {
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy* enemy = &gameState->enemies[i];
        
        if (!enemy->active || enemy->state != MORPHING) continue;
        
        enemy->morph_timer -= delta;
        
        if (enemy->morph_timer <= 0.0f) {
            // Complete morphing
            enemy->type = enemy->target_type;
            enemy->state = FORMATION;
            enemy->has_morphed = true;
            enemy->can_morph = false;
            
            // Update stats based on new type
            switch (enemy->type) {
                case FLAGSHIP:
                    enemy->health = 3;
                    enemy->shooting = true;
                    enemy->shoot_timer = 0.5f;
                    break;
                case HOSTILE_SHIP:
                    enemy->health = 1;
                    enemy->shooting = true;
                    enemy->shoot_timer = 0.3f;
                    break;
                default:
                    break;
            }
        }
    }
}

// Function to trigger morphing
void TriggerMorphing(Enemy* enemy) {
    if (!enemy->can_morph || enemy->has_morphed) return;
    
    enemy->state = MORPHING;
    enemy->morph_timer = MORPH_DURATION;
    enemy->original_type = enemy->type;
    
    // Determine target type based on original type
    if (enemy->type == NORMAL && (rand() % 100) < MORPH_CHANCE) {
        enemy->target_type = FLAGSHIP;
    } else if (enemy->type == ESCORT && (rand() % 100) < MORPH_CHANCE / 2) {
        enemy->target_type = FLAGSHIP;
    }
}

// Function to update captured ships
void UpdateCapturedShips(GameState* gameState, float delta) {
    for (int i = 0; i < MAX_CAPTURED_SHIPS; i++) {
        CapturedShip* ship = &gameState->captured_ships[i];
        
        if (!ship->active) continue;
        
        // Check if it's time to spawn as hostile
        if (ship->hostile && gameState->wave_number >= ship->spawn_wave) {
            SpawnHostileShip(gameState, ship->spawn_wave);
            ship->active = false;
        }
    }
}

// Function to spawn hostile ship
void SpawnHostileShip(GameState* gameState, int spawn_wave) {
    // Find an inactive enemy slot
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!gameState->enemies[i].active) {
            Enemy* enemy = &gameState->enemies[i];
            
            enemy->type = HOSTILE_SHIP;
            enemy->health = 1;
            enemy->formation_pos.x = SCREEN_WIDTH / 2.0f;
            enemy->formation_pos.y = 100.0f;
            enemy->position.x = SCREEN_WIDTH / 2.0f;
            enemy->position.y = -PLAYER_SIZE;
            enemy->entry_start = enemy->position;
            enemy->state = ENTERING;
            enemy->pattern = PATTERN_ARC;
            enemy->pattern_progress = 0.0f;
            enemy->pattern_param = 1.0f;
            enemy->timer = 1.0f;
            enemy->active = true;
            enemy->shooting = true;
            enemy->shoot_timer = 0.5f;
            enemy->tractor_active = false;
            enemy->is_escort_in_combo = false;
            enemy->escort_group_id = 0;
            enemy->can_morph = false;
            enemy->has_morphed = false;
            enemy->aggression_multiplier = 1.0f + gameState->wave_number * AGGRESSION_SCALE_RATE;
            
            break;
        }
    }
}

// Function to update aggression scaling
void UpdateAggressionScaling(GameState* gameState) {
    // Increase base aggression based on wave number
    gameState->base_aggression = 1.0f + gameState->wave_number * AGGRESSION_SCALE_RATE;
    
    // Apply aggression scaling to all enemies
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (gameState->enemies[i].active) {
            gameState->enemies[i].aggression_multiplier = gameState->base_aggression;
        }
    }
}

// Function to handle ship capture
void HandleShipCapture(GameState* gameState, Enemy* boss) {
    if (boss->type != BOSS || !boss->tractor_active) return;
    
    // Player loses a life but ship is captured
    gameState->player.lives--;
    gameState->player.captured = false;
    
    // Set boss to holding captured ship
    boss->has_captured_ship = true;
    boss->state = CAPTURED_SHIP_HOLDING;
    boss->tractor_active = false;
    
    // Add captured ship to tracking
    for (int i = 0; i < MAX_CAPTURED_SHIPS; i++) {
        if (!gameState->captured_ships[i].active) {
            gameState->captured_ships[i].active = true;
            gameState->captured_ships[i].hostile = false;
            gameState->captured_ships[i].spawn_wave = gameState->wave_number + HOSTILE_SHIP_DELAY;
            gameState->captured_ships[i].rescued = false;
            gameState->captured_ships[i].position = boss->position;
            gameState->total_captured_ships++;
            break;
        }
    }
    
    // Reset player position
    gameState->player.rect.x = SCREEN_WIDTH / 2.0f - PLAYER_SIZE / 2.0f;
    gameState->player.rect.y = SCREEN_HEIGHT - 80.0f;
    
    // Check for game over
    if (gameState->player.lives <= 0) {
        HandleGameOver(gameState);
    }
}

// Function to handle ship rescue
void HandleShipRescue(GameState* gameState, Enemy* boss) {
    if (!boss->has_captured_ship) return;
    
    // Check if boss was destroyed during dive (rescue) or in formation (hostile)
    bool rescued_during_dive = (boss->state == ATTACKING || boss->state == SPECIAL_ATTACK);
    
    if (rescued_during_dive) {
        // Successful rescue - award dual fighter
        gameState->player.has_captured_ship = true;
        gameState->player.dual_fire = true;
        AddScore(gameState, SCORE_CAPTURED_SHIP_RESCUE, boss->position);
        
        // Mark captured ship as rescued
        for (int i = 0; i < MAX_CAPTURED_SHIPS; i++) {
            if (gameState->captured_ships[i].active && !gameState->captured_ships[i].rescued) {
                gameState->captured_ships[i].rescued = true;
                gameState->captured_ships[i].hostile = false;
                gameState->captured_ships[i].active = false;
                break;
            }
        }
    } else {
        // Destroyed in formation - ship becomes hostile
        AddScore(gameState, SCORE_HOSTILE_SHIP_RESCUE, boss->position);
        
        // Mark captured ship as hostile
        for (int i = 0; i < MAX_CAPTURED_SHIPS; i++) {
            if (gameState->captured_ships[i].active && !gameState->captured_ships[i].rescued) {
                gameState->captured_ships[i].hostile = true;
                break;
            }
        }
    }
}

// Function to update tractor beam mechanics
void UpdateTractorBeam(GameState* gameState, Enemy* boss, float delta) {
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

// Enhanced enemy shooting with aggression scaling
void UpdateEnemyShooting(GameState* gameState, Enemy* enemy, float delta) {
    if (!enemy->shooting) return;
    
    // Faster shooting for aggressive enemies
    enemy->shoot_timer -= delta * enemy->aggression_multiplier;
    
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
                    float bullet_speed = ENEMY_BULLET_SPEED * enemy->aggression_multiplier;
                    gameState->enemy_bullets[i].velocity.x = (direction.x / distance) * bullet_speed;
                    gameState->enemy_bullets[i].velocity.y = (direction.y / distance) * bullet_speed;
                } else {
                    gameState->enemy_bullets[i].velocity.x = 0;
                    gameState->enemy_bullets[i].velocity.y = ENEMY_BULLET_SPEED * enemy->aggression_multiplier;
                }
                
                // Faster reload for aggressive enemies
                float base_reload = 1.0f + (rand() % 200) / 100.0f;
                enemy->shoot_timer = base_reload / enemy->aggression_multiplier;
                break;
            }
        }
    }
}

// Function to update enemy bullets
void UpdateEnemyBullets(GameState* gameState, float delta) {
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
        
        // Check collision with player using new bullet collision detection
        if (CheckCollisionBulletRec(gameState->enemy_bullets[i].position, gameState->player.rect, false)) {
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