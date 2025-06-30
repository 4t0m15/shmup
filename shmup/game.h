#ifndef GAME_H
#define GAME_H
#include <raylib.h>

// Game constants
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450
#define PLAYER_SIZE 30
#define PLAYER_SPEED 5

typedef struct Player {
    Rectangle rect;
    Color color;
} Player;

void InitGame(Player* player);
void UpdateGame(Player* player);
void DrawGame(const Player* player);

#endif // GAME_H
