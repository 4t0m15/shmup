use ggez::graphics::Color;

// Screen
pub const SCREEN_WIDTH: f32 = 800.0;
pub const SCREEN_HEIGHT: f32 = 600.0;

// Player
pub const PLAYER_SPEED: f32 = 300.0;
pub const PLAYER_SIZE: f32 = 20.0;

// Bullets
pub const BULLET_SPEED: f32 = 500.0;
pub const BULLET_SIZE: f32 = 5.0;

// Enemies
pub const ENEMY_SPEED: f32 = 150.0;
pub const ENEMY_SIZE: f32 = 25.0;
pub const FAST_ENEMY_SIZE: f32 = 15.0;
pub const FAST_ENEMY_SPEED: f32 = 300.0;
pub const BIG_ENEMY_SIZE: f32 = 40.0;
pub const BIG_ENEMY_SPEED: f32 = 80.0;
pub const BIG_ENEMY_SCORE: u32 = 30;

// Stars
pub const STAR_COUNT: usize = 100;
pub const STAR_SPEED: f32 = 50.0;

// Effects
pub const EXPLOSION_DURATION: f32 = 0.4;
pub const PARTICLE_COUNT: usize = 20;

// Power-ups
pub const POWERUP_DURATION: f32 = 10.0;

// Combo system
pub const COMBO_TIMEOUT: f32 = 2.0; // Time before combo resets
pub const COMBO_MULTIPLIER_BASE: f32 = 1.5; // Base multiplier increase per combo
pub const COMBO_MULTIPLIER_CAP: f32 = 5.0; // Maximum combo multiplier

// Laser weapon constants
pub const LASER_CHARGE_TIME: f32 = 2.0; // Time to fully charge laser
pub const LASER_DAMAGE_BASE: f32 = 50.0; // Base damage per tick
pub const LASER_DAMAGE_MAX: f32 = 200.0; // Maximum damage at full charge
pub const LASER_WIDTH_BASE: f32 = 8.0; // Base laser width
pub const LASER_WIDTH_MAX: f32 = 20.0; // Maximum laser width at full charge
pub const LASER_COLOR_BASE: Color = Color::new(0.0, 1.0, 0.0, 1.0); // Green
pub const LASER_COLOR_CHARGED: Color = Color::new(1.0, 0.0, 0.0, 1.0); // Red when fully charged 