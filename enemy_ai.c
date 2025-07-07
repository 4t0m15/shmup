#include "game.h"
#include <math.h>
#include <stdlib.h>

// =============================================================================
// AI UTILITY FUNCTIONS (defined first to avoid forward references)
// =============================================================================

// Update player position history for AI prediction
void UpdatePlayerPositionHistory(GameState* gameState) {
    if (!gameState) return;
    
    gameState->player_position_index = (gameState->player_position_index + 1) % AI_PREDICTION_FRAMES;
    gameState->player_positions[gameState->player_position_index] = (Vector2){
        gameState->player.rect.x + gameState->player.rect.width / 2,
        gameState->player.rect.y + gameState->player.rect.height / 2
    };
}

// Predict player position based on movement history
Vector2 PredictPlayerPosition(const GameState* gameState, float prediction_time) {
    if (!gameState || gameState->player_position_index < 2) {
        // Not enough history, return current position
        return (Vector2){
            gameState->player.rect.x + gameState->player.rect.width / 2,
            gameState->player.rect.y + gameState->player.rect.height / 2
        };
    }
    
    // Calculate velocity based on recent positions
    int current_index = gameState->player_position_index;
    int prev_index = (current_index - 1 + AI_PREDICTION_FRAMES) % AI_PREDICTION_FRAMES;
    
    Vector2 current_pos = gameState->player_positions[current_index];
    Vector2 prev_pos = gameState->player_positions[prev_index];
    
    Vector2 velocity = {
        (current_pos.x - prev_pos.x) * TARGET_FPS,
        (current_pos.y - prev_pos.y) * TARGET_FPS
    };
    
    // Predict future position
    return (Vector2){
        current_pos.x + velocity.x * prediction_time,
        current_pos.y + velocity.y * prediction_time
    };
}

// Calculate flanking position
Vector2 CalculateFlankingPosition(const GameState* gameState, const Enemy* enemy) {
    if (!gameState || !enemy) return (Vector2){0, 0};
    
    Vector2 player_pos = {
        gameState->player.rect.x + gameState->player.rect.width / 2,
        gameState->player.rect.y + gameState->player.rect.height / 2
    };
    
    // Calculate flanking angle
    float flank_angle = atan2f(player_pos.y - enemy->position.y, player_pos.x - enemy->position.x);
    flank_angle += PI / 2.0f; // Perpendicular to player direction
    
    return (Vector2){
        player_pos.x + cosf(flank_angle) * AI_FLANKING_DISTANCE,
        player_pos.y + sinf(flank_angle) * AI_FLANKING_DISTANCE
    };
}

// Check if enemy should evade
bool ShouldEnemyEvade(const GameState* gameState, const Enemy* enemy) {
    if (!gameState || !enemy) return false;
    
    Vector2 player_pos = {
        gameState->player.rect.x + gameState->player.rect.width / 2,
        gameState->player.rect.y + gameState->player.rect.height / 2
    };
    
    float distance = Vector2Distance(enemy->position, player_pos);
    return distance < AI_EVASION_THRESHOLD;
}

// Update swarm behavior
void UpdateSwarmBehavior(GameState* gameState, Enemy* enemy, float delta) {
    (void)delta; // Mark parameter as intentionally unused
    if (!gameState || !enemy) return;
    
    Vector2 swarm_center = {0, 0};
    int swarm_count = 0;
    
    // Find swarm center
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (gameState->enemies[i].active && gameState->enemies[i].ai_behavior == AI_SWARM_BEHAVIOR) {
            swarm_center.x += gameState->enemies[i].position.x;
            swarm_center.y += gameState->enemies[i].position.y;
            swarm_count++;
        }
    }
    
    if (swarm_count > 1) {
        swarm_center.x /= swarm_count;
        swarm_center.y /= swarm_count;
        
        // Calculate swarm position
        float angle = atan2f(enemy->position.y - swarm_center.y, enemy->position.x - swarm_center.x);
        Vector2 swarm_pos = {
            swarm_center.x + cosf(angle) * AI_SWARM_RADIUS,
            swarm_center.y + sinf(angle) * AI_SWARM_RADIUS
        };
        
        enemy->ai_target = swarm_pos;
    } else {
        // Single enemy, target player
        enemy->ai_target = (Vector2){
            gameState->player.rect.x + gameState->player.rect.width / 2,
            gameState->player.rect.y + gameState->player.rect.height / 2
        };
    }
}

// Update coordinated attack
void UpdateCoordinatedAttack(GameState* gameState, float delta) {
    (void)delta; // Mark parameter as intentionally unused
    if (!gameState) return;
    
    // Count coordinating enemies
    int coordinating_count = 0;
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (gameState->enemies[i].active && gameState->enemies[i].coordinating) {
            coordinating_count++;
        }
    }
    
    // If enough enemies are coordinating, trigger coordinated attack
    if (coordinating_count >= 3) {
        Vector2 target = PredictPlayerPosition(gameState, 1.5f);
        
        for (int i = 0; i < MAX_ENEMIES; i++) {
            if (gameState->enemies[i].active && gameState->enemies[i].coordinating) {
                gameState->enemies[i].ai_target = target;
                gameState->enemies[i].aggression_multiplier = 1.8f;
            }
        }
    }
}

