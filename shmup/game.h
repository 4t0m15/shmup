#ifndef GAME_H
#define GAME_H

// Disable deprecation warnings for MSVC
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <raylib.h>
#include <stdbool.h>

// =============================================================================
// GAME CONSTANTS
// =============================================================================

// Display constants
#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 450
#define TARGET_FPS 90

// Player constants
#define PLAYER_SIZE 30
#define PLAYER_SPEED 5
#define STARTING_LIVES 3
#define MAX_LIVES 5

// Projectile constants
#define MAX_BULLETS 10
#define BULLET_SPEED 400.0f
#define BULLET_SIZE 5
#define MAX_ENEMY_BULLETS 20
#define ENEMY_BULLET_SPEED 200.0f

// Enemy constants
#define MAX_ENEMIES 16
#define ENEMY_SIZE 25
#define BOSS_SIZE 50
#define FLAGSHIP_SIZE 60
#define ENEMY_FORMATION_SPEED 100.0f
#define ENEMY_ATTACK_SPEED 200.0f
#define ENEMY_SWAY_AMPLITUDE 15.0f
#define ENEMY_ATTACK_CHANCE 2

// Visual effects constants
#define BACKGROUND_SCROLL_SPEED 50.0f
#define LOOP_RADIUS 60.0f
#define TRACTOR_BEAM_RANGE 120.0f
#define TRACTOR_BEAM_STRENGTH 150.0f

// Advanced mechanics constants
#define MORPH_DURATION 2.0f
#define MORPH_CHANCE 15
#define MAX_CAPTURED_SHIPS 2
#define DUAL_FIGHTER_HITBOX_MULTIPLIER 1.5f
#define HOSTILE_SHIP_DELAY 3
#define AGGRESSION_SCALE_RATE 0.1f

// AI constants
#define AI_FLANKING_DISTANCE 80.0f
#define AI_SWARM_RADIUS 60.0f
#define AI_FORMATION_STRICTNESS 0.7f
#define AI_ATTACK_COOLDOWN 3.0f
#define AI_EVASION_THRESHOLD 50.0f
#define AI_PREDICTION_FRAMES 30

// Mathematical constants
#ifndef PI
#define PI 3.14159265359f
#endif
#define TWO_PI 6.28318530718f

// Scoring constants
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
#define SCORE_FLAGSHIP_FORMATION 200
#define SCORE_FLAGSHIP_DIVE 800
#define SCORE_HOSTILE_SHIP_RESCUE 2000
#define FIRST_EXTEND_SCORE 20000
#define SECOND_EXTEND_SCORE 70000

// =============================================================================
// ENUMERATIONS
// =============================================================================

typedef enum {
    MENU,
    PLAYING,
    GAME_OVER,
    PAUSED
} GameScreenState;

typedef enum {
    MAIN_MENU,
    OPTIONS_MENU,
    CREDITS_MENU
} MenuState;

typedef enum {
    NORMAL,
    BOSS,
    ESCORT,
    FLAGSHIP,
    HOSTILE_SHIP
} EnemyType;

typedef enum {
    INACTIVE,
    ENTERING,
    FORMATION,
    ATTACKING,
    SPECIAL_ATTACK,
    RETURNING,
    MORPHING,
    CAPTURED_SHIP_HOLDING,
    AI_FLANKING,
    AI_EVADING,
    AI_COORDINATING
} EnemyState;

typedef enum {
    PATTERN_STRAIGHT,
    PATTERN_ARC,
    PATTERN_SPIRAL,
    PATTERN_SWIRL,
    PATTERN_LOOP,
    PATTERN_BEAM,
    PATTERN_CURVE,
    PATTERN_ZIGZAG,
    PATTERN_SINE_WAVE,
    PATTERN_FIGURE_EIGHT
} MovementPattern;

typedef enum {
    AI_FORMATION_FLYING,
    AI_AGGRESSIVE_ATTACK,
    AI_FLANKING_MANEUVER,
    AI_SWARM_BEHAVIOR,
    AI_EVASIVE_MANEUVER,
    AI_COORDINATED_ATTACK,
    AI_DEFENSIVE_FORMATION
} AIBehavior;

// =============================================================================
// CORE STRUCTURES
// =============================================================================

typedef struct {
    Vector2 position;
    bool active;
} Bullet;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    bool active;
} EnemyBullet;

typedef struct {
    Rectangle rect;
    Color color;
    bool captured;
    Vector2 capture_target;
    bool dual_fire;
    int lives;
    bool extend_1_awarded;
    bool extend_2_awarded;
    
    // Enhanced dual fighter mechanics
    bool has_captured_ship;
    Vector2 captured_ship_offset;
    Rectangle dual_hitbox;
    float dual_fighter_timer;
} Player;

typedef struct {
    Vector2 position;
    Vector2 formation_pos;
    Vector2 entry_start;
    Vector2 attack_start;
    EnemyState state;
    EnemyType type;
    MovementPattern pattern;
    float timer;
    float pattern_progress;
    float pattern_param;
    int health;
    bool active;
    bool shooting;
    float shoot_timer;
    
    // Tractor beam mechanics
    bool tractor_active;
    float tractor_angle;
    Vector2 tractor_center;
    
    // Scoring and combo mechanics
    bool is_escort_in_combo;
    int escort_group_id;
    
    // Morphing mechanics
    EnemyType original_type;
    EnemyType target_type;
    float morph_timer;
    bool can_morph;
    bool has_morphed;
    
    // Captured ship mechanics
    bool has_captured_ship;
    bool captured_ship_hostile;
    int captured_ship_spawn_wave;
    
    // Difficulty scaling
    float aggression_multiplier;
    
    // Enhanced AI mechanics
    AIBehavior ai_behavior;
    float ai_timer;
    Vector2 ai_target;
    Vector2 predicted_player_pos;
    float last_player_distance;
    bool coordinating;
    int coordination_group;
    float evasion_direction;
    Vector2 last_velocity;
} Enemy;

