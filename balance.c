#include "game.h"
#include <math.h>
#include <stdlib.h>

// =============================================================================
// GAMEPLAY BALANCE AND DIFFICULTY SYSTEM
// =============================================================================

// Initialize balance system
void InitBalanceSystem(BalanceSystem* balance) {
    if (!balance) return;
    
    balance->difficulty_multiplier = 1.0f;
    balance->enemy_speed_multiplier = 1.0f;
    balance->enemy_health_multiplier = 1.0f;
    balance->player_damage_multiplier = 1.0f;
    balance->score_multiplier = 1.0f;
    balance->spawn_rate_multiplier = 1.0f;
    
    balance->adaptive_difficulty = true;
    balance->player_skill_rating = 0.5f;
    balance->recent_performance = 0.5f;
    balance->deaths_this_session = 0;
    balance->time_alive = 0.0f;
    balance->enemies_killed = 0;
    balance->accuracy_shots_fired = 0;
    balance->accuracy_shots_hit = 0;
    
    balance->power_level = 0;
    balance->combo_multiplier = 1.0f;
    balance->combo_timer = 0.0f;
    balance->consecutive_hits = 0;
    balance->max_combo = 0;
}

// Update dynamic difficulty based on player performance
void UpdateAdaptiveDifficulty(BalanceSystem* balance, GameState* gameState, float delta) {
    if (!balance || !balance->adaptive_difficulty || !gameState) return;
    
    balance->time_alive += delta;
    
    // Calculate accuracy
    float accuracy = 0.0f;
    if (balance->accuracy_shots_fired > 0) {
        accuracy = (float)balance->accuracy_shots_hit / balance->accuracy_shots_fired;
    }
    
    // Calculate kill rate (enemies per minute)
    float kill_rate = 0.0f;
    if (balance->time_alive > 0.0f) {
        kill_rate = (balance->enemies_killed * 60.0f) / balance->time_alive;
    }
    
    // Calculate survival score
    float survival_score = 1.0f;
    if (balance->deaths_this_session > 0) {
        survival_score = balance->time_alive / (balance->deaths_this_session * 60.0f);
    }
    
    // Update player skill rating
    float target_skill = (accuracy * 0.4f + (kill_rate / 10.0f) * 0.4f + (survival_score / 5.0f) * 0.2f);
    target_skill = Clamp(target_skill, 0.1f, 2.0f);
    
    // Smooth skill rating changes
    balance->player_skill_rating += (target_skill - balance->player_skill_rating) * delta * 0.1f;
    balance->player_skill_rating = Clamp(balance->player_skill_rating, 0.1f, 2.0f);
    
    // Update recent performance based on current wave performance
    float wave_performance = 1.0f;
    if (gameState->player.lives < STARTING_LIVES) {
        wave_performance = 0.5f; // Player took damage recently
    }
    if (gameState->wave_number > 1) {
        wave_performance += (float)gameState->wave_number * 0.1f; // Bonus for progression
    }
    
    balance->recent_performance = wave_performance * 0.3f + balance->recent_performance * 0.7f;
    balance->recent_performance = Clamp(balance->recent_performance, 0.1f, 2.0f);
    
    // Calculate final difficulty multiplier
    float base_difficulty = 1.0f + (gameState->wave_number * 0.05f);
    float skill_adjustment = 2.0f - balance->player_skill_rating; // Higher skill = harder game
    float performance_adjustment = 2.0f - balance->recent_performance; // Poor performance = easier game
    
    balance->difficulty_multiplier = base_difficulty * skill_adjustment * performance_adjustment;
    balance->difficulty_multiplier = Clamp(balance->difficulty_multiplier, 0.3f, 3.0f);
    
    // Apply difficulty to game systems
    balance->enemy_speed_multiplier = 0.8f + (balance->difficulty_multiplier * 0.4f);
    balance->enemy_health_multiplier = 0.7f + (balance->difficulty_multiplier * 0.6f);
    balance->spawn_rate_multiplier = 0.6f + (balance->difficulty_multiplier * 0.8f);
    
    // Player gets slight advantages when struggling
    if (balance->player_skill_rating < 0.3f) {
        balance->player_damage_multiplier = 1.2f;
        balance->score_multiplier = 1.3f; // More points to reach extends faster
    } else if (balance->player_skill_rating > 1.5f) {
        balance->player_damage_multiplier = 0.9f;
        balance->score_multiplier = 1.1f; // Expert players get bonus score
    } else {
        balance->player_damage_multiplier = 1.0f;
        balance->score_multiplier = 1.0f;
    }
}

