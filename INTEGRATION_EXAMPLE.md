# Integration Example - Adding New Systems to Galactic Shmup

This document provides practical examples of how to integrate the new weapon, achievement, and shader systems into your existing game.

## 1. Game State Initialization

Update your `InitGame()` function in `game.c`:

```c
void InitGame(GameState* gameState) {
    // Existing initialization...
    InitializeBullets(gameState);
    InitializeEnemyBullets(gameState);
    InitializePlayer(gameState);
    InitializeEnemies(gameState);
    InitializeScorePopups(gameState);
    InitializeCapturedShips(gameState);
    InitializeGameVariables(gameState);
    
    // NEW: Initialize new systems
    InitWeaponSystem(&gameState->weapons);
    InitAchievementSystem(&gameState->achievements);
    InitShaderSystem(&gameState->shaders);
    
    // Load saved data
    LoadHighScore(gameState);
    LoadAchievements(&gameState->achievements);
}
```

## 2. Player Control Integration

Update `HandlePlayerInput()` in `player.c`:

```c
void HandlePlayerInput(GameState* gameState, float delta) {
    // Existing movement code...
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        gameState->player.rect.x -= PLAYER_SPEED * delta * 60.0f;
    }
    // ... other movement code
    
    // NEW: Weapon switching
    UpdateWeaponSystem(&gameState->weapons, gameState, delta);
}
```

Update `HandlePlayerShooting()` in `player.c`:

```c
void HandlePlayerShooting(GameState* gameState) {
    // Get current weapon fire rate
    float fire_rate = GetWeaponFireRate(&gameState->weapons);
    
    if ((IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_Z)) && gameState->shootCooldown <= 0.0f) {
        // Register shot for achievements
        UpdateAchievementStats(&gameState->achievements, STAT_SHOT_FIRED, 1);
        
        // Fire current weapon
        Vector2 shoot_pos = {
            gameState->player.rect.x + gameState->player.rect.width / 2.0f,
            gameState->player.rect.y
        };
        FireWeapon(&gameState->weapons, gameState, shoot_pos);
        
        gameState->shootCooldown = fire_rate;
    }
}
```

## 3. Combat System Integration

Update `CheckBulletEnemyCollisions()` in `collision.c`:

```c
void CheckBulletEnemyCollisions(GameState* gameState) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!gameState->bullets[i].active) continue;
        
        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (!gameState->enemies[j].active) continue;
            
            // Existing collision detection...
            if (CheckCollisionBulletRec(gameState->bullets[i].position, enemy_rect, true)) {
                // NEW: Handle different bullet types
                Bullet* bullet = &gameState->bullets[i];
                Enemy* enemy = &gameState->enemies[j];
                
                // Apply damage based on weapon
                enemy->health -= bullet->damage;
                
                // Create enhanced hit effect
                CreateHitEffect(&gameState->effects, enemy->position, true);
                TriggerShaderHitEffect(&gameState->shaders);
                
                // Update achievement stats
                UpdateAchievementStats(&gameState->achievements, STAT_SHOT_HIT, 1);
                
                // Handle penetrating bullets
                if (!bullet->penetrating) {
                    bullet->active = false;
                }
                
                // Handle explosive bullets
                if (bullet->explosion_radius > 0.0f) {
                    CreateExplosion(&gameState->effects, bullet->position, GREEN, 15);
                    TriggerShaderExplosionEffect(&gameState->shaders);
                    
                    // Damage nearby enemies
                    for (int k = 0; k < MAX_ENEMIES; k++) {
                        if (k == j || !gameState->enemies[k].active) continue;
                        
                        float dist = Vector2Distance(bullet->position, gameState->enemies[k].position);
                        if (dist < bullet->explosion_radius) {
                            gameState->enemies[k].health--;
                        }
                    }
                }
                
                // Destroy enemy if health reaches 0
                if (enemy->health <= 0) {
                    UpdateAchievementStats(&gameState->achievements, STAT_ENEMY_KILLED, 1);
                    HandleEnemyDestroy(gameState, j, enemy->position);
                }
                
                break;
            }
        }
    }
}
```

## 4. Game Update Loop Integration

Update `UpdateGamePlaying()` in `game.c`:

