use ggez::{
    conf::{WindowMode, WindowSetup},
    event::{self, EventHandler},
    graphics::{self, Color, DrawMode, DrawParam, Mesh, Rect, Text},
    input::keyboard::{KeyCode, KeyInput},
    Context, GameResult,
};
use ggez::glam::Vec2;
use rand::Rng;
use std::fs;
use std::io::Write;
use std::collections::HashSet;

const SCREEN_WIDTH: f32 = 800.0;
const SCREEN_HEIGHT: f32 = 600.0;
const PLAYER_SPEED: f32 = 300.0;
const BULLET_SPEED: f32 = 500.0;
const ENEMY_SPEED: f32 = 150.0;
const PLAYER_SIZE: f32 = 20.0;
const BULLET_SIZE: f32 = 5.0;
const ENEMY_SIZE: f32 = 25.0;
const FAST_ENEMY_SIZE: f32 = 15.0;
const FAST_ENEMY_SPEED: f32 = 300.0;
const BIG_ENEMY_SIZE: f32 = 40.0;
const BIG_ENEMY_SPEED: f32 = 80.0;
const BIG_ENEMY_SCORE: u32 = 30;
const STAR_COUNT: usize = 100;
const STAR_SPEED: f32 = 50.0;
const EXPLOSION_DURATION: f32 = 0.4;
const PARTICLE_COUNT: usize = 20;
const POWERUP_DURATION: f32 = 10.0;
const COMBO_TIMEOUT: f32 = 2.0; // Time before combo resets
const COMBO_MULTIPLIER_BASE: f32 = 1.5; // Base multiplier increase per combo
const COMBO_MULTIPLIER_CAP: f32 = 5.0; // Maximum combo multiplier

// Laser weapon constants
const LASER_CHARGE_TIME: f32 = 2.0; // Time to fully charge laser
const LASER_DAMAGE_BASE: f32 = 50.0; // Base damage per tick
const LASER_DAMAGE_MAX: f32 = 200.0; // Maximum damage at full charge
const LASER_WIDTH_BASE: f32 = 8.0; // Base laser width
const LASER_WIDTH_MAX: f32 = 20.0; // Maximum laser width at full charge
const LASER_COLOR_BASE: Color = Color::new(0.0, 1.0, 0.0, 1.0); // Green
const LASER_COLOR_CHARGED: Color = Color::new(1.0, 0.0, 0.0, 1.0); // Red when fully charged

#[derive(Clone, Copy, PartialEq)]
enum EnemyType {
    Normal,
    Fast,
    Big,
}

#[derive(Clone, Copy, PartialEq)]
enum BossType {
    Destroyer,
    Carrier,
    Behemoth,
}

#[derive(Clone, Copy, PartialEq)]
enum PowerUpType {
    RapidFire,
    TripleShot,
    Shield,
    Laser,
}

#[derive(Clone)]
struct Boss {
    position: Vec2,
    velocity: Vec2,
    size: f32,
    health: f32,
    max_health: f32,
    boss_type: BossType,
    phase: u32,
    attack_timer: f32,
    attack_pattern: f32,
    movement_timer: f32,
    active: bool,
}

impl Boss {
    fn new(boss_type: BossType) -> Self {
        let (size, health) = match boss_type {
            BossType::Destroyer => (60.0, 300.0),
            BossType::Carrier => (80.0, 500.0),
            BossType::Behemoth => (100.0, 800.0),
        };
        
        Self {
            position: Vec2::new(SCREEN_WIDTH / 2.0, -size),
            velocity: Vec2::new(0.0, 50.0),
            size,
            health,
            max_health: health,
            boss_type,
            phase: 1,
            attack_timer: 0.0,
            attack_pattern: 0.0,
            movement_timer: 0.0,
            active: true,
        }
    }

    fn update(&mut self, dt: f32) {
        self.position += self.velocity * dt;
        self.attack_timer += dt;
        self.attack_pattern += dt;
        self.movement_timer += dt;
        
        // Move to battle position
        if self.position.y < 100.0 {
            self.velocity.y = 50.0;
        } else {
            self.velocity.y = 0.0;
            self.position.y = 100.0;
        }
        
        // Boss movement patterns
        match self.boss_type {
            BossType::Destroyer => {
                if self.movement_timer > 2.0 {
                    self.velocity.x = if self.position.x < SCREEN_WIDTH / 2.0 { 100.0 } else { -100.0 };
                    self.movement_timer = 0.0;
                }
            }
            BossType::Carrier => {
                let sine_wave = (self.movement_timer * 0.5).sin() * 200.0;
                self.velocity.x = sine_wave;
            }
            BossType::Behemoth => {
                if self.movement_timer > 3.0 {
                    self.velocity.x = if self.velocity.x > 0.0 { -80.0 } else { 80.0 };
                    self.movement_timer = 0.0;
                }
            }
        }
        
        // Keep boss in bounds
        self.position.x = self.position.x.clamp(self.size / 2.0, SCREEN_WIDTH - self.size / 2.0);
    }

    fn take_damage(&mut self, damage: f32) {
        self.health -= damage;
        if self.health <= 0.0 {
            self.active = false;
        } else if self.health <= self.max_health * 0.5 && self.phase == 1 {
            self.phase = 2;
            // Phase 2: More aggressive
        } else if self.health <= self.max_health * 0.25 && self.phase == 2 {
            self.phase = 3;
            // Phase 3: Desperate attacks
        }
    }

    fn get_bullet_spawn_points(&self) -> Vec<Vec2> {
        match self.boss_type {
            BossType::Destroyer => {
                vec![
                    self.position + Vec2::new(-20.0, self.size / 2.0),
                    self.position + Vec2::new(20.0, self.size / 2.0),
                ]
            }
            BossType::Carrier => {
                vec![
                    self.position + Vec2::new(-30.0, self.size / 2.0),
                    self.position + Vec2::new(0.0, self.size / 2.0),
                    self.position + Vec2::new(30.0, self.size / 2.0),
                ]
            }
            BossType::Behemoth => {
                vec![
                    self.position + Vec2::new(-40.0, self.size / 2.0),
                    self.position + Vec2::new(-20.0, self.size / 2.0),
                    self.position + Vec2::new(0.0, self.size / 2.0),
                    self.position + Vec2::new(20.0, self.size / 2.0),
                    self.position + Vec2::new(40.0, self.size / 2.0),
                ]
            }
        }
    }

    fn get_color(&self) -> Color {
        let _health_percent = self.health / self.max_health;
        match self.boss_type {
            BossType::Destroyer => Color::new(1.0, 0.2, 0.2, 1.0),
            BossType::Carrier => Color::new(0.8, 0.2, 0.8, 1.0),
            BossType::Behemoth => Color::new(0.2, 0.2, 1.0, 1.0),
        }
    }
}

#[derive(Clone)]
struct GameObject {
    position: Vec2,
    velocity: Vec2,
    size: f32,
    active: bool,
    enemy_type: Option<EnemyType>, // None for non-enemies
}

impl GameObject {
    fn new(position: Vec2, velocity: Vec2, size: f32) -> Self {
        Self {
            position,
            velocity,
            size,
            active: true,
            enemy_type: None,
        }
    }

    fn new_enemy(position: Vec2, velocity: Vec2, size: f32, enemy_type: EnemyType) -> Self {
        Self {
            position,
            velocity,
            size,
            active: true,
            enemy_type: Some(enemy_type),
        }
    }

    fn update(&mut self, dt: f32) {
        self.position += self.velocity * dt;
    }

    fn get_bounds(&self) -> Rect {
        Rect::new(
            self.position.x - self.size / 2.0,
            self.position.y - self.size / 2.0,
            self.size,
            self.size,
        )
    }

    fn collides_with(&self, other: &GameObject) -> bool {
        self.get_bounds().overlaps(&other.get_bounds())
    }
}

struct Star {
    position: Vec2,
    speed: f32,
    brightness: f32,
    pulse_timer: f32,
}

impl Star {
    fn new(rng: &mut rand::rngs::ThreadRng) -> Self {
        let x = rng.gen_range(0.0..SCREEN_WIDTH);
        let y = rng.gen_range(0.0..SCREEN_HEIGHT);
        let speed = rng.gen_range(0.5..1.5) * STAR_SPEED;
        let brightness = rng.gen_range(0.3..1.0);
        let pulse_timer = rng.gen_range(0.0..std::f32::consts::PI * 2.0);
        Self {
            position: Vec2::new(x, y),
            speed,
            brightness,
            pulse_timer,
        }
    }

