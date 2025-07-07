#ifndef GAME_H
#define GAME_H

// Disable deprecation warnings for MSVC
#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <raylib.h>
#include <stdbool.h>
#include <time.h>

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

// Particle system constants
#define MAX_PARTICLES 500

// Audio system constants
#define MAX_SOUNDS 16
#define MAX_MUSIC 8

// Balance and power-up system constants
#define MAX_POWERUPS 5

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
    GAME_OVER
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

typedef enum {
    PARTICLE_EXPLOSION,
    PARTICLE_SPARK,
    PARTICLE_TRAIL,
    PARTICLE_SPARKLE,
    PARTICLE_SMOKE
} ParticleType;

typedef enum {
    SOUND_PLAYER_SHOOT,
    SOUND_ENEMY_SHOOT,
    SOUND_EXPLOSION_SMALL,
    SOUND_EXPLOSION_LARGE,
    SOUND_POWERUP,
    SOUND_HIT,
    SOUND_MENU_SELECT,
    SOUND_MENU_MOVE,
    SOUND_COUNT
} SoundType;

typedef enum {
    MUSIC_MENU,
    MUSIC_GAME,
    MUSIC_BOSS,
    MUSIC_COUNT
} MusicType;

typedef enum {
    GAME_SOUND_PLAYER_SHOOT,
    GAME_SOUND_ENEMY_SHOOT,
    GAME_SOUND_ENEMY_HIT,
    GAME_SOUND_PLAYER_HIT,
    GAME_SOUND_ENEMY_DESTROY_SMALL,
    GAME_SOUND_ENEMY_DESTROY_LARGE,
    GAME_SOUND_POWERUP,
    GAME_SOUND_MENU_MOVE,
    GAME_SOUND_MENU_SELECT
} GameSoundContext;

typedef enum {
    WEAPON_SINGLE,
    WEAPON_DOUBLE,
    WEAPON_TRIPLE,
    WEAPON_SPREAD,
    WEAPON_RAPID,
    WEAPON_LASER,
    WEAPON_HOMING,
    WEAPON_PLASMA,
    WEAPON_COUNT
} WeaponType;

typedef enum {
    BULLET_NORMAL,
    BULLET_LASER,
    BULLET_HOMING,
    BULLET_PLASMA
} BulletType;

typedef enum {
    POWERUP_RAPID_FIRE,
    POWERUP_SHIELD,
    POWERUP_SPREAD_SHOT,
    POWERUP_SLOW_MOTION,
    POWERUP_EXTRA_LIFE,
    POWERUP_SCORE_MULTIPLIER,
    POWERUP_COUNT
} PowerUpType;

typedef enum {
    SHADER_DISTORTION,
    SHADER_CHROMATIC,
    SHADER_BLOOM,
    SHADER_BLUR,
    SHADER_ENERGY_FIELD,
    SHADER_COUNT
} ShaderType;

typedef enum {
    ACHIEVEMENT_FIRST_KILL,
    ACHIEVEMENT_KILL_100,
    ACHIEVEMENT_KILL_1000,
    ACHIEVEMENT_KILL_5000,
    ACHIEVEMENT_WAVE_10,
    ACHIEVEMENT_WAVE_25,
    ACHIEVEMENT_WAVE_50,
    ACHIEVEMENT_WAVE_100,
    ACHIEVEMENT_SCORE_10K,
    ACHIEVEMENT_SCORE_50K,
    ACHIEVEMENT_SCORE_100K,
    ACHIEVEMENT_SCORE_500K,
    ACHIEVEMENT_ACCURACY_75,
    ACHIEVEMENT_ACCURACY_90,
    ACHIEVEMENT_ACCURACY_95,
    ACHIEVEMENT_PERFECT_ACCURACY,
    ACHIEVEMENT_NO_DEATH_WAVE_5,
    ACHIEVEMENT_NO_DEATH_WAVE_10,
    ACHIEVEMENT_NO_DEATH_WAVE_20,
    ACHIEVEMENT_PACIFIST,
    ACHIEVEMENT_COMBO_10,
    ACHIEVEMENT_COMBO_25,
    ACHIEVEMENT_COMBO_50,
    ACHIEVEMENT_COMBO_100,
    ACHIEVEMENT_BOSS_NO_DAMAGE,
    ACHIEVEMENT_DUAL_FIGHTER,
    ACHIEVEMENT_ALL_WEAPONS,
    ACHIEVEMENT_PERFECT_BONUS,
    ACHIEVEMENT_SPEED_RUN,
    ACHIEVEMENT_HOARDER,
    ACHIEVEMENT_MORPHING_MASTER,
    ACHIEVEMENT_AI_OBSERVER,
    ACHIEVEMENT_PLAY_TIME_1H,
    ACHIEVEMENT_PLAY_TIME_10H,
    ACHIEVEMENT_PLAY_TIME_50H,
    ACHIEVEMENT_COUNT
} AchievementID;