```c
static void UpdateGamePlaying(GameState* gameState, float delta) {
    // Handle pause toggle
    if (IsKeyPressed(KEY_P) || IsKeyPressed(KEY_ESCAPE)) {
        gameState->is_paused = !gameState->is_paused;
    }
    
    if (gameState->is_paused) {
        gameState->pause_timer += delta;
        return;
    }
    
    // Existing updates...
    UpdatePlayer(gameState, delta);
    UpdateEnemyAI(gameState, delta);
    UpdateEnemies(gameState, delta);
    UpdateEnemyBullets(gameState, delta);
    UpdateParticleSystem(&gameState->effects, delta);
    UpdateAudioSystem(&gameState->audio, delta);
    UpdatePowerUpSystem(&gameState->powerups, gameState, delta);
    UpdateAdaptiveDifficulty(&gameState->balance, gameState, delta);
    UpdateComboSystem(&gameState->balance, delta);
    UpdateQoLSystem(&gameState->qol, gameState, delta);
    
    // NEW: Update new systems
    UpdateWeaponSystem(&gameState->weapons, gameState, delta);
    UpdateAchievementSystem(&gameState->achievements, gameState, delta);
    UpdateShaderSystem(&gameState->shaders, delta);
    
    // Update advanced bullets with new behaviors
    UpdateAdvancedBullets(gameState, delta);
    
    // Reset shader effects gradually
    ResetShaderEffects(&gameState->shaders, delta);
    
    // ... rest of existing update code
}
```

## 5. Rendering Integration

Update `DrawGamePlaying()` in `game.c`:

```c
static void DrawGamePlaying(const GameState* gameState) {
    // NEW: Begin shader post-processing
    BeginShaderMode(&gameState->shaders);
    
    // Apply screen shake offset (existing code)
    if (gameState->effects.screen_shake_duration > 0.0f) {
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
    
    // NEW: Draw advanced bullets instead of basic ones
    DrawAdvancedBullets(gameState);
    
    DrawEnemies(gameState);
    DrawPowerUps(&gameState->powerups);
    DrawParticleSystem(&gameState->effects);
    
    // End screen shake effect
    if (gameState->effects.screen_shake_duration > 0.0f) {
        EndMode2D();
    }
    
    // NEW: End shader post-processing
    EndShaderMode(&gameState->shaders);
    
    // Draw UI (unaffected by shaders)
    DrawUI(gameState);
    DrawQoLUI(&gameState->qol, gameState);
    
    // NEW: Draw new UI elements
    DrawWeaponUI(&gameState->weapons, gameState);
    DrawAchievementNotification(&gameState->achievements);
    
    // Debug information
    if (gameState->menu.show_fps) {
        DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, GREEN);
        DrawShaderDebugUI(&gameState->shaders);
    }
    
    // ... rest of existing draw code
}
```

## 6. Menu System Integration

Add achievement menu to `menu.c`:

```c
// Add to MenuState enum
typedef enum {
    MAIN_MENU,
    OPTIONS_MENU,
    ACHIEVEMENTS_MENU,  // NEW
    CREDITS_MENU
} MenuState;

// Update HandleMenuInput() function
void HandleMenuInput(GameState* gameState) {
    MenuSystem* menu = &gameState->menu;
    
    if (menu->current_menu == MAIN_MENU) {
        // ... existing main menu handling
        
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            switch (menu->selected_option) {
                case 0: // Start Game
                    UpdateAchievementStats(&gameState->achievements, STAT_GAME_STARTED, 1);
                    InitGame(gameState);
                    gameState->screen_state = PLAYING;
                    break;
                case 1: // Instructions
                    menu->show_instructions = true;
                    break;
                case 2: // Achievements - NEW
                    menu->current_menu = ACHIEVEMENTS_MENU;
                    menu->selected_option = 0;
                    break;
                case 3: // Options
                    menu->current_menu = OPTIONS_MENU;
                    menu->selected_option = 0;
                    break;
                case 4: // Credits
                    menu->current_menu = CREDITS_MENU;
                    menu->selected_option = 0;
                    break;
                case 5: // Quit
                    break;
            }
        }
    }
    // Add handling for ACHIEVEMENTS_MENU...
}
```

## 7. Achievement Triggers

Add achievement tracking throughout your game:

