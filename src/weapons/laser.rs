use ggez::{glam::Vec2, graphics::{self, Color, DrawMode, DrawParam, Mesh, Rect}};
use rand::Rng;
use crate::constants::*;
use crate::effects::particle::Particle;

#[derive(Debug)]
pub struct Laser {
    pub is_charging: bool,
    pub charge_time: f32,
    pub is_firing: bool,
    pub fire_duration: f32,
    pub max_fire_duration: f32,
    pub damage: f32,
    pub width: f32,
    pub color: Color,
    pub position: Vec2,
    pub charge_particles: Vec<Particle>,
}

impl Laser {
    pub fn new() -> Self {
        Self {
            is_charging: false,
            charge_time: 0.0,
            is_firing: false,
            fire_duration: 0.0,
            max_fire_duration: 0.5, // How long laser can fire
            damage: LASER_DAMAGE_BASE,
            width: LASER_WIDTH_BASE,
            color: LASER_COLOR_BASE,
            position: Vec2::ZERO,
            charge_particles: Vec::new(),
        }
    }

    pub fn start_charging(&mut self, position: Vec2) {
        self.is_charging = true;
        self.charge_time = 0.0;
        self.position = position;
        self.charge_particles.clear();
    }

    pub fn update_charging(&mut self, dt: f32) {
        if !self.is_charging {
            return;
        }

        self.charge_time += dt;
        
        // Update charge progress
        let charge_progress = (self.charge_time / LASER_CHARGE_TIME).min(1.0);
        self.damage = LASER_DAMAGE_BASE + (LASER_DAMAGE_MAX - LASER_DAMAGE_BASE) * charge_progress;
        self.width = LASER_WIDTH_BASE + (LASER_WIDTH_MAX - LASER_WIDTH_BASE) * charge_progress;
        
        // Update color based on charge
        let color_progress = charge_progress;
        self.color = Color::new(
            LASER_COLOR_BASE.r + (LASER_COLOR_CHARGED.r - LASER_COLOR_BASE.r) * color_progress,
            LASER_COLOR_BASE.g + (LASER_COLOR_CHARGED.g - LASER_COLOR_BASE.g) * color_progress,
            LASER_COLOR_BASE.b + (LASER_COLOR_CHARGED.b - LASER_COLOR_BASE.b) * color_progress,
            1.0,
        );

        // Create charge particles
        if self.charge_time > 0.1 {
            let mut rng = rand::thread_rng();
            for _ in 0..2 {
                let angle = rng.gen_range(0.0..std::f32::consts::PI * 2.0);
                let distance = rng.gen_range(20.0..40.0);
                let particle_pos = self.position + Vec2::new(angle.cos() * distance, angle.sin() * distance);
                let velocity = (self.position - particle_pos).normalize() * rng.gen_range(50.0..150.0);
                let life = rng.gen_range(0.3..0.8);
                let size = rng.gen_range(2.0..6.0);
                
                self.charge_particles.push(Particle::new(
                    particle_pos,
                    velocity,
                    life,
                    self.color,
                    size,
                ));
            }
        }

        // Update charge particles
        for particle in &mut self.charge_particles {
            particle.update(dt);
        }
        self.charge_particles.retain(|p| !p.is_dead());
    }

    pub fn fire(&mut self) {
        if self.is_charging && self.charge_time >= 0.2 { // Minimum charge time
            self.is_charging = false;
            self.is_firing = true;
            self.fire_duration = 0.0;
        }
    }

    pub fn update_position(&mut self, player_position: Vec2) {
        self.position = player_position;
    }

    pub fn update_firing(&mut self, dt: f32) {
        if !self.is_firing {
            return;
        }

        self.fire_duration += dt;
        if self.fire_duration >= self.max_fire_duration {
            self.is_firing = false;
            self.fire_duration = 0.0;
        }
    }

    pub fn get_laser_bounds(&self) -> Rect {
        if !self.is_firing {
            return Rect::new(0.0, 0.0, 0.0, 0.0);
        }
        
        // Laser extends from player position to top of screen
        Rect::new(
            self.position.x - self.width / 2.0,
            0.0,
            self.width,
            self.position.y,
        )
    }

    pub fn get_charge_progress(&self) -> f32 {
        (self.charge_time / LASER_CHARGE_TIME).min(1.0)
    }

    pub fn draw(&self, ctx: &mut ggez::Context, canvas: &mut graphics::Canvas) -> ggez::GameResult {
        // Draw laser charging particles
        if self.is_charging {
            for particle in &self.charge_particles {
                if !particle.is_dead() {
                    let color = Color::new(
                        particle.color.r,
                        particle.color.g,
                        particle.color.b,
                        particle.get_alpha(),
                    );
                    let mesh = Mesh::new_circle(
                        ctx,
                        DrawMode::fill(),
                        particle.position,
                        particle.size,
                        0.1,
                        color,
                    )?;
                    canvas.draw(&mesh, DrawParam::default());
                }
            }
        }

        // Draw laser beam
        if self.is_firing {
            let laser_bounds = self.get_laser_bounds();
            
            // Laser glow effect
            let glow_mesh = Mesh::new_rectangle(
                ctx,
                DrawMode::fill(),
                Rect::new(
                    laser_bounds.x - 5.0,
                    laser_bounds.y,
                    laser_bounds.w + 10.0,
                    laser_bounds.h,
                ),
                Color::new(
                    self.color.r,
                    self.color.g,
                    self.color.b,
                    0.3,
                ),
            )?;
            canvas.draw(&glow_mesh, DrawParam::default());
            
            // Main laser beam
            let laser_mesh = Mesh::new_rectangle(
                ctx,
                DrawMode::fill(),
                Rect::new(
                    laser_bounds.x,
                    laser_bounds.y,
                    laser_bounds.w,
                    laser_bounds.h,
                ),
                self.color,
            )?;
            canvas.draw(&laser_mesh, DrawParam::default());
            
            // Laser core (brighter center)
            let core_width = self.width * 0.3;
            let core_mesh = Mesh::new_rectangle(
                ctx,
                DrawMode::fill(),
                Rect::new(
                    laser_bounds.x + (laser_bounds.w - core_width) / 2.0,
                    laser_bounds.y,
                    core_width,
                    laser_bounds.h,
                ),
                Color::new(1.0, 1.0, 1.0, 0.8),
            )?;
            canvas.draw(&core_mesh, DrawParam::default());
        }
        
        Ok(())
    }
} 