typedef struct {
    Vector2 position;
    int score;
    float timer;
    bool active;
} ScorePopup;

typedef struct {
    Vector2 position;
    bool active;
    bool hostile;
    int spawn_wave;
    bool rescued;
} CapturedShip;

typedef struct {
    MenuState current_menu;
    int selected_option;
    float transition_timer;
    bool show_instructions;
    float instruction_timer;
    
    // Options
    float music_volume;
    float sfx_volume;
    int difficulty;
    bool show_fps;
} MenuSystem;

typedef struct {
    Player player;
    float backgroundScrollY;
    Bullet bullets[MAX_BULLETS];
    EnemyBullet enemy_bullets[MAX_ENEMY_BULLETS];
    float shootCooldown;
    Enemy enemies[MAX_ENEMIES];
    int wave_number;
    float wave_timer;
    int boss_wave_interval;
    
    // Scoring system
    int score;
    int high_score;
    ScorePopup score_popups[10];
    
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
    float game_over_timer;
    
    // Advanced mechanics
    CapturedShip captured_ships[MAX_CAPTURED_SHIPS];
    int total_captured_ships;
    float base_aggression;
    unsigned int random_seed;
    
    // Systems
    MenuSystem menu;
    Vector2 player_positions[AI_PREDICTION_FRAMES];
    int player_position_index;
    bool is_paused;
    float pause_timer;
} GameState;

typedef struct {
    // Difficulty settings
    int starting_lives;
    int boss_wave_interval;
    float base_aggression;
    float morph_chance_percentage;
    
    // Player settings
    Vector2 player_start_position;
    bool enable_dual_fighter;
    
    // Scoring settings
    int first_extend_score;
    int second_extend_score;
    int max_lives;
    
    // Feature flags
    bool enable_morphing;
    bool enable_captured_ships;
    bool enable_bonus_stages;
    bool enable_aggression_scaling;
    bool enable_enhanced_ai;
} GameConfig;

// Default game configuration
extern const GameConfig DEFAULT_CONFIG;

// =============================================================================
// FUNCTION DECLARATIONS
// =============================================================================

// Core game functions - game.c
void InitGame(GameState* gameState);
void InitGameWithConfig(GameState* gameState, const GameConfig* config);
bool ValidateGameState(const GameState* gameState);
void ResetGameState(GameState* gameState);
void UpdateGame(GameState* gameState, float delta);
void DrawGame(const GameState* gameState);
void HandleGameOver(GameState* gameState);
void UpdateGameOver(GameState* gameState, float delta);
void DrawGameOver(const GameState* gameState);

// Menu functions - menu.c
void InitMenu(MenuSystem* menu);
void UpdateMenu(GameState* gameState, float delta);
void DrawMenu(const GameState* gameState);
void LoadHighScore(GameState* gameState);
void SaveHighScore(GameState* gameState);

// Player functions - player.c
void UpdatePlayer(GameState* gameState, float delta);
void DrawPlayer(const GameState* gameState);
void DrawBullets(const GameState* gameState);

// Enemy functions - enemy.c
void UpdateEnemies(GameState* gameState, float delta);
void DrawEnemies(const GameState* gameState);
void SpawnEnemyWave(GameState* gameState);
void HandleShipCapture(GameState* gameState, Enemy* boss);
void HandleShipRescue(GameState* gameState, Enemy* boss);
void SpawnHostileShip(GameState* gameState, int spawn_wave);
void UpdateMorphing(GameState* gameState, float delta);
void TriggerMorphing(Enemy* enemy);
void UpdateCapturedShips(GameState* gameState, float delta);
void UpdateAggressionScaling(GameState* gameState);
void UpdateTractorBeam(GameState* gameState, Enemy* boss, float delta);
void UpdateEnemyShooting(GameState* gameState, Enemy* enemy, float delta);

// Enemy AI functions - enemy_ai.c
void UpdateEnemyAI(GameState* gameState, float delta);
void UpdateEnemyBullets(GameState* gameState, float delta);

// Collision functions - collision.c
bool CheckPlayerEnemyCollisions(GameState* gameState);
void CheckBulletEnemyCollisions(GameState* gameState);

// Utility functions - utils.c
void DrawBackground(const GameState* gameState);
void DrawUI(const GameState* gameState);
void UpdateScorePopups(GameState* gameState, float delta);
void CheckForExtends(GameState* gameState);
void UpdateBonusStage(GameState* gameState, float delta);

// Score functions - score.c
void AddScore(GameState* gameState, int points, Vector2 position);
void SpawnBonusStage(GameState* gameState);
void HandleEnemyDestroy(GameState* gameState, int enemy_index, Vector2 position);
int CalculateEnemyScore(const Enemy* enemy);

// Utility functions
Vector2 CalculateMovementPattern(Enemy* enemy, float delta);
Vector2 GameVector2Lerp(Vector2 start, Vector2 end, float t);
Vector2 BezierQuad(Vector2 start, Vector2 control, Vector2 end, float t);
void SeedRandomGenerator(GameState* gameState);

#endif // GAME_H
