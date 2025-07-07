#include "game.h"
#include <math.h>
#include <stdlib.h>

// =============================================================================
// FORWARD DECLARATIONS
// =============================================================================



// =============================================================================
// ENEMY MANAGEMENT FUNCTIONS
// =============================================================================

// Function to spawn hostile ship
void SpawnHostileShip(GameState* gameState, int spawn_wave) {
    (void)spawn_wave; // Mark parameter as intentionally unused
    if (!gameState) return;
    
    // Find an inactive enemy slot
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!gameState->enemies[i].active) {
            Enemy* enemy = &gameState->enemies[i];
            
            // Initialize all basic fields
            enemy->active = true;
            enemy->type = HOSTILE_SHIP;
            enemy->state = ENTERING;
            enemy->health = 1;
            enemy->position = (Vector2){SCREEN_WIDTH / 2.0f, -50.0f};
            enemy->formation_pos = (Vector2){SCREEN_WIDTH / 2.0f, 100.0f};
            enemy->entry_start = enemy->position;
            enemy->attack_start = (Vector2){0, 0};
            enemy->pattern = PATTERN_STRAIGHT;
            enemy->pattern_progress = 0.0f;
            enemy->pattern_param = 0.0f;
            enemy->timer = 0.0f;
            enemy->shooting = true;
            enemy->shoot_timer = 1.0f;
            enemy->aggression_multiplier = 1.5f;
            enemy->can_morph = false;
            enemy->has_morphed = false;
            
            // Initialize tractor beam fields
            enemy->tractor_active = false;
            enemy->tractor_angle = 0.0f;
            enemy->tractor_center = (Vector2){0, 0};
            
            // Initialize morphing fields
            enemy->original_type = enemy->type;
            enemy->target_type = enemy->type;
            enemy->morph_timer = 0.0f;
            
            // Initialize captured ship fields
            enemy->has_captured_ship = false;
            enemy->captured_ship_hostile = false;
            enemy->captured_ship_spawn_wave = 0;
            
            // Initialize combo fields
            enemy->is_escort_in_combo = false;
            enemy->escort_group_id = 0;
            
            // Initialize AI fields
            enemy->ai_behavior = AI_AGGRESSIVE_ATTACK;
            enemy->ai_timer = 0.0f;
            enemy->ai_target = enemy->formation_pos;
            enemy->predicted_player_pos = (Vector2){0, 0};
            enemy->last_player_distance = 0.0f;
            enemy->coordinating = false;
            enemy->coordination_group = 0;
            enemy->evasion_direction = 0.0f;
            enemy->last_velocity = (Vector2){0, 0};
            
            break;
        }
    }
}

// Function to update morphing mechanics
void UpdateMorphing(GameState* gameState, float delta) {
    if (!gameState) return;
    
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
    if (!enemy || !enemy->can_morph || enemy->has_morphed) return;
    
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
    if (!gameState) return;
    (void)delta; // Mark parameter as intentionally unused
    
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

// Function to update aggression scaling
void UpdateAggressionScaling(GameState* gameState) {
    if (!gameState) return;
    
    float wave_multiplier = 1.0f + (gameState->wave_number * AGGRESSION_SCALE_RATE);
    gameState->base_aggression = 1.0f + (gameState->wave_number * 0.05f);
    
    // Apply aggression scaling to all active enemies
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (gameState->enemies[i].active) {
            gameState->enemies[i].aggression_multiplier = wave_multiplier;
        }
    }
}

// Function to handle ship capture
void HandleShipCapture(GameState* gameState, Enemy* boss) {
    if (!gameState || !boss || boss->type != BOSS || boss->has_captured_ship) return;
    
    // Mark player as captured
    gameState->player.captured = true;
    gameState->player.capture_target = boss->position;
    
    // Mark boss as having captured ship
    boss->has_captured_ship = true;
    boss->captured_ship_hostile = true;
    boss->captured_ship_spawn_wave = gameState->wave_number + HOSTILE_SHIP_DELAY;
    
    // Play capture sound effect
    PlayGameSound(&gameState->audio, GAME_SOUND_POWERUP, 0.8f);
    
    // Add captured ship to tracking
    for (int i = 0; i < MAX_CAPTURED_SHIPS; i++) {
        if (!gameState->captured_ships[i].active) {
            gameState->captured_ships[i].active = true;
            gameState->captured_ships[i].hostile = true;
            gameState->captured_ships[i].spawn_wave = boss->captured_ship_spawn_wave;
            gameState->captured_ships[i].position = boss->position;
            gameState->total_captured_ships++;
            break;
        }
    }
}

