#include "game.h"
#include <math.h>
#include <stdlib.h>

// Enhanced enemy AI update function with advanced behaviors
void UpdateEnemyAI(GameState* gameState, float delta) {
    // Update player position history for AI prediction
    UpdatePlayerPositionHistory(gameState);
    
    // Update coordinated attacks first
    UpdateCoordinatedAttack(gameState, delta);
    
    static float formation_sway_timer = 0.0f;
    formation_sway_timer += delta;
    
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy* enemy = &gameState->enemies[i];
        
        if (!enemy->active) continue;
        
        // Update AI behavior timers
        enemy->ai_timer -= delta;
        enemy->timer -= delta;
        
        // Update individual enemy behavior
        UpdateEnemyBehavior(gameState, enemy, delta);
        
        switch (enemy->state) {
            case INACTIVE:
                break;
                
            case ENTERING: {
                Vector2 new_pos = CalculateMovementPattern(enemy, delta);
                enemy->position = new_pos;
                
                float dx = enemy->formation_pos.x - enemy->position.x;
                float dy = enemy->formation_pos.y - enemy->position.y;
                float distance = sqrtf(dx * dx + dy * dy);
                
                if (distance < 10.0f || enemy->pattern_progress >= 1.0f) {
                    enemy->position = enemy->formation_pos;
                    enemy->state = FORMATION;
                    enemy->timer = 2.0f + (i % 5) * 0.5f;
                    enemy->pattern_progress = 0.0f;
                    
                    // Set initial AI behavior
                    SetEnemyAIBehavior(enemy, AI_FORMATION_FLYING);
                    
                    if (enemy->type == BOSS || enemy->type == FLAGSHIP || (rand() % 3 == 0)) {
                        enemy->shooting = true;
                        enemy->shoot_timer = 1.0f + (rand() % 200) / 100.0f;
                    }
                }
                break;
            }
                
            case FORMATION: {
                // Enhanced formation behavior with AI
                switch (enemy->ai_behavior) {
                    case AI_FORMATION_FLYING:
                        enemy->position.x = enemy->formation_pos.x + 
                            sinf(formation_sway_timer * 1.5f + i * 0.3f) * ENEMY_SWAY_AMPLITUDE;
                        enemy->position.y = enemy->formation_pos.y + 
                            cosf(formation_sway_timer * 0.8f + i * 0.2f) * 8.0f;
                        break;
                        
                    case AI_DEFENSIVE_FORMATION:
                        // Tighter formation with less sway
                        enemy->position.x = enemy->formation_pos.x + 
                            sinf(formation_sway_timer * 0.8f + i * 0.3f) * (ENEMY_SWAY_AMPLITUDE * 0.5f);
                        enemy->position.y = enemy->formation_pos.y + 
                            cosf(formation_sway_timer * 0.5f + i * 0.2f) * 4.0f;
                        break;
                        
                    case AI_SWARM_BEHAVIOR:
                        UpdateSwarmBehavior(gameState, enemy, delta);
                        break;
                        
                    default:
                        enemy->position.x = enemy->formation_pos.x + 
                            sinf(formation_sway_timer * 1.5f + i * 0.3f) * ENEMY_SWAY_AMPLITUDE;
                        enemy->position.y = enemy->formation_pos.y + 
                            cosf(formation_sway_timer * 0.8f + i * 0.2f) * 8.0f;
                        break;
                }
                
                UpdateEnemyShooting(gameState, enemy, delta);
                
                // Check for morphing opportunity
                if (enemy->can_morph && !enemy->has_morphed && (rand() % 1000) < 2) {
                    TriggerMorphing(enemy);
                    continue;
                }
                
                // Enhanced AI decision making
                if (enemy->timer <= 0.0f) {
                    // Determine next action based on AI behavior
                    float attack_chance = ENEMY_ATTACK_CHANCE;
                    
                    // Modify attack chance based on AI behavior
                    switch (enemy->ai_behavior) {
                        case AI_AGGRESSIVE_ATTACK:
                            attack_chance = 8;
                            break;
                        case AI_DEFENSIVE_FORMATION:
                            attack_chance = 1;
                            break;
                        case AI_SWARM_BEHAVIOR:
                            attack_chance = 4;
                            break;
                        default:
                            break;
                    }
                    
                    if (enemy->type == BOSS || enemy->type == FLAGSHIP) {
                        attack_chance = 15;
                    }
                    
                    if ((rand() % 100) < attack_chance) {
                        // Choose attack pattern based on AI behavior
                        if (enemy->ai_behavior == AI_FLANKING_MANEUVER) {
                            enemy->state = AI_FLANKING;
                            enemy->ai_target = CalculateFlankingPosition(gameState, enemy);
                        } else if (enemy->coordinating) {
                            enemy->state = AI_COORDINATING;
                        } else {
                            enemy->state = (enemy->type == BOSS) ? SPECIAL_ATTACK : ATTACKING;
                        }
                        
                        enemy->timer = 0.0f;
                        enemy->pattern_progress = 0.0f;
                        enemy->attack_start = enemy->position;
                        
                        if (enemy->type == BOSS) {
                            enemy->pattern = PATTERN_BEAM;
                            enemy->tractor_active = true;
                        } else if (enemy->type == FLAGSHIP) {
                            enemy->pattern = PATTERN_SPIRAL;
                        } else {
                            // Enhanced pattern selection based on AI behavior
                            MovementPattern patterns[] = {PATTERN_LOOP, PATTERN_CURVE, PATTERN_SPIRAL, 
                                                        PATTERN_ZIGZAG, PATTERN_SINE_WAVE};
                            enemy->pattern = patterns[rand() % 5];
                        }
                        
                        enemy->pattern_param = (rand() % 100) / 50.0f - 1.0f;
                    } else {
                        enemy->timer = 1.0f + (rand() % 300) / 100.0f;
                        
                        // Occasionally change AI behavior
                        if ((rand() % 100) < 5) {
                            AIBehavior behaviors[] = {AI_FORMATION_FLYING, AI_AGGRESSIVE_ATTACK, 
                                                    AI_DEFENSIVE_FORMATION, AI_SWARM_BEHAVIOR};
                            SetEnemyAIBehavior(enemy, behaviors[rand() % 4]);
                        }
                    }
                }
                break;
            }
                
            case ATTACKING: {
                Vector2 new_pos = CalculateMovementPattern(enemy, delta);
                enemy->position = new_pos;
                
                UpdateEnemyShooting(gameState, enemy, delta);
                
                // Check for evasion if player is shooting nearby
                if (ShouldEnemyEvade(gameState, enemy)) {
                    enemy->state = AI_EVADING;
                    enemy->ai_timer = 1.0f;
                    enemy->evasion_direction = (rand() % 2 == 0) ? -1.0f : 1.0f;
                }
                
                if (enemy->pattern_progress >= 1.0f || 
                    enemy->position.y > SCREEN_HEIGHT + 50 || 
                    enemy->position.x < -50 || enemy->position.x > SCREEN_WIDTH + 50) {
                    
                    enemy->state = RETURNING;
                    enemy->pattern = PATTERN_STRAIGHT;
                    enemy->pattern_progress = 0.0f;
                    enemy->timer = 0.0f;
                    enemy->shooting = false;
                    
                    enemy->position.x = enemy->formation_pos.x;
                    enemy->position.y = -50.0f;
                    enemy->entry_start = enemy->position;
                }
                break;
            }
            
            case AI_FLANKING: {
                // Move towards flanking position
                Vector2 direction = {
                    enemy->ai_target.x - enemy->position.x,
                    enemy->ai_target.y - enemy->position.y
                };
                
                float distance = sqrtf(direction.x * direction.x + direction.y * direction.y);
                if (distance > 10.0f) {
                    float speed = ENEMY_ATTACK_SPEED * enemy->aggression_multiplier;
                    enemy->position.x += (direction.x / distance) * speed * delta;
                    enemy->position.y += (direction.y / distance) * speed * delta;
                } else {
                    // Reached flanking position, now attack
                    enemy->state = ATTACKING;
                    enemy->attack_start = enemy->position;
                    enemy->pattern = PATTERN_CURVE;
                    enemy->pattern_progress = 0.0f;
                }
                
                UpdateEnemyShooting(gameState, enemy, delta);
                break;
            }
            
            case AI_EVADING: {
                // Evasive maneuver
                float evasion_speed = ENEMY_ATTACK_SPEED * 1.5f * enemy->aggression_multiplier;
                enemy->position.x += enemy->evasion_direction * evasion_speed * delta;
                enemy->position.y += sinf(enemy->ai_timer * 8.0f) * evasion_speed * 0.5f * delta;
                
                // Keep enemy on screen
                if (enemy->position.x < 0 || enemy->position.x > SCREEN_WIDTH) {
                    enemy->evasion_direction *= -1.0f;
                }
                
                if (enemy->ai_timer <= 0.0f) {
                    enemy->state = RETURNING;
                    enemy->pattern = PATTERN_STRAIGHT;
                    enemy->pattern_progress = 0.0f;
                    enemy->timer = 0.0f;
                    enemy->shooting = false;
                    
                    enemy->position.x = enemy->formation_pos.x;
                    enemy->position.y = -50.0f;
                    enemy->entry_start = enemy->position;
                }
                break;
            }
            
            case AI_COORDINATING: {
                // Coordinated attack with other enemies
                Vector2 predicted_pos = PredictPlayerPosition(gameState, 1.0f);
                Vector2 target_pos = {
                    predicted_pos.x + (i - MAX_ENEMIES/2) * 40.0f,
                    predicted_pos.y + 50.0f
                };
                
                Vector2 direction = {
                    target_pos.x - enemy->position.x,
                    target_pos.y - enemy->position.y
                };
                
                float distance = sqrtf(direction.x * direction.x + direction.y * direction.y);
                if (distance > 10.0f) {
                    float speed = ENEMY_ATTACK_SPEED * enemy->aggression_multiplier;
                    enemy->position.x += (direction.x / distance) * speed * delta;
                    enemy->position.y += (direction.y / distance) * speed * delta;
                }
                
                UpdateEnemyShooting(gameState, enemy, delta);
                
                // Return to formation after coordinated attack
                if (enemy->timer <= -2.0f) {
                    enemy->state = RETURNING;
                    enemy->pattern = PATTERN_STRAIGHT;
                    enemy->pattern_progress = 0.0f;
                    enemy->timer = 0.0f;
                    enemy->shooting = false;
                    enemy->coordinating = false;
                    
                    enemy->position.x = enemy->formation_pos.x;
                    enemy->position.y = -50.0f;
                    enemy->entry_start = enemy->position;
                }
                break;
            }
            
            case SPECIAL_ATTACK: {
                UpdateTractorBeam(gameState, enemy, delta);
                
                Vector2 new_pos = CalculateMovementPattern(enemy, delta);
                enemy->position = new_pos;
                
                UpdateEnemyShooting(gameState, enemy, delta);
                
                if (enemy->tractor_active && gameState->player.captured) {
                    HandleShipCapture(gameState, enemy);
                }
                
                if (enemy->pattern_progress >= 1.0f) {
                    enemy->state = RETURNING;
                    enemy->pattern = PATTERN_STRAIGHT;
                    enemy->pattern_progress = 0.0f;
                    enemy->timer = 0.0f;
                    enemy->tractor_active = false;
                    enemy->shooting = false;
                    gameState->player.captured = false;
                    
                    enemy->position.x = enemy->formation_pos.x;
                    enemy->position.y = -50.0f;
                    enemy->entry_start = enemy->position;
                }
                break;
            }
            
            case RETURNING: {
                Vector2 new_pos = CalculateMovementPattern(enemy, delta);
                enemy->position = new_pos;
                
                float dx = enemy->formation_pos.x - enemy->position.x;
                float dy = enemy->formation_pos.y - enemy->position.y;
                float distance = sqrtf(dx * dx + dy * dy);
                
                if (distance < 10.0f) {
                    enemy->position = enemy->formation_pos;
                    enemy->state = FORMATION;
                    enemy->timer = 2.0f + (i % 5) * 0.5f;
                    enemy->pattern_progress = 0.0f;
                    
                    // Reset AI behavior to formation flying
                    SetEnemyAIBehavior(enemy, AI_FORMATION_FLYING);
                    
                    if (enemy->type == BOSS || enemy->type == FLAGSHIP || (rand() % 3 == 0)) {
                        enemy->shooting = true;
                        enemy->shoot_timer = 1.0f + (rand() % 200) / 100.0f;
                    }
                }
                break;
            }
            
            case MORPHING:
                // Morphing is handled in UpdateMorphing function
                break;
            
            case CAPTURED_SHIP_HOLDING: {
                enemy->position.x = enemy->formation_pos.x + 
                    sinf(formation_sway_timer * 1.5f + i * 0.3f) * ENEMY_SWAY_AMPLITUDE;
                enemy->position.y = enemy->formation_pos.y + 
                    cosf(formation_sway_timer * 0.8f + i * 0.2f) * 8.0f;
                
                UpdateEnemyShooting(gameState, enemy, delta);
                
                if (enemy->timer <= 0.0f && (rand() % 100) < 5) {
                    enemy->state = SPECIAL_ATTACK;
                    enemy->timer = 0.0f;
                    enemy->pattern_progress = 0.0f;
                    enemy->attack_start = enemy->position;
                    enemy->pattern = PATTERN_BEAM;
                    enemy->tractor_active = true;
                } else {
                    enemy->timer = 1.0f + (rand() % 300) / 100.0f;
                }
                break;
            }
        }
    }
}

