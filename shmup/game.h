#ifndef GAME_H
#define GAME_H
#include <raylib.h>

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
#define BOSS_SIZE 50
#define ENEMY_FORMATION_SPEED 100.0f
#define ENEMY_ATTACK_SPEED 200.0f
#define ENEMY_SWAY_AMPLITUDE 15.0f
#define ENEMY_ATTACK_CHANCE 2 // Percentage chance per frame when timer expires
#define TRACTOR_BEAM_RANGE 120.0f
#define TRACTOR_BEAM_STRENGTH 150.0f
#define MAX_ENEMY_BULLETS 20
#define ENEMY_BULLET_SPEED 200.0f

typedef struct Player {
    Rectangle rect;
    Color color;
    bool captured;          // For tractor beam mechanics
    Vector2 capture_target; // Target position when captured
} Player;

typedef struct Bullet {
    Vector2 position;
    bool active;
} Bullet;

typedef struct EnemyBullet {
    Vector2 position;
    Vector2 velocity;
    bool active;
} EnemyBullet;

typedef enum EnemyType {
    NORMAL,
    BOSS,
    ESCORT
} EnemyType;

typedef enum EnemyState {
    INACTIVE,       // Not on screen
    ENTERING,       // Moving into formation with patterns
    FORMATION,      // In formation, moving with the group
    ATTACKING,      // Diving/attacking the player
    SPECIAL_ATTACK, // Boss tractor beam attack
    RETURNING       // Returning to formation after attack
} EnemyState;

typedef enum MovementPattern {
    PATTERN_STRAIGHT,
    PATTERN_ARC,
    PATTERN_SPIRAL,
    PATTERN_SWIRL,
    PATTERN_LOOP,
    PATTERN_BEAM,
    PATTERN_CURVE
} MovementPattern;

typedef struct Enemy {
    Vector2 position;       // Current position
    Vector2 formation_pos;  // Target position in the formation
    Vector2 entry_start;    // Starting position for entry pattern
    Vector2 attack_start;   // Starting position for attack pattern
    EnemyState state;
    EnemyType type;
    MovementPattern pattern;
    float timer;            // For state transitions or movement patterns
    float pattern_progress; // Progress through movement pattern (0.0 to 1.0)
    float pattern_param;    // Additional parameter for pattern variation
    int health;             // Enemy health (normal=1, boss=5)
    bool active;
    bool shooting;          // Can this enemy shoot?
    float shoot_timer;      // Time until next shot
    
    // Tractor beam (boss only)
    bool tractor_active;
    float tractor_angle;
    Vector2 tractor_center;
} Enemy;

typedef struct GameState {
    Player player;
    float backgroundScrollY; // Y position for scrolling background
    Bullet bullets[MAX_BULLETS];
    EnemyBullet enemy_bullets[MAX_ENEMY_BULLETS];
    float shootCooldown;
    Enemy enemies[MAX_ENEMIES];
    int wave_number;
    float wave_timer;
    int boss_wave_interval; // Every N waves spawn a boss
} GameState;

void InitGame(GameState* gameState);
void UpdateGame(GameState* gameState, float delta);
void DrawGame(const GameState* gameState);

#endif // GAME_H