    fn update(&mut self, dt: f32) {
        self.position.y += self.speed * dt;
        self.pulse_timer += dt * 2.0;
        if self.position.y > SCREEN_HEIGHT {
            self.position.y = 0.0;
            self.position.x = rand::thread_rng().gen_range(0.0..SCREEN_WIDTH);
        }
    }
}

struct Particle {
    position: Vec2,
    velocity: Vec2,
    life: f32,
    max_life: f32,
    color: Color,
    size: f32,
}

struct ComboText {
    position: Vec2,
    velocity: Vec2,
    life: f32,
    max_life: f32,
    text: String,
    color: Color,
}

impl ComboText {
    fn new(position: Vec2, text: String) -> Self {
        Self {
            position,
            velocity: Vec2::new(0.0, -50.0), // Float upward
            life: 1.5,
            max_life: 1.5,
            text,
            color: Color::new(1.0, 0.8, 0.0, 1.0), // Orange
        }
    }

    fn update(&mut self, dt: f32) {
        self.position += self.velocity * dt;
        self.life -= dt;
        self.velocity.y -= 20.0 * dt; // Slow down upward movement
    }

    fn is_dead(&self) -> bool {
        self.life <= 0.0
    }

    fn get_alpha(&self) -> f32 {
        self.life / self.max_life
    }
}

impl Particle {
    fn new(position: Vec2, velocity: Vec2, life: f32, color: Color, size: f32) -> Self {
        Self {
            position,
            velocity,
            life,
            max_life: life,
            color,
            size,
        }
    }

    fn update(&mut self, dt: f32) {
        self.position += self.velocity * dt;
        self.velocity *= 0.98; // Air resistance
        self.life -= dt;
    }

    fn is_dead(&self) -> bool {
        self.life <= 0.0
    }

    fn get_alpha(&self) -> f32 {
        self.life / self.max_life
    }
}

struct Explosion {
    timer: f32,
    particles: Vec<Particle>,
}

impl Explosion {
    fn new(position: Vec2) -> Self {
        let mut particles = Vec::new();
        let mut rng = rand::thread_rng();
        
        for _ in 0..PARTICLE_COUNT {
            let angle = rng.gen_range(0.0..std::f32::consts::PI * 2.0);
            let speed = rng.gen_range(50.0..200.0);
            let velocity = Vec2::new(angle.cos() * speed, angle.sin() * speed);
            let life = rng.gen_range(0.5..1.5);
            let color = match rng.gen_range(0..3) {
                0 => Color::new(1.0, 0.8, 0.2, 1.0), // Yellow
                1 => Color::new(1.0, 0.4, 0.0, 1.0), // Orange
                _ => Color::new(1.0, 0.2, 0.0, 1.0), // Red
            };
            let size = rng.gen_range(2.0..6.0);
            
            particles.push(Particle::new(position, velocity, life, color, size));
        }
        
        Self { timer: 0.0, particles }
    }

    fn update(&mut self, dt: f32) {
        self.timer += dt;
        for particle in &mut self.particles {
            particle.update(dt);
        }
    }

    fn is_done(&self) -> bool {
        self.timer > EXPLOSION_DURATION
    }
}

struct PowerUp {
    position: Vec2,
    velocity: Vec2,
    power_type: PowerUpType,
    active: bool,
    pulse_timer: f32,
}

impl PowerUp {
    fn new(position: Vec2, power_type: PowerUpType) -> Self {
        Self {
            position,
            velocity: Vec2::new(0.0, 100.0),
            power_type,
            active: true,
            pulse_timer: 0.0,
        }
    }

    fn update(&mut self, dt: f32) {
        self.position += self.velocity * dt;
        self.pulse_timer += dt * 3.0;
        if self.position.y > SCREEN_HEIGHT + 20.0 {
            self.active = false;
        }
    }

    fn get_color(&self) -> Color {
        let pulse = (self.pulse_timer.sin() * 0.3 + 0.7) as f32;
        match self.power_type {
            PowerUpType::RapidFire => Color::new(0.2, 1.0, 0.2, pulse),
            PowerUpType::TripleShot => Color::new(1.0, 0.5, 0.0, pulse),
            PowerUpType::Shield => Color::new(0.2, 0.5, 1.0, pulse),
            PowerUpType::Laser => Color::new(0.8, 0.8, 0.2, pulse), // Laser color
        }
    }
}

struct GameState {
    player: GameObject,
    bullets: Vec<GameObject>,
    enemies: Vec<GameObject>,
    boss: Option<Boss>,
    boss_bullets: Vec<GameObject>,
    score: u32,
    enemy_spawn_timer: f32,
    game_over: bool,
    stars: Vec<Star>,
    explosions: Vec<Explosion>,
    lives: u32,
    invincible_timer: f32,
    high_score: u32,
    power_ups: Vec<PowerUp>,
    power_up_timers: [f32; 4], // [rapid_fire, triple_shot, shield, laser]
    power_up_spawn_timer: f32,
    screen_shake: f32,
    background_particles: Vec<Particle>,
    combo_counter: u32,
    combo_timer: f32,
    combo_multiplier: f32,
    max_combo: u32,
    combo_texts: Vec<ComboText>,
    // Input state tracking
    keys_pressed: std::collections::HashSet<KeyCode>,
    fire_cooldown: f32,
    fire_rate: f32, // Shots per second
    boss_spawn_score: u32, // Track when to spawn next boss
    boss_warning_timer: f32, // Timer for boss warning text
    laser: Laser,
    laser_instruction_timer: f32, // Timer for laser instruction popup
}

impl GameState {
    fn load_high_score() -> u32 {
        // Create saves directory if it doesn't exist
        if let Err(e) = fs::create_dir_all("saves") {
            eprintln!("Failed to create saves directory: {}", e);
        }
        
        match fs::read_to_string("saves/highscore.txt") {
            Ok(content) => content.trim().parse().unwrap_or(0),
            Err(_) => 0,
        }
    }

    fn save_high_score(score: u32) {
        // Create saves directory if it doesn't exist
        if let Err(e) = fs::create_dir_all("saves") {
            eprintln!("Failed to create saves directory: {}", e);
            return;
        }
        
        match fs::File::create("saves/highscore.txt") {
            Ok(mut file) => {
                if let Err(e) = writeln!(file, "{}", score) {
                    eprintln!("Failed to write high score: {}", e);
                } else {
                    eprintln!("High score saved: {}", score);
                }
            }
            Err(e) => {
                eprintln!("Failed to create high score file: {}", e);
            }
        }
    }

    fn new() -> Self {
        let mut rng = rand::thread_rng();
        let mut stars = Vec::with_capacity(STAR_COUNT);
        for _ in 0..STAR_COUNT {
            stars.push(Star::new(&mut rng));
        }
        
        let mut background_particles = Vec::new();
        for _ in 0..50 {
            let x = rng.gen_range(0.0..SCREEN_WIDTH);
            let y = rng.gen_range(0.0..SCREEN_HEIGHT);
            let velocity = Vec2::new(rng.gen_range(-10.0..10.0), rng.gen_range(-10.0..10.0));
            let color = Color::new(0.1, 0.1, 0.3, 0.3);
            background_particles.push(Particle::new(
                Vec2::new(x, y),
                velocity,
                rng.gen_range(2.0..5.0),
                color,
                rng.gen_range(1.0..3.0),
            ));
        }
        
        Self {
            player: GameObject::new(
                Vec2::new(SCREEN_WIDTH / 2.0, SCREEN_HEIGHT - 50.0),
                Vec2::ZERO,
                PLAYER_SIZE,
            ),
            bullets: Vec::new(),
            enemies: Vec::new(),
            boss: None,
            boss_bullets: Vec::new(),
            score: 0,
            enemy_spawn_timer: 0.0,
            game_over: false,
            stars,
            explosions: Vec::new(),
            lives: 3,
            invincible_timer: 0.0,
            high_score: Self::load_high_score(),
            power_ups: Vec::new(),
            power_up_timers: [0.0; 4],
            power_up_spawn_timer: 0.0,
            screen_shake: 0.0,
            background_particles,
            combo_counter: 0,
            combo_timer: 0.0,
            combo_multiplier: 1.0,
            max_combo: 0,
            combo_texts: Vec::new(),
            // Input state tracking
            keys_pressed: HashSet::new(),
            fire_cooldown: 0.0,
            fire_rate: 10.0, // Default fire rate
            boss_spawn_score: 0, // Initialize boss spawn score
            boss_warning_timer: 0.0, // Initialize boss warning timer
            laser: Laser::new(),
            laser_instruction_timer: 0.0, // Initialize laser instruction timer
        }
    }

