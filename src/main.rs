use ggez::{
    conf::{WindowMode, WindowSetup},
    event::{self, EventHandler},
    graphics::{self, Color, DrawMode, DrawParam, Mesh, Rect, Text},
    input::keyboard::{KeyCode, KeyInput},
    Context, GameResult,
};
use ggez::glam::Vec2;
use rand::Rng;

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

#[derive(Clone, Copy, PartialEq)]
enum EnemyType {
    Normal,
    Fast,
    Big,
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
}

impl Star {
    fn new(rng: &mut rand::rngs::ThreadRng) -> Self {
        let x = rng.gen_range(0.0..SCREEN_WIDTH);
        let y = rng.gen_range(0.0..SCREEN_HEIGHT);
        let speed = rng.gen_range(0.5..1.5) * STAR_SPEED;
        Self {
            position: Vec2::new(x, y),
            speed,
        }
    }

    fn update(&mut self, dt: f32) {
        self.position.y += self.speed * dt;
        if self.position.y > SCREEN_HEIGHT {
            self.position.y = 0.0;
            self.position.x = rand::thread_rng().gen_range(0.0..SCREEN_WIDTH);
        }
    }
}

struct Explosion {
    position: Vec2,
    timer: f32,
}

impl Explosion {
    fn new(position: Vec2) -> Self {
        Self { position, timer: 0.0 }
    }

    fn update(&mut self, dt: f32) {
        self.timer += dt;
    }

