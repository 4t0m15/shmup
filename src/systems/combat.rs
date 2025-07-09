use bevy::prelude::*;
use crate::components::{bullet::Bullet, enemy::Enemy, player::Player};
use crate::components::boss::{Boss, BossHealth};
use crate::resources::game_state::GameStats;
use crate::systems::boss::BossBullet;

pub fn bullet_enemy_collision(
    mut commands: Commands,
    mut stats: ResMut<GameStats>,
    bullet_query: Query<(Entity, &Transform), With<Bullet>>,
    enemy_query: Query<(Entity, &Transform), With<Enemy>>,
) {
    for (bullet_entity, bullet_tf) in &bullet_query {
        for (enemy_entity, enemy_tf) in &enemy_query {
            let distance = bullet_tf.translation.truncate().distance(enemy_tf.translation.truncate());
            if distance < 20.0 {
                commands.entity(bullet_entity).despawn();
                commands.entity(enemy_entity).despawn();
                stats.score += 100;
                break;
            }
        }
    }
}

pub fn enemy_player_collision(
    mut commands: Commands,
    mut stats: ResMut<GameStats>,
    enemy_query: Query<(Entity, &Transform), With<Enemy>>,
    player_query: Query<(Entity, &Transform), With<Player>>,
) {
    if let Ok((player_entity, player_tf)) = player_query.get_single() {
        for (enemy_entity, enemy_tf) in &enemy_query {
            let distance = player_tf.translation.truncate().distance(enemy_tf.translation.truncate());
            if distance < 28.0 {
                commands.entity(enemy_entity).despawn();
                stats.lives = stats.lives.saturating_sub(1);
                // Optionally: respawn player or handle game over here
            }
        }
    }
}

pub fn bullet_boss_collision(
    mut commands: Commands,
    mut boss_query: Query<(Entity, &mut BossHealth, &Transform), With<Boss>>,
    bullet_query: Query<(Entity, &Transform), With<Bullet>>,
    mut stats: ResMut<GameStats>,
) {
    if let Ok((boss_entity, mut boss_hp, boss_tf)) = boss_query.get_single_mut() {
        for (bullet_entity, bullet_tf) in &bullet_query {
            let distance = boss_tf.translation.truncate().distance(bullet_tf.translation.truncate());
            if distance < 32.0 {
                commands.entity(bullet_entity).despawn();
                if boss_hp.hp > 0 {
                    boss_hp.hp -= 1;
                }
                if boss_hp.hp == 0 {
                    commands.entity(boss_entity).despawn();
                    stats.score += 2000;
                    // TODO: spawn explosion effect
                }
                break;
            }
        }
    }
}

pub fn boss_bullet_player_collision(
    mut commands: Commands,
    mut stats: ResMut<GameStats>,
    boss_bullet_query: Query<(Entity, &Transform), With<BossBullet>>,
    player_query: Query<(Entity, &Transform), With<Player>>,
) {
    if let Ok((player_entity, player_tf)) = player_query.get_single() {
        for (bullet_entity, bullet_tf) in &boss_bullet_query {
            let distance = player_tf.translation.truncate().distance(bullet_tf.translation.truncate());
            if distance < 20.0 {
                commands.entity(bullet_entity).despawn();
                stats.lives = stats.lives.saturating_sub(1);
                // TODO: spawn explosion effect
            }
        }
    }
}
