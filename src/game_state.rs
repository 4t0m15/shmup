use ggez::{
    event::EventHandler,
    graphics::{self, Color, DrawMode, DrawParam, Mesh, Rect, Text},
    input::keyboard::{KeyCode, KeyInput},
    Context, GameResult,
};
use ggez::glam::Vec2;
use rand::Rng;
use std::fs;
use std::io::Write;
use serde::{Deserialize, Serialize};

use crate::constants::*;
use crate::input::InputState;
use crate::entities::*;
use crate::effects::*;
use crate::weapons::*;
use ggez::graphics::Drawable;
use crate::constants::{/* ... existing imports ... */ AGGRESSIVENESS};

#[derive(Debug, Serialize, Deserialize)]
pub struct GameStats {
    pub high_score: u32,
    pub total_games_played: u32,
    pub total_play_time: f32,
    pub total_enemies_killed: u32,
    pub normal_enemies_killed: u32,
    pub fast_enemies_killed: u32,
    pub big_enemies_killed: u32,
    pub behemoths_killed: u32,
    pub destroyers_killed: u32,
    pub carriers_killed: u32,
    pub zeniths_killed: u32,
    pub bosses_killed: u32,
    pub total_score: u32,
    pub max_combo: u32,
    pub power_ups_collected: u32,
}

impl Default for GameStats {
    fn default() -> Self {
        Self {
            high_score: 0,
            total_games_played: 0,
            total_play_time: 0.0,
            total_enemies_killed: 0,
            normal_enemies_killed: 0,
            fast_enemies_killed: 0,
            big_enemies_killed: 0,
            behemoths_killed: 0,
            destroyers_killed: 0,
            carriers_killed: 0,
            zeniths_killed: 0,
            bosses_killed: 0,
            total_score: 0,
            max_combo: 0,
            power_ups_collected: 0,
        }
    }
}

impl GameStats {
    pub fn load() -> Self {
        // Create saves directory if it doesn't exist
        if let Err(e) = fs::create_dir_all("saves") {
            eprintln!("Failed to create saves directory: {}", e);
        }
        
        match fs::read_to_string("saves/stats.json") {
            Ok(content) => {
                match serde_json::from_str(&content) {
                    Ok(stats) => stats,
                    Err(e) => {
                        eprintln!("Failed to parse stats file: {}", e);
                        Self::default()
                    }
                }
            }
            Err(_) => Self::default(),
        }
    }

    pub fn save(&self) {
        // Create saves directory if it doesn't exist
        if let Err(e) = fs::create_dir_all("saves") {
            eprintln!("Failed to create saves directory: {}", e);
            return;
        }
        
        match fs::File::create("saves/stats.json") {
            Ok(mut file) => {
                match serde_json::to_string_pretty(self) {
                    Ok(json) => {
                        if let Err(e) = writeln!(file, "{}", json) {
                            eprintln!("Failed to write stats: {}", e);
                        }
                    }
                    Err(e) => {
                        eprintln!("Failed to serialize stats: {}", e);
                    }
                }
            }
            Err(e) => {
                eprintln!("Failed to create stats file: {}", e);
            }
        }
    }


}

#[derive(Debug)]
pub struct GameState {
    pub player: Player,
    pub bullets: Vec<Bullet>,
    pub enemies: Vec<Enemy>,
    pub boss: Option<Boss>,
    pub boss_bullets: Vec<Bullet>,
    pub score: u32,
    pub enemy_spawn_timer: f32,
    pub game_over: bool,
    pub stars: Vec<Star>,
    pub explosions: Vec<Explosion>,
    pub lives: u32,
    pub invincible_timer: f32,
    pub high_score: u32,
    pub power_ups: Vec<PowerUp>,
    pub power_up_timers: [f32; 4], // [rapid_fire, triple_shot, shield, laser]
    pub power_up_spawn_timer: f32,
    pub screen_shake: f32,
    pub background_particles: Vec<Particle>,
    pub combo_counter: u32,
    pub combo_timer: f32,
    pub combo_multiplier: f32,
    pub max_combo: u32,
    pub combo_texts: Vec<ComboText>,
    pub input: InputState,
    pub fire_cooldown: f32,
    pub fire_rate: f32, // Shots per second
    pub boss_spawn_score: u32, // Track when to spawn next boss
    pub boss_warning_timer: f32, // Timer for boss warning text
    pub laser: Laser,
    pub laser_instruction_timer: f32, // Timer for laser instruction popup
    pub behemoths_killed: u32,
    pub destroyers_killed: u32,
    pub carriers_killed: u32,
    pub normal_enemies_killed: u32,
    pub fast_enemies_killed: u32,
    pub big_enemies_killed: u32,
    pub grabbed_by_zenith: Option<usize>, // Index of Zenith enemy grabbing the player
    pub zeniths_killed: u32,
    pub death_explosion: Option<Explosion>, // Massive explosion when player dies
    pub death_explosion_timer: f32, // Timer for death explosion
    pub game_over_delay_timer: f32, // Timer for delay before showing game over screen
    pub play_time: f32, // Track total play time for this session
    pub stats: GameStats, // Game statistics
    pub combo_flame_particles: Vec<crate::effects::particle::Particle>, // Flame particles for SSSS rank
}

impl GameState {
    pub fn load_high_score() -> u32 {
        // Create saves directory if it doesn't exist
        if let Err(e) = fs::create_dir_all("saves") {
            eprintln!("Failed to create saves directory: {}", e);
        }
        
        match fs::read_to_string("saves/highscore.txt") {
            Ok(content) => content.trim().parse().unwrap_or(0),
            Err(_) => 0,
        }
    }

    pub fn save_high_score(score: u32) {
        // Create saves directory if it doesn't exist
        if let Err(e) = fs::create_dir_all("saves") {
            eprintln!("Failed to create saves directory: {}", e);
            return;
        }
        
        match fs::File::create("saves/highscore.txt") {
            Ok(mut file) => {
                if let Err(e) = writeln!(file, "{}", score) {
                    eprintln!("Failed to write high score: {}", e);
                }
            }
            Err(e) => {
                eprintln!("Failed to create high score file: {}", e);
            }
        }
    }