// Update individual enemy behavior patterns
void UpdateEnemyBehavior(GameState* gameState, Enemy* enemy, float delta) {
    // Calculate distance to player
    Vector2 player_center = {
        gameState->player.rect.x + gameState->player.rect.width / 2.0f,
        gameState->player.rect.y + gameState->player.rect.height / 2.0f
    };
    
    float distance_to_player = sqrtf(
        powf(player_center.x - enemy->position.x, 2) +
        powf(player_center.y - enemy->position.y, 2)
    );
    
    enemy->last_player_distance = distance_to_player;
    
    // Update AI behavior based on situation
    if (enemy->ai_timer <= 0.0f) {
        // Decide on new behavior based on game state
        if (distance_to_player < AI_EVASION_THRESHOLD && enemy->state == FORMATION) {
            if (rand() % 100 < 20) {
                SetEnemyAIBehavior(enemy, AI_AGGRESSIVE_ATTACK);
            } else if (rand() % 100 < 30) {
                SetEnemyAIBehavior(enemy, AI_FLANKING_MANEUVER);
            }
        } else if (distance_to_player > 150.0f && enemy->state == FORMATION) {
            if (rand() % 100 < 15) {
                SetEnemyAIBehavior(enemy, AI_SWARM_BEHAVIOR);
            }
        }
        
        // Reset AI timer
        enemy->ai_timer = 2.0f + (rand() % 300) / 100.0f;
    }
}