typedef enum {
    ACHIEVEMENT_BRONZE,
    ACHIEVEMENT_SILVER,
    ACHIEVEMENT_GOLD,
    ACHIEVEMENT_PLATINUM,
    ACHIEVEMENT_SPECIAL
} AchievementTier;

typedef enum {
    STAT_ENEMY_KILLED,
    STAT_WAVE_REACHED,
    STAT_SCORE_ADDED,
    STAT_SHOT_FIRED,
    STAT_SHOT_HIT,
    STAT_COMBO_ACHIEVED,
    STAT_BOSS_DEFEATED,
    STAT_POWER_UP_COLLECTED,
    STAT_SHIP_RESCUED,
    STAT_MORPHING_WITNESSED,
    STAT_PERFECT_BONUS,
    STAT_DEATH_OCCURRED,
    STAT_WAVE_COMPLETED,
    STAT_WEAPON_UNLOCKED,
    STAT_PACIFIST_WAVE,
    STAT_AI_BEHAVIOR_SEEN,
    STAT_GAME_STARTED,
    STAT_SPEED_RUN_COMPLETE
} StatType;

typedef enum {
    REWARD_WEAPON_UNLOCK,
    REWARD_LIFE_BONUS,
    REWARD_SCORE_MULTIPLIER,
    REWARD_SPECIAL_ABILITY
} SpecialRewardType;

typedef struct {
    int damage;
    float fire_rate;
    float bullet_speed;
    int bullet_count;
    float spread_angle;
    bool penetration;
    bool homing;
    float explosion_radius;
    bool unlocked;
} WeaponStats;

typedef struct {
    WeaponType current_weapon;
    int weapon_level;
    int max_level;
    WeaponStats weapon_stats[WEAPON_COUNT];
    int upgrade_points;
    float weapon_select_timer;
    bool show_weapon_ui;
} WeaponSystem;

typedef struct {
    AchievementID id;
    const char* name;
    const char* description;
    AchievementTier tier;
    bool unlocked;
    time_t unlock_time;
    int target_value;
} Achievement;

typedef struct {
    Achievement achievements[ACHIEVEMENT_COUNT];
    int total_achievements;
    int unlocked_count;
    float notification_timer;
    AchievementID current_notification;
    bool show_notification;
    
    struct {
        int total_enemies_killed;
        int total_score;
        int highest_wave;
        float total_play_time;
        int total_shots_fired;
        int total_shots_hit;
        int highest_combo;
        int bosses_defeated;
        int power_ups_collected;
        int ships_rescued;
        int morphings_witnessed;
        int perfect_bonus_stages;
        int no_death_waves;
        int current_no_death_streak;
        int ai_behaviors_seen;
        int weapons_unlocked;
        int pacifist_waves;
        float speed_run_time;
        int games_played;
    } stats;
} AchievementSystem;

typedef struct {
    const char* name;
    const char* description;
    AchievementTier tier;
    bool unlocked;
    time_t unlock_time;
    int target_value;
} AchievementDef;

typedef struct {
    int total_enemies_killed;
    int total_score;
    int highest_wave;
    float total_play_time;
    int total_shots_fired;
    int total_shots_hit;
    int highest_combo;
    int bosses_defeated;
    int power_ups_collected;
    int ships_rescued;
    int morphings_witnessed;
    int perfect_bonus_stages;
    int no_death_waves;
    int current_no_death_streak;
    int ai_behaviors_seen;
    int weapons_unlocked;
    int pacifist_waves;
    float speed_run_time;
    int games_played;
} AchievementStats;