    pub fn new(ctx: &mut Context) -> Self {
        let mut rng = rand::thread_rng();
        let mut stars = Vec::with_capacity(STAR_COUNT);
        for _ in 0..STAR_COUNT {
            stars.push(Star::new(&mut rng));
        }
        
        let mut background_particles = Vec::new();
        for _ in 0..50 {
            let x = rng.gen_range(0.0..get_screen_width());
            let y = rng.gen_range(0.0..get_screen_height());
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
        
        // Get scaling for player position
        let scaling = get_scaling();
        let (player_x, player_y) = scaling.scale_position(get_screen_width() / 2.0, get_screen_height() - 50.0);
        
        Self {
            player: Player::new(Vec2::new(player_x, player_y)),
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
            input: InputState::default(),
            fire_cooldown: 0.0,
            fire_rate: 10.0, // Default fire rate
            boss_spawn_score: 0, // Initialize boss spawn score
            boss_warning_timer: 0.0, // Initialize boss warning timer
            laser: Laser::new(),
            laser_instruction_timer: 0.0, // Initialize laser instruction timer
            behemoths_killed: 0,
            destroyers_killed: 0,
            carriers_killed: 0,
            normal_enemies_killed: 0,
            fast_enemies_killed: 0,
            big_enemies_killed: 0,
            grabbed_by_zenith: None,
            zeniths_killed: 0,
            death_explosion: None,
            death_explosion_timer: 0.0,
            game_over_delay_timer: 0.0,
            play_time: 0.0,
            stats: GameStats::load(),
            combo_flame_particles: Vec::new(),
        }
    }

    pub fn check_high_score(&mut self) {
        if self.score > self.high_score {
            self.high_score = self.score;
            Self::save_high_score(self.high_score);
        }
    }

    pub fn create_death_explosion(&mut self) {
        // Create a massive explosion at player position
        let mut particles = Vec::new();
        let mut rng = rand::thread_rng();
        
        // Create many more particles for a massive explosion
        for _ in 0..200 { // Double the particles
            let angle = rng.gen_range(0.0..std::f32::consts::PI * 2.0);
            let speed = rng.gen_range(150.0..600.0); // Even faster particles
            let velocity = Vec2::new(angle.cos() * speed, angle.sin() * speed);
            let life = rng.gen_range(2.0..4.0); // Even longer life
            let color = match rng.gen_range(0..5) {
                0 => ggez::graphics::Color::new(1.0, 0.8, 0.2, 1.0), // Yellow
                1 => ggez::graphics::Color::new(1.0, 0.4, 0.0, 1.0), // Orange
                2 => ggez::graphics::Color::new(1.0, 0.2, 0.0, 1.0), // Red
                3 => ggez::graphics::Color::new(1.0, 1.0, 1.0, 1.0), // White
                _ => ggez::graphics::Color::new(1.0, 0.0, 0.0, 1.0), // Pure red
            };
            let size = rng.gen_range(5.0..15.0); // Even larger particles
            
            particles.push(crate::effects::Particle::new(
                self.player.position, 
                velocity, 
                life, 
                color, 
                size
            ));
        }
        
        self.death_explosion = Some(Explosion {
            timer: 0.0,
            particles,
        });
        self.death_explosion_timer = 2.0; // 2 seconds for death explosion
        self.game_over_delay_timer = 1.0; // 1 second delay before game over screen
        self.screen_shake = 0.8; // Even more massive screen shake
    }

    pub fn create_combo_text_effect(&mut self, position: Vec2, score: u32) {
        let combo_text = format!("{}", score);
        let combo_text_effect = ComboText::new(position, combo_text);
        self.combo_texts.push(combo_text_effect);
    }

    pub fn spawn_enemy(&mut self) {
        let mut rng = rand::thread_rng();
        let scaling = get_scaling();
        
        // Use scaled enemy size for spawn position calculation
        let scaled_enemy_size = scaling.scale_size(ENEMY_SIZE);
        let x = rng.gen_range(scaled_enemy_size..(get_screen_width() - scaled_enemy_size));
        
        // Check for Zenith spawn (scale with AGGRESSIVENESS, up to 20%)
        let zenith_chance = (0.03 * AGGRESSIVENESS).min(0.2);
        let enemy_type = if rng.gen::<f32>() < zenith_chance {
            EnemyType::Zenith
        } else {
            match rng.gen_range(0..10) {
                0..=5 => EnemyType::Normal, // 60% chance
                6..=7 => EnemyType::Fast,   // 20% chance
                _ => EnemyType::Big,        // 20% chance
            }
        };
        
        let (size, velocity) = match enemy_type {
            EnemyType::Normal => (ENEMY_SIZE, Vec2::new(0.0, ENEMY_SPEED)),
            EnemyType::Fast => (FAST_ENEMY_SIZE, Vec2::new(0.0, FAST_ENEMY_SPEED)),
            EnemyType::Big => (BIG_ENEMY_SIZE, Vec2::new(0.0, BIG_ENEMY_SPEED)),
            EnemyType::Zenith => (ZENITH_SIZE, Vec2::new(0.0, ZENITH_SPEED)),
        };
        
        // Use scaled size for spawn position
        let scaled_size = scaling.scale_size(size);
        let enemy = Enemy::new(
            Vec2::new(x, -scaled_size),
            velocity,
            size,
            enemy_type,
        );
        self.enemies.push(enemy);
    }

    pub fn spawn_power_up(&mut self) {
        let mut rng = rand::thread_rng();
        let x = rng.gen_range(50.0..(get_screen_width() - 50.0));
        let power_type = match rng.gen_range(0..4) { // Changed to 4 for Laser
            0 => PowerUpType::RapidFire,
            1 => PowerUpType::TripleShot,
            2 => PowerUpType::Shield,
            _ => PowerUpType::Laser, // Laser power-up
        };
        let power_up = PowerUp::new(Vec2::new(x, -20.0), power_type);
        self.power_ups.push(power_up);
    }

    pub fn spawn_boss(&mut self) {
        let mut rng = rand::thread_rng();
        let boss_type = match rng.gen_range(0..3) {
            0 => BossType::Destroyer,
            1 => BossType::Carrier,
            _ => BossType::Behemoth,
        };
        self.boss = Some(Boss::new(boss_type));
        self.boss_warning_timer = 3.0; // Show warning for 3 seconds
    }

    pub fn shoot(&mut self) {
        let base_bullet = Bullet::new(
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

    pub fn update_collisions(&mut self) {
        let mut enemies_to_destroy = Vec::new();
        let mut bullets_to_destroy = Vec::new();
        let mut boss_bullets_to_destroy = Vec::new();
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
                if bullet.collides_with(&Bullet::new(enemy.position, Vec2::ZERO, enemy.size)) {
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
                        EnemyType::Big => BIG_ENEMY_SCORE,
                        EnemyType::Zenith => 50, // Zenith gives more points
                        _ => 10,
                    };
                    let combo_score = (base_score as f32 * self.combo_multiplier) as u32;
                    self.score += combo_score;
                    self.explosions.push(Explosion::new(enemy.position));
                    self.screen_shake = 0.1; // Add screen shake
                    
                    // Store combo effect position and score
                    combo_effects.push((enemy.position, combo_score));
                    // Update kill counters
                    match enemy.enemy_type {
                        EnemyType::Normal => self.normal_enemies_killed += 1,
                        EnemyType::Fast => self.fast_enemies_killed += 1,
                        EnemyType::Big => self.big_enemies_killed += 1,
                        EnemyType::Zenith => self.normal_enemies_killed += 1, // Count as normal for now
                    }
                }
            }
        }
        
        // Check bullet-boss collisions
        if let Some(boss) = &mut self.boss {
            if boss.active {
                for (bullet_idx, bullet) in self.bullets.iter_mut().enumerate() {
                    if !bullet.active {
                        continue;
                    }
                    // Create a temporary bullet at boss position for collision detection
                    let boss_bullet = Bullet::new(boss.position, Vec2::ZERO, boss.size);
                    if bullet.collides_with(&boss_bullet) {
                        bullets_to_destroy.push(bullet_idx);
                        
                        // Damage the boss
                        boss.take_damage(10.0); // Base damage per bullet
                        
                        // Add screen shake for boss hits
                        self.screen_shake = 0.15;
                        
                        // Add explosion effect
                        self.explosions.push(Explosion::new(boss.position));
                        
                        // Check if boss is defeated
                        if !boss.active {
                            // Boss defeated!
                            match boss.boss_type {
                                BossType::Destroyer => self.destroyers_killed += 1,
                                BossType::Carrier => self.carriers_killed += 1,
                                BossType::Behemoth => self.behemoths_killed += 1,
                            }
                            let boss_score = match boss.boss_type {
                                BossType::Destroyer => 1000,
                                BossType::Carrier => 1500,
                                BossType::Behemoth => 2000,
                            };
                            self.score += boss_score;
                            self.screen_shake = 0.3; // Strong screen shake for boss defeat
                            
                            // Create multiple explosions for boss defeat
                            for _ in 0..5 {
                                let mut rng = rand::thread_rng();
                                let offset = Vec2::new(
                                    rng.gen_range(-boss.size..boss.size),
                                    rng.gen_range(-boss.size..boss.size),
                                );
                                self.explosions.push(Explosion::new(boss.position + offset));
                            }
                        }
                    }
                }
            }
        }
        
        // Check laser-enemy collisions
        if self.laser.is_firing {
            let laser_bounds = self.laser.get_laser_bounds();
            for (enemy_idx, enemy) in self.enemies.iter_mut().enumerate() {
                if !enemy.active {
                    continue;
                }
                // Check if enemy intersects with laser bounds
                let enemy_rect = Rect::new(
                    enemy.position.x - enemy.size / 2.0,
                    enemy.position.y - enemy.size / 2.0,
                    enemy.size,
                    enemy.size,
                );
                if laser_bounds.overlaps(&enemy_rect) {
                    enemies_to_destroy.push(enemy_idx);
                    
                    // Update combo system
                    self.combo_counter += 1;
                    self.combo_timer = COMBO_TIMEOUT;
                    self.combo_multiplier = (COMBO_MULTIPLIER_BASE * self.combo_counter as f32).min(COMBO_MULTIPLIER_CAP);
                    if self.combo_counter > self.max_combo {
                        self.max_combo = self.combo_counter;
                    }
                    
                    let base_score = match enemy.enemy_type {
                        EnemyType::Big => BIG_ENEMY_SCORE,
                        EnemyType::Zenith => 50, // Zenith gives more points
                        _ => 10,
                    };
                    let combo_score = (base_score as f32 * self.combo_multiplier) as u32;
                    self.score += combo_score;
                    self.explosions.push(Explosion::new(enemy.position));
                    self.screen_shake = 0.1;
                    
                    // Store combo effect position and score
                    combo_effects.push((enemy.position, combo_score));
                    // Update kill counters
                    match enemy.enemy_type {
                        EnemyType::Normal => self.normal_enemies_killed += 1,
                        EnemyType::Fast => self.fast_enemies_killed += 1,
                        EnemyType::Big => self.big_enemies_killed += 1,
                        EnemyType::Zenith => self.normal_enemies_killed += 1, // Count as normal for now
                    }
                }
            }
            
            // Check laser-boss collisions
            if let Some(boss) = &mut self.boss {
                if boss.active {
                    let boss_rect = Rect::new(
                        boss.position.x - boss.size / 2.0,
                        boss.position.y - boss.size / 2.0,
                        boss.size,
                        boss.size,
                    );
                                            if laser_bounds.overlaps(&boss_rect) {
                            // Damage the boss continuously while laser is firing
                            boss.take_damage(self.laser.damage * 0.016); // Fixed damage per frame (60 FPS)
                        
                        // Add screen shake for boss hits
                        self.screen_shake = 0.15;
                        
                        // Check if boss is defeated
                        if !boss.active {
                            // Boss defeated!
                            match boss.boss_type {
                                BossType::Destroyer => self.destroyers_killed += 1,
                                BossType::Carrier => self.carriers_killed += 1,
                                BossType::Behemoth => self.behemoths_killed += 1,
                            }
                            let boss_score = match boss.boss_type {
                                BossType::Destroyer => 1000,
                                BossType::Carrier => 1500,
                                BossType::Behemoth => 2000,
                            };
                            self.score += boss_score;
                            self.screen_shake = 0.3; // Strong screen shake for boss defeat
                            
                            // Create multiple explosions for boss defeat
                            for _ in 0..5 {
                                let mut rng = rand::thread_rng();
                                let offset = Vec2::new(
                                    rng.gen_range(-boss.size..boss.size),
                                    rng.gen_range(-boss.size..boss.size),
                                );
                                self.explosions.push(Explosion::new(boss.position + offset));
                            }
                        }
                    }
                }
            }
        }
        
        // Zenith beam grab logic
        if self.grabbed_by_zenith.is_none() {
            for (i, enemy) in self.enemies.iter_mut().enumerate() {
                if enemy.enemy_type == EnemyType::Zenith && enemy.is_zenith_beam_active() {
                    let beam_rect = enemy.get_zenith_beam_bounds();
                    let scaling = get_scaling();
                    let scaled_player_size = scaling.scale_size(PLAYER_SIZE);
                    let player_rect = Rect::new(
                        self.player.position.x - scaled_player_size / 2.0,
                        self.player.position.y - scaled_player_size / 2.0,
                        scaled_player_size,
                        scaled_player_size,
                    );
                    if beam_rect.overlaps(&player_rect) {
                        self.grabbed_by_zenith = Some(i);
                        break;
                    }
                }
            }
        }

        // Apply destruction and effects
        for &bullet_idx in bullets_to_destroy.iter().rev() {
            self.bullets[bullet_idx].active = false;
        }
        for &enemy_idx in enemies_to_destroy.iter().rev() {
            if let Some(enemy) = self.enemies.get(enemy_idx) {
                if enemy.enemy_type == EnemyType::Zenith {
                    self.zeniths_killed += 1;
                }
            }
            self.enemies[enemy_idx].active = false;
        }
        for (position, score) in combo_effects {
            self.create_combo_text_effect(position, score);
        }

        // Check player-powerup collisions
        for power_up in &mut self.power_ups {
            if power_up.active && self.player.collides_with(&Player::new(power_up.position)) {
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
                if enemy.active && self.player.collides_with(&Player::new(enemy.position)) {
                    player_hit = true;
                    break;
                }
            }
            
            // Check player-boss bullet collisions
            for (boss_bullet_idx, boss_bullet) in self.boss_bullets.iter_mut().enumerate() {
                if boss_bullet.active && self.player.collides_with(&Player::new(boss_bullet.position)) {
                    player_hit = true;
                    boss_bullets_to_destroy.push(boss_bullet_idx);
                    break;
                }
            }
            
            if player_hit {
                if self.lives > 1 {
                    self.lives -= 1;
                    self.player.position = Vec2::new(get_screen_width() / 2.0, get_screen_height() - 50.0);
                    self.player.velocity = Vec2::ZERO;
                    self.invincible_timer = 1.0; // 1 second invincibility
                    self.screen_shake = 0.2; // Strong screen shake
                    
                    // Reset combo when player is hit
                    self.combo_counter = 0;
                    self.combo_multiplier = 1.0;
                    self.combo_timer = 0.0;
                } else {
                    // Create massive death explosion
                    self.create_death_explosion();
                }
            }
        }

        // Clean up inactive objects
        self.bullets.retain(|bullet| bullet.active);
        self.enemies.retain(|enemy| enemy.active);
        self.power_ups.retain(|power_up| power_up.active);
        self.boss_bullets.retain(|bullet| bullet.active);
    }
}

