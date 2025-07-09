use bevy::prelude::*;
use crate::components::player::{Player, Velocity};
use crate::systems::bullet::spawn_bullet;
use crate::components::bullet::Bullet;

const PLAYER_SPEED: f32 = 200.0;
const PLAYER_SHOOT_COOLDOWN: f32 = 0.15;

#[derive(Resource, Default)]
pub struct PlayerShootTimer(pub Timer);

pub fn spawn_player(mut commands: Commands, asset_server: Res<AssetServer>) {
    let texture_handle = asset_server.load("sprites/player.png");
    commands.spawn((
        SpriteBundle {
            texture: texture_handle,
            transform: Transform::from_xyz(0.0, -200.0, 1.0),
            ..default()
        },
        Player,
        Velocity { x: 0.0, y: 0.0 },
    ));
}

pub fn player_movement(
    keyboard_input: Res<Input<KeyCode>>,
    time: Res<Time>,
    mut query: Query<(&mut Transform, &mut Velocity), With<Player>>,
) {
    for (mut transform, mut velocity) in &mut query {
        let mut direction = Vec2::ZERO;
        if keyboard_input.pressed(KeyCode::ArrowLeft) || keyboard_input.pressed(KeyCode::A) {
            direction.x -= 1.0;
        }
        if keyboard_input.pressed(KeyCode::ArrowRight) || keyboard_input.pressed(KeyCode::D) {
            direction.x += 1.0;
        }
        if keyboard_input.pressed(KeyCode::ArrowUp) || keyboard_input.pressed(KeyCode::W) {
            direction.y += 1.0;
        }
        if keyboard_input.pressed(KeyCode::ArrowDown) || keyboard_input.pressed(KeyCode::S) {
            direction.y -= 1.0;
        }
        if direction.length_squared() > 0.0 {
            direction = direction.normalize();
        }
        velocity.x = direction.x * PLAYER_SPEED;
        velocity.y = direction.y * PLAYER_SPEED;
        transform.translation.x += velocity.x * time.delta_secs();
        transform.translation.y += velocity.y * time.delta_secs();
        // Clamp to screen bounds
        transform.translation.x = transform.translation.x.clamp(-220.0, 220.0);
        transform.translation.y = transform.translation.y.clamp(-300.0, 300.0);
    }
}

pub fn player_shooting(
    mut commands: Commands,
    keyboard_input: Res<Input<KeyCode>>,
    time: Res<Time>,
    mut timer: ResMut<PlayerShootTimer>,
    asset_server: Res<AssetServer>,
    query: Query<&Transform, With<Player>>,
) {
    timer.0.tick(time.delta());
    if (keyboard_input.pressed(KeyCode::Space) || keyboard_input.pressed(KeyCode::Z)) && timer.0.finished() {
        if let Ok(transform) = query.get_single() {
            let bullet_texture = asset_server.load("sprites/bullet.png");
            let pos = transform.translation + Vec3::new(0.0, 24.0, 0.0);
            spawn_bullet(&mut commands, pos, bullet_texture);
            timer.0.reset();
        }
    }
}

use bevy::prelude::*;
use systems::background::*;
use systems::player::*;

fn main() {
    App::new()
        .insert_resource(ClearColor(Color::srgb(0.2, 0.4, 0.8)))
        .add_plugins(DefaultPlugins.set(WindowPlugin {
            primary_window: Some(Window {
                title: "1942 Shooter".to_string(),
                resolution: (480., 640.).into(),
                ..default()
            }),
            ..default()
        }))
        .add_systems(Startup, (setup_camera, spawn_background, spawn_player))
        .add_systems(Update, (scroll_background, player_movement, player_shooting))
        .run();
}

fn setup_camera(mut commands: Commands) {
    commands.spawn(Camera2dBundle::default());
}
