use ggez::{glam::Vec2, graphics::Color};

pub struct Particle {
    pub position: Vec2,
    pub velocity: Vec2,
    pub life: f32,
    pub max_life: f32,
    pub color: Color,
    pub size: f32,
}

impl Particle {
    pub fn new(position: Vec2, velocity: Vec2, life: f32, color: Color, size: f32) -> Self {
        Self {
            position,
            velocity,
            life,
            max_life: life,
            color,
            size,
        }
    }

    pub fn update(&mut self, dt: f32) {
        self.position += self.velocity * dt;
        self.velocity *= 0.98; // Air resistance
        self.life -= dt;
    }

    pub fn is_dead(&self) -> bool {
        self.life <= 0.0
    }

    pub fn get_alpha(&self) -> f32 {
        self.life / self.max_life
    }
} 