typedef struct {
    Shader shaders[SHADER_COUNT];
    bool shader_loaded[SHADER_COUNT];
    
    // Shader parameters
    bool post_process_enabled;
    bool bloom_enabled;
    bool chromatic_aberration_enabled;
    bool distortion_enabled;
    bool energy_field_enabled;
    
    float bloom_threshold;
    float bloom_intensity;
    float chromatic_intensity;
    float distortion_intensity;
    float blur_strength;
    float shader_time;
    
    // Shader uniform locations
    int time_loc[SHADER_COUNT];
    int intensity_loc[SHADER_COUNT];
    int threshold_loc[SHADER_COUNT];
    int direction_loc[SHADER_COUNT];
    int strength_loc[SHADER_COUNT];
    int screen_size_loc[SHADER_COUNT];
    
    // Render textures
    RenderTexture2D screen_texture;
    RenderTexture2D bloom_texture;
    RenderTexture2D temp_texture;
} ShaderSystem;

// =============================================================================
// PARTICLE AND EFFECTS STRUCTURES
// =============================================================================

typedef struct {
    Vector2 position;
    Vector2 velocity;
    Color color;
    float size;
    float life;
    float max_life;
    bool active;
    ParticleType type;
} Particle;

typedef struct {
    Particle particles[MAX_PARTICLES];
    
    // Screen effects
    float screen_shake_intensity;
    float screen_shake_duration;
    Vector2 screen_offset;
    
    // Flash effects
    float flash_intensity;
    float flash_duration;
    Color flash_color;
} ParticleSystem;

typedef struct {
    Sound sound;
    bool loaded;
    float volume;
    float pitch;
} GameSound;

typedef struct {
    Music music;
    bool loaded;
    float volume;
} GameMusic;

typedef struct {
    GameSound sounds[MAX_SOUNDS];
    GameMusic music[MAX_MUSIC];
    
    float master_volume;
    float sfx_volume;
    float music_volume;
    
    int current_music;
    float fade_timer;
    float fade_duration;
    float fade_target_volume;
} AudioSystem;

typedef struct {
    Vector2 position;
    Vector2 velocity;
    PowerUpType type;
    float timer;
    float pulse_timer;
    bool active;
} PowerUp;

typedef struct {
    PowerUp powerups[MAX_POWERUPS];
    float spawn_timer;
    
    // Active power-up timers
    float rapid_fire_timer;
    float shield_timer;
    float spread_shot_timer;
    float slow_motion_timer;
} PowerUpSystem;

typedef struct {
    float difficulty_multiplier;
    float enemy_speed_multiplier;
    float enemy_health_multiplier;
    float player_damage_multiplier;
    float score_multiplier;
    float spawn_rate_multiplier;
    
    // Adaptive difficulty
    bool adaptive_difficulty;
    float player_skill_rating;
    float recent_performance;
    int deaths_this_session;
    float time_alive;
    int enemies_killed;
    int accuracy_shots_fired;
    int accuracy_shots_hit;
    
    // Combo system
    int power_level;
    float combo_multiplier;
    float combo_timer;
    int consecutive_hits;
    int max_combo;
} BalanceSystem;

typedef struct {
    bool auto_pause_on_focus_loss;
    bool show_hit_indicators;
    bool show_damage_numbers;
    bool screen_edge_warning;
    bool bullet_time_on_near_miss;
    bool auto_collect_powerups;
    
    float near_miss_timer;
    bool focus_lost;
    float edge_warning_timer;
} QoLSystem;

// =============================================================================
// CORE STRUCTURES
// =============================================================================

