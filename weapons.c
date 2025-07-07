#include "game.h"
#include <math.h>
#include <stdlib.h>

// =============================================================================
// WEAPON SYSTEM
// =============================================================================

// Initialize weapon system
void InitWeaponSystem(WeaponSystem* weapons) {
    if (!weapons) return;
    
    weapons->current_weapon = WEAPON_SINGLE;
    weapons->weapon_level = 1;
    weapons->max_level = 5;
    
    // Initialize weapon stats
    for (int i = 0; i < WEAPON_COUNT; i++) {
        weapons->weapon_stats[i].damage = 1;
        weapons->weapon_stats[i].fire_rate = 0.2f;
        weapons->weapon_stats[i].bullet_speed = BULLET_SPEED;
        weapons->weapon_stats[i].bullet_count = 1;
        weapons->weapon_stats[i].spread_angle = 0.0f;
        weapons->weapon_stats[i].penetration = false;
        weapons->weapon_stats[i].homing = false;
        weapons->weapon_stats[i].explosion_radius = 0.0f;
        weapons->weapon_stats[i].unlocked = false;
    }
    
    // Set default weapon stats
    InitWeaponStats(weapons);
    
    // Unlock starting weapon
    weapons->weapon_stats[WEAPON_SINGLE].unlocked = true;
    
    weapons->upgrade_points = 0;
    weapons->weapon_select_timer = 0.0f;
    weapons->show_weapon_ui = false;
}

// Initialize weapon statistics
void InitWeaponStats(WeaponSystem* weapons) {
    if (!weapons) return;
    
    // Single Shot
    weapons->weapon_stats[WEAPON_SINGLE].damage = 1;
    weapons->weapon_stats[WEAPON_SINGLE].fire_rate = 0.2f;
    weapons->weapon_stats[WEAPON_SINGLE].bullet_speed = BULLET_SPEED;
    weapons->weapon_stats[WEAPON_SINGLE].bullet_count = 1;
    weapons->weapon_stats[WEAPON_SINGLE].unlocked = true;
    
    // Double Shot
    weapons->weapon_stats[WEAPON_DOUBLE].damage = 1;
    weapons->weapon_stats[WEAPON_DOUBLE].fire_rate = 0.25f;
    weapons->weapon_stats[WEAPON_DOUBLE].bullet_speed = BULLET_SPEED;
    weapons->weapon_stats[WEAPON_DOUBLE].bullet_count = 2;
    weapons->weapon_stats[WEAPON_DOUBLE].spread_angle = 0.1f;
    
    // Triple Shot
    weapons->weapon_stats[WEAPON_TRIPLE].damage = 1;
    weapons->weapon_stats[WEAPON_TRIPLE].fire_rate = 0.3f;
    weapons->weapon_stats[WEAPON_TRIPLE].bullet_speed = BULLET_SPEED;
    weapons->weapon_stats[WEAPON_TRIPLE].bullet_count = 3;
    weapons->weapon_stats[WEAPON_TRIPLE].spread_angle = 0.3f;
    
    // Spread Shot
    weapons->weapon_stats[WEAPON_SPREAD].damage = 1;
    weapons->weapon_stats[WEAPON_SPREAD].fire_rate = 0.4f;
    weapons->weapon_stats[WEAPON_SPREAD].bullet_speed = BULLET_SPEED * 0.9f;
    weapons->weapon_stats[WEAPON_SPREAD].bullet_count = 5;
    weapons->weapon_stats[WEAPON_SPREAD].spread_angle = 0.8f;
    
    // Rapid Fire
    weapons->weapon_stats[WEAPON_RAPID].damage = 1;
    weapons->weapon_stats[WEAPON_RAPID].fire_rate = 0.08f;
    weapons->weapon_stats[WEAPON_RAPID].bullet_speed = BULLET_SPEED * 1.2f;
    weapons->weapon_stats[WEAPON_RAPID].bullet_count = 1;
    
    // Laser
    weapons->weapon_stats[WEAPON_LASER].damage = 2;
    weapons->weapon_stats[WEAPON_LASER].fire_rate = 0.5f;
    weapons->weapon_stats[WEAPON_LASER].bullet_speed = BULLET_SPEED * 1.5f;
    weapons->weapon_stats[WEAPON_LASER].bullet_count = 1;
    weapons->weapon_stats[WEAPON_LASER].penetration = true;
    
    // Homing Missiles
    weapons->weapon_stats[WEAPON_HOMING].damage = 3;
    weapons->weapon_stats[WEAPON_HOMING].fire_rate = 0.8f;
    weapons->weapon_stats[WEAPON_HOMING].bullet_speed = BULLET_SPEED * 0.7f;
    weapons->weapon_stats[WEAPON_HOMING].bullet_count = 1;
    weapons->weapon_stats[WEAPON_HOMING].homing = true;
    
    // Plasma Cannon
    weapons->weapon_stats[WEAPON_PLASMA].damage = 4;
    weapons->weapon_stats[WEAPON_PLASMA].fire_rate = 1.0f;
    weapons->weapon_stats[WEAPON_PLASMA].bullet_speed = BULLET_SPEED * 0.8f;
    weapons->weapon_stats[WEAPON_PLASMA].bullet_count = 1;
    weapons->weapon_stats[WEAPON_PLASMA].explosion_radius = 30.0f;
}