// Set AI behavior for enemy
void SetEnemyAIBehavior(Enemy* enemy, AIBehavior behavior) {
    enemy->ai_behavior = behavior;
    enemy->ai_timer = 3.0f + (rand() % 200) / 100.0f;
}

// Predict player position for AI targeting
Vector2 PredictPlayerPosition(const GameState* gameState, float prediction_time) {
    Vector2 current_pos = {
        gameState->player.rect.x + gameState->player.rect.width / 2.0f,
        gameState->player.rect.y + gameState->player.rect.height / 2.0f
    };
    
    // Calculate velocity based on recent positions
    Vector2 velocity = {0, 0};
    int samples = 0;
    
    for (int i = 0; i < AI_PREDICTION_FRAMES - 1; i++) {
        Vector2 pos1 = gameState->player_positions[i];
        Vector2 pos2 = gameState->player_positions[i + 1];
        
        if (pos1.x != 0 || pos1.y != 0) {
            velocity.x += pos2.x - pos1.x;
            velocity.y += pos2.y - pos1.y;
            samples++;
        }
    }
    
    if (samples > 0) {
        velocity.x /= samples;
        velocity.y /= samples;
    }
    
    // Predict future position
    Vector2 predicted = {
        current_pos.x + velocity.x * prediction_time * 60.0f, // Assume 60 FPS
        current_pos.y + velocity.y * prediction_time * 60.0f
    };
    
    return predicted;
}