// Set enemy AI behavior
void SetEnemyAIBehavior(Enemy* enemy, AIBehavior behavior) {
    if (!enemy) return;
    enemy->ai_behavior = behavior;
    enemy->ai_timer = 0.0f;
}

// =============================================================================
// AI BEHAVIOR FUNCTIONS
// =============================================================================

// Enhanced AI behavior update function
void UpdateEnemyBehavior(GameState* gameState, Enemy* enemy, float delta) {
    (void)delta; // Mark parameter as intentionally unused
    if (!gameState || !enemy) return;
    
    // Update AI target based on behavior
    switch (enemy->ai_behavior) {
        case AI_FORMATION_FLYING:
            enemy->ai_target = enemy->formation_pos;
            break;
            
        case AI_AGGRESSIVE_ATTACK:
            enemy->ai_target = PredictPlayerPosition(gameState, 1.0f);
            break;
            
        case AI_FLANKING_MANEUVER:
            enemy->ai_target = CalculateFlankingPosition(gameState, enemy);
            break;
            
        case AI_SWARM_BEHAVIOR:
            // Swarm behavior is handled in UpdateSwarmBehavior
            break;
            
        case AI_EVASIVE_MANEUVER: {
            // Calculate evasion target
            Vector2 player_pos = {
                gameState->player.rect.x + gameState->player.rect.width / 2,
                gameState->player.rect.y + gameState->player.rect.height / 2
            };
            
            Vector2 evasion_dir = {
                enemy->position.x - player_pos.x,
                enemy->position.y - player_pos.y
            };
            
            float length = sqrtf(evasion_dir.x * evasion_dir.x + evasion_dir.y * evasion_dir.y);
            if (length > 0) {
                evasion_dir.x /= length;
                evasion_dir.y /= length;
            }
            
            enemy->ai_target = (Vector2){
                enemy->position.x + evasion_dir.x * 100.0f,
                enemy->position.y + evasion_dir.y * 100.0f
            };
            break;
        }
            
        case AI_COORDINATED_ATTACK:
            // Coordinated attack target
            enemy->ai_target = PredictPlayerPosition(gameState, 2.0f);
            break;
            
        case AI_DEFENSIVE_FORMATION:
            // Defensive formation target
            enemy->ai_target = (Vector2){
                enemy->formation_pos.x,
                enemy->formation_pos.y - 50.0f
            };
            break;
    }
}

// =============================================================================
// MAIN AI UPDATE FUNCTION
// =============================================================================

// Function to update enemy AI
void UpdateEnemyAI(GameState* gameState, float delta) {
    if (!gameState) return;
    
    // Update player position history for prediction
    UpdatePlayerPositionHistory(gameState);
    
    // Update coordinated attacks
    UpdateCoordinatedAttack(gameState, delta);
    
    // Update individual enemy AI behaviors
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy* enemy = &gameState->enemies[i];
        
        if (!enemy->active) continue;
        
        // Update AI behavior
        UpdateEnemyBehavior(gameState, enemy, delta);
        
        // Check for behavior changes
        enemy->ai_timer += delta;
        if (enemy->ai_timer > 3.0f) {
            SetEnemyAIBehavior(enemy, (AIBehavior)(rand() % 7));
            enemy->ai_timer = 0.0f;
        }
        
        // Update enhanced AI behaviors
        switch (enemy->ai_behavior) {
            case AI_FORMATION_FLYING:
                // Standard formation behavior
                break;
                
            case AI_AGGRESSIVE_ATTACK:
                // More aggressive targeting
                enemy->aggression_multiplier = 1.5f;
                break;
                
            case AI_FLANKING_MANEUVER:
                // Flanking behavior
                enemy->ai_target = CalculateFlankingPosition(gameState, enemy);
                break;
                
            case AI_SWARM_BEHAVIOR:
                // Swarm coordination
                UpdateSwarmBehavior(gameState, enemy, delta);
                break;
                
            case AI_EVASIVE_MANEUVER:
                // Evasive behavior
                if (ShouldEnemyEvade(gameState, enemy)) {
                    enemy->evasion_direction = (rand() % 2 == 0) ? -1.0f : 1.0f;
                }
                break;
                
            case AI_COORDINATED_ATTACK:
                // Coordinated group attack
                enemy->coordinating = true;
                break;
                
            case AI_DEFENSIVE_FORMATION:
                // Defensive positioning
                enemy->aggression_multiplier = 0.7f;
                break;
        }
    }
    
    // Update aggression scaling
    UpdateAggressionScaling(gameState);
}