// Function to handle ship rescue
void HandleShipRescue(GameState* gameState, Enemy* boss) {
    if (!gameState || !boss || boss->type != BOSS || !boss->has_captured_ship) return;
    
    // Player gets dual fighter ability
    gameState->player.has_captured_ship = true;
    gameState->player.dual_fire = true;
    gameState->player.captured = false;
    
    // Mark boss as no longer having captured ship
    boss->has_captured_ship = false;
    boss->captured_ship_hostile = false;
    
    // Play rescue/powerup sound
    PlayGameSound(&gameState->audio, GAME_SOUND_POWERUP, 1.0f);
    
    // Remove from captured ships tracking
    for (int i = 0; i < MAX_CAPTURED_SHIPS; i++) {
        if (gameState->captured_ships[i].active && !gameState->captured_ships[i].rescued) {
            gameState->captured_ships[i].rescued = true;
            gameState->captured_ships[i].active = false;
            break;
        }
    }
}

// Function to spawn enemy wave
void SpawnEnemyWave(GameState* gameState) {
    if (!gameState) return;
    
    // Check if all enemies are defeated OR this is the initial wave (wave 0)
    bool all_defeated = true;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (gameState->enemies[i].active) {
            all_defeated = false;
            break;
        }
    }
    
    // Only spawn if all enemies are defeated OR this is the initial wave
    if (!all_defeated && gameState->wave_number > 0) return;
    
    // Advance wave
    gameState->wave_number++;
    
    // Update aggression scaling
    UpdateAggressionScaling(gameState);
    
    // Play wave start sound or music change
    if (gameState->wave_number % 6 == 0) {
        // Boss wave - change to boss music
        PlayMusicTrack(&gameState->audio, MUSIC_BOSS);
    } else if (gameState->wave_number == 1) {
        // First wave - start game music
        PlayMusicTrack(&gameState->audio, MUSIC_GAME);
    }
    
    // New wave progression: 5 normal stages, 1 boss stage, 1 bonus stage (repeating)
    // Calculate position in the 7-wave cycle (waves 1-5 normal, wave 6 boss, wave 7 bonus)
    int cycle_position = ((gameState->wave_number - 1) % 7) + 1;
    
    // Spawn bonus stage on wave 7 of each cycle
    if (cycle_position == 7) {
        SpawnBonusStage(gameState);
        return;
    }
    
    // Spawn regular wave (with boss on wave 6 of each cycle)
    int enemies_to_spawn = 8 + (gameState->wave_number / 2);
    if (enemies_to_spawn > MAX_ENEMIES) enemies_to_spawn = MAX_ENEMIES;
    
    // Spawn enemies in formation
    for (int i = 0; i < enemies_to_spawn; i++) {
        Enemy* enemy = &gameState->enemies[i];
        
        // Initialize all basic fields
        enemy->active = true;
        enemy->type = (i < 2) ? ESCORT : NORMAL;
        enemy->state = ENTERING;
        enemy->health = (enemy->type == ESCORT) ? 2 : 1;
        enemy->position = (Vector2){50.0f + i * 80.0f, -50.0f};
        enemy->formation_pos = (Vector2){50.0f + i * 80.0f, 100.0f};
        enemy->entry_start = enemy->position;
        enemy->attack_start = (Vector2){0, 0};
        enemy->pattern = PATTERN_STRAIGHT;
        enemy->pattern_progress = 0.0f;
        enemy->pattern_param = 0.0f;
        enemy->timer = 0.0f;
        enemy->shooting = (enemy->type == ESCORT);
        enemy->shoot_timer = 1.0f + (rand() % 100) / 100.0f;
        enemy->aggression_multiplier = gameState->base_aggression;
        enemy->can_morph = (rand() % 100) < MORPH_CHANCE;
        enemy->has_morphed = false;
        
        // Initialize tractor beam fields
        enemy->tractor_active = false;
        enemy->tractor_angle = 0.0f;
        enemy->tractor_center = (Vector2){0, 0};
        
        // Initialize morphing fields
        enemy->original_type = enemy->type;
        enemy->target_type = enemy->type;
        enemy->morph_timer = 0.0f;
        
        // Initialize captured ship fields
        enemy->has_captured_ship = false;
        enemy->captured_ship_hostile = false;
        enemy->captured_ship_spawn_wave = 0;
        
        // Initialize combo fields
        enemy->is_escort_in_combo = false;
        enemy->escort_group_id = 0;
        
        // Initialize AI fields
        enemy->ai_behavior = AI_FORMATION_FLYING;
        enemy->ai_timer = 0.0f;
        enemy->ai_target = enemy->formation_pos;
        enemy->predicted_player_pos = (Vector2){0, 0};
        enemy->last_player_distance = 0.0f;
        enemy->coordinating = false;
        enemy->coordination_group = 0;
        enemy->evasion_direction = 0.0f;
        enemy->last_velocity = (Vector2){0, 0};
    }
    
    // Spawn boss on wave 6 of each cycle (waves 6, 13, 20, etc.)
    if (cycle_position == 6 && enemies_to_spawn > 0) {
        Enemy* boss = &gameState->enemies[enemies_to_spawn - 1];
        boss->type = BOSS;
        boss->health = 5;
        boss->position = (Vector2){SCREEN_WIDTH / 2.0f, -50.0f};
        boss->formation_pos = (Vector2){SCREEN_WIDTH / 2.0f, 80.0f};
        boss->shooting = true;
        boss->tractor_active = true;
        boss->tractor_center = boss->position;
        boss->ai_behavior = AI_AGGRESSIVE_ATTACK;
        boss->ai_target = boss->formation_pos;
    }
}

