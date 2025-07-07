#include "game.h"

// Function to update dual fighter mechanics
void UpdateDualFighter(GameState* gameState, float delta) {
    Player* player = &gameState->player;
    
    if (player->has_captured_ship) {
        // Update dual fighter timer
        player->dual_fighter_timer += delta;
        
        // Update captured ship position (side-by-side formation)
        player->captured_ship_offset.x = player->rect.x + PLAYER_SIZE + 10.0f;
        player->captured_ship_offset.y = player->rect.y;
        
        // Update dual hitbox (larger when dual fighter)
        player->dual_hitbox.x = player->rect.x - 5.0f;
        player->dual_hitbox.y = player->rect.y - 5.0f;
        player->dual_hitbox.width = (player->rect.width + 10.0f) * DUAL_FIGHTER_HITBOX_MULTIPLIER;
        player->dual_hitbox.height = (player->rect.height + 10.0f) * DUAL_FIGHTER_HITBOX_MULTIPLIER;
        
        // Enhanced dual fire capability
        player->dual_fire = true;
    } else {
        // Normal single fighter
        player->dual_hitbox = player->rect;
        player->dual_fighter_timer = 0.0f;
    }
}

// Handle player input
void HandlePlayerInput(GameState* gameState, float delta) {
    // Handle player movement
    if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
        gameState->player.rect.x -= PLAYER_SPEED * delta * 60.0f;
    }
    if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
        gameState->player.rect.x += PLAYER_SPEED * delta * 60.0f;
    }
    if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
        gameState->player.rect.y -= PLAYER_SPEED * delta * 60.0f;
    }
    if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
        gameState->player.rect.y += PLAYER_SPEED * delta * 60.0f;
    }
    
    // Keep player within screen bounds
    if (gameState->player.rect.x < 0) gameState->player.rect.x = 0;
    if (gameState->player.rect.x + gameState->player.rect.width > SCREEN_WIDTH) 
        gameState->player.rect.x = SCREEN_WIDTH - gameState->player.rect.width;
    if (gameState->player.rect.y < 0) gameState->player.rect.y = 0;
    if (gameState->player.rect.y + gameState->player.rect.height > SCREEN_HEIGHT) 
        gameState->player.rect.y = SCREEN_HEIGHT - gameState->player.rect.height;
}

// Handle player shooting
void HandlePlayerShooting(GameState* gameState) {
    if ((IsKeyDown(KEY_SPACE) || IsKeyDown(KEY_Z)) && gameState->shootCooldown <= 0.0f) {
        // Register shot for balance system
        RegisterPlayerShot(&gameState->balance);

        // Use weapon system for firing
        Vector2 shoot_pos = {
            gameState->player.rect.x + gameState->player.rect.width / 2.0f,
            gameState->player.rect.y
        };
        FireWeapon(&gameState->weapons, gameState, shoot_pos);

        // Use weapon system fire rate
        float fire_rate = GetWeaponFireRate(&gameState->weapons);
        gameState->shootCooldown = fire_rate;
    }
}

// Update player
void UpdatePlayer(GameState* gameState, float delta) {
    // Apply slow motion power-up to player movement
    float time_scale = IsPowerUpActive(&gameState->powerups, POWERUP_SLOW_MOTION) ? 0.5f : 1.0f;
    float adjusted_delta = delta * time_scale;
    
    // Update dual fighter mechanics
    UpdateDualFighter(gameState, adjusted_delta);
    
    // Handle input
    HandlePlayerInput(gameState, adjusted_delta);
    
    // Handle shooting
    HandlePlayerShooting(gameState);
    
    // Update shooting cooldown
    if (gameState->shootCooldown > 0.0f) {
        gameState->shootCooldown -= adjusted_delta;
    }
    
    // Reset player color after hit
    if (gameState->player.color.r > 0 && gameState->player.color.g < 255) {
        gameState->player.color = BLUE;
    }
    
    // Apply shield visual effect
    if (IsPowerUpActive(&gameState->powerups, POWERUP_SHIELD)) {
        // Shield effect will be drawn in render.c
        gameState->player.color = (Color){100, 150, 255, 255};
    }
}