// Update weapon system
void UpdateWeaponSystem(WeaponSystem* weapons, GameState* gameState, float delta) {
    if (!weapons || !gameState) return;
    
    weapons->weapon_select_timer -= delta;
    if (weapons->weapon_select_timer <= 0.0f) {
        weapons->show_weapon_ui = false;
    }
    
    // Handle weapon switching
    if (IsKeyPressed(KEY_Q)) {
        SwitchWeapon(weapons, -1);
        weapons->weapon_select_timer = 3.0f;
        weapons->show_weapon_ui = true;
        PlayGameSound(&gameState->audio, GAME_SOUND_MENU_MOVE, 0.8f);
    }
    if (IsKeyPressed(KEY_E)) {
        SwitchWeapon(weapons, 1);
        weapons->weapon_select_timer = 3.0f;
        weapons->show_weapon_ui = true;
        PlayGameSound(&gameState->audio, GAME_SOUND_MENU_MOVE, 0.8f);
    }
    
    // Handle weapon upgrades
    if (IsKeyPressed(KEY_U) && weapons->upgrade_points > 0) {
        UpgradeCurrentWeapon(weapons);
        PlayGameSound(&gameState->audio, GAME_SOUND_POWERUP, 1.0f);
    }
}

// Switch to next/previous weapon
void SwitchWeapon(WeaponSystem* weapons, int direction) {
    if (!weapons) return;
    
    int original_weapon = weapons->current_weapon;
    do {
        weapons->current_weapon += direction;
        if (weapons->current_weapon < 0) {
            weapons->current_weapon = WEAPON_COUNT - 1;
        }
        if (weapons->current_weapon >= WEAPON_COUNT) {
            weapons->current_weapon = 0;
        }
    } while (!weapons->weapon_stats[weapons->current_weapon].unlocked && 
             weapons->current_weapon != original_weapon);
}

// Fire current weapon
void FireWeapon(WeaponSystem* weapons, GameState* gameState, Vector2 position) {
    if (!weapons || !gameState) return;
    
    WeaponStats* stats = &weapons->weapon_stats[weapons->current_weapon];
    
    switch (weapons->current_weapon) {
        case WEAPON_SINGLE:
            FireSingleShot(gameState, position, stats);
            break;
        case WEAPON_DOUBLE:
            FireDoubleShot(gameState, position, stats);
            break;
        case WEAPON_TRIPLE:
            FireTripleShot(gameState, position, stats);
            break;
        case WEAPON_SPREAD:
            FireSpreadShot(gameState, position, stats);
            break;
        case WEAPON_RAPID:
            FireSingleShot(gameState, position, stats);
            break;
        case WEAPON_LASER:
            FireLaser(gameState, position, stats);
            break;
        case WEAPON_HOMING:
            FireHomingMissile(gameState, position, stats);
            break;
        case WEAPON_PLASMA:
            FirePlasma(gameState, position, stats);
            break;
    }
    
    PlayGameSound(&gameState->audio, GAME_SOUND_PLAYER_SHOOT, 1.0f);
}