// Update combo system
void UpdateComboSystem(BalanceSystem* balance, float delta) {
    if (!balance) return;
    
    // Update combo timer
    if (balance->combo_timer > 0.0f) {
        balance->combo_timer -= delta;
        if (balance->combo_timer <= 0.0f) {
            // Combo expired
            balance->consecutive_hits = 0;
            balance->combo_multiplier = 1.0f;
        }
    }
    
    // Calculate combo multiplier
    if (balance->consecutive_hits > 0) {
        balance->combo_multiplier = 1.0f + (balance->consecutive_hits * 0.1f);
        balance->combo_multiplier = Clamp(balance->combo_multiplier, 1.0f, 3.0f);
    }
    
    // Track max combo
    if (balance->consecutive_hits > balance->max_combo) {
        balance->max_combo = balance->consecutive_hits;
    }
}

// Register player shot
void RegisterPlayerShot(BalanceSystem* balance) {
    if (!balance) return;
    balance->accuracy_shots_fired++;
}

// Register successful hit
void RegisterHit(BalanceSystem* balance, bool was_enemy_killed) {
    if (!balance) return;
    
    balance->accuracy_shots_hit++;
    balance->consecutive_hits++;
    balance->combo_timer = 3.0f; // 3 seconds to maintain combo
    
    if (was_enemy_killed) {
        balance->enemies_killed++;
    }
}

// Register player death
void RegisterPlayerDeath(BalanceSystem* balance) {
    if (!balance) return;
    
    balance->deaths_this_session++;
    balance->consecutive_hits = 0;
    balance->combo_multiplier = 1.0f;
    balance->combo_timer = 0.0f;
}

// Calculate final score with multipliers
int CalculateScoreWithMultipliers(BalanceSystem* balance, int base_score) {
    if (!balance) return base_score;
    
    float final_score = base_score * balance->score_multiplier * balance->combo_multiplier;
    return (int)final_score;
}

// Get difficulty-adjusted enemy health
int GetAdjustedEnemyHealth(BalanceSystem* balance, EnemyType type) {
    if (!balance) return 1;
    
    int base_health = 1;
    switch (type) {
        case BOSS: base_health = 5; break;
        case ESCORT: base_health = 2; break;
        case FLAGSHIP: base_health = 3; break;
        case HOSTILE_SHIP: base_health = 1; break;
        default: base_health = 1; break;
    }
    
    float adjusted = base_health * balance->enemy_health_multiplier;
    return (int)Clamp(adjusted, 1.0f, 20.0f);
}

// Get difficulty-adjusted enemy speed
float GetAdjustedEnemySpeed(BalanceSystem* balance, float base_speed) {
    if (!balance) return base_speed;
    
    return base_speed * balance->enemy_speed_multiplier;
}

// =============================================================================
// POWER-UP SYSTEM
// =============================================================================

// Initialize power-up system
void InitPowerUpSystem(PowerUpSystem* powerups) {
    if (!powerups) return;
    
    for (int i = 0; i < MAX_POWERUPS; i++) {
        powerups->powerups[i].active = false;
        powerups->powerups[i].type = POWERUP_RAPID_FIRE;
        powerups->powerups[i].position = (Vector2){0, 0};
        powerups->powerups[i].velocity = (Vector2){0, 50};
        powerups->powerups[i].timer = 10.0f;
        powerups->powerups[i].pulse_timer = 0.0f;
    }
    
    powerups->spawn_timer = 15.0f; // First powerup in 15 seconds
    powerups->rapid_fire_timer = 0.0f;
    powerups->shield_timer = 0.0f;
    powerups->spread_shot_timer = 0.0f;
    powerups->slow_motion_timer = 0.0f;
}

