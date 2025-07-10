use ggez::graphics::Color;
use std::sync::OnceLock;

// Base resolution (design resolution)
pub const BASE_SCREEN_WIDTH: f32 = 1920.0;
pub const BASE_SCREEN_HEIGHT: f32 = 1080.0;

// Screen (will be updated with actual screen dimensions)
pub static mut SCREEN_WIDTH: f32 = 1920.0;
pub static mut SCREEN_HEIGHT: f32 = 1080.0;

// Global scaling instance
static GLOBAL_SCALING: OnceLock<Scaling> = OnceLock::new();

// Scaling system
#[derive(Debug)]
pub struct Scaling {
    pub scale_x: f32,
    pub scale_y: f32,
    pub scale_factor: f32, // Use the smaller scale to maintain aspect ratio
}

impl Scaling {
    pub fn new(actual_width: f32, actual_height: f32) -> Self {
        let scale_x = actual_width / BASE_SCREEN_WIDTH;
        let scale_y = actual_height / BASE_SCREEN_HEIGHT;
        let scale_factor = scale_x.min(scale_y); // Use smaller scale to maintain aspect ratio
        
        Self {
            scale_x,
            scale_y,
            scale_factor,
        }
    }
    
    pub fn scale_position(&self, x: f32, y: f32) -> (f32, f32) {
        (x * self.scale_x, y * self.scale_y)
    }
    
    pub fn scale_size(&self, size: f32) -> f32 {
        // Use AGGRESSIVENESS for size scaling
        size * self.scale_factor * AGGRESSIVENESS
    }
    
    pub fn scale_speed(&self, speed: f32) -> f32 {
        // Use AGGRESSIVENESS for speed scaling (less aggressive than size)
        speed * self.scale_factor * (1.0 + (AGGRESSIVENESS - 1.0) * 0.5)
    }
}

// Global scaling functions
pub fn init_scaling(actual_width: f32, actual_height: f32) {
    let scaling = Scaling::new(actual_width, actual_height);
    GLOBAL_SCALING.set(scaling).expect("Failed to set global scaling");
}

pub fn get_scaling() -> &'static Scaling {
    GLOBAL_SCALING.get().expect("Global scaling not initialized")
}

// Safe screen dimension getters
pub fn get_screen_width() -> f32 {
    unsafe { SCREEN_WIDTH }
}

pub fn get_screen_height() -> f32 {
    unsafe { SCREEN_HEIGHT }
}

// Player
pub const PLAYER_SPEED: f32 = 300.0;
pub const PLAYER_SIZE: f32 = 20.0;

// Bullets
pub const BULLET_SPEED: f32 = 500.0;
pub const BULLET_SIZE: f32 = 5.0;

// Enemies
pub const ENEMY_SPEED: f32 = 150.0;
pub const ENEMY_SIZE: f32 = 15.0;
pub const FAST_ENEMY_SIZE: f32 = 15.0;
pub const FAST_ENEMY_SPEED: f32 = 300.0;
pub const BIG_ENEMY_SIZE: f32 = 40.0;
pub const BIG_ENEMY_SPEED: f32 = 80.0;
pub const BIG_ENEMY_SCORE: u32 = 30;
// Base amount of enemies per spawn wave (scales with AGGRESSIVENESS and screen size)
pub const ENEMY_BASE_AMOUNT: f32 = 3.0;

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
pub const COMBO_MULTIPLIER_CAP: f32 = 50.0; // Maximum combo multiplier (increased for new tiers)

// Laser weapon constants
pub const LASER_CHARGE_TIME: f32 = 2.0; // Time to fully charge laser
pub const LASER_DAMAGE_BASE: f32 = 50.0; // Base damage per tick
pub const LASER_DAMAGE_MAX: f32 = 200.0; // Maximum damage at full charge
pub const LASER_WIDTH_BASE: f32 = 8.0; // Base laser width
pub const LASER_WIDTH_MAX: f32 = 20.0; // Maximum laser width at full charge
pub const LASER_COLOR_BASE: Color = Color::new(0.0, 1.0, 0.0, 1.0); // Green
pub const LASER_COLOR_CHARGED: Color = Color::new(1.0, 0.0, 0.0, 1.0); // Red when fully charged

// Zenith enemy constants - much more aggressive
pub const ZENITH_SIZE: f32 = 35.0;
pub const ZENITH_SPEED: f32 = 120.0;
pub const ZENITH_BEAM_CHARGE: f32 = 0.8; // Much faster charge time
pub const ZENITH_BEAM_DURATION: f32 = 3.0; // Longer beam duration
pub const ZENITH_COOLDOWN: f32 = 1.5; // Much shorter cooldown
pub const ZENITH_BEAM_WIDTH: f32 = 20.0; // Wider beam
pub const ZENITH_BEAM_SPEED: f32 = 250.0; // Faster pull speed
// Zenith spawn chance is now hardcoded in spawn_enemy() for more aggressive spawning 

// Aggressiveness factor: controls enemy size, speed, and spawn count
pub const AGGRESSIVENESS: f32 = 1.5; // 1.0 = normal, higher = more aggressive 