// Update player position history for AI prediction
void UpdatePlayerPositionHistory(GameState* gameState) {
    Vector2 current_pos = {
        gameState->player.rect.x + gameState->player.rect.width / 2.0f,
        gameState->player.rect.y + gameState->player.rect.height / 2.0f
    };
    
    gameState->player_positions[gameState->player_position_index] = current_pos;
    gameState->player_position_index = (gameState->player_position_index + 1) % AI_PREDICTION_FRAMES;
}

// Check if enemy should evade based on nearby player bullets
bool ShouldEnemyEvade(const GameState* gameState, const Enemy* enemy) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!gameState->bullets[i].active) continue;
        
        float distance = sqrtf(
            powf(gameState->bullets[i].position.x - enemy->position.x, 2) +
            powf(gameState->bullets[i].position.y - enemy->position.y, 2)
        );
        
        if (distance < AI_EVASION_THRESHOLD) {
            return true;
        }
    }
    return false;
}

// Update coordinated attack behavior
void UpdateCoordinatedAttack(GameState* gameState, float delta) {
    // Check if we should trigger a coordinated attack
    if (gameState->wave_number > 2 && (rand() % 1000) < 2) {
        int coordination_group = gameState->wave_number;
        int coordinating_enemies = 0;
        
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (gameState->enemies[i].active && 
                gameState->enemies[i].state == FORMATION && 
                !gameState->enemies[i].coordinating) {
                
                gameState->enemies[i].coordinating = true;
                gameState->enemies[i].coordination_group = coordination_group;
                coordinating_enemies++;
                
                if (coordinating_enemies >= 3) break;
            }
        }
    }
}