// Spawn a power-up
void SpawnPowerUp(PowerUpSystem* powerups, Vector2 position, PowerUpType type) {
    if (!powerups) return;
    
    // Find inactive powerup slot
    for (int i = 0; i < MAX_POWERUPS; i++) {
        if (!powerups->powerups[i].active) {
            powerups->powerups[i].active = true;
            powerups->powerups[i].type = type;
            powerups->powerups[i].position = position;
            powerups->powerups[i].velocity = (Vector2){0, 30};
            powerups->powerups[i].timer = 15.0f; // 15 seconds to collect
            powerups->powerups[i].pulse_timer = 0.0f;
            break;
        }
    }
}

// Update power-up system
void UpdatePowerUpSystem(PowerUpSystem* powerups, GameState* gameState, float delta) {
    if (!powerups || !gameState) return;
    
    // Update spawn timer
    powerups->spawn_timer -= delta;
    if (powerups->spawn_timer <= 0.0f) {
        // Spawn random powerup at random position
        PowerUpType type = (PowerUpType)(rand() % POWERUP_COUNT);
        Vector2 spawn_pos = {
            (float)(rand() % (SCREEN_WIDTH - 60)) + 30,
            -20
        };
        SpawnPowerUp(powerups, spawn_pos, type);
        powerups->spawn_timer = 20.0f + (rand() % 20); // 20-40 seconds between spawns
    }
    
    // Update active power-ups
    for (int i = 0; i < MAX_POWERUPS; i++) {
        if (!powerups->powerups[i].active) continue;
        
        PowerUp* powerup = &powerups->powerups[i];
        
        // Update position
        powerup->position.x += powerup->velocity.x * delta;
        powerup->position.y += powerup->velocity.y * delta;
        
        // Update timers
        powerup->timer -= delta;
        powerup->pulse_timer += delta;
        
        // Remove if expired or off screen
        if (powerup->timer <= 0.0f || powerup->position.y > SCREEN_HEIGHT + 20) {
            powerup->active = false;
            continue;
        }
        
        // Check collision with player
        Rectangle powerup_rect = {
            powerup->position.x - 15,
            powerup->position.y - 15,
            30, 30
        };
        
        Rectangle player_rect = gameState->player.has_captured_ship ? 
            gameState->player.dual_hitbox : gameState->player.rect;
            
        if (CheckCollisionRecs(powerup_rect, player_rect)) {
            // Collected!
            CollectPowerUp(powerups, gameState, powerup->type);
            powerup->active = false;
            
            // Create collection effect
            CreatePowerUpEffect(&gameState->effects, powerup->position);
            PlayGameSound(&gameState->audio, GAME_SOUND_POWERUP, 1.0f);
        }
    }
    
    // Update active power-up effects
    if (powerups->rapid_fire_timer > 0.0f) {
        powerups->rapid_fire_timer -= delta;
    }
    
    if (powerups->shield_timer > 0.0f) {
        powerups->shield_timer -= delta;
    }
    
    if (powerups->spread_shot_timer > 0.0f) {
        powerups->spread_shot_timer -= delta;
    }
    
    if (powerups->slow_motion_timer > 0.0f) {
        powerups->slow_motion_timer -= delta;
    }
}

// Collect a power-up
void CollectPowerUp(PowerUpSystem* powerups, GameState* gameState, PowerUpType type) {
    if (!powerups || !gameState) return;
    
    switch (type) {
        case POWERUP_RAPID_FIRE:
            powerups->rapid_fire_timer = 10.0f;
            break;
            
        case POWERUP_SHIELD:
            powerups->shield_timer = 15.0f;
            break;
            
        case POWERUP_SPREAD_SHOT:
            powerups->spread_shot_timer = 8.0f;
            break;
            
        case POWERUP_SLOW_MOTION:
            powerups->slow_motion_timer = 6.0f;
            break;
            
        case POWERUP_EXTRA_LIFE:
            if (gameState->player.lives < MAX_LIVES) {
                gameState->player.lives++;
            }
            break;
            
        case POWERUP_SCORE_MULTIPLIER:
            // Award bonus points
            AddScore(gameState, 5000, (Vector2){
                gameState->player.rect.x + gameState->player.rect.width / 2,
                gameState->player.rect.y
            });
            break;
    }
}