// Function to update tractor beam
void UpdateTractorBeam(GameState* gameState, Enemy* boss, float delta) {
    if (!gameState || !boss || !boss->tractor_active) return;
    
    // Update tractor beam angle
    boss->tractor_angle += delta * 2.0f;
    boss->tractor_center = boss->position;
    
    // Check if player is in tractor beam range
    Vector2 player_center = {
        gameState->player.rect.x + gameState->player.rect.width / 2,
        gameState->player.rect.y + gameState->player.rect.height / 2
    };
    
    float distance = Vector2Distance(player_center, boss->tractor_center);
    
    if (distance < TRACTOR_BEAM_RANGE && !gameState->player.captured) {
        // Attempt to capture player
        if (rand() % 100 < 5) { // 5% chance per frame
            HandleShipCapture(gameState, boss);
        }
    }
}

// Function to update enemy shooting
void UpdateEnemyShooting(GameState* gameState, Enemy* enemy, float delta) {
    if (!gameState || !enemy || !enemy->shooting) return;
    
    enemy->shoot_timer -= delta;
    
    if (enemy->shoot_timer <= 0.0f) {
        // Find an inactive enemy bullet
        for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
            if (!gameState->enemy_bullets[i].active) {
                gameState->enemy_bullets[i].active = true;
                gameState->enemy_bullets[i].position = enemy->position;
                
                // Aim at player
                Vector2 player_center = {
                    gameState->player.rect.x + gameState->player.rect.width / 2,
                    gameState->player.rect.y + gameState->player.rect.height / 2
                };
                
                Vector2 direction = {
                    player_center.x - enemy->position.x,
                    player_center.y - enemy->position.y
                };
                
                float length = sqrtf(direction.x * direction.x + direction.y * direction.y);
                if (length > 0) {
                    direction.x /= length;
                    direction.y /= length;
                }
                
                gameState->enemy_bullets[i].velocity = (Vector2){
                    direction.x * ENEMY_BULLET_SPEED,
                    direction.y * ENEMY_BULLET_SPEED
                };
                
                // Play enemy shooting sound
                PlayGameSound(&gameState->audio, GAME_SOUND_ENEMY_SHOOT, 1.0f);
                
                break;
            }
        }
        
        // Reset shoot timer
        enemy->shoot_timer = 1.0f + (rand() % 100) / 100.0f;
    }
}

// Function to update enemy bullets
void UpdateEnemyBullets(GameState* gameState, float delta) {
    if (!gameState) return;
    
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (gameState->enemy_bullets[i].active) {
            gameState->enemy_bullets[i].position.x += gameState->enemy_bullets[i].velocity.x * delta;
            gameState->enemy_bullets[i].position.y += gameState->enemy_bullets[i].velocity.y * delta;
            
            // Remove bullets that go off screen
            if (gameState->enemy_bullets[i].position.x < -10 || 
                gameState->enemy_bullets[i].position.x > SCREEN_WIDTH + 10 ||
                gameState->enemy_bullets[i].position.y < -10 || 
                gameState->enemy_bullets[i].position.y > SCREEN_HEIGHT + 10) {
                gameState->enemy_bullets[i].active = false;
            }
        }
    }
}

// Function to update enemies
void UpdateEnemies(GameState* gameState, float delta) {
    if (!gameState) return;
    
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy* enemy = &gameState->enemies[i];
        
        if (!enemy->active) continue;
        
        // Update enemy position based on movement pattern
        enemy->position = CalculateMovementPattern(enemy, delta);
        
        // Update enemy shooting
        UpdateEnemyShooting(gameState, enemy, delta);
        
        // Update tractor beam for boss
        if (enemy->type == BOSS) {
            UpdateTractorBeam(gameState, enemy, delta);
        }
        
        // Update timers
        enemy->timer += delta;
        
        // Check for morphing trigger
        if (enemy->can_morph && !enemy->has_morphed && enemy->timer > 5.0f) {
            if (rand() % 100 < 2) { // 2% chance per frame after 5 seconds
                TriggerMorphing(enemy);
            }
        }
        
        // Remove enemies that go off screen
        if (enemy->position.y > SCREEN_HEIGHT + 50 || enemy->position.y < -100) {
            enemy->active = false;
        }
    }
    
    // Update morphing
    UpdateMorphing(gameState, delta);
    
    // Update captured ships
    UpdateCapturedShips(gameState, delta);
}