// Fire single shot
void FireSingleShot(GameState* gameState, Vector2 position, WeaponStats* stats) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!gameState->bullets[i].active) {
            gameState->bullets[i].active = true;
            gameState->bullets[i].position = position;
            gameState->bullets[i].velocity = (Vector2){0, -stats->bullet_speed};
            gameState->bullets[i].damage = stats->damage;
            gameState->bullets[i].penetrating = stats->penetration;
            gameState->bullets[i].homing = stats->homing;
            gameState->bullets[i].explosion_radius = stats->explosion_radius;
            gameState->bullets[i].lifetime = 5.0f;
            break;
        }
    }
}

// Fire double shot
void FireDoubleShot(GameState* gameState, Vector2 position, WeaponStats* stats) {
    float offset = 10.0f;
    
    for (int shot = 0; shot < 2; shot++) {
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!gameState->bullets[i].active) {
                gameState->bullets[i].active = true;
                gameState->bullets[i].position = (Vector2){
                    position.x + (shot == 0 ? -offset : offset),
                    position.y
                };
                gameState->bullets[i].velocity = (Vector2){0, -stats->bullet_speed};
                gameState->bullets[i].damage = stats->damage;
                gameState->bullets[i].penetrating = stats->penetration;
                gameState->bullets[i].homing = stats->homing;
                gameState->bullets[i].explosion_radius = stats->explosion_radius;
                gameState->bullets[i].lifetime = 5.0f;
                break;
            }
        }
    }
}

// Fire triple shot
void FireTripleShot(GameState* gameState, Vector2 position, WeaponStats* stats) {
    float angles[3] = {-stats->spread_angle, 0, stats->spread_angle};
    
    for (int shot = 0; shot < 3; shot++) {
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!gameState->bullets[i].active) {
                gameState->bullets[i].active = true;
                gameState->bullets[i].position = position;
                gameState->bullets[i].velocity = (Vector2){
                    sinf(angles[shot]) * stats->bullet_speed,
                    -cosf(angles[shot]) * stats->bullet_speed
                };
                gameState->bullets[i].damage = stats->damage;
                gameState->bullets[i].penetrating = stats->penetration;
                gameState->bullets[i].homing = stats->homing;
                gameState->bullets[i].explosion_radius = stats->explosion_radius;
                gameState->bullets[i].lifetime = 5.0f;
                break;
            }
        }
    }
}

// Fire spread shot
void FireSpreadShot(GameState* gameState, Vector2 position, WeaponStats* stats) {
    float start_angle = -stats->spread_angle / 2.0f;
    float angle_step = stats->spread_angle / (stats->bullet_count - 1);
    
    for (int shot = 0; shot < stats->bullet_count; shot++) {
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!gameState->bullets[i].active) {
                float angle = start_angle + shot * angle_step;
                gameState->bullets[i].active = true;
                gameState->bullets[i].position = position;
                gameState->bullets[i].velocity = (Vector2){
                    sinf(angle) * stats->bullet_speed,
                    -cosf(angle) * stats->bullet_speed
                };
                gameState->bullets[i].damage = stats->damage;
                gameState->bullets[i].penetrating = stats->penetration;
                gameState->bullets[i].homing = stats->homing;
                gameState->bullets[i].explosion_radius = stats->explosion_radius;
                gameState->bullets[i].lifetime = 5.0f;
                break;
            }
        }
    }
}

// Fire laser
void FireLaser(GameState* gameState, Vector2 position, WeaponStats* stats) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!gameState->bullets[i].active) {
            gameState->bullets[i].active = true;
            gameState->bullets[i].position = position;
            gameState->bullets[i].velocity = (Vector2){0, -stats->bullet_speed};
            gameState->bullets[i].damage = stats->damage;
            gameState->bullets[i].penetrating = true;
            gameState->bullets[i].homing = false;
            gameState->bullets[i].explosion_radius = 0.0f;
            gameState->bullets[i].lifetime = 5.0f;
            gameState->bullets[i].bullet_type = BULLET_LASER;
            break;
        }
    }
}