    fn check_high_score(&mut self) {
        eprintln!("Checking high score: current={}, high={}", self.score, self.high_score);
        if self.score > self.high_score {
            eprintln!("New high score! {} > {}", self.score, self.high_score);
            self.high_score = self.score;
            Self::save_high_score(self.high_score);
        }
    }

    fn create_combo_text_effect(&mut self, position: Vec2) {
        let combo_text = format!("{}x{}!", self.combo_counter, self.combo_multiplier);
        let combo_text_effect = ComboText::new(position, combo_text);
        self.combo_texts.push(combo_text_effect);
    }

    fn spawn_enemy(&mut self) {
        let mut rng = rand::thread_rng();
        let x = rng.gen_range(ENEMY_SIZE..(SCREEN_WIDTH - ENEMY_SIZE));
        let enemy_type = match rng.gen_range(0..10) {
            0..=5 => EnemyType::Normal, // 60% chance
            6..=7 => EnemyType::Fast,   // 20% chance
            _ => EnemyType::Big,        // 20% chance
        };
        let (size, velocity) = match enemy_type {
            EnemyType::Normal => (ENEMY_SIZE, Vec2::new(0.0, ENEMY_SPEED)),
            EnemyType::Fast => (FAST_ENEMY_SIZE, Vec2::new(0.0, FAST_ENEMY_SPEED)),
            EnemyType::Big => (BIG_ENEMY_SIZE, Vec2::new(0.0, BIG_ENEMY_SPEED)),
        };
        let enemy = GameObject::new_enemy(
            Vec2::new(x, -size),
            velocity,
            size,
            enemy_type,
        );
        self.enemies.push(enemy);
    }

    fn spawn_power_up(&mut self) {
        let mut rng = rand::thread_rng();
        let x = rng.gen_range(50.0..(SCREEN_WIDTH - 50.0));
        let power_type = match rng.gen_range(0..4) { // Changed to 4 for Laser
            0 => PowerUpType::RapidFire,
            1 => PowerUpType::TripleShot,
            2 => PowerUpType::Shield,
            _ => PowerUpType::Laser, // Laser power-up
        };
        let power_up = PowerUp::new(Vec2::new(x, -20.0), power_type);
        self.power_ups.push(power_up);
    }

    fn spawn_boss(&mut self) {
        let mut rng = rand::thread_rng();
        let boss_type = match rng.gen_range(0..3) {
            0 => BossType::Destroyer,
            1 => BossType::Carrier,
            _ => BossType::Behemoth,
        };
        self.boss = Some(Boss::new(boss_type));
        self.boss_warning_timer = 3.0; // Show warning for 3 seconds
    }



    fn shoot(&mut self) {
        let base_bullet = GameObject::new(
            self.player.position,
            Vec2::new(0.0, -BULLET_SPEED),
            BULLET_SIZE,
        );
        
        if self.power_up_timers[1] > 0.0 { // Triple shot
            // Center bullet
            self.bullets.push(base_bullet.clone());
            // Left bullet
            let mut left_bullet = base_bullet.clone();
            left_bullet.velocity = Vec2::new(-100.0, -BULLET_SPEED);
            self.bullets.push(left_bullet);
            // Right bullet
            let mut right_bullet = base_bullet;
            right_bullet.velocity = Vec2::new(100.0, -BULLET_SPEED);
            self.bullets.push(right_bullet);
        } else {
            self.bullets.push(base_bullet);
        }
    }

