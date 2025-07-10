use ggez::{glam::Vec2, graphics::{self, Color, DrawMode, DrawParam, Mesh, Rect}};
use crate::constants::get_scaling;

#[derive(Clone, Debug)]
pub struct Bullet {
    pub position: Vec2,
    pub velocity: Vec2,
    pub size: f32,
    pub active: bool,
}

impl Bullet {
    pub fn new(position: Vec2, velocity: Vec2, size: f32) -> Self {
        let scaling = get_scaling();
        Self {
            position,
            velocity: Vec2::new(velocity.x, scaling.scale_speed(velocity.y)),
            size: scaling.scale_size(size),
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

    pub fn collides_with(&self, other: &Bullet) -> bool {
        self.get_bounds().overlaps(&other.get_bounds())
    }

    pub fn draw(&self, ctx: &mut ggez::Context, canvas: &mut graphics::Canvas) -> ggez::GameResult {
        if !self.active {
            return Ok(());
        }

        // Bullet trail
        for i in 1..4 {
            let trail_pos = self.position + self.velocity * (i as f32 * -0.01);
            let trail_alpha = 0.3 / i as f32;
            let trail_mesh = Mesh::new_circle(
                ctx,
                DrawMode::fill(),
                trail_pos,
                self.size / 2.0 * (1.0 - i as f32 * 0.2),
                0.1,
                Color::new(1.0, 1.0, 0.0, trail_alpha),
            )?;
            canvas.draw(&trail_mesh, DrawParam::default());
        }
        
        // Main bullet
        let bullet_mesh = Mesh::new_circle(
            ctx,
            DrawMode::fill(),
            self.position,
            self.size / 2.0,
            0.1,
            Color::YELLOW,
        )?;
        canvas.draw(&bullet_mesh, DrawParam::default());
        
        Ok(())
    }
} 