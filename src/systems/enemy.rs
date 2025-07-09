use bevy::prelude::*;
use bevy::sprite::SpriteBundle;
use bevy::input::ButtonInput;
use crate::components::enemy::Enemy;

const ENEMY_SPEED: f32 = 80.0;

pub fn spawn_enemy(commands: &mut Commands, position: Vec3, texture: Handle<Image>) {
    commands.spawn((
        SpriteBundle {
            texture,
            transform: Transform::from_translation(position),
            ..default()
        },
        Enemy,
    ));
}

pub fn enemy_movement(
    time: Res<Time>,
    mut commands: Commands,
    mut query: Query<(Entity, &mut Transform), With<Enemy>>,
) {
    for (entity, mut transform) in &mut query {
        transform.translation.y -= ENEMY_SPEED * time.delta_secs();
        if transform.translation.y < -400.0 {
            commands.entity(entity).despawn();
        }
    }
}

pub fn enemy_wave_spawner(
    mut commands: Commands,
    time: Res<Time>,
    asset_server: Res<AssetServer>,
    mut timer: Local<Timer>,
) {
    if timer.tick(time.delta()).just_finished() {
        let texture = asset_server.load("sprites/enemies.png");
        for i in 0..5 {
            let x = -160.0 + i as f32 * 80.0;
            let y = 360.0;
            spawn_enemy(&mut commands, Vec3::new(x, y, 1.0), texture.clone());
        }
    }
}