impl EventHandler for GameState {
    fn update(&mut self, ctx: &mut Context) -> GameResult {
        let dt = ctx.time.delta().as_secs_f32();
        
        // Handle death explosion
        if let Some(ref mut explosion) = self.death_explosion {
            explosion.update(dt);
            self.death_explosion_timer -= dt;
            self.game_over_delay_timer -= dt;
            
            // Set game over after delay
            if !self.game_over && self.game_over_delay_timer <= 0.0 {
                self.game_over = true;
                self.check_high_score();
                // Save stats when game ends
                let play_time = self.play_time;
                let score = self.score;
                let max_combo = self.max_combo;
                let normal_enemies_killed = self.normal_enemies_killed;
                let fast_enemies_killed = self.fast_enemies_killed;
                let big_enemies_killed = self.big_enemies_killed;
                let behemoths_killed = self.behemoths_killed;
                let destroyers_killed = self.destroyers_killed;
                let carriers_killed = self.carriers_killed;
                let zeniths_killed = self.zeniths_killed;
                
                self.stats.total_games_played += 1;
                self.stats.total_play_time += play_time;
                self.stats.total_score += score;
                
                if score > self.stats.high_score {
                    self.stats.high_score = score;
                }
                
                if max_combo > self.stats.max_combo {
                    self.stats.max_combo = max_combo;
                }
                
                self.stats.normal_enemies_killed += normal_enemies_killed;
                self.stats.fast_enemies_killed += fast_enemies_killed;
                self.stats.big_enemies_killed += big_enemies_killed;
                self.stats.behemoths_killed += behemoths_killed;
                self.stats.destroyers_killed += destroyers_killed;
                self.stats.carriers_killed += carriers_killed;
                self.stats.zeniths_killed += zeniths_killed;
                
                self.stats.total_enemies_killed = self.stats.normal_enemies_killed + self.stats.fast_enemies_killed + 
                                               self.stats.big_enemies_killed + self.stats.behemoths_killed + 
                                               self.stats.destroyers_killed + self.stats.carriers_killed + 
                                               self.stats.zeniths_killed;
                
                self.stats.save();
                // Keep screen shake going for dramatic effect
            }
            
            // When explosion finishes, clean up
            if self.death_explosion_timer <= 0.0 {
                self.death_explosion = None;
            }
        }
        
        if self.game_over {
            return Ok(());
        }

        // Update play time when game is active
        self.play_time += dt;

        self.input.poll(ctx);

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
                    rng.gen_range(0.0..get_screen_width()),
                    rng.gen_range(0.0..get_screen_height()),
                );
                particle.life = rng.gen_range(2.0..5.0);
            }
        }

        // Handle continuous input for movement
        self.player.velocity = Vec2::ZERO;
        if self.input.pressed(KeyCode::A) || self.input.pressed(KeyCode::Left) {
            self.player.velocity.x -= PLAYER_SPEED;
        }
        if self.input.pressed(KeyCode::D) || self.input.pressed(KeyCode::Right) {
            self.player.velocity.x += PLAYER_SPEED;
        }
        if self.input.pressed(KeyCode::W) || self.input.pressed(KeyCode::Up) {
            self.player.velocity.y -= PLAYER_SPEED;
        }
        if self.input.pressed(KeyCode::S) || self.input.pressed(KeyCode::Down) {
            self.player.velocity.y += PLAYER_SPEED;
        }
        
        // Normalize diagonal movement to prevent faster diagonal speed
        if self.player.velocity.length() > PLAYER_SPEED {
            self.player.velocity = self.player.velocity.normalize() * PLAYER_SPEED;
        }

        // Handle firing with cooldown
        if self.input.pressed(KeyCode::Space) && !self.game_over {
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
            if self.input.pressed(KeyCode::Space) && !self.laser.is_charging && !self.laser.is_firing {
                // Start charging laser
                self.laser.start_charging(self.player.position);
            } else if !self.input.pressed(KeyCode::Space) && self.laser.is_charging {
                // Fire laser when Space key is released
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
        let scaling = get_scaling();
        let scaled_player_size = scaling.scale_size(PLAYER_SIZE);
        self.player.position.x = self.player.position.x.clamp(
            scaled_player_size / 2.0,
            get_screen_width() - scaled_player_size / 2.0,
        );
        self.player.position.y = self.player.position.y.clamp(
            scaled_player_size / 2.0,
            get_screen_height() - scaled_player_size / 2.0,
        );

        // Update bullets
        for bullet in &mut self.bullets {
            bullet.update(dt);
            // Remove bullets that go off screen
            let scaling = get_scaling();
            let scaled_bullet_size = scaling.scale_size(BULLET_SIZE);
            if bullet.position.y < -scaled_bullet_size {
                bullet.active = false;
            }
        }

        // Update enemies
        for enemy in &mut self.enemies {
            enemy.update(dt);
            // Remove enemies that go off screen
            if enemy.position.y > get_screen_height() + enemy.size {
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
            // Update laser position to follow player
            self.laser.update_position(self.player.position);
        } else {
            // Reset laser when powerup expires
            self.laser = Laser::new();
        }

        // Spawn enemies - AGGRESSIVENESS controls frequency and count
        self.enemy_spawn_timer += dt;
        if self.enemy_spawn_timer >= 0.7 / AGGRESSIVENESS {
            let base_enemies = 5.0 * AGGRESSIVENESS;
            let base_area = 1920.0 * 1080.0;
            let screen_area = crate::constants::get_screen_width() * crate::constants::get_screen_height();
            let scale = screen_area / base_area;
            let num_enemies = (base_enemies * scale * AGGRESSIVENESS).ceil() as usize;
            for _ in 0..num_enemies {
                self.spawn_enemy();
            }
            self.enemy_spawn_timer = 0.0;
        }

        // Spawn power-ups - scale with AGGRESSIVENESS (more frequent if higher)
        self.power_up_spawn_timer += dt;
        if self.power_up_spawn_timer >= 8.0 / AGGRESSIVENESS {
            self.spawn_power_up();
            self.power_up_spawn_timer = 0.0;
        }

        // Check for boss spawning (scale with AGGRESSIVENESS)
        if self.score >= self.boss_spawn_score + (10000.0 / AGGRESSIVENESS) as u32 && self.boss.is_none() {
            self.spawn_boss();
            self.boss_spawn_score = self.score;
        }

        // Update boss
        if let Some(boss) = &mut self.boss {
            boss.update(dt);
            
            // Spawn boss bullets based on attack patterns - scale with AGGRESSIVENESS
            let attack_interval = match boss.boss_type {
                BossType::Destroyer => if boss.phase == 1 { 0.8 / AGGRESSIVENESS } else { 0.5 / AGGRESSIVENESS },
                BossType::Carrier => if boss.phase == 1 { 1.2 / AGGRESSIVENESS } else { 0.8 / AGGRESSIVENESS },
                BossType::Behemoth => if boss.phase == 1 { 1.0 / AGGRESSIVENESS } else { 0.6 / AGGRESSIVENESS },
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
                    
                    let bullet = Bullet::new(spawn_point, velocity, 8.0);
                    self.boss_bullets.push(bullet);
                }
                
                boss.attack_timer = 0.0;
            }
        }

        // Update boss bullets
        for boss_bullet in &mut self.boss_bullets {
            boss_bullet.update(dt);
            // Remove boss bullets that go off screen
            if boss_bullet.position.y > get_screen_height() + boss_bullet.size || 
            boss_bullet.position.x < -boss_bullet.size || 
            boss_bullet.position.x > get_screen_width() + boss_bullet.size {
                boss_bullet.active = false;
            }
        }

        // Update collisions
        self.update_collisions();

        // Zenith grab movement
        if let Some(zenith_idx) = self.grabbed_by_zenith {
            if let Some(zenith) = self.enemies.get(zenith_idx) {
                if zenith.enemy_type == EnemyType::Zenith && zenith.is_zenith_beam_active() {
                    // Lerp player toward Zenith's y
                    let scaling = get_scaling();
                    let scaled_player_size = scaling.scale_size(PLAYER_SIZE);
                    let target = Vec2::new(zenith.position.x, zenith.position.y + zenith.size / 2.0 + scaled_player_size / 2.0);
                    let dir = (target - self.player.position).normalize_or_zero();
                    self.player.velocity = dir * ZENITH_BEAM_SPEED;
                    self.player.position += self.player.velocity * dt;
                    // If player reaches Zenith or top, lose a life
                    if (self.player.position.y <= zenith.position.y + zenith.size / 2.0) || (self.player.position.y < 0.0) {
                        if self.lives > 1 {
                            self.lives -= 1;
                            self.player.position = Vec2::new(get_screen_width() / 2.0, get_screen_height() - 50.0);
                            self.player.velocity = Vec2::ZERO;
                            self.invincible_timer = 1.0;
                            self.screen_shake = 0.2;
                        } else {
                            // Create massive death explosion
                            self.create_death_explosion();
                        }
                        self.grabbed_by_zenith = None;
                    }
                } else {
                    // Beam ended, release player
                    self.grabbed_by_zenith = None;
                }
            } else {
                self.grabbed_by_zenith = None;
            }
        }

        // Update combo flame particles for SSSS+ ranks
        let s_count = if self.combo_counter >= 40 && self.combo_counter < 90 {
            // Count the number of S's in the rank string
            let rank = if self.combo_counter > 0 {
                match self.combo_counter {
                    90..=u32::MAX => "SSSSSSSSS",
                    80..=89 => "SSSSSSSS",
                    70..=79 => "SSSSSSS",
                    60..=69 => "SSSSSS",
                    50..=59 => "SSSSS",
                    40..=49 => "SSSS",
                    _ => "",
                }
            } else { "" };
            rank.matches('S').count().max(4) // At least 4 for SSSS
        } else { 0 };
        if s_count > 0 {
            // Spawn new flame particles if not too many
            let max_particles = 10 * s_count;
            if self.combo_flame_particles.len() < max_particles {
                let mut rng = rand::thread_rng();
                for _ in 0..s_count {
                    let angle = rng.gen_range(-0.7..0.7);
                    let speed = rng.gen_range(20.0..60.0);
                    let velocity = ggez::glam::Vec2::new((angle as f32).cos() * speed, (angle as f32).sin() * speed - 10.0);
                    let life = rng.gen_range(0.4..0.8);
                    let color = match rng.gen_range(0..3) {
                        0 => ggez::graphics::Color::new(1.0, 0.8, 0.2, 1.0), // Yellow
                        1 => ggez::graphics::Color::new(1.0, 0.4, 0.0, 1.0), // Orange
                        _ => ggez::graphics::Color::new(1.0, 0.2, 0.0, 1.0), // Red
                    };
                    let size = rng.gen_range(3.0..8.0);
                    // Centered behind the SSSS text
                    let pos = ggez::glam::Vec2::new(
                        rng.gen_range(-10.0..10.0),
                        rng.gen_range(-5.0..5.0),
                    );
                    self.combo_flame_particles.push(crate::effects::particle::Particle::new(pos, velocity, life, color, size));
                }
            }
            // Update and retain alive particles
            for p in &mut self.combo_flame_particles {
                p.update(dt);
            }
            self.combo_flame_particles.retain(|p| !p.is_dead());
        } else {
            self.combo_flame_particles.clear();
        }

        // Apply strong screen shake for 9S rank
        if self.combo_counter >= 150 {
            self.screen_shake = 0.8;
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
            let heart_x = crate::constants::get_screen_width() - 20.0 - (self.lives - 1 - i) as f32 * 30.0;
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

        if self.game_over && self.game_over_delay_timer <= 0.0 {
            // Enhanced game over screen
            let score_text = Text::new(graphics::TextFragment::new(format!("SCORE: {}", self.score)).scale(48.0));
            let game_over_text = Text::new(graphics::TextFragment::new("Game Over").scale(36.0));
            let stats_title = Text::new(graphics::TextFragment::new("Stats:").scale(28.0));
            let behemoth_text = Text::new(graphics::TextFragment::new(format!("Behemoths killed: {}", self.behemoths_killed)).scale(24.0));
            let destroyer_text = Text::new(graphics::TextFragment::new(format!("Destroyers killed: {}", self.destroyers_killed)).scale(24.0));
            let carrier_text = Text::new(graphics::TextFragment::new(format!("Carriers killed: {}", self.carriers_killed)).scale(24.0));
            let normal_text = Text::new(graphics::TextFragment::new(format!("Normal enemies killed: {}", self.normal_enemies_killed)).scale(24.0));
            let fast_text = Text::new(graphics::TextFragment::new(format!("Fast enemies killed: {}", self.fast_enemies_killed)).scale(24.0));
            let big_text = Text::new(graphics::TextFragment::new(format!("Big enemies killed: {}", self.big_enemies_killed)).scale(24.0));
            let zenith_text = Text::new(graphics::TextFragment::new(format!("Zeniths killed: {}", self.zeniths_killed)).scale(24.0));
            let high_score_text = Text::new(graphics::TextFragment::new(format!("High Score: {}", self.high_score)).scale(20.0));
            let restart_text = Text::new(graphics::TextFragment::new("Press R to restart or ESC for menu").scale(20.0));

            let center_x = crate::constants::get_screen_width() / 2.0;
            let mut y = 60.0;
            // Score
            canvas.draw(
                &score_text,
                DrawParam::default()
                    .dest(Vec2::new(center_x - score_text.dimensions(ctx).unwrap().w as f32 / 2.0, y))
                    .color(Color::WHITE),
            );
            y += 60.0;
            // Game Over
            canvas.draw(
                &game_over_text,
                DrawParam::default()
                    .dest(Vec2::new(center_x - game_over_text.dimensions(ctx).unwrap().w as f32 / 2.0, y))
                    .color(Color::RED),
            );
            y += 60.0;
            // Stats title
            canvas.draw(
                &stats_title,
                DrawParam::default()
                    .dest(Vec2::new(60.0, y))
                    .color(Color::WHITE),
            );
            y += 40.0;
            // Behemoths
            canvas.draw(
                &behemoth_text,
                DrawParam::default()
                    .dest(Vec2::new(80.0, y))
                    .color(Color::WHITE),
            );
            y += 30.0;
            // Destroyers
            canvas.draw(
                &destroyer_text,
                DrawParam::default()
                    .dest(Vec2::new(80.0, y))
                    .color(Color::WHITE),
            );
            y += 30.0;
            // Carriers
            canvas.draw(
                &carrier_text,
                DrawParam::default()
                    .dest(Vec2::new(80.0, y))
                    .color(Color::WHITE),
            );
            y += 30.0;
            // Normal enemies
            canvas.draw(
                &normal_text,
                DrawParam::default()
                    .dest(Vec2::new(80.0, y))
                    .color(Color::WHITE),
            );
            y += 30.0;
            // Fast enemies
            canvas.draw(
                &fast_text,
                DrawParam::default()
                    .dest(Vec2::new(80.0, y))
                    .color(Color::WHITE),
            );
            y += 30.0;
            // Big enemies
            canvas.draw(
                &big_text,
                DrawParam::default()
                    .dest(Vec2::new(80.0, y))
                    .color(Color::WHITE),
            );
            y += 30.0;
            // Zeniths
            canvas.draw(
                &zenith_text,
                DrawParam::default()
                    .dest(Vec2::new(80.0, y))
                    .color(Color::WHITE),
            );
            y += 50.0;
            // High score
            canvas.draw(
                &high_score_text,
                DrawParam::default()
                    .dest(Vec2::new(center_x - high_score_text.dimensions(ctx).unwrap().w as f32 / 2.0, y))
                    .color(Color::YELLOW),
            );
            y += 30.0;
            // Restart
            canvas.draw(
                &restart_text,
                DrawParam::default()
                    .dest(Vec2::new(center_x - restart_text.dimensions(ctx).unwrap().w as f32 / 2.0, y))
                    .color(Color::WHITE),
            );
        } else {
            // Draw player (only if not in death explosion)
            if self.death_explosion.is_none() {
                self.player.draw(ctx, &mut canvas, self.invincible_timer, self.power_up_timers[2] > 0.0)?;
            }

            // Draw bullets
            for bullet in &self.bullets {
                bullet.draw(ctx, &mut canvas)?;
            }

            // Draw enemies
            for enemy in &self.enemies {
                enemy.draw(ctx, &mut canvas)?;
            }

            // Draw power-ups
            for power_up in &self.power_ups {
                power_up.draw(ctx, &mut canvas)?;
            }

            // Draw boss
            if let Some(boss) = &self.boss {
                boss.draw(ctx, &mut canvas)?;
            }

            // Draw boss bullets
            for boss_bullet in &self.boss_bullets {
                boss_bullet.draw(ctx, &mut canvas)?;
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

            // Draw death explosion (if active)
            if let Some(ref explosion) = self.death_explosion {
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

            // Draw combo system (always visible, new panel)
            self.draw_combo_panel(ctx, &mut canvas)?;

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
                        .dest(Vec2::new(crate::constants::get_screen_width() / 2.0 - 80.0, 80.0) + shake_offset)
                        .color(warning_color),
                );
            }

            // Draw laser instruction popup
            if self.laser_instruction_timer > 0.0 {
                let laser_title = Text::new("LASER WEAPON ACQUIRED!");
                let laser_instruction1 = Text::new("Hold SPACE to charge the laser");
                let laser_instruction2 = Text::new("Release SPACE to fire!");
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
                        .dest(Vec2::new(crate::constants::get_screen_width() / 2.0 - 150.0, 80.0) + shake_offset)
                        .color(laser_color),
                );
                
                // Instructions
                canvas.draw(
                    &laser_instruction1,
                    DrawParam::default()
                        .dest(Vec2::new(crate::constants::get_screen_width() / 2.0 - 120.0, 110.0) + shake_offset)
                        .color(Color::WHITE),
                );
                
                canvas.draw(
                    &laser_instruction2,
                    DrawParam::default()
                        .dest(Vec2::new(crate::constants::get_screen_width() / 2.0 - 100.0, 130.0) + shake_offset)
                        .color(Color::WHITE),
                );
                
                canvas.draw(
                    &laser_tip,
                    DrawParam::default()
                        .dest(Vec2::new(crate::constants::get_screen_width() / 2.0 - 120.0, 150.0) + shake_offset)
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
                        .dest(Vec2::new(crate::constants::get_screen_width() / 2.0 - 100.0, crate::constants::get_screen_height() - 80.0) + shake_offset)
                        .color(charge_color),
                );
                
                // Draw charge bar
                let bar_width = 200.0;
                let bar_height = 10.0;
                let bar_x = crate::constants::get_screen_width() / 2.0 - bar_width / 2.0;
                let bar_y = crate::constants::get_screen_height() - 60.0;
                
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
                self.laser.draw(ctx, &mut canvas)?;
            }
        }

        // Draw death explosion even during game over screen
        if let Some(ref explosion) = self.death_explosion {
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
            self.input.held.insert(keycode);
            
            // Handle restart key
            if keycode == KeyCode::R && self.game_over {
                *self = GameState::new(_ctx);
                self.check_high_score();
            }
        }
        Ok(())
    }

    fn key_up_event(&mut self, _ctx: &mut Context, input: KeyInput) -> GameResult {
        if let Some(keycode) = input.keycode {
            self.input.held.remove(&keycode);
        }
        Ok(())
    }
} 

