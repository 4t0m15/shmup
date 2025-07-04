// Disable deprecation warnings for MSVC
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

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

// New constants for advanced features
#define MORPH_DURATION 2.0f        // Time for morphing animation
#define FLAGSHIP_SIZE 60           // Size of Galaxian Flagship
#define MAX_CAPTURED_SHIPS 2       // Maximum ships that can be captured
#define DUAL_FIGHTER_HITBOX_MULTIPLIER 1.5f  // Larger hitbox for dual fighter
#define HOSTILE_SHIP_DELAY 3       // Waves to wait before hostile ship appears
#define AGGRESSION_SCALE_RATE 0.1f // Rate at which aggression increases per wave
#define MORPH_CHANCE 15            // Percentage chance for enemy to morph

// Enhanced AI constants
#define AI_FLANKING_DISTANCE 80.0f // Distance for flanking maneuvers
#define AI_SWARM_RADIUS 60.0f      // Radius for swarm behavior
#define AI_FORMATION_STRICTNESS 0.7f // How strict formation flying is
#define AI_ATTACK_COOLDOWN 3.0f    // Cooldown between attacks
#define AI_EVASION_THRESHOLD 50.0f // Distance to trigger evasion
#define AI_PREDICTION_FRAMES 30    // Frames to predict player movement

// Mathematical constants
#ifndef PI
#define PI 3.14159265359f
#endif
#define TWO_PI 6.28318530718f

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
#define SCORE_FLAGSHIP_FORMATION 200
#define SCORE_FLAGSHIP_DIVE 800
#define SCORE_HOSTILE_SHIP_RESCUE 2000

// Lives and extends
#define STARTING_LIVES 3
#define FIRST_EXTEND_SCORE 20000
#define SECOND_EXTEND_SCORE 70000
#define MAX_LIVES 5

// Game states
typedef enum GameScreenState {
    MENU,
    PLAYING,
    GAME_OVER,
    PAUSED
} GameScreenState;

// Menu states
typedef enum MenuState {
    MAIN_MENU,
    OPTIONS_MENU,
    CREDITS_MENU
} MenuState;

// AI behavior types
typedef enum AIBehavior {
    AI_FORMATION_FLYING,
    AI_AGGRESSIVE_ATTACK,
    AI_FLANKING_MANEUVER,
    AI_SWARM_BEHAVIOR,
    AI_EVASIVE_MANEUVER,
    AI_COORDINATED_ATTACK,
    AI_DEFENSIVE_FORMATION
} AIBehavior;

typedef struct Player {
    Rectangle rect;
    Color color;
    bool captured;          // For tractor beam mechanics
    Vector2 capture_target; // Target position when captured
    bool dual_fire;         // For rescued ship mechanic
    int lives;
    bool extend_1_awarded;  // Track if first extend was awarded
    bool extend_2_awarded;  // Track if second extend was awarded
    
    // Enhanced dual fighter mechanics
    bool has_captured_ship;     // Player has a captured ship attached
    Vector2 captured_ship_offset; // Offset position for captured ship
    Rectangle dual_hitbox;      // Larger hitbox when dual fighter
    float dual_fighter_timer;   // Timer for dual fighter effects
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
    ESCORT,     // Butterfly (Goei)
    FLAGSHIP,   // Galaxian Flagship (morphed enemy)
    HOSTILE_SHIP // Hostile captured ship
} EnemyType;

typedef enum EnemyState {
    INACTIVE,       // Not on screen
    ENTERING,       // Moving into formation with patterns
    FORMATION,      // In formation, moving with the group
    ATTACKING,      // Diving/attacking the player
    SPECIAL_ATTACK, // Boss tractor beam attack
    RETURNING,      // Returning to formation after attack
    MORPHING,       // Morphing into a different enemy type
    CAPTURED_SHIP_HOLDING, // Boss holding a captured ship
    AI_FLANKING,    // Performing flanking maneuver
    AI_EVADING,     // Evading player attacks
    AI_COORDINATING // Coordinating with other enemies
} EnemyState;

typedef enum MovementPattern {
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
    
    // Morphing mechanics
    EnemyType original_type;  // Original type before morphing
    EnemyType target_type;    // Target type after morphing
    float morph_timer;        // Timer for morphing animation
    bool can_morph;           // Can this enemy morph?
    bool has_morphed;         // Has this enemy already morphed?
    
    // Captured ship mechanics
    bool has_captured_ship;   // Boss has a captured ship
    bool captured_ship_hostile; // Captured ship becomes hostile when destroyed
    int captured_ship_spawn_wave; // Wave when hostile ship should spawn
    
    // Aggression scaling
    float aggression_multiplier; // Multiplier for speed/fire rate based on wave
    
    // Enhanced AI mechanics
    AIBehavior ai_behavior;      // Current AI behavior
    float ai_timer;              // Timer for AI behavior changes
    Vector2 ai_target;           // AI target position
    Vector2 predicted_player_pos; // Predicted player position
    float last_player_distance;  // Distance to player last frame
    bool coordinating;           // Is coordinating with other enemies
    int coordination_group;      // Group ID for coordination
    float evasion_direction;     // Direction for evasion (-1 to 1)
    Vector2 last_velocity;       // Last frame's velocity for momentum
} Enemy;

