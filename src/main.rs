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

#[derive(Clone, Copy, PartialEq)]
enum EnemyType {
    Normal,
    Fast,
    Big,
}

#[derive(Clone, Copy, PartialEq)]
enum PowerUpType {
    RapidFire,
    TripleShot,
    Shield,
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
    position: Vec2,
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
        
        Self { position, timer: 0.0, particles }
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
        }
    }
}

struct GameState {
    player: GameObject,
    bullets: Vec<GameObject>,
    enemies: Vec<GameObject>,
    score: u32,
    enemy_spawn_timer: f32,
    game_over: bool,
    stars: Vec<Star>,
    explosions: Vec<Explosion>,
    lives: u32,
    invincible_timer: f32,
    high_score: u32,
    power_ups: Vec<PowerUp>,
    power_up_timers: [f32; 3], // [rapid_fire, triple_shot, shield]
    power_up_spawn_timer: f32,
    screen_shake: f32,
    background_particles: Vec<Particle>,
    combo_counter: u32,
    combo_timer: f32,
    combo_multiplier: f32,
    max_combo: u32,
    combo_texts: Vec<ComboText>,
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
            score: 0,
            enemy_spawn_timer: 0.0,
            game_over: false,
            stars,
            explosions: Vec::new(),
            lives: 3,
            invincible_timer: 0.0,
            high_score: Self::load_high_score(),
            power_ups: Vec::new(),
            power_up_timers: [0.0; 3],
            power_up_spawn_timer: 0.0,
            screen_shake: 0.0,
            background_particles,
            combo_counter: 0,
            combo_timer: 0.0,
            combo_multiplier: 1.0,
            max_combo: 0,
            combo_texts: Vec::new(),
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
        let power_type = match rng.gen_range(0..3) {
            0 => PowerUpType::RapidFire,
            1 => PowerUpType::TripleShot,
            _ => PowerUpType::Shield,
        };
        let power_up = PowerUp::new(Vec2::new(x, -20.0), power_type);
        self.power_ups.push(power_up);
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
                    self.enemies.clear();
                    self.bullets.clear();
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
        match input.keycode {
            Some(KeyCode::A) | Some(KeyCode::Left) => {
                self.player.velocity.x = -PLAYER_SPEED;
            }
            Some(KeyCode::D) | Some(KeyCode::Right) => {
                self.player.velocity.x = PLAYER_SPEED;
            }
            Some(KeyCode::W) | Some(KeyCode::Up) => {
                self.player.velocity.y = -PLAYER_SPEED;
            }
            Some(KeyCode::S) | Some(KeyCode::Down) => {
                self.player.velocity.y = PLAYER_SPEED;
            }
            Some(KeyCode::Space) => {
                if !self.game_over {
                    if self.power_up_timers[0] > 0.0 { // Rapid fire
                        self.shoot();
                    } else {
                        self.shoot();
                    }
                }
            }
            Some(KeyCode::R) => {
                if self.game_over {
                    *self = GameState::new();
                    self.check_high_score();
                }
            }
            _ => {}
        }
        Ok(())
    }

    fn key_up_event(&mut self, _ctx: &mut Context, input: KeyInput) -> GameResult {
        match input.keycode {
            Some(KeyCode::A) | Some(KeyCode::D) | Some(KeyCode::Left) | Some(KeyCode::Right) => {
                self.player.velocity.x = 0.0;
            }
            Some(KeyCode::W) | Some(KeyCode::S) | Some(KeyCode::Up) | Some(KeyCode::Down) => {
                self.player.velocity.y = 0.0;
            }
            _ => {}
        }
        Ok(())
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
