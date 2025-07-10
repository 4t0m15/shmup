use ggez::{glam::Vec2, graphics::{self, Color, DrawMode, DrawParam, Mesh, Rect}};

#[derive(Clone, Copy, PartialEq, Debug)]
pub enum PowerUpType {
    RapidFire,
    TripleShot,
    Shield,
    Laser,
}

#[derive(Clone, Debug)]
pub struct PowerUp {
    pub position: Vec2,
    pub velocity: Vec2,
    pub power_type: PowerUpType,
    pub active: bool,
    pub pulse_timer: f32,
}

impl PowerUp {
    pub fn new(position: Vec2, power_type: PowerUpType) -> Self {
        Self {
            position,
            velocity: Vec2::new(0.0, 100.0),
            power_type,
            active: true,
            pulse_timer: 0.0,
        }
    }

    pub fn update(&mut self, dt: f32) {
        self.position += self.velocity * dt;
        self.pulse_timer += dt * 3.0;
        if self.position.y > crate::constants::SCREEN_HEIGHT + 20.0 {
            self.active = false;
        }
    }

    pub fn get_color(&self) -> Color {
        let pulse = (self.pulse_timer.sin() * 0.3 + 0.7) as f32;
        match self.power_type {
            PowerUpType::RapidFire => Color::new(0.2, 1.0, 0.2, pulse),
            PowerUpType::TripleShot => Color::new(1.0, 0.5, 0.0, pulse),
            PowerUpType::Shield => Color::new(0.2, 0.5, 1.0, pulse),
            PowerUpType::Laser => Color::new(0.8, 0.8, 0.2, pulse), // Laser color
        }
    }

    pub fn draw(&self, ctx: &mut ggez::Context, canvas: &mut graphics::Canvas) -> ggez::GameResult {
        if !self.active {
            return Ok(());
        }

        let color = self.get_color();
        let power_up_mesh = Mesh::new_rectangle(
            ctx,
            DrawMode::fill(),
            Rect::new(-15.0, -15.0, 30.0, 30.0),
            color,
        )?;
        canvas.draw(
            &power_up_mesh,
            DrawParam::default().dest(self.position),
        );

        Ok(())
    }
} 