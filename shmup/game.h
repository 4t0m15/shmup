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
#define MAX_ENEMIES 16
#define ENEMY_SIZE 25
#define ENEMY_FORMATION_SPEED 100.0f
#define ENEMY_ATTACK_SPEED 200.0f
#define ENEMY_SWAY_AMPLITUDE 15.0f
#define ENEMY_ATTACK_CHANCE 2 // Percentage chance per frame when timer expires

typedef struct Player {
    Rectangle rect;
    Color color;
} Player;

typedef struct Bullet {
    Vector2 position;
    bool active;
} Bullet;

typedef enum EnemyState {
    INACTIVE,  // Not on screen
    ENTERING,  // Moving into formation
    FORMATION, // In formation, moving with the group
    ATTACKING  // Diving/attacking the player
} EnemyState;

typedef struct Enemy {
    Vector2 position;       // Current position
    Vector2 formation_pos;  // Target position in the formation
    EnemyState state;
    float timer;            // For state transitions or movement patterns
    bool active;
} Enemy;

typedef struct GameState {
    Player player;
    float backgroundScrollY; // Y position for scrolling background
    Bullet bullets[MAX_BULLETS];
    float shootCooldown;
    Enemy enemies[MAX_ENEMIES];
} GameState;

void InitGame(GameState* gameState);
void UpdateGame(GameState* gameState, float delta, const Assets* assets);
void DrawGame(const GameState* gameState, const Assets* assets);

#endif // GAME_H
