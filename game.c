// Disable deprecation warnings for MSVC
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "game.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

// =============================================================================
// FORWARD DECLARATIONS (to resolve circular dependencies)
// =============================================================================

static void UpdateGamePlaying(GameState* gameState, float delta);
static void DrawGamePlaying(const GameState* gameState);

// =============================================================================
// CONFIGURATION
// =============================================================================

const GameConfig DEFAULT_CONFIG = {
    .starting_lives = STARTING_LIVES,
    .boss_wave_interval = 6, // Updated to reflect new 7-wave cycle (boss on wave 6)
    .base_aggression = 1.0f,
    .morph_chance_percentage = MORPH_CHANCE,
    .player_start_position = {SCREEN_WIDTH / 2.0f - PLAYER_SIZE / 2.0f, SCREEN_HEIGHT - 80.0f},
    .enable_dual_fighter = true,
    .first_extend_score = FIRST_EXTEND_SCORE,
    .second_extend_score = SECOND_EXTEND_SCORE,
    .max_lives = MAX_LIVES,
    .enable_morphing = true,
    .enable_captured_ships = true,
    .enable_bonus_stages = true,
    .enable_aggression_scaling = true,
    .enable_enhanced_ai = true
};

// =============================================================================
// INITIALIZATION FUNCTIONS
// =============================================================================

void InitializeBullets(GameState* gameState) {
    if (!gameState) return;
    
    for (int i = 0; i < MAX_BULLETS; i++) {
        gameState->bullets[i].active = false;
        gameState->bullets[i].position = (Vector2){0, 0};
    }
}

void InitializeEnemyBullets(GameState* gameState) {
    if (!gameState) return;
    
    for (int i = 0; i < MAX_ENEMY_BULLETS; i++) {
        gameState->enemy_bullets[i].active = false;
        gameState->enemy_bullets[i].position = (Vector2){0, 0};
        gameState->enemy_bullets[i].velocity = (Vector2){0, 0};
    }
}

void InitializePlayer(GameState* gameState) {
    if (!gameState) return;
    
    Player* player = &gameState->player;
    
    // Calculate player starting position
    const float player_start_x = SCREEN_WIDTH / 2.0f - PLAYER_SIZE / 2.0f;
    const float player_start_y = SCREEN_HEIGHT - 80.0f;
    
    // Initialize player rectangle
    player->rect = (Rectangle){player_start_x, player_start_y, PLAYER_SIZE, PLAYER_SIZE};
    
    // Initialize player properties
    player->color = BLUE;
    player->captured = false;
    player->dual_fire = false;
    player->lives = STARTING_LIVES;
    player->extend_1_awarded = false;
    player->extend_2_awarded = false;
    
    // Initialize enhanced dual fighter mechanics
    player->has_captured_ship = false;
    player->captured_ship_offset = (Vector2){0, 0};
    player->dual_hitbox = player->rect;
    player->dual_fighter_timer = 0.0f;
    player->capture_target = (Vector2){0, 0};
}

void InitializeEnemies(GameState* gameState) {
    if (!gameState) return;
    
    for (int i = 0; i < MAX_ENEMIES; i++) {
        Enemy* enemy = &gameState->enemies[i];
        
        // Initialize basic state
        enemy->active = false;
        enemy->state = INACTIVE;
        enemy->type = NORMAL;
        enemy->health = 1;
        enemy->shooting = false;
        enemy->tractor_active = false;
        enemy->is_escort_in_combo = false;
        enemy->escort_group_id = 0;
        
        // Initialize timing and progress
        enemy->timer = 0.0f;
        enemy->pattern_progress = 0.0f;
        enemy->pattern_param = 0.0f;
        enemy->shoot_timer = 0.0f;
        enemy->tractor_angle = 0.0f;
        enemy->pattern = PATTERN_STRAIGHT;
        
        // Initialize positions
        enemy->position = (Vector2){0, 0};
        enemy->formation_pos = (Vector2){0, 0};
        enemy->entry_start = (Vector2){0, 0};
        enemy->attack_start = (Vector2){0, 0};
        enemy->tractor_center = (Vector2){0, 0};
        
        // Initialize morphing mechanics
        enemy->original_type = NORMAL;
        enemy->target_type = NORMAL;
        enemy->morph_timer = 0.0f;
        enemy->can_morph = (rand() % 100) < MORPH_CHANCE;
        enemy->has_morphed = false;
        
        // Initialize captured ship mechanics
        enemy->has_captured_ship = false;
        enemy->captured_ship_hostile = false;
        enemy->captured_ship_spawn_wave = 0;
        
        // Initialize difficulty scaling
        enemy->aggression_multiplier = 1.0f;
        
        // Initialize enhanced AI
        enemy->ai_behavior = AI_FORMATION_FLYING;
        enemy->ai_timer = 0.0f;
        enemy->ai_target = (Vector2){0, 0};
        enemy->predicted_player_pos = (Vector2){0, 0};
        enemy->last_player_distance = 0.0f;
        enemy->coordinating = false;
        enemy->coordination_group = 0;
        enemy->evasion_direction = 0.0f;
        enemy->last_velocity = (Vector2){0, 0};
    }
}