    fn update_collisions(&mut self) {
        let mut enemies_to_destroy = Vec::new();
        let mut bullets_to_destroy = Vec::new();
        let mut combo_effects = Vec::new();
        
        // Check bullet-enemy collisions
        for (bullet_idx, bullet) in self.bullets.iter_mut().enumerate() {
            if !bullet.active {
                continue;
            }
            for (enemy_idx, enemy) in self.enemies.iter_mut().enumerate() {
                if !enemy.active {
                    continue;
                }
                if bullet.collides_with(enemy) {
                    bullets_to_destroy.push(bullet_idx);
                    enemies_to_destroy.push(enemy_idx);
                    
                    // Update combo system
                    self.combo_counter += 1;
                    self.combo_timer = COMBO_TIMEOUT;
                    self.combo_multiplier = (COMBO_MULTIPLIER_BASE * self.combo_counter as f32).min(COMBO_MULTIPLIER_CAP);
                    if self.combo_counter > self.max_combo {
                        self.max_combo = self.combo_counter;
                    }
                    
                    let base_score = match enemy.enemy_type {
                        Some(EnemyType::Big) => BIG_ENEMY_SCORE,
                        _ => 10,
                    };
                    let combo_score = (base_score as f32 * self.combo_multiplier) as u32;
                    self.score += combo_score;
                    self.explosions.push(Explosion::new(enemy.position));
                    self.screen_shake = 0.1; // Add screen shake
                    
                    // Store combo effect position
                    if self.combo_counter > 1 {
                        combo_effects.push(enemy.position);
                    }
                }
            }
        }
        
        // Apply destruction and effects
        for &bullet_idx in bullets_to_destroy.iter().rev() {
            self.bullets[bullet_idx].active = false;
        }
        for &enemy_idx in enemies_to_destroy.iter().rev() {
            self.enemies[enemy_idx].active = false;
        }
        for position in combo_effects {
            self.create_combo_text_effect(position);
        }

        // Check player-powerup collisions
        for power_up in &mut self.power_ups {
            if power_up.active && self.player.collides_with(&GameObject::new(power_up.position, Vec2::ZERO, 20.0)) {
                power_up.active = false;
                match power_up.power_type {
                    PowerUpType::RapidFire => self.power_up_timers[0] = POWERUP_DURATION,
                    PowerUpType::TripleShot => self.power_up_timers[1] = POWERUP_DURATION,
                    PowerUpType::Shield => self.power_up_timers[2] = POWERUP_DURATION,
                    PowerUpType::Laser => {
                        self.power_up_timers[3] = POWERUP_DURATION;
                        self.laser_instruction_timer = 5.0; // Show instruction for 5 seconds
                    }
                }
            }
        }

        // Check player-enemy collisions (skip if invincible or has shield)
        if self.invincible_timer <= 0.0 && self.power_up_timers[2] <= 0.0 {
            let mut player_hit = false;
            for enemy in &self.enemies {
                if enemy.active && self.player.collides_with(enemy) {
                    player_hit = true;
                    break;
                }
            }
            
            if player_hit {
                if self.lives > 1 {
                    self.lives -= 1;
                    self.player.position = Vec2::new(SCREEN_WIDTH / 2.0, SCREEN_HEIGHT - 50.0);
                    self.player.velocity = Vec2::ZERO;
                    self.invincible_timer = 1.0; // 1 second invincibility
                    self.screen_shake = 0.2; // Strong screen shake
                    
                    // Reset combo when player is hit
                    self.combo_counter = 0;
                    self.combo_multiplier = 1.0;
                    self.combo_timer = 0.0;
                } else {
                    self.game_over = true;
                    self.check_high_score();
                }
            }
        }

        // Clean up inactive objects
        self.bullets.retain(|bullet| bullet.active);
        self.enemies.retain(|enemy| enemy.active);
        self.power_ups.retain(|power_up| power_up.active);
        
        // Check bullet-boss collisions
        if let Some(boss) = &mut self.boss {
            if boss.active {
                let mut bullets_to_destroy = Vec::new();
                let mut boss_defeated = false;
                let mut boss_position = boss.position;
                let mut boss_type = boss.boss_type;
                let mut boss_combo_effects = Vec::new();
                
                for (bullet_idx, bullet) in self.bullets.iter_mut().enumerate() {
                    if bullet.active && bullet.collides_with(&GameObject::new(boss.position, Vec2::ZERO, boss.size)) {
                        bullets_to_destroy.push(bullet_idx);
                        boss.take_damage(10.0);
                        
                        // Update combo system for boss hits
                        self.combo_counter += 1;
                        self.combo_timer = COMBO_TIMEOUT;
                        self.combo_multiplier = (COMBO_MULTIPLIER_BASE * self.combo_counter as f32).min(COMBO_MULTIPLIER_CAP);
                        if self.combo_counter > self.max_combo {
                            self.max_combo = self.combo_counter;
                        }
                        
                        // Boss damage gives more points
                        let boss_score = match boss.boss_type {
                            BossType::Destroyer => 50,
                            BossType::Carrier => 75,
                            BossType::Behemoth => 100,
                        };
                        let combo_score = (boss_score as f32 * self.combo_multiplier) as u32;
                        self.score += combo_score;
                        
                        // Create explosion effect
                        self.explosions.push(Explosion::new(bullet.position));
                        self.screen_shake = 0.15;
                        
                        // Store combo effect position
                        if self.combo_counter > 1 {
                            boss_combo_effects.push(bullet.position);
                        }
                        
                        // Check if boss is defeated
                        if !boss.active {
                            boss_defeated = true;
                            boss_position = boss.position;
                            boss_type = boss.boss_type;
                        }
                    }
                }
                
                // Apply bullet destruction
                for &bullet_idx in bullets_to_destroy.iter().rev() {
                    self.bullets[bullet_idx].active = false;
                }
                
                // Create combo text effects for boss hits
                for position in boss_combo_effects {
                    self.create_combo_text_effect(position);
                }
                
                // Handle boss defeat
                if boss_defeated {
                    // Big explosion for boss defeat
                    for _ in 0..3 {
                        self.explosions.push(Explosion::new(boss_position));
                    }
                    self.screen_shake = 0.3;
                    
                    // Bonus score for defeating boss
                    let defeat_bonus = match boss_type {
                        BossType::Destroyer => 1000,
                        BossType::Carrier => 1500,
                        BossType::Behemoth => 2000,
                    };
                    self.score += defeat_bonus;
                    
                    // Reset combo after boss defeat
                    self.combo_counter = 0;
                    self.combo_multiplier = 1.0;
                    self.combo_timer = 0.0;
                }
            }
        }
        
        // Check player-boss bullet collisions
        let mut boss_bullets_to_destroy = Vec::new();
        let mut player_hit_by_boss_bullet = false;
        
        for (bullet_idx, boss_bullet) in self.boss_bullets.iter_mut().enumerate() {
            if boss_bullet.active && self.player.collides_with(boss_bullet) {
                boss_bullets_to_destroy.push(bullet_idx);
                player_hit_by_boss_bullet = true;
            }
        }
        
        // Apply boss bullet destruction
        for &bullet_idx in boss_bullets_to_destroy.iter().rev() {
            self.boss_bullets[bullet_idx].active = false;
        }
        
        // Handle player hit by boss bullet
        if player_hit_by_boss_bullet {
            if self.invincible_timer <= 0.0 && self.power_up_timers[2] <= 0.0 {
                if self.lives > 1 {
                    self.lives -= 1;
                    self.player.position = Vec2::new(SCREEN_WIDTH / 2.0, SCREEN_HEIGHT - 50.0);
                    self.player.velocity = Vec2::ZERO;
                    self.invincible_timer = 1.0;
                    self.screen_shake = 0.2;
                    
                    // Reset combo when player is hit
                    self.combo_counter = 0;
                    self.combo_multiplier = 1.0;
                    self.combo_timer = 0.0;
                } else {
                    self.game_over = true;
                    self.check_high_score();
                }
            }
        }
        
        // Clean up boss bullets
        self.boss_bullets.retain(|bullet| bullet.active);
        
        // Check laser collisions
        if self.laser.is_firing {
            let laser_bounds = self.laser.get_laser_bounds();
            
            // Check laser-enemy collisions
            let mut enemies_to_destroy = Vec::new();
            for (enemy_idx, enemy) in self.enemies.iter_mut().enumerate() {
                if enemy.active && laser_bounds.overlaps(&enemy.get_bounds()) {
                    enemies_to_destroy.push(enemy_idx);
                    
                    // Update combo system
                    self.combo_counter += 1;
                    self.combo_timer = COMBO_TIMEOUT;
                    self.combo_multiplier = (COMBO_MULTIPLIER_BASE * self.combo_counter as f32).min(COMBO_MULTIPLIER_CAP);
                    if self.combo_counter > self.max_combo {
                        self.max_combo = self.combo_counter;
                    }
                    
                    let base_score = match enemy.enemy_type {
                        Some(EnemyType::Big) => BIG_ENEMY_SCORE,
                        _ => 10,
                    };
                    let combo_score = (base_score as f32 * self.combo_multiplier) as u32;
                    self.score += combo_score;
                    self.explosions.push(Explosion::new(enemy.position));
                    self.screen_shake = 0.1;
                }
            }
            
            // Apply enemy destruction
            for &enemy_idx in enemies_to_destroy.iter().rev() {
                self.enemies[enemy_idx].active = false;
            }
            
            // Check laser-boss collisions
            if let Some(boss) = &mut self.boss {
                if boss.active && laser_bounds.overlaps(&GameObject::new(boss.position, Vec2::ZERO, boss.size).get_bounds()) {
                    boss.take_damage(self.laser.damage * 0.1); // Laser does continuous damage
                    
                    // Update combo system for boss hits
                    self.combo_counter += 1;
                    self.combo_timer = COMBO_TIMEOUT;
                    self.combo_multiplier = (COMBO_MULTIPLIER_BASE * self.combo_counter as f32).min(COMBO_MULTIPLIER_CAP);
                    if self.combo_counter > self.max_combo {
                        self.max_combo = self.combo_counter;
                    }
                    
                    // Boss damage gives more points
                    let boss_score = match boss.boss_type {
                        BossType::Destroyer => 50,
                        BossType::Carrier => 75,
                        BossType::Behemoth => 100,
                    };
                    let combo_score = (boss_score as f32 * self.combo_multiplier) as u32;
                    self.score += combo_score;
                    
                    // Create explosion effect
                    self.explosions.push(Explosion::new(boss.position));
                    self.screen_shake = 0.15;
                }
            }
        }
    }
}

