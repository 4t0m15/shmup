#include "game.h"
#include <math.h>

// Helper function to check collision between circle and rectangle (renamed to avoid conflict)
bool CheckCollisionCircleRect(Vector2 center, float radius, Rectangle rec) {
    float dx = center.x - (rec.x + rec.width / 2.0f);
    float dy = center.y - (rec.y + rec.height / 2.0f);
    
    float closest_x = center.x;
    float closest_y = center.y;
    
    if (center.x < rec.x) closest_x = rec.x;
    else if (center.x > rec.x + rec.width) closest_x = rec.x + rec.width;
    
    if (center.y < rec.y) closest_y = rec.y;
    else if (center.y > rec.y + rec.height) closest_y = rec.y + rec.height;
    
    dx = center.x - closest_x;
    dy = center.y - closest_y;
    
    return (dx * dx + dy * dy) <= (radius * radius);
}

// Helper function to check collision between bullet and rectangle
bool CheckCollisionBulletRec(Vector2 bullet_pos, Rectangle rec, bool is_player_bullet) {
    float bullet_width = is_player_bullet ? 4.0f : 3.0f;
    float bullet_length = is_player_bullet ? 12.0f : 10.0f;
    
    Rectangle bullet_rect = {
        bullet_pos.x - bullet_width / 2.0f,
        bullet_pos.y - bullet_length / 2.0f,
        bullet_width,
        bullet_length
    };
    
    return CheckCollisionRecs(bullet_rect, rec);
}

// Function to check bullet-enemy collisions
void CheckBulletEnemyCollisions(GameState* gameState) {
    for (int i = 0; i < MAX_BULLETS; i++) {
        if (!gameState->bullets[i].active) continue;
        
        for (int j = 0; j < MAX_ENEMIES; j++) {
            if (!gameState->enemies[j].active) continue;
            
            float enemy_size = (gameState->enemies[j].type == BOSS) ? BOSS_SIZE : ENEMY_SIZE;
            Rectangle enemy_rect = {
                gameState->enemies[j].position.x - enemy_size / 2.0f,
                gameState->enemies[j].position.y - enemy_size / 2.0f,
                enemy_size,
                enemy_size
            };
            
            if (CheckCollisionBulletRec(gameState->bullets[i].position, enemy_rect, true)) {
                // Hit! Deactivate bullet
                gameState->bullets[i].active = false;
                
                // Reduce enemy health
                gameState->enemies[j].health--;
                
                // Destroy enemy if health reaches 0
                if (gameState->enemies[j].health <= 0) {
                    HandleEnemyDestroy(gameState, j, gameState->enemies[j].position);
                }
                break;
            }
        }
    }
}

// Function to check player-enemy collisions
bool CheckPlayerEnemyCollisions(GameState* gameState) {
    Rectangle player_rect = gameState->player.rect;
    
    for (int i = 0; i < MAX_ENEMIES; i++) {
        if (!gameState->enemies[i].active) continue;
        
        float enemy_size = (gameState->enemies[i].type == BOSS) ? BOSS_SIZE : ENEMY_SIZE;
        Rectangle enemy_rect = {
            gameState->enemies[i].position.x - enemy_size / 2.0f,
            gameState->enemies[i].position.y - enemy_size / 2.0f,
            enemy_size,
            enemy_size
        };
        
        if (CheckCollisionRecs(player_rect, enemy_rect)) {
            return true; // Collision detected
        }
    }
    
    return false;
}