    fn is_done(&self) -> bool {
        self.timer > EXPLOSION_DURATION
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
    invincible_timer: f32, // Add invincibility timer
}

impl GameState {
    fn new() -> Self {
        let mut rng = rand::thread_rng();
        let mut stars = Vec::with_capacity(STAR_COUNT);
        for _ in 0..STAR_COUNT {
            stars.push(Star::new(&mut rng));
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
            invincible_timer: 0.0, // Initialize
        }
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

    fn shoot(&mut self) {
        let bullet = GameObject::new(
            self.player.position,
            Vec2::new(0.0, -BULLET_SPEED),
            BULLET_SIZE,
        );
        self.bullets.push(bullet);
    }

    fn update_collisions(&mut self) {
        // Check bullet-enemy collisions
        for bullet in &mut self.bullets {
            if !bullet.active {
                continue;
            }
            for enemy in &mut self.enemies {
                if !enemy.active {
                    continue;
                }
                if bullet.collides_with(enemy) {
                    bullet.active = false;
                    enemy.active = false;
                    let score = match enemy.enemy_type {
                        Some(EnemyType::Big) => BIG_ENEMY_SCORE,
                        _ => 10,
                    };
                    self.score += score;
                    self.explosions.push(Explosion::new(enemy.position));
                }
            }
        }

        // Check player-enemy collisions (skip if invincible)
        if self.invincible_timer <= 0.0 {
            for enemy in &self.enemies {
                if enemy.active && self.player.collides_with(enemy) {
                    if self.lives > 1 {
                        self.lives -= 1;
                        self.player.position = Vec2::new(SCREEN_WIDTH / 2.0, SCREEN_HEIGHT - 50.0);
                        self.player.velocity = Vec2::ZERO;
                        self.enemies.clear();
                        self.bullets.clear();
                        self.invincible_timer = 1.0; // 1 second invincibility
                        break;
                    } else {
                        self.game_over = true;
                    }
                }
            }
        }

        // Clean up inactive objects
        self.bullets.retain(|bullet| bullet.active);
        self.enemies.retain(|enemy| enemy.active);
    }
}

impl EventHandler for GameState {
    fn update(&mut self, ctx: &mut Context) -> GameResult {
        if self.game_over {
            return Ok(());
        }

        let dt = ctx.time.delta().as_secs_f32();

        // Update invincibility timer
        if self.invincible_timer > 0.0 {
            self.invincible_timer -= dt;
            if self.invincible_timer < 0.0 {
                self.invincible_timer = 0.0;
            }
        }

        // Update stars
        for star in &mut self.stars {
            star.update(dt);
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

        // Update explosions
        for explosion in &mut self.explosions {
            explosion.update(dt);
        }
        self.explosions.retain(|e| !e.is_done());

        // Spawn enemies
        self.enemy_spawn_timer += dt;
        if self.enemy_spawn_timer >= 0.7 {
            for _ in 0..5 {
                self.spawn_enemy();
            }
            self.enemy_spawn_timer = 0.0;
        }

        // Update collisions
        self.update_collisions();

        Ok(())
    }

    fn draw(&mut self, ctx: &mut Context) -> GameResult {
        let mut canvas = graphics::Canvas::from_frame(ctx, Color::BLACK);

        // Draw starfield
        for star in &self.stars {
            let color = Color::new(1.0, 1.0, 1.0, 0.7);
            let star_mesh = Mesh::new_circle(
                ctx,
                DrawMode::fill(),
                star.position,
                1.5,
                0.1,
                color,
            )?;
            canvas.draw(&star_mesh, DrawParam::default());
        }

        // Draw lives as hearts (top left)
        for i in 0..self.lives {
            let heart_x = 20.0 + i as f32 * 30.0;
            let heart_y = 40.0;
            let heart_mesh = Mesh::new_circle(
                ctx,
                DrawMode::fill(),
                Vec2::new(heart_x, heart_y),
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
            let restart_text = Text::new("Press R to restart");

            canvas.draw(
                &game_over_text,
                DrawParam::default()
                    .dest(Vec2::new(SCREEN_WIDTH / 2.0 - 100.0, SCREEN_HEIGHT / 2.0 - 50.0))
                    .color(Color::RED),
            );
            canvas.draw(
                &score_text,
                DrawParam::default()
                    .dest(Vec2::new(SCREEN_WIDTH / 2.0 - 80.0, SCREEN_HEIGHT / 2.0))
                    .color(Color::WHITE),
            );
            canvas.draw(
                &restart_text,
                DrawParam::default()
                    .dest(Vec2::new(SCREEN_WIDTH / 2.0 - 80.0, SCREEN_HEIGHT / 2.0 + 50.0))
                    .color(Color::WHITE),
            );
        } else {
            // Draw player (with blinking if invincible)
            let player_mesh = Mesh::new_rectangle(
                ctx,
                DrawMode::fill(),
                Rect::new(
                    -PLAYER_SIZE / 2.0,
                    -PLAYER_SIZE / 2.0,
                    PLAYER_SIZE,
                    PLAYER_SIZE,
                ),
                if self.invincible_timer > 0.0 && ((self.invincible_timer * 10.0) as i32) % 2 == 0 {
                    Color::WHITE // Blink: invisible every other frame
                } else {
                    Color::BLUE
                },
            )?;
            if !(self.invincible_timer > 0.0 && ((self.invincible_timer * 10.0) as i32) % 2 == 0) {
                canvas.draw(
                    &player_mesh,
                    DrawParam::default().dest(self.player.position),
                );
            }

            // Draw bullets
            for bullet in &self.bullets {
                if bullet.active {
                    let bullet_mesh = Mesh::new_circle(
                        ctx,
                        DrawMode::fill(),
                        bullet.position,
                        BULLET_SIZE / 2.0,
                        0.1,
                        Color::YELLOW,
                    )?;
                    canvas.draw(&bullet_mesh, DrawParam::default());
                }
            }

            // Draw enemies
            for enemy in &self.enemies {
                if enemy.active {
                    let (color, size) = match enemy.enemy_type {
                        Some(EnemyType::Normal) | None => (Color::RED, ENEMY_SIZE),
                        Some(EnemyType::Fast) => (Color::GREEN, FAST_ENEMY_SIZE),
                        Some(EnemyType::Big) => (Color::MAGENTA, BIG_ENEMY_SIZE),
                    };
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
                        DrawParam::default().dest(enemy.position),
                    );
                }
            }

            // Draw explosions
            for explosion in &self.explosions {
                let t = explosion.timer / EXPLOSION_DURATION;
                let radius = 10.0 + 30.0 * t;
                let alpha = 1.0 - t;
                let color = Color::new(1.0, 0.8, 0.2, alpha as f32);
                let mesh = Mesh::new_circle(
                    ctx,
                    DrawMode::stroke(3.0),
                    explosion.position,
                    radius,
                    0.2,
                    color,
                )?;
                canvas.draw(&mesh, DrawParam::default());
            }

            // Draw score
            let score_text = Text::new(format!("Score: {}", self.score));
            canvas.draw(
                &score_text,
                DrawParam::default()
                    .dest(Vec2::new(10.0, 10.0))
                    .color(Color::WHITE),
            );
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
                    self.shoot();
                }
            }
            Some(KeyCode::R) => {
                if self.game_over {
                    *self = GameState::new();
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