impl GameState {
    fn draw_combo_panel(&self, ctx: &mut Context, canvas: &mut graphics::Canvas) -> GameResult {
        use ggez::graphics::{Color, DrawMode, DrawParam, Mesh, Rect, Text, TextFragment};
        use ggez::glam::Vec2;
        // Panel position (bottom right)
        let panel_w = 420.0; // Increased width for 9 S's and long timer bar
        let panel_h = 70.0;
        let margin = 20.0;
        let panel_x = crate::constants::get_screen_width() - panel_w - margin;
        let panel_y = crate::constants::get_screen_height() - panel_h - margin;

        // --- Rank/Label (e.g. "SSS" or "DESTRUCTIVE") ---
        let rank = if self.combo_counter > 0 {
            match self.combo_counter {
                150..=u32::MAX => "SSSSSSSSS",
                80..=149 => "SSSSSSSS",
                70..=79 => "SSSSSSS",
                60..=69 => "SSSSSS",
                50..=59 => "SSSSS",
                40..=49 => "SSSS",
                30..=39 => "SSS",
                20..=29 => "SS",
                15..=19 => "S",
                10..=14 => "A",
                7..=9 => "B",
                5..=6 => "C",
                3..=4 => "D",
                1..=2 => "E",
                _ => "",
            }
        } else {
            ""
        };
        // Color order for S's: Blue, Green, Yellow, Orange, Red, Gold
        let s_colors = [
            Color::from_rgb(0, 120, 255),   // Blue
            Color::from_rgb(0, 200, 70),    // Green
            Color::from_rgb(255, 220, 0),   // Yellow
            Color::from_rgb(255, 140, 0),   // Orange
            Color::from_rgb(226, 33, 33),   // Red
            Color::from_rgb(255, 215, 0),   // Gold
        ];
        // Determine S count for color
        let s_count = rank.matches('S').count();
        let s_color = if s_count == 0 {
            Color::from_rgb(226, 33, 33)
        } else if s_count <= 5 {
            s_colors[s_count - 1]
        } else if s_count < 9 {
            s_colors[5]
        } else {
            Color::WHITE
        };
        // Draw flame particles behind SSSS+ ranks
        if rank.starts_with("SSSS") && rank != "SSSSSSSSS" {
            for particle in &self.combo_flame_particles {
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
                        Vec2::new(panel_x + 50.0, panel_y + 30.0) + particle.position,
                        particle.size,
                        0.1,
                        color,
                    )?;
                    canvas.draw(&mesh, DrawParam::default());
                }
            }
        }
        // Draw the rank (or label)
        let rank_frag = TextFragment::new(rank)
            .scale(44.0)
            .color(s_color);
        let rank_text = Text::new(rank_frag);
        let rank_x = panel_x + 10.0;
        let rank_y = panel_y + 2.0;
        // Shake effect for 9S rank
        let mut shake_offset = Vec2::ZERO;
        if rank == "SSSSSSSSS" {
            use rand::Rng;
            let mut rng = rand::thread_rng();
            shake_offset = Vec2::new(rng.gen_range(-6.0..6.0), rng.gen_range(-6.0..6.0));
            for i in 0..8 {
                let angle = (i as f32) * std::f32::consts::PI * 2.0 / 8.0;
                let offset = (2.0 * angle.cos(), 2.0 * angle.sin());
                let glow_frag = TextFragment::new(rank)
                    .scale(44.0)
                    .color(Color::new(1.0, 1.0, 1.0, 0.15));
                let glow_text = Text::new(glow_frag);
                canvas.draw(
                    &glow_text,
                    DrawParam::default().dest(Vec2::new(rank_x + offset.0, rank_y + offset.1) + shake_offset),
                );
            }
        }
        canvas.draw(
            &rank_text,
            DrawParam::default().dest(Vec2::new(rank_x, rank_y) + shake_offset),
        );

        // --- Multiplier (right-aligned, bold, white) ---
        let mult_val = if self.combo_counter > 0 {
            format!("{:.1}x", self.combo_multiplier.min(50.0))
        } else {
            "0.0x".to_string()
        };
        let mult_text = Text::new(TextFragment::new(mult_val).scale(36.0).color(Color::WHITE));
        let mult_dim = mult_text.dimensions(ctx).unwrap_or_default();
        let mult_x = panel_x + panel_w - mult_dim.w as f32 - 10.0;
        let mult_y = panel_y + 8.0;
        canvas.draw(
            &mult_text,
            DrawParam::default().dest(Vec2::new(mult_x, mult_y)),
        );

        // --- Progress Bar (red, with white background) ---
        let bar_y = panel_y + 50.0;
        let bar_h = 14.0;
        let bar_w = panel_w - 20.0; // Now much longer
        let bar_x = panel_x + 10.0;
        let max_time = crate::constants::COMBO_TIMEOUT;
        // Timer decreases faster for higher ranks
        let time_factor = if self.combo_counter >= 150 {
            0.25
        } else if self.combo_counter >= 130 {
            0.4
        } else if self.combo_counter >= 110 {
            0.6
        } else if self.combo_counter >= 90 {
            0.8
        } else {
            1.0
        };
        let frac = ((self.combo_timer / max_time) * time_factor).clamp(0.0, 1.0);
        let filled_w = bar_w * frac;
        // Background (white)
        let bar_bg = Mesh::new_rectangle(
            ctx,
            DrawMode::fill(),
            Rect::new(bar_x, bar_y, bar_w, bar_h),
            Color::WHITE,
        )?;
        canvas.draw(&bar_bg, DrawParam::default());
        // Filled (red or white for 9S)
        let bar_color = if rank == "SSSSSSSSS" { Color::WHITE } else { Color::from_rgb(226, 33, 33) };
        let bar_fill = Mesh::new_rectangle(
            ctx,
            DrawMode::fill(),
            Rect::new(bar_x, bar_y, filled_w, bar_h),
            bar_color,
        )?;
        canvas.draw(&bar_fill, DrawParam::default());

        // --- Time Remaining (small, gray, right-aligned under bar) ---
        let time_val = if self.combo_timer > 0.0 {
            format!("{:.1}s", self.combo_timer * time_factor)
        } else {
            "0.0s".to_string()
        };
        let time_text = Text::new(TextFragment::new(time_val).scale(18.0).color(Color::from_rgb(111, 111, 111)));
        let time_dim = time_text.dimensions(ctx).unwrap_or_default();
        let time_x = panel_x + panel_w - time_dim.w as f32 - 10.0;
        let time_y = bar_y + bar_h + 2.0;
        canvas.draw(
            &time_text,
            DrawParam::default().dest(Vec2::new(time_x, time_y)),
        );

        Ok(())
    }
} 