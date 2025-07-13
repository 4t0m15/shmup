use ggez::{glam::Vec2, graphics::{self, Color, DrawMode, DrawParam, Mesh, Rect}};
use crate::constants::*;

#[derive(Clone, Copy, PartialEq, Debug)]
pub enum BossType {
    Destroyer,
    Carrier,
    Behemoth,
}

#[derive(Clone, Debug)]
pub struct Boss {
    pub position: Vec2,
    pub velocity: Vec2,
    pub size: f32,
    pub health: f32,
    pub max_health: f32,
    pub boss_type: BossType,
    pub phase: u32,
    pub attack_timer: f32,
    pub attack_pattern: f32,
    pub movement_timer: f32,
    pub active: bool,
}

impl Boss {
    pub fn new(boss_type: BossType) -> Self {
        let scaling = get_scaling();
        let (size, health) = match boss_type {
            BossType::Destroyer => (60.0, 300.0),
            BossType::Carrier => (80.0, 500.0),
            BossType::Behemoth => (100.0, 800.0),
        };
        
        Self {
            position: Vec2::new(get_screen_width() / 2.0, -scaling.scale_size(size)),
            velocity: Vec2::new(0.0, scaling.scale_speed(50.0)),
            size: scaling.scale_size(size),
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

    pub fn update(&mut self, dt: f32) {
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
                    self.velocity.x = if self.position.x < get_screen_width() / 2.0 { 100.0 } else { -100.0 };
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
        self.position.x = self.position.x.clamp(self.size / 2.0, get_screen_width() - self.size / 2.0);
    }

    pub fn take_damage(&mut self, damage: f32) {
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

    pub fn get_bullet_spawn_points(&self) -> Vec<Vec2> {
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

    pub fn get_color(&self) -> Color {
        let _health_percent = self.health / self.max_health;
        match self.boss_type {
            BossType::Destroyer => Color::new(1.0, 0.2, 0.2, 1.0),
            BossType::Carrier => Color::new(0.8, 0.2, 0.8, 1.0),
            BossType::Behemoth => Color::new(0.2, 0.2, 1.0, 1.0),
        }
    }

    pub fn draw(&self, ctx: &mut ggez::Context, canvas: &mut graphics::Canvas) -> ggez::GameResult {
        if !self.active {
            return Ok(());
        }

        let boss_color = self.get_color();
        
        // Boss glow effect
        let glow_mesh = Mesh::new_circle(
            ctx,
            DrawMode::fill(),
            self.position,
            self.size * 1.2,
            0.2,
            Color::new(boss_color.r, boss_color.g, boss_color.b, 0.3),
        )?;
        canvas.draw(&glow_mesh, DrawParam::default());
        
        // Main boss body
        let boss_mesh = Mesh::new_rectangle(
            ctx,
            DrawMode::fill(),
            Rect::new(
                -self.size / 2.0,
                -self.size / 2.0,
                self.size,
                self.size,
            ),
            boss_color,
        )?;
        canvas.draw(
            &boss_mesh,
            DrawParam::default().dest(self.position),
        );
        
        // Boss health bar
        let health_percent = self.health / self.max_health;
        let bar_width = self.size;
        let bar_height = 8.0;
        let bar_x = self.position.x - bar_width / 2.0;
        let bar_y = self.position.y - self.size / 2.0 - 20.0;
        
        // Background bar
        let bg_bar = Mesh::new_rectangle(
            ctx,
            DrawMode::fill(),
            Rect::new(bar_x, bar_y, bar_width, bar_height),
            Color::new(0.3, 0.3, 0.3, 0.8),
        )?;
        canvas.draw(&bg_bar, DrawParam::default());
        
        // Health bar
        let health_bar = Mesh::new_rectangle(
            ctx,
            DrawMode::fill(),
            Rect::new(bar_x, bar_y, bar_width * health_percent, bar_height),
            Color::new(1.0 - health_percent, health_percent, 0.0, 1.0),
        )?;
        canvas.draw(&health_bar, DrawParam::default());

        Ok(())
    }
} 