// Fire homing missile
void FireHomingMissile(GameState* gameState, Vector2 position, WeaponStats* stats) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!gameState->bullets[i].active) {
            gameState->bullets[i].active = true;
            gameState->bullets[i].position = position;
            gameState->bullets[i].velocity = (Vector2){0, -stats->bullet_speed};
            gameState->bullets[i].damage = stats->damage;
            gameState->bullets[i].penetrating = false;
            gameState->bullets[i].homing = true;
            gameState->bullets[i].explosion_radius = 0.0f;
            gameState->bullets[i].lifetime = 8.0f;
            gameState->bullets[i].bullet_type = BULLET_HOMING;
            gameState->bullets[i].target_index = -1;
            break;
        }
    }
}

// Fire plasma cannon
void FirePlasma(GameState* gameState, Vector2 position, WeaponStats* stats) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!gameState->bullets[i].active) {
            gameState->bullets[i].active = true;
            gameState->bullets[i].position = position;
            gameState->bullets[i].velocity = (Vector2){0, -stats->bullet_speed};
            gameState->bullets[i].damage = stats->damage;
            gameState->bullets[i].penetrating = false;
            gameState->bullets[i].homing = false;
            gameState->bullets[i].explosion_radius = stats->explosion_radius;
            gameState->bullets[i].lifetime = 5.0f;
            gameState->bullets[i].bullet_type = BULLET_PLASMA;
            break;
        }
    }
}

// Upgrade current weapon
void UpgradeCurrentWeapon(WeaponSystem* weapons) {
    if (!weapons || weapons->upgrade_points <= 0) return;
    
    WeaponStats* stats = &weapons->weapon_stats[weapons->current_weapon];
    
    if (weapons->weapon_level < weapons->max_level) {
        weapons->weapon_level++;
        weapons->upgrade_points--;
        
        // Improve weapon stats
        stats->damage += 1;
        stats->fire_rate *= 0.9f; // Faster firing
        stats->bullet_speed *= 1.1f; // Faster bullets
        
        // Special upgrades per weapon type
        switch (weapons->current_weapon) {
            case WEAPON_SPREAD:
                if (weapons->weapon_level >= 3) {
                    stats->bullet_count++;
                }
                break;
            case WEAPON_HOMING:
                if (weapons->weapon_level >= 4) {
                    stats->explosion_radius = 20.0f;
                }
                break;
            case WEAPON_PLASMA:
                stats->explosion_radius += 5.0f;
                break;
            case WEAPON_LASER:
                if (weapons->weapon_level >= 3) {
                    stats->damage += 1; // Extra damage for laser
                }
                break;
            default:
                break;
        }
    }
}

// Unlock new weapon
void UnlockWeapon(WeaponSystem* weapons, WeaponType weapon) {
    if (!weapons || weapon >= WEAPON_COUNT) return;
    
    weapons->weapon_stats[weapon].unlocked = true;
    weapons->upgrade_points++;
}

// Get weapon fire rate
float GetWeaponFireRate(WeaponSystem* weapons) {
    if (!weapons) return 0.2f;
    
    return weapons->weapon_stats[weapons->current_weapon].fire_rate;
}

// Check if weapon is unlocked
bool IsWeaponUnlocked(WeaponSystem* weapons, WeaponType weapon) {
    if (!weapons || weapon >= WEAPON_COUNT) return false;
    
    return weapons->weapon_stats[weapon].unlocked;
}

// Get weapon name
const char* GetWeaponName(WeaponType weapon) {
    switch (weapon) {
        case WEAPON_SINGLE: return "Single Shot";
        case WEAPON_DOUBLE: return "Double Shot";
        case WEAPON_TRIPLE: return "Triple Shot";
        case WEAPON_SPREAD: return "Spread Shot";
        case WEAPON_RAPID: return "Rapid Fire";
        case WEAPON_LASER: return "Laser";
        case WEAPON_HOMING: return "Homing Missiles";
        case WEAPON_PLASMA: return "Plasma Cannon";
        default: return "Unknown";
    }
}

