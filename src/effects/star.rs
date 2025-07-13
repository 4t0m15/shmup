use ggez::glam::Vec2;
use crate::constants::*;
use rand::Rng;

#[derive(Clone, Debug)]
pub struct Star {
    pub position: Vec2,
    pub velocity: Vec2,
    pub brightness: f32,
    pub pulse_timer: f32,
}

impl Star {
    pub fn new(rng: &mut rand::rngs::ThreadRng) -> Self {
        let scaling = get_scaling();
        let x = rng.gen_range(0.0..get_screen_width());
        let y = rng.gen_range(0.0..get_screen_height());
        Self {
            position: Vec2::new(x, y),
            velocity: Vec2::new(0.0, scaling.scale_speed(STAR_SPEED)),
            brightness: rng.gen_range(0.3..1.0),
            pulse_timer: rng.gen_range(0.0..std::f32::consts::PI * 2.0),
        }
    }

    pub fn update(&mut self, dt: f32) {
        self.position += self.velocity * dt;
        self.pulse_timer += dt * 2.0;
        
        // Reset star when it goes off screen
        if self.position.y > get_screen_height() {
            self.position.y = -5.0;
            self.position.x = rand::thread_rng().gen_range(0.0..get_screen_width());
        }
    }
} 