// Calculate flanking position for enemy
Vector2 CalculateFlankingPosition(const GameState* gameState, const Enemy* enemy) {
    Vector2 player_center = {
        gameState->player.rect.x + gameState->player.rect.width / 2.0f,
        gameState->player.rect.y + gameState->player.rect.height / 2.0f
    };
    
    // Choose left or right flank
    float flank_side = (enemy->position.x < player_center.x) ? -1.0f : 1.0f;
    
    Vector2 flank_pos = {
        player_center.x + flank_side * AI_FLANKING_DISTANCE,
        player_center.y - 50.0f
    };
    
    // Keep flanking position on screen
    if (flank_pos.x < 50) flank_pos.x = 50;
    if (flank_pos.x > SCREEN_WIDTH - 50) flank_pos.x = SCREEN_WIDTH - 50;
    
    return flank_pos;
}

// Update swarm behavior for enemy
void UpdateSwarmBehavior(GameState* gameState, Enemy* enemy, float delta) {
    Vector2 swarm_center = {SCREEN_WIDTH / 2.0f, 120.0f};
    Vector2 to_center = {
        swarm_center.x - enemy->position.x,
        swarm_center.y - enemy->position.y
    };
    
    float distance_to_center = sqrtf(to_center.x * to_center.x + to_center.y * to_center.y);
    
    // Move towards swarm center but maintain some distance
    if (distance_to_center > AI_SWARM_RADIUS) {
        float pull_strength = 30.0f;
        enemy->position.x += (to_center.x / distance_to_center) * pull_strength * delta;
        enemy->position.y += (to_center.y / distance_to_center) * pull_strength * delta;
    } else {
        // Swarm behavior - move in circular pattern
        float angle = atan2f(enemy->position.y - swarm_center.y, enemy->position.x - swarm_center.x);
        angle += delta * 0.5f;
        
        enemy->position.x = swarm_center.x + cosf(angle) * AI_SWARM_RADIUS;
        enemy->position.y = swarm_center.y + sinf(angle) * AI_SWARM_RADIUS;
    }
}