impl EventHandler for GameState {
    fn update(&mut self, ctx: &mut Context) -> GameResult {
        if self.game_over {
            return Ok(());
        }

        let dt = ctx.time.delta().as_secs_f32();

        // Update screen shake
        if self.screen_shake > 0.0 {
            self.screen_shake -= dt * 5.0;
            if self.screen_shake < 0.0 {
                self.screen_shake = 0.0;
            }
        }

        // Update invincibility timer
        if self.invincible_timer > 0.0 {
            self.invincible_timer -= dt;
            if self.invincible_timer < 0.0 {
                self.invincible_timer = 0.0;
            }
        }

        // Update power-up timers
        for timer in &mut self.power_up_timers {
            if *timer > 0.0 {
                *timer -= dt;
                if *timer < 0.0 {
                    *timer = 0.0;
                }
            }
        }

        // Update combo timer
        if self.combo_timer > 0.0 {
            self.combo_timer -= dt;
            if self.combo_timer <= 0.0 {
                // Reset combo when timer expires
                self.combo_counter = 0;
                self.combo_multiplier = 1.0;
                self.combo_timer = 0.0;
            }
        }

        // Update boss warning timer
        if self.boss_warning_timer > 0.0 {
            self.boss_warning_timer -= dt;
            if self.boss_warning_timer < 0.0 {
                self.boss_warning_timer = 0.0;
            }
        }

        // Update laser instruction timer
        if self.laser_instruction_timer > 0.0 {
            self.laser_instruction_timer -= dt;
            if self.laser_instruction_timer < 0.0 {
                self.laser_instruction_timer = 0.0;
            }
        }

        // Update stars
        for star in &mut self.stars {
            star.update(dt);
        }

        // Update background particles
        for particle in &mut self.background_particles {
            particle.update(dt);
            if particle.is_dead() {
                let mut rng = rand::thread_rng();
                particle.position = Vec2::new(
                    rng.gen_range(0.0..SCREEN_WIDTH),
                    rng.gen_range(0.0..SCREEN_HEIGHT),
                );
                particle.life = rng.gen_range(2.0..5.0);
            }
        }

        // Handle continuous input for movement
        self.player.velocity = Vec2::ZERO;
        for &key in &self.keys_pressed {
            match key {
                KeyCode::A | KeyCode::Left => {
                    self.player.velocity.x -= PLAYER_SPEED;
                }
                KeyCode::D | KeyCode::Right => {
                    self.player.velocity.x += PLAYER_SPEED;
                }
                KeyCode::W | KeyCode::Up => {
                    self.player.velocity.y -= PLAYER_SPEED;
                }
                KeyCode::S | KeyCode::Down => {
                    self.player.velocity.y += PLAYER_SPEED;
                }
                _ => {}
            }
        }
        
        // Normalize diagonal movement to prevent faster diagonal speed
        if self.player.velocity.length() > PLAYER_SPEED {
            self.player.velocity = self.player.velocity.normalize() * PLAYER_SPEED;
        }

        // Handle firing with cooldown
        if self.keys_pressed.contains(&KeyCode::Space) && !self.game_over {
            if self.fire_cooldown <= 0.0 {
                self.shoot();
                // Set cooldown based on fire rate and power-ups
                let base_cooldown = 1.0 / self.fire_rate;
                let rapid_fire_multiplier = if self.power_up_timers[0] > 0.0 { 0.3 } else { 1.0 };
                self.fire_cooldown = base_cooldown * rapid_fire_multiplier;
            }
        }

        // Handle laser charging and firing
        if self.power_up_timers[3] > 0.0 && !self.game_over { // Laser powerup active
            if self.keys_pressed.contains(&KeyCode::L) && !self.laser.is_charging && !self.laser.is_firing {
                // Start charging laser
                self.laser.start_charging(self.player.position);
            } else if !self.keys_pressed.contains(&KeyCode::L) && self.laser.is_charging {
                // Fire laser when L key is released
                self.laser.fire();
            }
        }

        // Update fire cooldown
        if self.fire_cooldown > 0.0 {
            self.fire_cooldown -= dt;
        }

        // Update player
        self.player.update(dt);

        // Keep player in bounds
        self.player.position.x = self.player.position.x.clamp(
            PLAYER_SIZE / 2.0,
            SCREEN_WIDTH - PLAYER_SIZE / 2.0,
        );
        self.player.position.y = self.player.position.y.clamp(
            PLAYER_SIZE / 2.0,
            SCREEN_HEIGHT - PLAYER_SIZE / 2.0,
        );

        // Update bullets
        for bullet in &mut self.bullets {
            bullet.update(dt);
            // Remove bullets that go off screen
            if bullet.position.y < -BULLET_SIZE {
                bullet.active = false;
            }
        }

        // Update enemies
        for enemy in &mut self.enemies {
            enemy.update(dt);
            // Remove enemies that go off screen
            if enemy.position.y > SCREEN_HEIGHT + enemy.size {
                enemy.active = false;
            }
        }

        // Update power-ups
        for power_up in &mut self.power_ups {
            power_up.update(dt);
        }

        // Update explosions
        for explosion in &mut self.explosions {
            explosion.update(dt);
        }
        self.explosions.retain(|e| !e.is_done());

        // Update combo text effects
        for combo_text in &mut self.combo_texts {
            combo_text.update(dt);
        }
        self.combo_texts.retain(|ct| !ct.is_dead());

        // Update laser
        if self.power_up_timers[3] > 0.0 { // Laser powerup active
            self.laser.update_charging(dt);
            self.laser.update_firing(dt);
        } else {
            // Reset laser when powerup expires
            self.laser = Laser::new();
        }

        // Spawn enemies
        self.enemy_spawn_timer += dt;
        if self.enemy_spawn_timer >= 0.7 {
            for _ in 0..5 {
                self.spawn_enemy();
            }
            self.enemy_spawn_timer = 0.0;
        }

        // Spawn power-ups
        self.power_up_spawn_timer += dt;
        if self.power_up_spawn_timer >= 8.0 {
            self.spawn_power_up();
            self.power_up_spawn_timer = 0.0;
        }

        // Check for boss spawning (every 10,000 points)
        if self.score >= self.boss_spawn_score + 10000 && self.boss.is_none() {
            self.spawn_boss();
            self.boss_spawn_score = self.score;
        }

        // Update boss
        if let Some(boss) = &mut self.boss {
            boss.update(dt);
            
            // Spawn boss bullets based on attack patterns
            let attack_interval = match boss.boss_type {
                BossType::Destroyer => if boss.phase == 1 { 0.8 } else { 0.5 },
                BossType::Carrier => if boss.phase == 1 { 1.2 } else { 0.8 },
                BossType::Behemoth => if boss.phase == 1 { 1.0 } else { 0.6 },
            };
            
            if boss.attack_timer >= attack_interval {
                // Store boss info for bullet spawning
                let spawn_points = boss.get_bullet_spawn_points();
                let boss_type = boss.boss_type;
                let phase = boss.phase;
                let attack_pattern = boss.attack_pattern;
                
                // Spawn bullets without borrowing self
                for spawn_point in spawn_points {
                    let bullet_speed = match boss_type {
                        BossType::Destroyer => 200.0,
                        BossType::Carrier => 150.0,
                        BossType::Behemoth => 180.0,
                    };
                    
                    let mut rng = rand::thread_rng();
                    let velocity = match boss_type {
                        BossType::Destroyer => {
                            if phase == 1 {
                                Vec2::new(0.0, bullet_speed)
                            } else {
                                let angle: f32 = rng.gen_range(-0.3..0.3);
                                Vec2::new(angle.sin() * bullet_speed, angle.cos() * bullet_speed)
                            }
                        }
                        BossType::Carrier => {
                            let spread = if phase == 1 { 0.2 } else { 0.4 };
                            let angle: f32 = rng.gen_range(-spread..spread);
                            Vec2::new(angle.sin() * bullet_speed, angle.cos() * bullet_speed)
                        }
                        BossType::Behemoth => {
                            let angle = (attack_pattern * 2.0_f32).sin() * 0.5;
                            Vec2::new(angle * bullet_speed, bullet_speed)
                        }
                    };
                    
                    let bullet = GameObject::new(spawn_point, velocity, 8.0);
                    self.boss_bullets.push(bullet);
                }
                
                boss.attack_timer = 0.0;
            }
        }

        // Update boss bullets
        for boss_bullet in &mut self.boss_bullets {
            boss_bullet.update(dt);
            // Remove boss bullets that go off screen
            if boss_bullet.position.y > SCREEN_HEIGHT + boss_bullet.size || 
               boss_bullet.position.x < -boss_bullet.size || 
               boss_bullet.position.x > SCREEN_WIDTH + boss_bullet.size {
                boss_bullet.active = false;
            }
        }

        // Update collisions
        self.update_collisions();

        // Temporary: Check high score every frame for testing
        if self.score > 0 && self.score % 100 == 0 {
            self.check_high_score();
        }

        Ok(())
    }

