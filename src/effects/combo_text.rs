use ggez::{glam::Vec2, graphics::Color};

pub struct ComboText {
    pub position: Vec2,
    pub velocity: Vec2,
    pub life: f32,
    pub max_life: f32,
    pub text: String,
    pub color: Color,
}

impl ComboText {
    pub fn new(position: Vec2, text: String) -> Self {
        Self {
            position,
            velocity: Vec2::new(0.0, -50.0), // Float upward
            life: 1.5,
            max_life: 1.5,
            text,
            color: Color::new(1.0, 0.8, 0.0, 1.0), // Orange
        }
    }

    pub fn update(&mut self, dt: f32) {
        self.position += self.velocity * dt;
        self.life -= dt;
        self.velocity.y -= 20.0 * dt; // Slow down upward movement
    }

    pub fn is_dead(&self) -> bool {
        self.life <= 0.0
    }

    pub fn get_alpha(&self) -> f32 {
        self.life / self.max_life
    }
} 