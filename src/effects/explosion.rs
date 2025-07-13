use ggez::glam::Vec2;
use rand::Rng;
use crate::constants::*;
use super::particle::Particle;

#[derive(Debug)]
pub struct Explosion {
    pub timer: f32,
    pub particles: Vec<Particle>,
}

impl Explosion {
    pub fn new(position: Vec2) -> Self {
        let mut particles = Vec::new();
        let mut rng = rand::thread_rng();
        
        for _ in 0..PARTICLE_COUNT {
            let angle = rng.gen_range(0.0..std::f32::consts::PI * 2.0);
            let speed = rng.gen_range(50.0..200.0);
            let velocity = Vec2::new(angle.cos() * speed, angle.sin() * speed);
            let life = rng.gen_range(0.5..1.5);
            let color = match rng.gen_range(0..3) {
                0 => ggez::graphics::Color::new(1.0, 0.8, 0.2, 1.0), // Yellow
                1 => ggez::graphics::Color::new(1.0, 0.4, 0.0, 1.0), // Orange
                _ => ggez::graphics::Color::new(1.0, 0.2, 0.0, 1.0), // Red
            };
            let size = rng.gen_range(2.0..6.0);
            
            particles.push(Particle::new(position, velocity, life, color, size));
        }
        
        Self { timer: 0.0, particles }
    }

    pub fn update(&mut self, dt: f32) {
        self.timer += dt;
        for particle in &mut self.particles {
            particle.update(dt);
        }
    }

    pub fn is_done(&self) -> bool {
        self.timer > EXPLOSION_DURATION
    }
} 