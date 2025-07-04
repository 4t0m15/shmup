#include "game.h"
#include <stdio.h>

// Load high score from file
void LoadHighScore(GameState* gameState) {
    FILE* file = fopen("highscore.txt", "r");
    if (file) {
        fscanf(file, "%d", &gameState->high_score);
        fclose(file);
    }
}

// Save high score to file
void SaveHighScore(const GameState* gameState) {
    FILE* file = fopen("highscore.txt", "w");
    if (file) {
        fprintf(file, "%d", gameState->high_score);
        fclose(file);
    }
}

// Add score with visual popup
void AddScore(GameState* gameState, int points, Vector2 position) {
    gameState->score += points;
    
    // Find an inactive score popup
    for (int i = 0; i < 10; i++) {
        if (!gameState->score_popups[i].active) {
            gameState->score_popups[i].active = true;
            gameState->score_popups[i].position = position;
            gameState->score_popups[i].score = points;
            gameState->score_popups[i].timer = 2.0f;
            break;
        }
    }
}

// Check for life extends
void CheckForExtends(GameState* gameState) {
    if (!gameState->player.extend_1_awarded && gameState->score >= FIRST_EXTEND_SCORE) {
        gameState->player.extend_1_awarded = true;
        if (gameState->player.lives < MAX_LIVES) {
            gameState->player.lives++;
        }
    }
    
    if (!gameState->player.extend_2_awarded && gameState->score >= SECOND_EXTEND_SCORE) {
        gameState->player.extend_2_awarded = true;
        if (gameState->player.lives < MAX_LIVES) {
            gameState->player.lives++;
        }
    }
}

// Calculate score for enemy type
int CalculateEnemyScore(const Enemy* enemy) {
    switch (enemy->type) {
        case NORMAL:
            return (enemy->state == FORMATION) ? SCORE_BEE_FORMATION : SCORE_BEE_DIVE;
        case ESCORT:
            return (enemy->state == FORMATION) ? SCORE_BUTTERFLY_FORMATION : SCORE_BUTTERFLY_DIVE;
        case BOSS:
            return (enemy->state == FORMATION) ? SCORE_BOSS_FORMATION : SCORE_BOSS_DIVE;
        case FLAGSHIP:
            return (enemy->state == FORMATION) ? SCORE_FLAGSHIP_FORMATION : SCORE_FLAGSHIP_DIVE;
        case HOSTILE_SHIP:
            return SCORE_HOSTILE_SHIP_RESCUE;
        default:
            return 100;
    }
}

// Handle enemy destruction
void HandleEnemyDestroy(GameState* gameState, int enemy_index, Vector2 position) {
    Enemy* enemy = &gameState->enemies[enemy_index];
    
    // Calculate score
    int score = CalculateEnemyScore(enemy);
    
    // Handle ship rescue if boss has captured ship
    if (enemy->type == BOSS && enemy->has_captured_ship) {
        HandleShipRescue(gameState, enemy);
    }
    
    // Add score
    AddScore(gameState, score, position);
    
    // Deactivate enemy
    enemy->active = false;
}

// Update score popups
void UpdateScorePopups(GameState* gameState, float delta) {
    for (int i = 0; i < 10; i++) {
        if (gameState->score_popups[i].active) {
            gameState->score_popups[i].timer -= delta;
            gameState->score_popups[i].position.y -= 50.0f * delta;
            
            if (gameState->score_popups[i].timer <= 0.0f) {
                gameState->score_popups[i].active = false;
            }
        }
    }
}

// Spawn bonus stage
void SpawnBonusStage(GameState* gameState) {
    gameState->is_bonus_stage = true;
    gameState->bonus_stage_enemies_hit = 0;
    gameState->bonus_stage_total_enemies = 40;
    gameState->bonus_stage_timer = 30.0f; // 30 seconds
    
    // Spawn bonus enemies in formation
    for (int i = 0; i < 8; i++) {
        gameState->enemies[i].type = NORMAL;
        gameState->enemies[i].health = 1;
        gameState->enemies[i].formation_pos.x = 100.0f + i * 80.0f;
        gameState->enemies[i].formation_pos.y = 100.0f;
        gameState->enemies[i].position.x = gameState->enemies[i].formation_pos.x;
        gameState->enemies[i].position.y = -50.0f;
        gameState->enemies[i].entry_start = gameState->enemies[i].position;
        gameState->enemies[i].state = ENTERING;
        gameState->enemies[i].pattern = PATTERN_STRAIGHT;
        gameState->enemies[i].pattern_progress = 0.0f;
        gameState->enemies[i].pattern_param = 0.0f;
        gameState->enemies[i].timer = 0.0f;
        gameState->enemies[i].active = true;
        gameState->enemies[i].shooting = false;
        gameState->enemies[i].tractor_active = false;
        gameState->enemies[i].is_escort_in_combo = false;
        gameState->enemies[i].escort_group_id = 0;
        gameState->enemies[i].can_morph = false;
        gameState->enemies[i].has_morphed = false;
        gameState->enemies[i].aggression_multiplier = 1.0f;
    }
}

// Update bonus stage
void UpdateBonusStage(GameState* gameState, float delta) {
    gameState->bonus_stage_timer -= delta;
    
    if (gameState->bonus_stage_timer <= 0.0f || gameState->bonus_stage_enemies_hit >= gameState->bonus_stage_total_enemies) {
        // End bonus stage
        gameState->is_bonus_stage = false;
        
        // Award bonus score
        int bonus_score = SCORE_BONUS_STAGE_BASE;
        if (gameState->bonus_stage_enemies_hit == gameState->bonus_stage_total_enemies) {
            bonus_score = SCORE_BONUS_STAGE_PERFECT;
        } else if (gameState->bonus_stage_enemies_hit >= 39) {
            bonus_score = SCORE_BONUS_STAGE_39;
        } else if (gameState->bonus_stage_enemies_hit >= 38) {
            bonus_score = SCORE_BONUS_STAGE_38;
        } else if (gameState->bonus_stage_enemies_hit >= 37) {
            bonus_score = SCORE_BONUS_STAGE_37;
        } else if (gameState->bonus_stage_enemies_hit >= 36) {
            bonus_score = SCORE_BONUS_STAGE_36;
        }
        
        AddScore(gameState, bonus_score, (Vector2){SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2});
        
        // Deactivate all enemies
        for (int i = 0; i < MAX_ENEMIES; i++) {
            gameState->enemies[i].active = false;
        }
    }
}