// Check if power-up is active
bool IsPowerUpActive(PowerUpSystem* powerups, PowerUpType type) {
    if (!powerups) return false;
    
    switch (type) {
        case POWERUP_RAPID_FIRE: return powerups->rapid_fire_timer > 0.0f;
        case POWERUP_SHIELD: return powerups->shield_timer > 0.0f;
        case POWERUP_SPREAD_SHOT: return powerups->spread_shot_timer > 0.0f;
        case POWERUP_SLOW_MOTION: return powerups->slow_motion_timer > 0.0f;
        default: return false;
    }
}

// Get power-up time remaining
float GetPowerUpTimeRemaining(PowerUpSystem* powerups, PowerUpType type) {
    if (!powerups) return 0.0f;
    
    switch (type) {
        case POWERUP_RAPID_FIRE: return powerups->rapid_fire_timer;
        case POWERUP_SHIELD: return powerups->shield_timer;
        case POWERUP_SPREAD_SHOT: return powerups->spread_shot_timer;
        case POWERUP_SLOW_MOTION: return powerups->slow_motion_timer;
        default: return 0.0f;
    }
}

// =============================================================================
// QUALITY OF LIFE FEATURES
// =============================================================================

// Initialize quality of life system
void InitQoLSystem(QoLSystem* qol) {
    if (!qol) return;
    
    qol->auto_pause_on_focus_loss = true;
    qol->show_hit_indicators = true;
    qol->show_damage_numbers = true;
    qol->screen_edge_warning = true;
    qol->bullet_time_on_near_miss = true;
    qol->auto_collect_powerups = false;
    
    qol->near_miss_timer = 0.0f;
    qol->focus_lost = false;
    qol->edge_warning_timer = 0.0f;
}

// Update quality of life features
void UpdateQoLSystem(QoLSystem* qol, GameState* gameState, float delta) {
    if (!qol || !gameState) return;
    
    // Auto-pause on focus loss
    if (qol->auto_pause_on_focus_loss && !IsWindowFocused() && !qol->focus_lost) {
        qol->focus_lost = true;
        if (gameState->screen_state == PLAYING && !gameState->is_paused) {
            gameState->is_paused = true;
        }
    } else if (IsWindowFocused() && qol->focus_lost) {
        qol->focus_lost = false;
    }
    
    // Update near miss effect
    if (qol->near_miss_timer > 0.0f) {
        qol->near_miss_timer -= delta;
    }
    
    // Screen edge warning
    if (qol->screen_edge_warning) {
        bool near_edge = false;
        if (gameState->player.rect.x < 50 || 
            gameState->player.rect.x + gameState->player.rect.width > SCREEN_WIDTH - 50 ||
            gameState->player.rect.y < 50 || 
            gameState->player.rect.y + gameState->player.rect.height > SCREEN_HEIGHT - 50) {
            near_edge = true;
        }
        
        if (near_edge) {
            qol->edge_warning_timer += delta;
        } else {
            qol->edge_warning_timer = 0.0f;
        }
    }
    
    // Check for near misses
    if (qol->bullet_time_on_near_miss) {
        CheckForNearMisses(qol, gameState);
    }
}

// Check for near miss situations
void CheckForNearMisses(QoLSystem* qol, GameState* gameState) {
    if (!qol || !gameState) return;
    
    Vector2 player_center = {
        gameState->player.rect.x + gameState->player.rect.width / 2,
        gameState->player.rect.y + gameState->player.rect.height / 2
    };
    
    float near_miss_distance = 25.0f;
    
    // Check enemy bullets
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        if (!gameState->enemy_bullets[i].active) continue;
        
        float distance = Vector2Distance(player_center, gameState->enemy_bullets[i].position);
        if (distance < near_miss_distance && distance > 10.0f) {
            // Near miss!
            qol->near_miss_timer = 0.5f;
            TriggerScreenFlash(&gameState->effects, (Color){255, 255, 255, 50}, 0.1f);
            break;
        }
    }
}

