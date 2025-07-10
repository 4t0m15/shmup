use ggez::{glam::Vec2, graphics::{self, Color, DrawMode, DrawParam, Mesh, Rect}};
use crate::constants::*;
use rand::Rng;

#[derive(Clone, Copy, PartialEq)]
pub enum EnemyType {
    Normal,
    Fast,
    Big,
    Zenith,
}

#[derive(Clone, Copy, PartialEq)]
pub enum ZenithState {
    Idle,
    Charging,
    Firing,
    Cooldown,
}

#[derive(Clone)]
pub struct Enemy {
    pub position: Vec2,
    pub velocity: Vec2,
    pub size: f32,
    pub active: bool,
    pub enemy_type: EnemyType,
    // Zenith-specific fields
    pub zenith_state: ZenithState,
    pub zenith_timer: f32,
    pub zenith_direction: f32, // For zig-zag movement
    pub zenith_beam_active: bool,
    pub zenith_beam_target: Option<Vec2>, // Player position when beam hits
}

impl Enemy {
    pub fn new(position: Vec2, velocity: Vec2, size: f32, enemy_type: EnemyType) -> Self {
        let mut rng = rand::thread_rng();
        Self {
            position,
            velocity,
            size,
            active: true,
            enemy_type,
            zenith_state: ZenithState::Idle,
            zenith_timer: rng.gen_range(2.0..4.0), // Random idle time
            zenith_direction: rng.gen_range(-1.0..1.0),
            zenith_beam_active: false,
            zenith_beam_target: None,
        }
    }

    pub fn update(&mut self, dt: f32) {
        if self.enemy_type == EnemyType::Zenith {
            self.update_zenith(dt);
        } else {
            self.position += self.velocity * dt;
        }
    }

    fn update_zenith(&mut self, dt: f32) {
        self.zenith_timer -= dt;
        
        match self.zenith_state {
            ZenithState::Idle => {
                // Zig-zag movement
                self.zenith_direction += dt * 2.0; // Change direction over time
                let horizontal_speed = ZENITH_SPEED * 0.5;
                self.velocity.x = self.zenith_direction.sin() * horizontal_speed;
                self.velocity.y = ZENITH_SPEED * 0.3; // Slow downward movement
                self.position += self.velocity * dt;
                
                // Transition to charging
                if self.zenith_timer <= 0.0 {
                    self.zenith_state = ZenithState::Charging;
                    self.zenith_timer = ZENITH_BEAM_CHARGE;
                    self.velocity = Vec2::ZERO; // Stop moving
                }
            }
            ZenithState::Charging => {
                // Stay in place, timer counts down
                if self.zenith_timer <= 0.0 {
                    self.zenith_state = ZenithState::Firing;
                    self.zenith_timer = ZENITH_BEAM_DURATION;
                    self.zenith_beam_active = true;
                }
            }
            ZenithState::Firing => {
                // Beam is active, timer counts down
                if self.zenith_timer <= 0.0 {
                    self.zenith_state = ZenithState::Cooldown;
                    self.zenith_timer = ZENITH_COOLDOWN;
                    self.zenith_beam_active = false;
                    self.zenith_beam_target = None;
                }
            }
            ZenithState::Cooldown => {
                // Resume normal movement
                self.zenith_direction += dt * 2.0;
                let horizontal_speed = ZENITH_SPEED * 0.5;
                self.velocity.x = self.zenith_direction.sin() * horizontal_speed;
                self.velocity.y = ZENITH_SPEED * 0.3;
                self.position += self.velocity * dt;
                
                if self.zenith_timer <= 0.0 {
                    self.zenith_state = ZenithState::Idle;
                    let mut rng = rand::thread_rng();
                    self.zenith_timer = rng.gen_range(2.0..4.0);
                }
            }
        }
    }

    pub fn is_zenith_beam_active(&self) -> bool {
        self.enemy_type == EnemyType::Zenith && self.zenith_beam_active
    }