    fn draw(&mut self, ctx: &mut Context) -> GameResult {
        let mut canvas = graphics::Canvas::from_frame(ctx, Color::BLACK);

        // Apply screen shake
        let shake_offset = if self.screen_shake > 0.0 {
            let mut rng = rand::thread_rng();
            Vec2::new(
                rng.gen_range(-self.screen_shake * 10.0..self.screen_shake * 10.0),
                rng.gen_range(-self.screen_shake * 10.0..self.screen_shake * 10.0),
            )
        } else {
            Vec2::ZERO
        };

        // Draw background particles
        for particle in &self.background_particles {
            let color = Color::new(
                particle.color.r,
                particle.color.g,
                particle.color.b,
                particle.get_alpha() * 0.3,
            );
            let mesh = Mesh::new_circle(
                ctx,
                DrawMode::fill(),
                particle.position + shake_offset,
                particle.size,
                0.1,
                color,
            )?;
            canvas.draw(&mesh, DrawParam::default());
        }

        // Draw starfield
        for star in &self.stars {
            let pulse = (star.pulse_timer.sin() * 0.3 + 0.7) as f32;
            let color = Color::new(1.0, 1.0, 1.0, 0.7 * pulse * star.brightness);
            let star_mesh = Mesh::new_circle(
                ctx,
                DrawMode::fill(),
                star.position + shake_offset,
                1.5,
                0.1,
                color,
            )?;
            canvas.draw(&star_mesh, DrawParam::default());
        }

        // Draw lives as hearts (top right)
        for i in 0..self.lives {
            let heart_x = SCREEN_WIDTH - 20.0 - (self.lives - 1 - i) as f32 * 30.0;
            let heart_y = 40.0;
            let heart_mesh = Mesh::new_circle(
                ctx,
                DrawMode::fill(),
                Vec2::new(heart_x, heart_y) + shake_offset,
                12.0,
                0.2,
                Color::new(1.0, 0.2, 0.3, 1.0),
            )?;
            canvas.draw(&heart_mesh, DrawParam::default());
        }

        if self.game_over {
            // Draw game over screen
            let game_over_text = Text::new("GAME OVER");
            let score_text = Text::new(format!("Final Score: {}", self.score));
            let high_score_text = Text::new(format!("High Score: {}", self.high_score));
            let restart_text = Text::new("Press R to restart");

            canvas.draw(
                &game_over_text,
                DrawParam::default()
                    .dest(Vec2::new(SCREEN_WIDTH / 2.0 - 100.0, SCREEN_HEIGHT / 2.0 - 80.0) + shake_offset)
                    .color(Color::RED),
            );
            canvas.draw(
                &score_text,
                DrawParam::default()
                    .dest(Vec2::new(SCREEN_WIDTH / 2.0 - 80.0, SCREEN_HEIGHT / 2.0 - 30.0) + shake_offset)
                    .color(Color::WHITE),
            );
            canvas.draw(
                &high_score_text,
                DrawParam::default()
                    .dest(Vec2::new(SCREEN_WIDTH / 2.0 - 80.0, SCREEN_HEIGHT / 2.0) + shake_offset)
                    .color(Color::YELLOW),
            );
            canvas.draw(
                &restart_text,
                DrawParam::default()
                    .dest(Vec2::new(SCREEN_WIDTH / 2.0 - 80.0, SCREEN_HEIGHT / 2.0 + 30.0) + shake_offset)
                    .color(Color::WHITE),
            );
        } else {
            // Draw player (with blinking if invincible)
            let player_color = if self.invincible_timer > 0.0 && ((self.invincible_timer * 10.0) as i32) % 2 == 0 {
                Color::WHITE // Blink: invisible every other frame
            } else if self.power_up_timers[2] > 0.0 {
                Color::new(0.2, 0.5, 1.0, 0.8) // Shield color
            } else {
                Color::BLUE
            };
            
            let player_mesh = Mesh::new_rectangle(
                ctx,
                DrawMode::fill(),
                Rect::new(
                    -PLAYER_SIZE / 2.0,
                    -PLAYER_SIZE / 2.0,
                    PLAYER_SIZE,
                    PLAYER_SIZE,
                ),
                player_color,
            )?;
            
            if !(self.invincible_timer > 0.0 && ((self.invincible_timer * 10.0) as i32) % 2 == 0) {
                canvas.draw(
                    &player_mesh,
                    DrawParam::default().dest(self.player.position + shake_offset),
                );
            }

            // Draw shield effect
            if self.power_up_timers[2] > 0.0 {
                let shield_mesh = Mesh::new_circle(
                    ctx,
                    DrawMode::stroke(3.0),
                    self.player.position + shake_offset,
                    PLAYER_SIZE + 10.0,
                    0.2,
                    Color::new(0.2, 0.5, 1.0, 0.5),
                )?;
                canvas.draw(&shield_mesh, DrawParam::default());
            }

            // Draw bullets with trail effect
            for bullet in &self.bullets {
                if bullet.active {
                    // Bullet trail
                    for i in 1..4 {
                        let trail_pos = bullet.position + bullet.velocity * (i as f32 * -0.01);
                        let trail_alpha = 0.3 / i as f32;
                        let trail_mesh = Mesh::new_circle(
                            ctx,
                            DrawMode::fill(),
                            trail_pos + shake_offset,
                            BULLET_SIZE / 2.0 * (1.0 - i as f32 * 0.2),
                            0.1,
                            Color::new(1.0, 1.0, 0.0, trail_alpha),
                        )?;
                        canvas.draw(&trail_mesh, DrawParam::default());
                    }
                    
                    // Main bullet
                    let bullet_mesh = Mesh::new_circle(
                        ctx,
                        DrawMode::fill(),
                        bullet.position + shake_offset,
                        BULLET_SIZE / 2.0,
                        0.1,
                        Color::YELLOW,
                    )?;
                    canvas.draw(&bullet_mesh, DrawParam::default());
                }
            }

            // Draw enemies with glow effect
            for enemy in &self.enemies {
                if enemy.active {
                    let (color, size) = match enemy.enemy_type {
                        Some(EnemyType::Normal) | None => (Color::RED, ENEMY_SIZE),
                        Some(EnemyType::Fast) => (Color::GREEN, FAST_ENEMY_SIZE),
                        Some(EnemyType::Big) => (Color::MAGENTA, BIG_ENEMY_SIZE),
                    };
                    
                    // Enemy glow
                    let glow_mesh = Mesh::new_circle(
                        ctx,
                        DrawMode::fill(),
                        enemy.position + shake_offset,
                        size * 0.8,
                        0.2,
                        Color::new(color.r, color.g, color.b, 0.3),
                    )?;
                    canvas.draw(&glow_mesh, DrawParam::default());
                    
                    // Main enemy
                    let enemy_mesh = Mesh::new_rectangle(
                        ctx,
                        DrawMode::fill(),
                        Rect::new(
                            -size / 2.0,
                            -size / 2.0,
                            size,
                            size,
                        ),
                        color,
                    )?;
                    canvas.draw(
                        &enemy_mesh,
                        DrawParam::default().dest(enemy.position + shake_offset),
                    );
                }
            }

            // Draw power-ups
            for power_up in &self.power_ups {
                if power_up.active {
                    let color = power_up.get_color();
                    let power_up_mesh = Mesh::new_rectangle(
                        ctx,
                        DrawMode::fill(),
                        Rect::new(-15.0, -15.0, 30.0, 30.0),
                        color,
                    )?;
                    canvas.draw(
                        &power_up_mesh,
                        DrawParam::default().dest(power_up.position + shake_offset),
                    );
                }
            }

            // Draw boss
            if let Some(boss) = &self.boss {
                if boss.active {
                    let boss_color = boss.get_color();
                    
                    // Boss glow effect
                    let glow_mesh = Mesh::new_circle(
                        ctx,
                        DrawMode::fill(),
                        boss.position + shake_offset,
                        boss.size * 1.2,
                        0.2,
                        Color::new(boss_color.r, boss_color.g, boss_color.b, 0.3),
                    )?;
                    canvas.draw(&glow_mesh, DrawParam::default());
                    
                    // Main boss body
                    let boss_mesh = Mesh::new_rectangle(
                        ctx,
                        DrawMode::fill(),
                        Rect::new(
                            -boss.size / 2.0,
                            -boss.size / 2.0,
                            boss.size,
                            boss.size,
                        ),
                        boss_color,
                    )?;
                    canvas.draw(
                        &boss_mesh,
                        DrawParam::default().dest(boss.position + shake_offset),
                    );
                    
                    // Boss health bar
                    let health_percent = boss.health / boss.max_health;
                    let bar_width = boss.size;
                    let bar_height = 8.0;
                    let bar_x = boss.position.x - bar_width / 2.0;
                    let bar_y = boss.position.y - boss.size / 2.0 - 20.0;
                    
                    // Background bar
                    let bg_bar = Mesh::new_rectangle(
                        ctx,
                        DrawMode::fill(),
                        Rect::new(bar_x, bar_y, bar_width, bar_height),
                        Color::new(0.3, 0.3, 0.3, 0.8),
                    )?;
                    canvas.draw(&bg_bar, DrawParam::default().dest(shake_offset));
                    
                    // Health bar
                    let health_bar = Mesh::new_rectangle(
                        ctx,
                        DrawMode::fill(),
                        Rect::new(bar_x, bar_y, bar_width * health_percent, bar_height),
                        Color::new(1.0 - health_percent, health_percent, 0.0, 1.0),
                    )?;
                    canvas.draw(&health_bar, DrawParam::default().dest(shake_offset));
                }
            }

            // Draw boss bullets
            for boss_bullet in &self.boss_bullets {
                if boss_bullet.active {
                    // Boss bullet trail
                    for i in 1..4 {
                        let trail_pos = boss_bullet.position + boss_bullet.velocity * (i as f32 * -0.01);
                        let trail_alpha = 0.4 / i as f32;
                        let trail_mesh = Mesh::new_circle(
                            ctx,
                            DrawMode::fill(),
                            trail_pos + shake_offset,
                            boss_bullet.size / 2.0 * (1.0 - i as f32 * 0.2),
                            0.1,
                            Color::new(1.0, 0.2, 0.2, trail_alpha),
                        )?;
                        canvas.draw(&trail_mesh, DrawParam::default());
                    }
                    
                    // Main boss bullet
                    let boss_bullet_mesh = Mesh::new_circle(
                        ctx,
                        DrawMode::fill(),
                        boss_bullet.position + shake_offset,
                        boss_bullet.size / 2.0,
                        0.1,
                        Color::new(1.0, 0.2, 0.2, 1.0),
                    )?;
                    canvas.draw(&boss_bullet_mesh, DrawParam::default());
                }
            }

            // Draw explosions with particles
            for explosion in &self.explosions {
                for particle in &explosion.particles {
                    if !particle.is_dead() {
                        let color = Color::new(
                            particle.color.r,
                            particle.color.g,
                            particle.color.b,
                            particle.get_alpha(),
                        );
                        let mesh = Mesh::new_circle(
                            ctx,
                            DrawMode::fill(),
                            particle.position + shake_offset,
                            particle.size,
                            0.1,
                            color,
                        )?;
                        canvas.draw(&mesh, DrawParam::default());
                    }
                }
            }

            // Draw combo text effects
            for combo_text in &self.combo_texts {
                let text = Text::new(&combo_text.text);
                let color = Color::new(
                    combo_text.color.r,
                    combo_text.color.g,
                    combo_text.color.b,
                    combo_text.get_alpha(),
                );
                canvas.draw(
                    &text,
                    DrawParam::default()
                        .dest(combo_text.position + shake_offset)
                        .color(color),
                );
            }

            // Draw score and high score
            let score_text = Text::new(format!("Score: {}", self.score));
            let high_score_text = Text::new(format!("High Score: {}", self.high_score));
            canvas.draw(
                &score_text,
                DrawParam::default()
                    .dest(Vec2::new(10.0, 10.0) + shake_offset)
                    .color(Color::WHITE),
            );
            canvas.draw(
                &high_score_text,
                DrawParam::default()
                    .dest(Vec2::new(10.0, 30.0) + shake_offset)
                    .color(Color::YELLOW),
            );

            // Draw combo system
            if self.combo_counter > 0 {
                let combo_text = Text::new(format!("Combo: {}x{}", self.combo_counter, self.combo_multiplier));
                let combo_timer_text = Text::new(format!("Timer: {:.1}s", self.combo_timer));
                let max_combo_text = Text::new(format!("Max Combo: {}", self.max_combo));
                
                // Combo text with pulsing effect
                let pulse = (std::time::SystemTime::now()
                    .duration_since(std::time::UNIX_EPOCH)
                    .unwrap()
                    .as_secs_f32() * 3.0)
                    .sin() * 0.3 + 0.7;
                let combo_color = Color::new(1.0, 0.8, 0.0, pulse);
                
                canvas.draw(
                    &combo_text,
                    DrawParam::default()
                        .dest(Vec2::new(SCREEN_WIDTH - 200.0, 10.0) + shake_offset)
                        .color(combo_color),
                );
                canvas.draw(
                    &combo_timer_text,
                    DrawParam::default()
                        .dest(Vec2::new(SCREEN_WIDTH - 200.0, 30.0) + shake_offset)
                        .color(Color::new(1.0, 0.5, 0.0, 1.0)),
                );
                canvas.draw(
                    &max_combo_text,
                    DrawParam::default()
                        .dest(Vec2::new(SCREEN_WIDTH - 200.0, 50.0) + shake_offset)
                        .color(Color::YELLOW),
                );
            }

            // Draw power-up indicators
            let mut indicator_y = 60.0;
            if self.power_up_timers[0] > 0.0 {
                let rapid_text = Text::new(format!("Rapid Fire: {:.1}s", self.power_up_timers[0]));
                canvas.draw(
                    &rapid_text,
                    DrawParam::default()
                        .dest(Vec2::new(10.0, indicator_y) + shake_offset)
                        .color(Color::new(0.2, 1.0, 0.2, 1.0)),
                );
                indicator_y += 20.0;
            }
            if self.power_up_timers[1] > 0.0 {
                let triple_text = Text::new(format!("Triple Shot: {:.1}s", self.power_up_timers[1]));
                canvas.draw(
                    &triple_text,
                    DrawParam::default()
                        .dest(Vec2::new(10.0, indicator_y) + shake_offset)
                        .color(Color::new(1.0, 0.5, 0.0, 1.0)),
                );
                indicator_y += 20.0;
            }
            if self.power_up_timers[2] > 0.0 {
                let shield_text = Text::new(format!("Shield: {:.1}s", self.power_up_timers[2]));
                canvas.draw(
                    &shield_text,
                    DrawParam::default()
                        .dest(Vec2::new(10.0, indicator_y) + shake_offset)
                        .color(Color::new(0.2, 0.5, 1.0, 1.0)),
                );
                indicator_y += 20.0;
            }
            if self.power_up_timers[3] > 0.0 {
                let laser_text = Text::new(format!("Laser: {:.1}s", self.power_up_timers[3]));
                canvas.draw(
                    &laser_text,
                    DrawParam::default()
                        .dest(Vec2::new(10.0, indicator_y) + shake_offset)
                        .color(Color::new(0.8, 0.8, 0.2, 1.0)),
                );
            }

            // Draw boss warning
            if self.boss.is_some() && self.boss_warning_timer > 0.0 {
                let boss_warning = Text::new("BOSS BATTLE!");
                let pulse = (std::time::SystemTime::now()
                    .duration_since(std::time::UNIX_EPOCH)
                    .unwrap()
                    .as_secs_f32() * 4.0)
                    .sin() * 0.5 + 0.5;
                let warning_color = Color::new(1.0, 0.0, 0.0, pulse);
                canvas.draw(
                    &boss_warning,
                    DrawParam::default()
                        .dest(Vec2::new(SCREEN_WIDTH / 2.0 - 80.0, 80.0) + shake_offset)
                        .color(warning_color),
                );
            }

            // Draw laser instruction popup
            if self.laser_instruction_timer > 0.0 {
                let laser_title = Text::new("LASER WEAPON ACQUIRED!");
                let laser_instruction1 = Text::new("Hold L key to charge the laser");
                let laser_instruction2 = Text::new("Release L key to fire!");
                let laser_tip = Text::new("Longer charge = more damage!");
                
                let pulse = (std::time::SystemTime::now()
                    .duration_since(std::time::UNIX_EPOCH)
                    .unwrap()
                    .as_secs_f32() * 3.0)
                    .sin() * 0.3 + 0.7;
                let laser_color = Color::new(0.8, 0.8, 0.2, pulse);
                
                // Title
                canvas.draw(
                    &laser_title,
                    DrawParam::default()
                        .dest(Vec2::new(SCREEN_WIDTH / 2.0 - 150.0, 80.0) + shake_offset)
                        .color(laser_color),
                );
                
                // Instructions
                canvas.draw(
                    &laser_instruction1,
                    DrawParam::default()
                        .dest(Vec2::new(SCREEN_WIDTH / 2.0 - 120.0, 110.0) + shake_offset)
                        .color(Color::WHITE),
                );
                
                canvas.draw(
                    &laser_instruction2,
                    DrawParam::default()
                        .dest(Vec2::new(SCREEN_WIDTH / 2.0 - 100.0, 130.0) + shake_offset)
                        .color(Color::WHITE),
                );
                
                canvas.draw(
                    &laser_tip,
                    DrawParam::default()
                        .dest(Vec2::new(SCREEN_WIDTH / 2.0 - 120.0, 150.0) + shake_offset)
                        .color(Color::new(1.0, 0.8, 0.0, 1.0)),
                );
            }



            // Draw laser charging particles
            if self.laser.is_charging {
                for particle in &self.laser.charge_particles {
                    if !particle.is_dead() {
                        let color = Color::new(
                            particle.color.r,
                            particle.color.g,
                            particle.color.b,
                            particle.get_alpha(),
                        );
                        let mesh = Mesh::new_circle(
                            ctx,
                            DrawMode::fill(),
                            particle.position + shake_offset,
                            particle.size,
                            0.1,
                            color,
                        )?;
                        canvas.draw(&mesh, DrawParam::default());
                    }
                }
                
                // Draw charge indicator
                let charge_progress = self.laser.get_charge_progress();
                let charge_text = Text::new(format!("LASER CHARGE: {:.0}%", charge_progress * 100.0));
                let charge_color = Color::new(
                    self.laser.color.r,
                    self.laser.color.g,
                    self.laser.color.b,
                    1.0,
                );
                canvas.draw(
                    &charge_text,
                    DrawParam::default()
                        .dest(Vec2::new(SCREEN_WIDTH / 2.0 - 100.0, SCREEN_HEIGHT - 80.0) + shake_offset)
                        .color(charge_color),
                );
                
                // Draw charge bar
                let bar_width = 200.0;
                let bar_height = 10.0;
                let bar_x = SCREEN_WIDTH / 2.0 - bar_width / 2.0;
                let bar_y = SCREEN_HEIGHT - 60.0;
                
                // Background bar
                let bg_bar = Mesh::new_rectangle(
                    ctx,
                    DrawMode::fill(),
                    Rect::new(bar_x, bar_y, bar_width, bar_height),
                    Color::new(0.3, 0.3, 0.3, 0.8),
                )?;
                canvas.draw(&bg_bar, DrawParam::default().dest(shake_offset));
                
                // Charge bar
                let charge_bar = Mesh::new_rectangle(
                    ctx,
                    DrawMode::fill(),
                    Rect::new(bar_x, bar_y, bar_width * charge_progress, bar_height),
                    charge_color,
                )?;
                canvas.draw(&charge_bar, DrawParam::default().dest(shake_offset));
            }

            // Draw laser beam
            if self.laser.is_firing {
                let laser_bounds = self.laser.get_laser_bounds();
                
                // Laser glow effect
                let glow_mesh = Mesh::new_rectangle(
                    ctx,
                    DrawMode::fill(),
                    Rect::new(
                        laser_bounds.x - 5.0,
                        laser_bounds.y,
                        laser_bounds.w + 10.0,
                        laser_bounds.h,
                    ),
                    Color::new(
                        self.laser.color.r,
                        self.laser.color.g,
                        self.laser.color.b,
                        0.3,
                    ),
                )?;
                canvas.draw(&glow_mesh, DrawParam::default().dest(shake_offset));
                
                // Main laser beam
                let laser_mesh = Mesh::new_rectangle(
                    ctx,
                    DrawMode::fill(),
                    Rect::new(
                        laser_bounds.x,
                        laser_bounds.y,
                        laser_bounds.w,
                        laser_bounds.h,
                    ),
                    self.laser.color,
                )?;
                canvas.draw(&laser_mesh, DrawParam::default().dest(shake_offset));
                
                // Laser core (brighter center)
                let core_width = self.laser.width * 0.3;
                let core_mesh = Mesh::new_rectangle(
                    ctx,
                    DrawMode::fill(),
                    Rect::new(
                        laser_bounds.x + (laser_bounds.w - core_width) / 2.0,
                        laser_bounds.y,
                        core_width,
                        laser_bounds.h,
                    ),
                    Color::new(1.0, 1.0, 1.0, 0.8),
                )?;
                canvas.draw(&core_mesh, DrawParam::default().dest(shake_offset));
            }
        }

        canvas.finish(ctx)?;
        Ok(())
    }

