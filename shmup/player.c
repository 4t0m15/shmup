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
        // Find an inactive bullet
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!gameState->bullets[i].active) {
                gameState->bullets[i].active = true;
                gameState->bullets[i].position.x = gameState->player.rect.x + gameState->player.rect.width / 2.0f;
                gameState->bullets[i].position.y = gameState->player.rect.y;
                gameState->shootCooldown = 0.2f; // Shooting cooldown
                break;
            }
        }
        
        // Dual fire for captured ship
        if (gameState->player.dual_fire) {
            for (int i = 0; i < MAX_BULLETS; i++) {
                if (!gameState->bullets[i].active) {
                    gameState->bullets[i].active = true;
                    gameState->bullets[i].position.x = gameState->player.captured_ship_offset.x + PLAYER_SIZE / 2.0f;
                    gameState->bullets[i].position.y = gameState->player.captured_ship_offset.y;
                    break;
                }
            }
        }
    }
}

// Update player
void UpdatePlayer(GameState* gameState, float delta) {
    // Update dual fighter mechanics
    UpdateDualFighter(gameState, delta);
    
    // Handle input
    HandlePlayerInput(gameState, delta);
    
    // Handle shooting
    HandlePlayerShooting(gameState);
    
    // Update shooting cooldown
    if (gameState->shootCooldown > 0.0f) {
        gameState->shootCooldown -= delta;
    }
    
    // Reset player color after hit
    if (gameState->player.color.r > 0 && gameState->player.color.g < 255) {
        gameState->player.color = BLUE;
    }
}