typedef struct ScorePopup {
    Vector2 position;
    int score;
    float timer;
    bool active;
} ScorePopup;

typedef struct CapturedShip {
    Vector2 position;
    bool active;
    bool hostile;           // Whether this ship is hostile
    int spawn_wave;         // Wave when this ship should spawn as hostile
    bool rescued;           // Whether this ship was rescued
} CapturedShip;

// Menu system
typedef struct MenuSystem {
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
    
    // Captured ship tracking
    CapturedShip captured_ships[MAX_CAPTURED_SHIPS];
    int total_captured_ships;
    
    // Aggression scaling
    float base_aggression;  // Base aggression level
    
    // Random seed tracking
    unsigned int random_seed;  // Current random seed for display
    
    // Menu system
    MenuSystem menu;
    
    // Player position tracking for AI
    Vector2 player_positions[AI_PREDICTION_FRAMES];
    int player_position_index;
    
    // Pause functionality
    bool is_paused;
    float pause_timer;
} GameState;

// Game configuration structure for customizable initialization
typedef struct GameConfig {
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
    
    // Advanced features
    bool enable_morphing;
    bool enable_captured_ships;
    bool enable_bonus_stages;
    bool enable_aggression_scaling;
    bool enable_enhanced_ai;
} GameConfig;

// Default game configuration
extern const GameConfig DEFAULT_CONFIG;

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
void HandleMenuInput(GameState* gameState);

// Enhanced Enemy AI functions - enemy_ai.c
void UpdateEnemyAI(GameState* gameState, float delta);
void UpdateEnemyBehavior(GameState* gameState, Enemy* enemy, float delta);
void SetEnemyAIBehavior(Enemy* enemy, AIBehavior behavior);
Vector2 PredictPlayerPosition(const GameState* gameState, float prediction_time);
void UpdatePlayerPositionHistory(GameState* gameState);
bool ShouldEnemyEvade(const GameState* gameState, const Enemy* enemy);
void UpdateCoordinatedAttack(GameState* gameState, float delta);
Vector2 CalculateFlankingPosition(const GameState* gameState, const Enemy* enemy);
void UpdateSwarmBehavior(GameState* gameState, Enemy* enemy, float delta);

// Enemy functions - enemy.c
void UpdateEnemies(GameState* gameState, float delta);
void UpdateMorphing(GameState* gameState, float delta);
void TriggerMorphing(Enemy* enemy);
void UpdateCapturedShips(GameState* gameState, float delta);
void SpawnHostileShip(GameState* gameState, int spawn_wave);
void UpdateAggressionScaling(GameState* gameState);
void HandleShipCapture(GameState* gameState, Enemy* boss);
void HandleShipRescue(GameState* gameState, Enemy* boss);
void SpawnEnemyWave(GameState* gameState);
void UpdateTractorBeam(GameState* gameState, Enemy* boss, float delta);
void UpdateEnemyShooting(GameState* gameState, Enemy* enemy, float delta);
void UpdateEnemyBullets(GameState* gameState, float delta);

// Player functions - player.c
void UpdatePlayer(GameState* gameState, float delta);
void UpdateDualFighter(GameState* gameState, float delta);
void HandlePlayerInput(GameState* gameState, float delta);
void HandlePlayerShooting(GameState* gameState);

// Collision functions - collision.c
void CheckBulletEnemyCollisions(GameState* gameState);
bool CheckPlayerEnemyCollisions(GameState* gameState);
bool CheckCollisionCircleRect(Vector2 center, float radius, Rectangle rec);
bool CheckCollisionBulletRec(Vector2 bullet_pos, Rectangle rec, bool is_player_bullet);

// Score functions - score.c
void AddScore(GameState* gameState, int points, Vector2 position);
void CheckForExtends(GameState* gameState);
int CalculateEnemyScore(const Enemy* enemy);
void HandleEnemyDestroy(GameState* gameState, int enemy_index, Vector2 position);
void UpdateScorePopups(GameState* gameState, float delta);
void SpawnBonusStage(GameState* gameState);
void UpdateBonusStage(GameState* gameState, float delta);
void LoadHighScore(GameState* gameState);
void SaveHighScore(const GameState* gameState);

// Render functions - render.c
void DrawBullet(Vector2 position, Color color, bool is_player_bullet);
void DrawBackground(const GameState* gameState);
void DrawPlayer(const GameState* gameState);
void DrawEnemies(const GameState* gameState);
void DrawBullets(const GameState* gameState);
void DrawUI(const GameState* gameState);

// Utility functions - utils.c
Vector2 CalculateMovementPattern(Enemy* enemy, float delta);
Vector2 GameVector2Lerp(Vector2 start, Vector2 end, float t);
Vector2 BezierQuad(Vector2 start, Vector2 control, Vector2 end, float t);
void SeedRandomGenerator(GameState* gameState);

// Initialization functions - game.c
void InitializeBullets(GameState* gameState);
void InitializeEnemyBullets(GameState* gameState);
void InitializePlayer(GameState* gameState);
void InitializeEnemies(GameState* gameState);
void InitializeScorePopups(GameState* gameState);
void InitializeCapturedShips(GameState* gameState);
void InitializeGameVariables(GameState* gameState);

#endif // GAME_H