    fn key_down_event(
        &mut self,
        _ctx: &mut Context,
        input: KeyInput,
        _repeat: bool,
    ) -> GameResult {
        if let Some(keycode) = input.keycode {
            self.keys_pressed.insert(keycode);
            
            // Handle restart key
            if keycode == KeyCode::R && self.game_over {
                *self = GameState::new();
                self.check_high_score();
            }
        }
        Ok(())
    }

    fn key_up_event(&mut self, _ctx: &mut Context, input: KeyInput) -> GameResult {
        if let Some(keycode) = input.keycode {
            self.keys_pressed.remove(&keycode);
        }
        Ok(())
    }
}

struct Laser {
    is_charging: bool,
    charge_time: f32,
    is_firing: bool,
    fire_duration: f32,
    max_fire_duration: f32,
    damage: f32,
    width: f32,
    color: Color,
    position: Vec2,
    charge_particles: Vec<Particle>,
}

impl Laser {
    fn new() -> Self {
        Self {
            is_charging: false,
            charge_time: 0.0,
            is_firing: false,
            fire_duration: 0.0,
            max_fire_duration: 0.5, // How long laser can fire
            damage: LASER_DAMAGE_BASE,
            width: LASER_WIDTH_BASE,
            color: LASER_COLOR_BASE,
            position: Vec2::ZERO,
            charge_particles: Vec::new(),
        }
    }