// Get weapon description
const char* GetWeaponDescription(WeaponType weapon) {
    switch (weapon) {
        case WEAPON_SINGLE: return "Basic single projectile";
        case WEAPON_DOUBLE: return "Fires two parallel shots";
        case WEAPON_TRIPLE: return "Fires three spread shots";
        case WEAPON_SPREAD: return "Wide spread pattern";
        case WEAPON_RAPID: return "High rate of fire";
        case WEAPON_LASER: return "Penetrating beam";
        case WEAPON_HOMING: return "Seeks nearest enemy";
        case WEAPON_PLASMA: return "Explosive projectiles";
        default: return "Unknown weapon";
    }
}

// Update bullet behavior
void UpdateAdvancedBullets(GameState* gameState, float delta) {
    if (!gameState) return;
    
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!gameState->bullets[i].active) continue;
        
        Bullet* bullet = &gameState->bullets[i];
        
        // Update lifetime
        bullet->lifetime -= delta;
        if (bullet->lifetime <= 0.0f) {
            bullet->active = false;
            continue;
        }
        
        // Update homing behavior
        if (bullet->homing) {
            UpdateHomingBehavior(gameState, bullet, delta);
        }
        
        // Update position
        bullet->position.x += bullet->velocity.x * delta;
        bullet->position.y += bullet->velocity.y * delta;
        
        // Create particle trails for special bullets
        switch (bullet->bullet_type) {
            case BULLET_LASER:
                CreateBulletTrail(&gameState->effects, bullet->position, bullet->velocity, PURPLE);
                break;
            case BULLET_HOMING:
                CreateBulletTrail(&gameState->effects, bullet->position, bullet->velocity, ORANGE);
                break;
            case BULLET_PLASMA:
                CreateBulletTrail(&gameState->effects, bullet->position, bullet->velocity, GREEN);
                break;
            default:
                CreateBulletTrail(&gameState->effects, bullet->position, bullet->velocity, YELLOW);
                break;
        }
        
        // Remove bullets that go off screen
        if (bullet->position.x < -50 || bullet->position.x > SCREEN_WIDTH + 50 ||
            bullet->position.y < -50 || bullet->position.y > SCREEN_HEIGHT + 50) {
            bullet->active = false;
        }
    }
}

// Update homing behavior
void UpdateHomingBehavior(GameState* gameState, Bullet* bullet, float delta) {
    if (!gameState || !bullet) return;
    
    float closest_distance = 9999.0f;
    Vector2 target_pos = {0, 0};
    bool target_found = false;
    
    // Find closest enemy
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!gameState->enemies[i].active) continue;
        
        float distance = Vector2Distance(bullet->position, gameState->enemies[i].position);
        if (distance < closest_distance && distance < 200.0f) { // Homing range
            closest_distance = distance;
            target_pos = gameState->enemies[i].position;
            target_found = true;
            bullet->target_index = i;
        }
    }
    
    if (target_found) {
        // Calculate direction to target
        Vector2 direction = {
            target_pos.x - bullet->position.x,
            target_pos.y - bullet->position.y
        };
        
        float length = sqrtf(direction.x * direction.x + direction.y * direction.y);
        if (length > 0) {
            direction.x /= length;
            direction.y /= length;
            
            // Adjust velocity towards target
            float turn_rate = 3.0f * delta;
            bullet->velocity.x += direction.x * turn_rate * 100.0f;
            bullet->velocity.y += direction.y * turn_rate * 100.0f;
            
            // Limit velocity
            float vel_length = sqrtf(bullet->velocity.x * bullet->velocity.x + bullet->velocity.y * bullet->velocity.y);
            if (vel_length > BULLET_SPEED) {
                bullet->velocity.x = (bullet->velocity.x / vel_length) * BULLET_SPEED;
                bullet->velocity.y = (bullet->velocity.y / vel_length) * BULLET_SPEED;
            }
        }
    }
}

