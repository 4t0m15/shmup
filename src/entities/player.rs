use ggez::{glam::Vec2, graphics::{self, Color, DrawMode, DrawParam, Mesh, Rect}};
use crate::constants::*;

#[derive(Clone)]
pub struct Player {
    pub position: Vec2,
    pub velocity: Vec2,
    pub size: f32,
    pub active: bool,
}

impl Player {
    pub fn new(position: Vec2) -> Self {
        Self {
            position,
            velocity: Vec2::ZERO,
            size: PLAYER_SIZE,
            active: true,
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

    pub fn collides_with(&self, other: &Player) -> bool {
        self.get_bounds().overlaps(&other.get_bounds())
    }

    pub fn draw(&self, ctx: &mut ggez::Context, canvas: &mut graphics::Canvas, 
                invincible_timer: f32, shield_active: bool) -> ggez::GameResult {
        if !self.active {
            return Ok(());
        }

        // Draw player (with blinking if invincible)
        let player_color = if invincible_timer > 0.0 && ((invincible_timer * 10.0) as i32) % 2 == 0 {
            Color::WHITE // Blink: invisible every other frame
        } else if shield_active {
            Color::new(0.2, 0.5, 1.0, 0.8) // Shield color
        } else {
            Color::BLUE
        };
        
        let player_mesh = Mesh::new_rectangle(
            ctx,
            DrawMode::fill(),
            Rect::new(
                -self.size / 2.0,
                -self.size / 2.0,
                self.size,
                self.size,
            ),
            player_color,
        )?;
        
        if !(invincible_timer > 0.0 && ((invincible_timer * 10.0) as i32) % 2 == 0) {
            canvas.draw(
                &player_mesh,
                DrawParam::default().dest(self.position),
            );
        }

        // Draw shield effect
        if shield_active {
            let shield_mesh = Mesh::new_circle(
                ctx,
                DrawMode::stroke(3.0),
                self.position,
                self.size + 10.0,
                0.2,
                Color::new(0.2, 0.5, 1.0, 0.5),
            )?;
            canvas.draw(&shield_mesh, DrawParam::default());
        }

        Ok(())
    }
} 