    fn start_charging(&mut self, position: Vec2) {
        self.is_charging = true;
        self.charge_time = 0.0;
        self.position = position;
        self.charge_particles.clear();
    }

    fn update_charging(&mut self, dt: f32) {
        if !self.is_charging {
            return;
        }

        self.charge_time += dt;
        
        // Update charge progress
        let charge_progress = (self.charge_time / LASER_CHARGE_TIME).min(1.0);
        self.damage = LASER_DAMAGE_BASE + (LASER_DAMAGE_MAX - LASER_DAMAGE_BASE) * charge_progress;
        self.width = LASER_WIDTH_BASE + (LASER_WIDTH_MAX - LASER_WIDTH_BASE) * charge_progress;
        
        // Update color based on charge
        let color_progress = charge_progress;
        self.color = Color::new(
            LASER_COLOR_BASE.r + (LASER_COLOR_CHARGED.r - LASER_COLOR_BASE.r) * color_progress,
            LASER_COLOR_BASE.g + (LASER_COLOR_CHARGED.g - LASER_COLOR_BASE.g) * color_progress,
            LASER_COLOR_BASE.b + (LASER_COLOR_CHARGED.b - LASER_COLOR_BASE.b) * color_progress,
            1.0,
        );

        // Create charge particles
        if self.charge_time > 0.1 {
            let mut rng = rand::thread_rng();
            for _ in 0..2 {
                let angle = rng.gen_range(0.0..std::f32::consts::PI * 2.0);
                let distance = rng.gen_range(20.0..40.0);
                let particle_pos = self.position + Vec2::new(angle.cos() * distance, angle.sin() * distance);
                let velocity = (self.position - particle_pos).normalize() * rng.gen_range(50.0..150.0);
                let life = rng.gen_range(0.3..0.8);
                let size = rng.gen_range(2.0..6.0);
                
                self.charge_particles.push(Particle::new(
                    particle_pos,
                    velocity,
                    life,
                    self.color,
                    size,
                ));
            }
        }

        // Update charge particles
        for particle in &mut self.charge_particles {
            particle.update(dt);
        }
        self.charge_particles.retain(|p| !p.is_dead());
    }

    fn fire(&mut self) {
        if self.is_charging && self.charge_time >= 0.2 { // Minimum charge time
            self.is_charging = false;
            self.is_firing = true;
            self.fire_duration = 0.0;
        }
    }

    fn update_firing(&mut self, dt: f32) {
        if !self.is_firing {
            return;
        }

        self.fire_duration += dt;
        if self.fire_duration >= self.max_fire_duration {
            self.is_firing = false;
            self.fire_duration = 0.0;
        }
    }

    fn get_laser_bounds(&self) -> Rect {
        if !self.is_firing {
            return Rect::new(0.0, 0.0, 0.0, 0.0);
        }
        
        // Laser extends from player position to top of screen
        Rect::new(
            self.position.x - self.width / 2.0,
            0.0,
            self.width,
            self.position.y,
        )
    }

    fn is_active(&self) -> bool {
        self.is_charging || self.is_firing
    }

    fn get_charge_progress(&self) -> f32 {
        (self.charge_time / LASER_CHARGE_TIME).min(1.0)
    }
}

fn main() -> GameResult {
    let cb = ggez::ContextBuilder::new("wipshmup", "ggez")
        .window_setup(WindowSetup::default().title("Shoot 'Em Up"))
        .window_mode(
            WindowMode::default()
                .dimensions(SCREEN_WIDTH, SCREEN_HEIGHT)
                .resizable(false),
        );
    let (ctx, event_loop) = cb.build()?;
    let state = GameState::new();
    event::run(ctx, event_loop, state)
}