```c
// In enemy destruction
void HandleEnemyDestroy(GameState* gameState, int enemy_index, Vector2 position) {
    Enemy* enemy = &gameState->enemies[enemy_index];
    
    // Existing code...
    int score = CalculateEnemyScore(enemy);
    AddScore(gameState, score, position);
    
    // NEW: Achievement tracking
    UpdateAchievementStats(&gameState->achievements, STAT_ENEMY_KILLED, 1);
    
    if (enemy->type == BOSS) {
        UpdateAchievementStats(&gameState->achievements, STAT_BOSS_DEFEATED, 1);
    }
    
    if (enemy->has_captured_ship) {
        UpdateAchievementStats(&gameState->achievements, STAT_SHIP_RESCUED, 1);
    }
    
    // Check for combo achievements
    if (gameState->balance.consecutive_hits >= 10) {
        UpdateAchievementStats(&gameState->achievements, STAT_COMBO_ACHIEVED, 
                              gameState->balance.consecutive_hits);
    }
    
    enemy->active = false;
}

// In wave completion
void CompleteWave(GameState* gameState) {
    UpdateAchievementStats(&gameState->achievements, STAT_WAVE_REACHED, gameState->wave_number);
    UpdateAchievementStats(&gameState->achievements, STAT_WAVE_COMPLETED, 1);
    
    // Check for speed run achievement
    if (gameState->wave_number == 10) {
        float elapsed_time = GetTime(); // You'll need to track this properly
        if (elapsed_time < 300.0f) { // 5 minutes
            UpdateAchievementStats(&gameState->achievements, STAT_SPEED_RUN_COMPLETE, (int)elapsed_time);
        }
    }
}
```

## 8. Power-Up Integration with Weapons

Update power-up collection to unlock weapons:

```c
void CollectPowerUp(PowerUpSystem* powerups, GameState* gameState, PowerUpType type) {
    // Existing power-up logic...
    
    // NEW: Weapon unlocks based on score/wave
    if (gameState->score >= 10000 && !IsWeaponUnlocked(&gameState->weapons, WEAPON_DOUBLE)) {
        UnlockWeapon(&gameState->weapons, WEAPON_DOUBLE);
        UpdateAchievementStats(&gameState->achievements, STAT_WEAPON_UNLOCKED, 1);
        TriggerShaderPowerUpEffect(&gameState->shaders);
    }
    
    if (gameState->wave_number >= 5 && !IsWeaponUnlocked(&gameState->weapons, WEAPON_LASER)) {
        UnlockWeapon(&gameState->weapons, WEAPON_LASER);
        UpdateAchievementStats(&gameState->achievements, STAT_WEAPON_UNLOCKED, 1);
    }
    
    // ... more unlock conditions
}
```

## 9. Save/Load Integration

Update save system to include new data:

```c
void SaveGameData(GameState* gameState) {
    SaveHighScore(gameState);
    SaveAchievements(&gameState->achievements);
    
    // NEW: Save weapon progression
    FILE* file = fopen("weapons.dat", "wb");
    if (file) {
        fwrite(&gameState->weapons, sizeof(WeaponSystem), 1, file);
        fclose(file);
    }
}

void LoadGameData(GameState* gameState) {
    LoadHighScore(gameState);
    LoadAchievements(&gameState->achievements);
    
    // NEW: Load weapon progression
    FILE* file = fopen("weapons.dat", "rb");
    if (file) {
        fread(&gameState->weapons, sizeof(WeaponSystem), 1, file);
        fclose(file);
    }
}
```

## 10. Cleanup Integration

Update cleanup in `main()`:

```c
int main() {
    // ... existing main loop
    
    // NEW: Cleanup new systems before exit
    CleanupShaderSystem(&gameState.shaders);
    CloseWindow();
    return 0;
}
```

## Example Usage in Practice

Here's how the systems work together in practice:

1. **Player shoots with laser weapon**: 
   - Weapon system fires penetrating bullet
   - Bullet travels through multiple enemies
   - Each hit triggers shader effect and particles
   - Achievements track accuracy and kill count

2. **Boss destruction**:
   - Multiple hits with different weapons
   - Screen shakes and shaders activate
   - Achievement unlocked notification appears
   - New weapon becomes available
   - Score increases with combo multiplier

3. **Power-up collection**:
   - Particle effect spawns
   - Shader energy field activates briefly
   - Achievement progress updates
   - Weapon upgrade points awarded

This integration maintains your existing game flow while adding rich new features that enhance the player experience without disrupting core gameplay.