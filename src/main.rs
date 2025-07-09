use bevy::prelude::*;

mod components;
mod systems;
pub mod resources;

use components::player::Player;
use components::player::Velocity;
use resources::game_state::GameStats;
use systems::background::*;
use systems::player::*;
use systems::bullet::*;
use systems::enemy::*;
use systems::hud::*;
use systems::boss::*;
use systems::combat::*;

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
        .insert_resource(PlayerShootTimer(Timer::from_seconds(0.15, TimerMode::Repeating)))
        .insert_resource(GameStats { score: 0, lives: 3 })
        .insert_resource(BossFireTimer(Timer::from_seconds(1.2, TimerMode::Repeating)))
        .add_systems(Startup, (setup_camera, spawn_background, spawn_player, setup_hud, setup_boss_health_bar))
        .add_systems(Update, (
            scroll_background,
            player_movement,
            player_shooting,
            bullet_movement,
            enemy_movement,
            enemy_wave_spawner,
            bullet_enemy_collision,
            enemy_player_collision,
            boss_spawner,
            boss_movement,
            boss_fire_system,
            boss_bullet_movement,
            bullet_boss_collision,
            boss_bullet_player_collision,
            update_hud,
            update_boss_health_bar,
        ))
        .run();
}

fn setup_camera(mut commands: Commands) {
    commands.spawn(Camera2dBundle::default());
}

fn spawn_player(mut commands: Commands) {
    commands
        .spawn(SpriteBundle {
            sprite: Sprite {
                color: Color::srgb(0.7, 0.7, 1.0),
                ..Default::default()
            },
            ..Default::default()
        })
        .insert(Player)
        .insert(Velocity { x: 0.0, y: 0.0 });
}

fn scroll_background(time: Res<Time>, mut query: Query<(&Player, &mut Transform)>) {
    for (_, mut transform) in query.iter_mut() {
        transform.translation.x -= 100.0 * time.delta_secs();
        if transform.translation.x < -400.0 {
            transform.translation.x = 400.0;
        }
    }
}

fn player_movement(
    keyboard_input: Res<Input<KeyCode>>,
    mut query: Query<(&Player, &mut Velocity)>,
) {
    for (_, mut velocity) in query.iter_mut() {
        velocity.x = 0.0;
        velocity.y = 0.0;

        if keyboard_input.pressed(KeyCode::ArrowLeft) {
            velocity.x -= 1.0;
        }
        if keyboard_input.pressed(KeyCode::ArrowRight) {
            velocity.x += 1.0;
        }
        if keyboard_input.pressed(KeyCode::ArrowUp) {
            velocity.y += 1.0;
        }
        if keyboard_input.pressed(KeyCode::ArrowDown) {
            velocity.y -= 1.0;
        }
    }
}