    pub fn get_zenith_beam_bounds(&self) -> Rect {
        if self.enemy_type != EnemyType::Zenith || !self.zenith_beam_active {
            return Rect::new(0.0, 0.0, 0.0, 0.0);
        }
        
        // Beam extends from zenith to bottom of screen
        Rect::new(
            self.position.x - ZENITH_BEAM_WIDTH / 2.0,
            self.position.y,
            ZENITH_BEAM_WIDTH,
            SCREEN_HEIGHT - self.position.y,
        )
    }

    pub fn draw(&self, ctx: &mut ggez::Context, canvas: &mut graphics::Canvas) -> ggez::GameResult {
        if !self.active {
            return Ok(());
        }

        let (color, size) = match self.enemy_type {
            EnemyType::Normal => (Color::RED, ENEMY_SIZE),
            EnemyType::Fast => (Color::GREEN, FAST_ENEMY_SIZE),
            EnemyType::Big => (Color::MAGENTA, BIG_ENEMY_SIZE),
            EnemyType::Zenith => (Color::WHITE, ZENITH_SIZE), // White Zenith
        };
        
        // Enemy glow
        let glow_mesh = Mesh::new_circle(
            ctx,
            DrawMode::fill(),
            self.position,
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
            DrawParam::default().dest(self.position),
        );

        // Draw Zenith-specific effects
        if self.enemy_type == EnemyType::Zenith {
            self.draw_zenith_effects(ctx, canvas)?;
        }

        Ok(())
    }

    fn draw_zenith_effects(&self, ctx: &mut ggez::Context, canvas: &mut graphics::Canvas) -> ggez::GameResult {
        match self.zenith_state {
            ZenithState::Charging => {
                // Flickering warning effect
                let time = std::time::SystemTime::now()
                    .duration_since(std::time::UNIX_EPOCH)
                    .unwrap()
                    .as_secs_f32();
                let flicker = (time * 8.0).sin() * 0.5 + 0.5;
                
                // Warning beam preview
                let warning_color = Color::new(1.0, 0.0, 0.0, 0.3 * flicker);
                let warning_rect = Rect::new(
                    self.position.x - ZENITH_BEAM_WIDTH / 2.0,
                    self.position.y,
                    ZENITH_BEAM_WIDTH,
                    SCREEN_HEIGHT - self.position.y,
                );
                let warning_mesh = Mesh::new_rectangle(
                    ctx,
                    DrawMode::fill(),
                    warning_rect,
                    warning_color,
                )?;
                canvas.draw(&warning_mesh, DrawParam::default());
                
                // Zenith sprite flash
                let flash_color = Color::new(1.0, 1.0, 1.0, flicker);
                let flash_mesh = Mesh::new_rectangle(
                    ctx,
                    DrawMode::fill(),
                    Rect::new(
                        -self.size / 2.0,
                        -self.size / 2.0,
                        self.size,
                        self.size,
                    ),
                    flash_color,
                )?;
                canvas.draw(&flash_mesh, DrawParam::default().dest(self.position));
            }
            ZenithState::Firing => {
                // Active beam
                let beam_color = Color::new(1.0, 0.0, 0.0, 0.8);
                let beam_rect = Rect::new(
                    self.position.x - ZENITH_BEAM_WIDTH / 2.0,
                    self.position.y,
                    ZENITH_BEAM_WIDTH,
                    SCREEN_HEIGHT - self.position.y,
                );
                let beam_mesh = Mesh::new_rectangle(
                    ctx,
                    DrawMode::fill(),
                    beam_rect,
                    beam_color,
                )?;
                canvas.draw(&beam_mesh, DrawParam::default());
                
                // Beam glow effect
                let glow_color = Color::new(1.0, 0.5, 0.5, 0.4);
                let glow_rect = Rect::new(
                    self.position.x - ZENITH_BEAM_WIDTH,
                    self.position.y,
                    ZENITH_BEAM_WIDTH * 2.0,
                    SCREEN_HEIGHT - self.position.y,
                );
                let glow_mesh = Mesh::new_rectangle(
                    ctx,
                    DrawMode::fill(),
                    glow_rect,
                    glow_color,
                )?;
                canvas.draw(&glow_mesh, DrawParam::default());
            }
            _ => {}
        }
        
        Ok(())
    }
} 