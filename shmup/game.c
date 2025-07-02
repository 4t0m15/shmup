#include "game.h"

void InitGame(GameState* gameState) {
    // Initialize player in the center of the screen
    gameState->player.rect.x = SCREEN_WIDTH / 2.0f - PLAYER_SIZE / 2.0f;
    gameState->player.rect.y = SCREEN_HEIGHT / 2.0f - PLAYER_SIZE / 2.0f;
    gameState->player.rect.width = PLAYER_SIZE;
    gameState->player.rect.height = PLAYER_SIZE;
    gameState->player.color = BLUE;
    
    // Initialize background scroll position
    gameState->backgroundScrollY = 0.0f;
    
    // Initialize bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        gameState->bullets[i].active = false;
    }
    
    // Initialize shoot cooldown
    gameState->shootCooldown = 0.0f;
}

void UpdateGame(GameState* gameState, float delta, const Assets* assets) {
    // Move player with arrow keys (frame-rate independent)
    float moveAmount = PLAYER_SPEED * 200.0f * delta; // 200 is a tuning factor for speed
    if (IsKeyDown(KEY_LEFT)) gameState->player.rect.x -= moveAmount;
    if (IsKeyDown(KEY_RIGHT)) gameState->player.rect.x += moveAmount;
    if (IsKeyDown(KEY_UP)) gameState->player.rect.y -= moveAmount;
    if (IsKeyDown(KEY_DOWN)) gameState->player.rect.y += moveAmount;

    // Prevent player from leaving the screen
    if (gameState->player.rect.x < 0) gameState->player.rect.x = 0;
    if (gameState->player.rect.x + gameState->player.rect.width > SCREEN_WIDTH) 
        gameState->player.rect.x = SCREEN_WIDTH - gameState->player.rect.width;
    if (gameState->player.rect.y < 0) gameState->player.rect.y = 0;
    if (gameState->player.rect.y + gameState->player.rect.height > SCREEN_HEIGHT) 
        gameState->player.rect.y = SCREEN_HEIGHT - gameState->player.rect.height;
    
    // Update shoot cooldown
    if (gameState->shootCooldown > 0.0f) {
        gameState->shootCooldown -= delta;
    }
    
    // Handle shooting
    if (IsKeyDown(KEY_SPACE) && gameState->shootCooldown <= 0.0f) {
        // Find an inactive bullet to use
        for (int i = 0; i < MAX_BULLETS; i++) {
            if (!gameState->bullets[i].active) {
                gameState->bullets[i].active = true;
                gameState->bullets[i].position.x = gameState->player.rect.x + gameState->player.rect.width / 2.0f;
                gameState->bullets[i].position.y = gameState->player.rect.y;
                gameState->shootCooldown = 0.2f; // 200ms cooldown
                
                // Play shoot sound if available
                if (assets->shootSound.frameCount > 0) {
                    PlaySound(assets->shootSound);
                }
                break;
            }
        }
    }
    
    // Update bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (gameState->bullets[i].active) {
            gameState->bullets[i].position.y -= BULLET_SPEED * delta;
            
            // Deactivate bullet if it goes off screen
            if (gameState->bullets[i].position.y < -BULLET_SIZE) {
                gameState->bullets[i].active = false;
            }
        }
    }
    
    // Update background scrolling
    gameState->backgroundScrollY += BACKGROUND_SCROLL_SPEED * delta;
    
    // Reset background position when it scrolls off screen
    // This assumes the background texture height is used for seamless scrolling
    if (gameState->backgroundScrollY >= SCREEN_HEIGHT) {
        gameState->backgroundScrollY = 0.0f;
    }
}

void DrawGame(const GameState* gameState, const Assets* assets) {
    ClearBackground(RAYWHITE);
    
    // Draw scrolling background
    // Draw the background texture twice to create seamless scrolling
    DrawTextureEx(assets->backgroundTexture, 
                  (Vector2){0, gameState->backgroundScrollY - SCREEN_HEIGHT}, 
                  0.0f, 1.0f, WHITE);
    DrawTextureEx(assets->backgroundTexture, 
                  (Vector2){0, gameState->backgroundScrollY}, 
                  0.0f, 1.0f, WHITE);
    
    // Draw bullets
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (gameState->bullets[i].active) {
            DrawCircle((int)gameState->bullets[i].position.x, 
                      (int)gameState->bullets[i].position.y, 
                      BULLET_SIZE, YELLOW);
        }
    }
    
    // Draw player using texture if available, otherwise use rectangle
    if (assets->playerTexture.id != 0) {
        // Calculate scale to fit player size
        float scaleX = gameState->player.rect.width / (float)assets->playerTexture.width;
        float scaleY = gameState->player.rect.height / (float)assets->playerTexture.height;
        float scale = (scaleX < scaleY) ? scaleX : scaleY; // Use smaller scale to maintain aspect ratio
        
        Vector2 position = {
            gameState->player.rect.x + (gameState->player.rect.width - assets->playerTexture.width * scale) / 2,
            gameState->player.rect.y + (gameState->player.rect.height - assets->playerTexture.height * scale) / 2
        };
        
        DrawTextureEx(assets->playerTexture, position, 0.0f, scale, WHITE);
    } else {
        // Fallback to rectangle if no texture
        DrawRectangleRec(gameState->player.rect, gameState->player.color);
    }
    
    // Draw UI
    DrawText("Use arrow keys to move. Press SPACE to shoot.", 10, 10, 20, DARKGRAY);
    // Draw FPS counter in the top-right corner
    DrawFPS(SCREEN_WIDTH - 100, 10);
}