// Draw weapon UI
void DrawWeaponUI(const WeaponSystem* weapons, const GameState* gameState) {
    if (!weapons || !gameState) return;
    
    if (weapons->show_weapon_ui) {
        // Draw weapon selection UI
        int ui_x = SCREEN_WIDTH - 250;
        int ui_y = 50;
        int ui_width = 240;
        int ui_height = 150;
        
        DrawRectangle(ui_x, ui_y, ui_width, ui_height, (Color){0, 0, 50, 200});
        DrawRectangleLines(ui_x, ui_y, ui_width, ui_height, WHITE);
        
        DrawText("WEAPON SYSTEM", ui_x + 10, ui_y + 10, 16, WHITE);
        DrawText(GetWeaponName(weapons->current_weapon), ui_x + 10, ui_y + 30, 14, YELLOW);
        DrawText(GetWeaponDescription(weapons->current_weapon), ui_x + 10, ui_y + 50, 12, GRAY);
        
        DrawText(TextFormat("Level: %d/%d", weapons->weapon_level, weapons->max_level), 
                 ui_x + 10, ui_y + 70, 12, WHITE);
        DrawText(TextFormat("Damage: %d", weapons->weapon_stats[weapons->current_weapon].damage), 
                 ui_x + 10, ui_y + 85, 12, WHITE);
        DrawText(TextFormat("Fire Rate: %.2f", weapons->weapon_stats[weapons->current_weapon].fire_rate), 
                 ui_x + 10, ui_y + 100, 12, WHITE);
        
        if (weapons->upgrade_points > 0) {
            DrawText(TextFormat("Upgrades: %d (Press U)", weapons->upgrade_points), 
                     ui_x + 10, ui_y + 120, 12, GREEN);
        }
        
        DrawText("Q/E to switch weapons", ui_x + 10, ui_y + 135, 10, LIGHTGRAY);
    }
    
    // Draw weapon indicator in corner
    DrawText(TextFormat("Weapon: %s", GetWeaponName(weapons->current_weapon)), 
             10, SCREEN_HEIGHT - 40, 16, WHITE);
    
    // Draw weapon level indicator
    for (int i = 0; i < weapons->weapon_level; i++) {
        DrawRectangle(10 + i * 8, SCREEN_HEIGHT - 20, 6, 6, YELLOW);
    }
    for (int i = weapons->weapon_level; i < weapons->max_level; i++) {
        DrawRectangleLines(10 + i * 8, SCREEN_HEIGHT - 20, 6, 6, GRAY);
    }
}

// Draw enhanced bullets
void DrawAdvancedBullets(const GameState* gameState) {
    if (!gameState) return;
    
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!gameState->bullets[i].active) continue;
        
        const Bullet* bullet = &gameState->bullets[i];
        Color bullet_color = YELLOW;
        float size_multiplier = 1.0f;
        
        switch (bullet->bullet_type) {
            case BULLET_NORMAL:
                bullet_color = YELLOW;
                break;
            case BULLET_LASER:
                bullet_color = PURPLE;
                size_multiplier = 1.5f;
                break;
            case BULLET_HOMING:
                bullet_color = ORANGE;
                // Pulsing effect for homing missiles
                size_multiplier = 1.0f + 0.3f * sinf(GetTime() * 8.0f);
                break;
            case BULLET_PLASMA:
                bullet_color = GREEN;
                size_multiplier = 1.2f;
                // Add glow effect
                DrawCircleV(bullet->position, 8.0f * size_multiplier, (Color){0, 255, 0, 50});
                break;
        }
        
        // Draw bullet based on type
        if (bullet->bullet_type == BULLET_LASER) {
            DrawRectangle(
                (int)(bullet->position.x - 2 * size_multiplier),
                (int)(bullet->position.y - 8 * size_multiplier),
                (int)(4 * size_multiplier),
                (int)(16 * size_multiplier),
                bullet_color
            );
        } else {
            DrawCircleV(bullet->position, 3.0f * size_multiplier, bullet_color);
        }
        
        // Draw explosion radius preview for plasma
        if (bullet->bullet_type == BULLET_PLASMA && bullet->explosion_radius > 0) {
            DrawCircleLinesV(bullet->position, bullet->explosion_radius, (Color){0, 255, 0, 100});
        }
    }
}