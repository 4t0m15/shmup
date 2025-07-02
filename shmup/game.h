#ifndef GAME_H
#define GAME_H
#include <raylib.h>
#include "assets.h"

// Game constants
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450
#define PLAYER_SIZE 30
#define PLAYER_SPEED 5
#define BACKGROUND_SCROLL_SPEED 50.0f
#define MAX_BULLETS 10
#define BULLET_SPEED 400.0f
#define BULLET_SIZE 5

typedef struct Player {
    Rectangle rect;
    Color color;
} Player;

typedef struct Bullet {
    Vector2 position;
    bool active;
} Bullet;

typedef struct GameState {
    Player player;
    float backgroundScrollY; // Y position for scrolling background
    Bullet bullets[MAX_BULLETS];
    float shootCooldown;
} GameState;

void InitGame(GameState* gameState);
void UpdateGame(GameState* gameState, float delta, const Assets* assets);
void DrawGame(const GameState* gameState, const Assets* assets);

#endif // GAME_H