// Draw quality of life UI elements
void DrawQoLUI(const QoLSystem* qol, const GameState* gameState) {
    if (!qol || !gameState) return;
    
    // Screen edge warning
    if (qol->screen_edge_warning && qol->edge_warning_timer > 0.5f) {
        Color warning_color = {255, 0, 0, (unsigned char)(100 + 100 * sinf(qol->edge_warning_timer * 10.0f))};
        
        // Draw warning borders
        DrawRectangle(0, 0, SCREEN_WIDTH, 5, warning_color);
        DrawRectangle(0, SCREEN_HEIGHT - 5, SCREEN_WIDTH, 5, warning_color);
        DrawRectangle(0, 0, 5, SCREEN_HEIGHT, warning_color);
        DrawRectangle(SCREEN_WIDTH - 5, 0, 5, SCREEN_HEIGHT, warning_color);
    }
    
    // Near miss effect
    if (qol->near_miss_timer > 0.0f) {
        float alpha = qol->near_miss_timer / 0.5f;
        Color effect_color = {255, 255, 255, (unsigned char)(alpha * 100)};
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, effect_color);
        
        DrawText("NEAR MISS!", SCREEN_WIDTH / 2 - 60, SCREEN_HEIGHT / 2 - 20, 20, YELLOW);
    }
    
    // Focus loss indicator
    if (qol->focus_lost && gameState->is_paused) {
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0, 0, 0, 100});
        DrawText("WINDOW FOCUS LOST", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 40, 20, WHITE);
        DrawText("Click to resume", SCREEN_WIDTH / 2 - 70, SCREEN_HEIGHT / 2 - 10, 16, GRAY);
    }
}

// Draw power-ups
void DrawPowerUps(const PowerUpSystem* powerups) {
    if (!powerups) return;
    
    for (int i = 0; i < MAX_POWERUPS; i++) {
        if (!powerups->powerups[i].active) continue;
        
        const PowerUp* powerup = &powerups->powerups[i];
        
        // Pulsing effect
        float pulse = 0.8f + 0.2f * sinf(powerup->pulse_timer * 8.0f);
        
        // Color based on type
        Color powerup_color = WHITE;
        const char* symbol = "?";
        
        switch (powerup->type) {
            case POWERUP_RAPID_FIRE:
                powerup_color = ORANGE;
                symbol = "R";
                break;
            case POWERUP_SHIELD:
                powerup_color = BLUE;
                symbol = "S";
                break;
            case POWERUP_SPREAD_SHOT:
                powerup_color = GREEN;
                symbol = "T";
                break;
            case POWERUP_SLOW_MOTION:
                powerup_color = PURPLE;
                symbol = "M";
                break;
            case POWERUP_EXTRA_LIFE:
                powerup_color = RED;
                symbol = "L";
                break;
            case POWERUP_SCORE_MULTIPLIER:
                powerup_color = GOLD;
                symbol = "$";
                break;
        }
        
        powerup_color.r = (unsigned char)(powerup_color.r * pulse);
        powerup_color.g = (unsigned char)(powerup_color.g * pulse);
        powerup_color.b = (unsigned char)(powerup_color.b * pulse);
        
        // Draw powerup
        DrawCircleV(powerup->position, 15, powerup_color);
        DrawCircleLinesV(powerup->position, 15, WHITE);
        DrawText(symbol, (int)powerup->position.x - 5, (int)powerup->position.y - 8, 16, BLACK);
        
        // Draw timer warning
        if (powerup->timer < 3.0f) {
            float warning_alpha = sinf(powerup->timer * 10.0f);
            Color warning_color = {255, 0, 0, (unsigned char)(warning_alpha * 255)};
            DrawCircleLinesV(powerup->position, 18, warning_color);
        }
    }
}