void InitializeScorePopups(GameState* gameState) {
    if (!gameState) return;
    
    for (int i = 0; i < 10; i++) {
        gameState->score_popups[i].active = false;
        gameState->score_popups[i].position = (Vector2){0, 0};
        gameState->score_popups[i].score = 0;
        gameState->score_popups[i].timer = 0.0f;
    }
}

void InitializeCapturedShips(GameState* gameState) {
    if (!gameState) return;
    
    for (int i = 0; i < MAX_CAPTURED_SHIPS; i++) {
        gameState->captured_ships[i].active = false;
        gameState->captured_ships[i].hostile = false;
        gameState->captured_ships[i].spawn_wave = 0;
        gameState->captured_ships[i].rescued = false;
        gameState->captured_ships[i].position = (Vector2){0, 0};
    }
    gameState->total_captured_ships = 0;
}

void InitializeGameVariables(GameState* gameState) {
    if (!gameState) return;
    
    // Initialize core game state
    gameState->wave_number = 0;
    gameState->wave_timer = 0.0f;
    gameState->boss_wave_interval = 6; // Updated to match the new 7-wave cycle system
    gameState->backgroundScrollY = 0.0f;
    gameState->shootCooldown = 0.0f;
    
    // Initialize scoring
    gameState->score = 0;
    gameState->high_score = 0;
    
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
    gameState->screen_state = MENU;
    gameState->game_over_timer = 0.0f;
    
    // Initialize advanced mechanics
    gameState->base_aggression = 1.0f;
    gameState->random_seed = (unsigned int)time(NULL);
    srand(gameState->random_seed);
    
    // Initialize systems
    InitMenu(&gameState->menu);
    
    // Initialize player tracking for AI
    for (int i = 0; i < AI_PREDICTION_FRAMES; i++) {
        gameState->player_positions[i] = (Vector2){0, 0};
    }
    gameState->player_position_index = 0;
    
    // Initialize pause system
    gameState->is_paused = false;
    gameState->pause_timer = 0.0f;
}

// =============================================================================
// CORE GAME FUNCTIONS
// =============================================================================

void InitGameWithConfig(GameState* gameState, const GameConfig* config) {
    if (!gameState) return;
    
    // Initialize all subsystems
    InitializeBullets(gameState);
    InitializeEnemyBullets(gameState);
    InitializePlayer(gameState);
    InitializeEnemies(gameState);
    InitializeScorePopups(gameState);
    InitializeCapturedShips(gameState);
    InitializeGameVariables(gameState);
    
    // Apply configuration if provided
    if (config) {
        gameState->player.lives = config->starting_lives;
        gameState->boss_wave_interval = config->boss_wave_interval;
        gameState->base_aggression = config->base_aggression;
        gameState->player.rect.x = config->player_start_position.x;
        gameState->player.rect.y = config->player_start_position.y;
    }
    
    // Load high score
    LoadHighScore(gameState);
}

void InitGame(GameState* gameState) {
    InitGameWithConfig(gameState, &DEFAULT_CONFIG);
}

bool ValidateGameState(const GameState* gameState) {
    if (!gameState) return false;
    
    // Validate player state
    if (gameState->player.lives < 0 || gameState->player.lives > MAX_LIVES) {
        return false;
    }
    
    // Validate wave progression
    if (gameState->wave_number < 0 || gameState->wave_number > 9999) {
        return false;
    }
    
    // Validate score
    if (gameState->score < 0 || gameState->score > 999999999) {
        return false;
    }
    
    // Validate screen state
    if (gameState->screen_state < MENU || gameState->screen_state > GAME_OVER) {
        return false;
    }
    
    return true;
}

void ResetGameState(GameState* gameState) {
    if (!gameState) return;
    InitGame(gameState);
}

// =============================================================================
// GAME OVER FUNCTIONS (Moved before UpdateGame to avoid forward references)
// =============================================================================

void HandleGameOver(GameState* gameState) {
    if (!gameState) return;
    
    gameState->screen_state = GAME_OVER;
    gameState->game_over_timer = 0.0f;
    
    // Save high score if achieved
    if (gameState->score > gameState->high_score) {
        gameState->high_score = gameState->score;
        SaveHighScore(gameState);
    }
}

void UpdateGameOver(GameState* gameState, float delta) {
    if (!gameState) return;
    
    gameState->game_over_timer += delta;
    
    // Allow input after delay
    if (gameState->game_over_timer > 2.0f) {
        if (IsKeyPressed(KEY_SPACE)) {
            InitGame(gameState);
            gameState->screen_state = PLAYING;
        } else if (IsKeyPressed(KEY_ESCAPE)) {
            gameState->screen_state = MENU;
            gameState->menu.current_menu = MAIN_MENU;
            gameState->menu.selected_option = 0;
        }
    }
}

