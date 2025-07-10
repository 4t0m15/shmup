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

use crate::constants::*;
use crate::input::InputState;
use crate::entities::*;
use crate::effects::*;
use crate::weapons::*;
use ggez::graphics::Drawable;

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

    pub fn new() -> Self {
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
            player: Player::new(Vec2::new(SCREEN_WIDTH / 2.0, SCREEN_HEIGHT - 50.0)),
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
        }
    }

    pub fn check_high_score(&mut self) {
        if self.score > self.high_score {
            self.high_score = self.score;
            Self::save_high_score(self.high_score);
        }
    }

    pub fn create_combo_text_effect(&mut self, position: Vec2) {
        let combo_text = format!("{}x{}!", self.combo_counter, self.combo_multiplier);
        let combo_text_effect = ComboText::new(position, combo_text);
        self.combo_texts.push(combo_text_effect);
    }

    pub fn spawn_enemy(&mut self) {
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
        let enemy = Enemy::new(
            Vec2::new(x, -size),
            velocity,
            size,
            enemy_type,
        );
        self.enemies.push(enemy);
    }

    pub fn spawn_power_up(&mut self) {
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
                    // Update kill counters
                    match enemy.enemy_type {
                        EnemyType::Normal => self.normal_enemies_killed += 1,
                        EnemyType::Fast => self.fast_enemies_killed += 1,
                        EnemyType::Big => self.big_enemies_killed += 1,
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
        self.boss_bullets.retain(|bullet| bullet.active);
    }
}

impl EventHandler for GameState {
    fn update(&mut self, ctx: &mut Context) -> GameResult {
        if self.game_over {
            return Ok(());
        }

        let dt = ctx.time.delta().as_secs_f32();
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
                    rng.gen_range(0.0..SCREEN_WIDTH),
                    rng.gen_range(0.0..SCREEN_HEIGHT),
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
            if self.input.pressed(KeyCode::L) && !self.laser.is_charging && !self.laser.is_firing {
                // Start charging laser
                self.laser.start_charging(self.player.position);
            } else if !self.input.pressed(KeyCode::L) && self.laser.is_charging {
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
            if boss_bullet.position.y > SCREEN_HEIGHT + boss_bullet.size || 
            boss_bullet.position.x < -boss_bullet.size || 
            boss_bullet.position.x > SCREEN_WIDTH + boss_bullet.size {
                boss_bullet.active = false;
            }
        }

        // Update collisions
        self.update_collisions();

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
            let high_score_text = Text::new(graphics::TextFragment::new(format!("High Score: {}", self.high_score)).scale(20.0));
            let restart_text = Text::new(graphics::TextFragment::new("Press R to restart").scale(20.0));

            let center_x = SCREEN_WIDTH / 2.0;
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
            // Draw player
            self.player.draw(ctx, &mut canvas, self.invincible_timer, self.power_up_timers[2] > 0.0)?;

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
                self.laser.draw(ctx, &mut canvas)?;
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
                *self = GameState::new();
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