// Function to spawn a new wave of enemies with enhanced AI
void SpawnEnemyWave(GameState* gameState) {
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
        
        // Update aggression scaling for new wave
        UpdateAggressionScaling(gameState);
        
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
            gameState->enemies[boss_index].aggression_multiplier = gameState->base_aggression;
            
            // Enhanced AI initialization for boss
            SetEnemyAIBehavior(&gameState->enemies[boss_index], AI_AGGRESSIVE_ATTACK);
            gameState->enemies[boss_index].coordinating = false;
            gameState->enemies[boss_index].coordination_group = 0;
            
            // Enhanced morphing and captured ship mechanics
            gameState->enemies[boss_index].can_morph = false;
            gameState->enemies[boss_index].has_morphed = false;
            gameState->enemies[boss_index].has_captured_ship = false;
            gameState->enemies[boss_index].captured_ship_hostile = false;
            gameState->enemies[boss_index].captured_ship_spawn_wave = 0;
            
            // Spawn escorts with enhanced AI
            for (int i = 0; i < 6; i++) {
                if (i == boss_index) continue;
                
                gameState->enemies[i].type = ESCORT;
                gameState->enemies[i].health = 1;
                gameState->enemies[i].formation_pos.x = SCREEN_WIDTH / 2.0f + (i - 3) * 80.0f;
                gameState->enemies[i].formation_pos.y = 150.0f;
                gameState->enemies[i].position.x = gameState->enemies[i].formation_pos.x;
                gameState->enemies[i].position.y = -ENEMY_SIZE + i * 30;
                gameState->enemies[i].entry_start = gameState->enemies[i].position;
                gameState->enemies[i].state = ENTERING;
                gameState->enemies[i].is_escort_in_combo = true;
                gameState->enemies[i].escort_group_id = escort_group_id;
                gameState->enemies[i].aggression_multiplier = gameState->base_aggression;
                
                // Enhanced AI behavior for escorts
                AIBehavior escort_behaviors[] = {AI_FORMATION_FLYING, AI_FLANKING_MANEUVER, AI_SWARM_BEHAVIOR};
                SetEnemyAIBehavior(&gameState->enemies[i], escort_behaviors[i % 3]);
                gameState->enemies[i].coordinating = false;
                gameState->enemies[i].coordination_group = 0;
                
                // Morphing mechanics for escorts
                gameState->enemies[i].can_morph = (rand() % 100) < MORPH_CHANCE;
                gameState->enemies[i].has_morphed = false;
                gameState->enemies[i].has_captured_ship = false;
                gameState->enemies[i].captured_ship_hostile = false;
                gameState->enemies[i].captured_ship_spawn_wave = 0;
                
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
            // Spawn normal wave with enhanced AI
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
                gameState->enemies[i].aggression_multiplier = gameState->base_aggression;
                
                // Enhanced AI behavior assignment
                AIBehavior behaviors[] = {AI_FORMATION_FLYING, AI_AGGRESSIVE_ATTACK, AI_DEFENSIVE_FORMATION, AI_SWARM_BEHAVIOR};
                SetEnemyAIBehavior(&gameState->enemies[i], behaviors[i % 4]);
                gameState->enemies[i].coordinating = false;
                gameState->enemies[i].coordination_group = 0;
                
                // Enhanced morphing mechanics
                gameState->enemies[i].can_morph = (rand() % 100) < MORPH_CHANCE;
                gameState->enemies[i].has_morphed = false;
                gameState->enemies[i].has_captured_ship = false;
                gameState->enemies[i].captured_ship_hostile = false;
                gameState->enemies[i].captured_ship_spawn_wave = 0;
                
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