use bevy::prelude::*;
use bevy::sprite::SpriteBundle;
use bevy::input::ButtonInput;
use crate::components::bullet::Bullet;

const BULLET_SPEED: f32 = 400.0;

pub fn spawn_bullet(commands: &mut Commands, position: Vec3, texture: Handle<Image>) {
    commands.spawn((
        SpriteBundle {
            texture,
            transform: Transform::from_translation(position),
            ..default()
        },
        Bullet,
    ));
}

pub fn bullet_movement(
    time: Res<Time>,
    mut commands: Commands,
    mut query: Query<(Entity, &mut Transform), With<Bullet>>,
) {
    for (entity, mut transform) in &mut query {
        transform.translation.y += BULLET_SPEED * time.delta_secs();
        if transform.translation.y > 400.0 {
            commands.entity(entity).despawn();
        }
    }
}
