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
#define LOOP_RADIUS 60.0f

// Scoring constants (based on Galaga scoring system)
#define SCORE_BEE_FORMATION 50
#define SCORE_BEE_DIVE 100
#define SCORE_BUTTERFLY_FORMATION 80
#define SCORE_BUTTERFLY_DIVE 160
#define SCORE_BOSS_FORMATION 150
#define SCORE_BOSS_DIVE 400
#define SCORE_BOSS_ESCORT_COMBO 1600
#define SCORE_CAPTURED_SHIP_RESCUE 1000
#define SCORE_BONUS_STAGE_PERFECT 10000
#define SCORE_BONUS_STAGE_39 5000
#define SCORE_BONUS_STAGE_38 2000
#define SCORE_BONUS_STAGE_37 1000
#define SCORE_BONUS_STAGE_36 500
#define SCORE_BONUS_STAGE_BASE 100

// Lives and extends
#define STARTING_LIVES 3
#define FIRST_EXTEND_SCORE 20000
#define SECOND_EXTEND_SCORE 70000
#define MAX_LIVES 5

// Game states
typedef enum GameScreenState {
    PLAYING,
    GAME_OVER
} GameScreenState;

typedef struct Player {
    Rectangle rect;
    Color color;
    bool captured;          // For tractor beam mechanics
    Vector2 capture_target; // Target position when captured
    bool dual_fire;         // For rescued ship mechanic
    int lives;
    bool extend_1_awarded;  // Track if first extend was awarded
    bool extend_2_awarded;  // Track if second extend was awarded
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
    NORMAL,     // Bee (Zako)
    BOSS,       // Boss Galaga
    ESCORT      // Butterfly (Goei)
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
    
    // Scoring related
    bool is_escort_in_combo; // For boss+escort combo scoring
    int escort_group_id;     // Group ID for escort formations
} Enemy;

typedef struct ScorePopup {
    Vector2 position;
    int score;
    float timer;
    bool active;
} ScorePopup;

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
    
    // Scoring system
    int score;
    int high_score;
    ScorePopup score_popups[10]; // Visual score popups
    
    // Bonus stage tracking
    bool is_bonus_stage;
    int bonus_stage_enemies_hit;
    int bonus_stage_total_enemies;
    float bonus_stage_timer;
    
    // Combo tracking
    bool boss_escort_combo_active;
    int boss_escort_combo_count;
    float combo_timer;
    
    // Game state management
    GameScreenState screen_state;
    float game_over_timer;  // Timer for game over screen
} GameState;

void InitGame(GameState* gameState);
void UpdateGame(GameState* gameState, float delta);
void DrawGame(const GameState* gameState);

// Game state functions
void HandleGameOver(GameState* gameState);
void UpdateGameOver(GameState* gameState, float delta);
void DrawGameOver(const GameState* gameState);

// High score functions
void LoadHighScore(GameState* gameState);
void SaveHighScore(const GameState* gameState);

// Scoring functions
void AddScore(GameState* gameState, int points, Vector2 position);
void CheckForExtends(GameState* gameState);
int CalculateEnemyScore(const Enemy* enemy);
void HandleEnemyDestroy(GameState* gameState, int enemy_index, Vector2 position);
void UpdateScorePopups(GameState* gameState, float delta);
void SpawnBonusStage(GameState* gameState);
void UpdateBonusStage(GameState* gameState, float delta);

#endif // GAME_H
