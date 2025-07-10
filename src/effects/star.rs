use ggez::glam::Vec2;
use rand::Rng;
use crate::constants::*;

#[derive(Debug)]
pub struct Star {
    pub position: Vec2,
    pub speed: f32,
    pub brightness: f32,
    pub pulse_timer: f32,
}

impl Star {
    pub fn new(rng: &mut rand::rngs::ThreadRng) -> Self {
        let x = rng.gen_range(0.0..SCREEN_WIDTH);
        let y = rng.gen_range(0.0..SCREEN_HEIGHT);
        let speed = rng.gen_range(0.5..1.5) * STAR_SPEED;
        let brightness = rng.gen_range(0.3..1.0);
        let pulse_timer = rng.gen_range(0.0..std::f32::consts::PI * 2.0);
        Self {
            position: Vec2::new(x, y),
            speed,
            brightness,
            pulse_timer,
        }
    }

    pub fn update(&mut self, dt: f32) {
        self.position.y += self.speed * dt;
        self.pulse_timer += dt * 2.0;
        if self.position.y > SCREEN_HEIGHT {
            self.position.y = 0.0;
            self.position.x = rand::thread_rng().gen_range(0.0..SCREEN_WIDTH);
        }
    }
} 