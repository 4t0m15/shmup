use bevy::prelude::*;

use crate::components::boss::{Boss, BossHealth};
use crate::resources::game_state::GameStats;

const BOSS_ENTRY_SCORE: u32 = 2000;
const BOSS_HP: u32 = 40;
const BOSS_SPEED: f32 = 60.0;
const BOSS_BULLET_SPEED: f32 = 180.0;
const BOSS_FIRE_INTERVAL: f32 = 1.2;

#[derive(Resource, Default)]
pub struct BossFireTimer(pub Timer);

pub fn boss_spawner(
    mut commands: Commands,
    stats: Res<GameStats>,
    asset_server: Res<AssetServer>,
    boss_query: Query<(), With<Boss>>,
) {
    if stats.score >= BOSS_ENTRY_SCORE && boss_query.is_empty() {
        let texture = asset_server.load("sprites/boss.png");
        commands.spawn((
            SpriteBundle {
                texture,
                transform: Transform::from_xyz(0.0, 400.0, 2.0),
                ..default()
            },
            Boss,
            BossHealth { hp: BOSS_HP, max_hp: BOSS_HP },
        ));
    }
}

pub fn boss_movement(
    time: Res<Time>,
    mut query: Query<&mut Transform, With<Boss>>,
) {
    for mut transform in &mut query {
        if transform.translation.y > 200.0 {
            transform.translation.y -= BOSS_SPEED * time.delta_secs();
        }
        // Optionally: add horizontal movement or attack pattern here
    }
}

pub fn boss_fire_system(
    mut commands: Commands,
    time: Res<Time>,
    mut timer: ResMut<BossFireTimer>,
    asset_server: Res<AssetServer>,
    boss_query: Query<&Transform, With<Boss>>,
) {
    timer.0.tick(time.delta());
    if timer.0.finished() {
        if let Ok(boss_tf) = boss_query.get_single() {
            let bullet_texture = asset_server.load("sprites/boss_bullet.png");
            // Fire 3-way spread
            for angle in [-0.3_f32, 0.0, 0.3] {
                let mut dir = Vec2::new(angle.sin(), -1.0).normalize();
                let pos = boss_tf.translation + Vec3::new(0.0, -32.0, 0.0);
                commands.spawn((
                    SpriteBundle {
                        texture: bullet_texture.clone(),
                        transform: Transform::from_translation(pos),
                        ..default()
                    },
                    BossBullet { velocity: dir * BOSS_BULLET_SPEED },
                ));
            }
        }
        timer.0.reset();
    }
}

#[derive(Component)]
pub struct BossBullet {
    pub velocity: Vec2,
}

pub fn boss_bullet_movement(
    time: Res<Time>,
    mut commands: Commands,
    mut query: Query<(Entity, &mut Transform, &BossBullet)>,
) {
    for (entity, mut transform, bullet) in &mut query {
        transform.translation.x += bullet.velocity.x * time.delta_secs();
        transform.translation.y += bullet.velocity.y * time.delta_secs();
        if transform.translation.y < -400.0 || transform.translation.x.abs() > 300.0 {
            commands.entity(entity).despawn();
        }
    }
}