typedef struct {
    Vector2 position;
    Vector2 velocity;
    int damage;
    float lifetime;
    bool active;
    bool penetrating;
    bool homing;
    float explosion_radius;
    BulletType bullet_type;
    int target_index;
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
    ParticleSystem effects;
    AudioSystem audio;
    PowerUpSystem powerups;
    BalanceSystem balance;
    QoLSystem qol;
    WeaponSystem weapons;
    AchievementSystem achievements;
    ShaderSystem shaders;
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
bool CheckEnemyBulletPlayerCollisions(GameState* gameState);

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
float Vector2Distance(Vector2 v1, Vector2 v2);

// Effects functions - effects.c
void InitParticleSystem(ParticleSystem* system);
void SpawnParticle(ParticleSystem* system, Vector2 position, Vector2 velocity, 
                   Color color, float size, float life, ParticleType type);
void CreateExplosion(ParticleSystem* system, Vector2 position, Color color, int particle_count);
void CreateBulletTrail(ParticleSystem* system, Vector2 position, Vector2 velocity, Color color);
void CreatePowerUpEffect(ParticleSystem* system, Vector2 position);
void CreateHitEffect(ParticleSystem* system, Vector2 position, bool is_enemy_hit);
void UpdateParticleSystem(ParticleSystem* system, float delta);
void TriggerScreenShake(ParticleSystem* system, float intensity, float duration);
void TriggerScreenFlash(ParticleSystem* system, Color color, float duration);
void DrawParticleSystem(const ParticleSystem* system);
void DrawEnhancedBullet(Vector2 position, Color color, bool is_player_bullet, float trail_alpha);
void DrawEnhancedEnemy(const Enemy* enemy, float pulse_factor);

// Audio functions - audio.c
void InitAudioSystem(AudioSystem* audio);
void CleanupAudioSystem(AudioSystem* audio);
bool LoadSoundEffect(AudioSystem* audio, SoundType type);
bool LoadMusicTrack(AudioSystem* audio, MusicType type);
void PlaySoundEffect(AudioSystem* audio, SoundType type, float volume, float pitch);
void PlayMusicTrack(AudioSystem* audio, MusicType type);
void StopMusic(AudioSystem* audio);
void FadeMusic(AudioSystem* audio, float target_volume, float duration);
void UpdateAudioSystem(AudioSystem* audio, float delta);
void SetGameMasterVolume(AudioSystem* audio, float volume);
void SetGameSFXVolume(AudioSystem* audio, float volume);
void SetGameMusicVolume(AudioSystem* audio, float volume);
void InitAllAudioAssets(AudioSystem* audio);
void PlayGameSound(AudioSystem* audio, GameSoundContext context, float intensity);
float Clamp(float value, float min, float max);

// Balance and power-up functions - balance.c
void InitBalanceSystem(BalanceSystem* balance);
void UpdateAdaptiveDifficulty(BalanceSystem* balance, GameState* gameState, float delta);
void UpdateComboSystem(BalanceSystem* balance, float delta);
void RegisterPlayerShot(BalanceSystem* balance);
void RegisterHit(BalanceSystem* balance, bool was_enemy_killed);
void RegisterPlayerDeath(BalanceSystem* balance);
int CalculateScoreWithMultipliers(BalanceSystem* balance, int base_score);
int GetAdjustedEnemyHealth(BalanceSystem* balance, EnemyType type);
float GetAdjustedEnemySpeed(BalanceSystem* balance, float base_speed);

void InitPowerUpSystem(PowerUpSystem* powerups);
void SpawnPowerUp(PowerUpSystem* powerups, Vector2 position, PowerUpType type);
void UpdatePowerUpSystem(PowerUpSystem* powerups, GameState* gameState, float delta);
void CollectPowerUp(PowerUpSystem* powerups, GameState* gameState, PowerUpType type);
bool IsPowerUpActive(PowerUpSystem* powerups, PowerUpType type);
float GetPowerUpTimeRemaining(PowerUpSystem* powerups, PowerUpType type);
void DrawPowerUps(const PowerUpSystem* powerups);

void InitQoLSystem(QoLSystem* qol);
void UpdateQoLSystem(QoLSystem* qol, GameState* gameState, float delta);
void CheckForNearMisses(QoLSystem* qol, GameState* gameState);
void DrawQoLUI(const QoLSystem* qol, const GameState* gameState);

// Weapon system functions - weapons.c
void InitWeaponSystem(WeaponSystem* weapons);
void InitWeaponStats(WeaponSystem* weapons);
void UpdateWeaponSystem(WeaponSystem* weapons, GameState* gameState, float delta);
void SwitchWeapon(WeaponSystem* weapons, int direction);
void FireWeapon(WeaponSystem* weapons, GameState* gameState, Vector2 position);
void FireSingleShot(GameState* gameState, Vector2 position, WeaponStats* stats);
void FireDoubleShot(GameState* gameState, Vector2 position, WeaponStats* stats);
void FireTripleShot(GameState* gameState, Vector2 position, WeaponStats* stats);
void FireSpreadShot(GameState* gameState, Vector2 position, WeaponStats* stats);
void FireLaser(GameState* gameState, Vector2 position, WeaponStats* stats);
void FireHomingMissile(GameState* gameState, Vector2 position, WeaponStats* stats);
void FirePlasma(GameState* gameState, Vector2 position, WeaponStats* stats);
void UpgradeCurrentWeapon(WeaponSystem* weapons);
void UnlockWeapon(WeaponSystem* weapons, WeaponType weapon);
float GetWeaponFireRate(WeaponSystem* weapons);
bool IsWeaponUnlocked(WeaponSystem* weapons, WeaponType weapon);
const char* GetWeaponName(WeaponType weapon);
const char* GetWeaponDescription(WeaponType weapon);
void UpdateAdvancedBullets(GameState* gameState, float delta);
void UpdateHomingBehavior(GameState* gameState, Bullet* bullet, float delta);
void DrawWeaponUI(const WeaponSystem* weapons, const GameState* gameState);
void DrawAdvancedBullets(const GameState* gameState);

// Achievement system functions - achievements.c
void InitAchievementSystem(AchievementSystem* achievements);
void UpdateAchievementSystem(AchievementSystem* achievements, GameState* gameState, float delta);
void CheckAchievements(AchievementSystem* achievements, GameState* gameState);
void UnlockAchievement(AchievementSystem* achievements, AchievementID id);
void ShowAchievementNotification(AchievementSystem* achievements, AchievementID id);
int GetAchievementPoints(AchievementTier tier);
float GetAchievementProgress(const AchievementSystem* achievements, AchievementID id);
void UpdateAchievementStats(AchievementSystem* achievements, StatType stat, int value);
void DrawAchievementNotification(const AchievementSystem* achievements);
void DrawAchievementMenu(const AchievementSystem* achievements, int selected_index);
Color GetTierColor(AchievementTier tier);
const char* GetTierName(AchievementTier tier);
void SaveAchievements(const AchievementSystem* achievements);
void LoadAchievements(AchievementSystem* achievements);
void ResetAchievements(AchievementSystem* achievements);
int GetTotalAchievementScore(const AchievementSystem* achievements);
bool IsEligibleForSpecialReward(const AchievementSystem* achievements, SpecialRewardType reward);

// Shader system functions - shaders.c
void InitShaderSystem(ShaderSystem* shaders);
void LoadAllShaders(ShaderSystem* shaders);
void CreateShaderTextures(ShaderSystem* shaders);
void UpdateShaderSystem(ShaderSystem* shaders, float delta_time);
void BeginGameShaderMode(ShaderSystem* shaders);
void EndGameShaderMode(ShaderSystem* shaders);
void ApplyPostProcessing(ShaderSystem* shaders);
Texture2D ApplyBloomEffect(ShaderSystem* shaders, Texture2D input_texture);
Texture2D ApplyChromaticAberration(ShaderSystem* shaders, Texture2D input_texture);
Texture2D ApplyDistortion(ShaderSystem* shaders, Texture2D input_texture);
Texture2D ApplyEnergyField(ShaderSystem* shaders, Texture2D input_texture);
void SetShaderEffect(ShaderSystem* shaders, ShaderType type, bool enabled);
void SetShaderParameter(ShaderSystem* shaders, ShaderType type, const char* param_name, float value);
void TriggerShaderHitEffect(ShaderSystem* shaders);
void TriggerShaderExplosionEffect(ShaderSystem* shaders);
void TriggerShaderPowerUpEffect(ShaderSystem* shaders);
void ResetShaderEffects(ShaderSystem* shaders, float delta_time);
void CleanupShaderSystem(ShaderSystem* shaders);
void DrawShaderDebugUI(const ShaderSystem* shaders);

#endif // GAME_H