void DrawGameOver(const GameState* gameState) {
    if (!gameState) return;
    
    // Draw background
    DrawBackground(gameState);
    
    // Draw game over overlay
    DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, (Color){0, 0, 0, 180});
    
    // Draw game over text
    DrawText("GAME OVER", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 - 80, 40, RED);
    DrawText(TextFormat("Final Score: %d", gameState->score), SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT / 2 - 20, 20, WHITE);
    DrawText(TextFormat("High Score: %d", gameState->high_score), SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT / 2 + 5, 20, WHITE);
    DrawText(TextFormat("Wave Reached: %d", gameState->wave_number), SCREEN_WIDTH / 2 - 80, SCREEN_HEIGHT / 2 + 30, 20, WHITE);
    
    // Draw restart instructions
    if (gameState->game_over_timer > 2.0f) {
        DrawText("Press SPACE to restart", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 70, 20, WHITE);
        DrawText("Press ESC for main menu", SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT / 2 + 95, 20, WHITE);
    }
}

// =============================================================================
// GAME UPDATE FUNCTIONS
// =============================================================================

static void UpdateGamePlaying(GameState* gameState, float delta) {
    if (!gameState) return;
    
    // Handle pause toggle
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) {
        gameState->is_paused = !gameState->is_paused;
        gameState->pause_timer = 0.0f;
    }
    
    // Skip updates if paused
    if (gameState->is_paused) {
        gameState->pause_timer += delta;
        return;
    }
    
    // Update game systems
    UpdatePlayer(gameState, delta);
    UpdateEnemyAI(gameState, delta);
    UpdateEnemies(gameState, delta);
    UpdateEnemyBullets(gameState, delta);
    
    // Update player bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (gameState->bullets[i].active) {
            gameState->bullets[i].position.y -= BULLET_SPEED * delta;
            
            // Remove bullets that go off screen
            if (gameState->bullets[i].position.y < -BULLET_SIZE) {
                gameState->bullets[i].active = false;
            }
        }
    }
    
    // Check collisions
    CheckBulletEnemyCollisions(gameState);
    
    // Handle player-enemy bullet collisions
    if (CheckEnemyBulletPlayerCollisions(gameState)) {
        gameState->player.lives--;
        if (gameState->player.lives <= 0) {
            HandleGameOver(gameState);
        } else {
            // Reset player position
            gameState->player.rect.x = SCREEN_WIDTH / 2.0f - PLAYER_SIZE / 2.0f;
            gameState->player.rect.y = SCREEN_HEIGHT - 80.0f;
        }
    }
    
    // Handle player-enemy collisions
    if (CheckPlayerEnemyCollisions(gameState)) {
        gameState->player.lives--;
        if (gameState->player.lives <= 0) {
            HandleGameOver(gameState);
        } else {
            // Reset player position
            gameState->player.rect.x = SCREEN_WIDTH / 2.0f - PLAYER_SIZE / 2.0f;
            gameState->player.rect.y = SCREEN_HEIGHT - 80.0f;
        }
    }
    
    // Update background scroll
    gameState->backgroundScrollY += BACKGROUND_SCROLL_SPEED * delta;
    if (gameState->backgroundScrollY >= SCREEN_HEIGHT) {
        gameState->backgroundScrollY = 0.0f;
    }
    
    // Update game systems
    UpdateScorePopups(gameState, delta);
    CheckForExtends(gameState);
    SpawnEnemyWave(gameState);
    
    // Update bonus stage
    if (gameState->is_bonus_stage) {
        UpdateBonusStage(gameState, delta);
    }
    
    // Reset player color after hit
    if (gameState->player.color.r > 0 && gameState->player.color.g < 255) {
        gameState->player.color = BLUE;
    }
}

void UpdateGame(GameState* gameState, float delta) {
    if (!gameState || !ValidateGameState(gameState)) return;
    
    switch (gameState->screen_state) {
        case MENU:
            UpdateMenu(gameState, delta);
            break;
            
        case PLAYING:
            UpdateGamePlaying(gameState, delta);
            break;
            
        case GAME_OVER:
            UpdateGameOver(gameState, delta);
            break;
            

    }
}

// =============================================================================
// GAME DRAWING FUNCTIONS
// =============================================================================

static void DrawGamePlaying(const GameState* gameState) {
    if (!gameState) return;
    
    // Draw background
    DrawBackground(gameState);
    
    // Draw game objects
    DrawPlayer(gameState);
    DrawBullets(gameState);
    DrawEnemies(gameState);
    
    // Draw UI
    DrawUI(gameState);
    
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
    }
}

void DrawGame(const GameState* gameState) {
    if (!gameState) return;
    
    ClearBackground(BLACK);
    
    switch (gameState->screen_state) {
        case MENU:
            DrawMenu(gameState);
            break;
            
        case PLAYING:
            DrawGamePlaying(gameState);
            break;
            
        case GAME_OVER:
            DrawGameOver(gameState);
            break;
            

    }
}
