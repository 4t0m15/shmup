use ggez::{glam::Vec2, graphics::{self, Color, DrawMode, DrawParam, Mesh, Rect}};
use crate::constants::*;

#[derive(Clone, Copy, PartialEq)]
pub enum EnemyType {
    Normal,
    Fast,
    Big,
}

#[derive(Clone)]
pub struct Enemy {
    pub position: Vec2,
    pub velocity: Vec2,
    pub size: f32,
    pub active: bool,
    pub enemy_type: EnemyType,
}

impl Enemy {
    pub fn new(position: Vec2, velocity: Vec2, size: f32, enemy_type: EnemyType) -> Self {
        Self {
            position,
            velocity,
            size,
            active: true,
            enemy_type,
        }
    }

    pub fn update(&mut self, dt: f32) {
        self.position += self.velocity * dt;
    }

    pub fn get_bounds(&self) -> Rect {
        Rect::new(
            self.position.x - self.size / 2.0,
            self.position.y - self.size / 2.0,
            self.size,
            self.size,
        )
    }

    pub fn collides_with(&self, other: &Enemy) -> bool {
        self.get_bounds().overlaps(&other.get_bounds())
    }

    pub fn draw(&self, ctx: &mut ggez::Context, canvas: &mut graphics::Canvas) -> ggez::GameResult {
        if !self.active {
            return Ok(());
        }

        let (color, size) = match self.enemy_type {
            EnemyType::Normal => (Color::RED, ENEMY_SIZE),
            EnemyType::Fast => (Color::GREEN, FAST_ENEMY_SIZE),
            EnemyType::Big => (Color::MAGENTA, BIG_ENEMY_SIZE),
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

        Ok(())
    }
} 