#include "game.h"

void InitGame(Player* player) {
    // Initialize player in the center of the screen
    player->rect.x = SCREEN_WIDTH / 2.0f - PLAYER_SIZE / 2.0f;
    player->rect.y = SCREEN_HEIGHT / 2.0f - PLAYER_SIZE / 2.0f;
    player->rect.width = PLAYER_SIZE;
    player->rect.height = PLAYER_SIZE;
    player->color = BLUE;
}

void UpdateGame(Player* player) {
    // Move player with arrow keys
    if (IsKeyDown(KEY_LEFT)) player->rect.x -= PLAYER_SPEED;
    if (IsKeyDown(KEY_RIGHT)) player->rect.x += PLAYER_SPEED;
    if (IsKeyDown(KEY_UP)) player->rect.y -= PLAYER_SPEED;
    if (IsKeyDown(KEY_DOWN)) player->rect.y += PLAYER_SPEED;

    // Prevent player from leaving the screen
    if (player->rect.x < 0) player->rect.x = 0;
    if (player->rect.x + player->rect.width > SCREEN_WIDTH) player->rect.x = SCREEN_WIDTH - player->rect.width;
    if (player->rect.y < 0) player->rect.y = 0;
    if (player->rect.y + player->rect.height > SCREEN_HEIGHT) player->rect.y = SCREEN_HEIGHT - player->rect.height;
}

void DrawGame(const Player* player) {
    ClearBackground(RAYWHITE);
    DrawRectangleRec(player->rect, player->color);
    DrawText("Use arrow keys to move the square", 10, 10, 20, DARKGRAY);
    // Draw FPS counter in the top-right corner
    DrawFPS(SCREEN_WIDTH - 100, 10);
}
