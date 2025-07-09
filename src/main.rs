use bevy::prelude::*;
use rand::prelude::*;

const PLAYER_SPEED: f32 = 500.0;
const BULLET_SPEED: f32 = 700.0;
const ENEMY_SPEED: f32 = 150.0;
const ENEMY_SPAWN_TIME: f32 = 1.0;
const PLAYER_SIZE: Vec2 = Vec2::new(40.0, 40.0);
const ENEMY_SIZE: Vec2 = Vec2::new(40.0, 40.0);
const BULLET_SIZE: Vec2 = Vec2::new(8.0, 16.0);
const WINDOW_WIDTH: f32 = 480.0;
const WINDOW_HEIGHT: f32 = 640.0;

fn main() {
    App::new()
        .insert_resource(ClearColor(Color::rgb(0.1, 0.1, 0.2)))
        .add_plugins(DefaultPlugins.set(WindowPlugin {
            primary_window: Some(Window {
                resolution: (WINDOW_WIDTH, WINDOW_HEIGHT).into(),
                title: "Bevy 1942 Clone".to_string(),
                ..default()
            }),
            ..default()
        }))
        .add_systems(Startup, setup)
        .add_systems(Update, (
            player_movement,
            player_shooting,
            bullet_movement,
            enemy_spawner,
            enemy_movement,
            bullet_enemy_collision,
        ))
        .run();
}

#[derive(Component)]
struct Player;

#[derive(Component)]
struct Enemy;

#[derive(Component)]
struct Bullet;

#[derive(Resource)]
struct EnemySpawnTimer(Timer);

fn setup(mut commands: Commands) {
    // Camera
    commands.spawn(Camera2dBundle::default());
    // Player
    commands.spawn((
        SpriteBundle {
            sprite: Sprite {
                color: Color::rgb(0.3, 0.7, 1.0),
                custom_size: Some(PLAYER_SIZE),
                ..default()
            },
            transform: Transform::from_xyz(0.0, -WINDOW_HEIGHT / 2.0 + 60.0, 0.0),
            ..default()
        },
        Player,
    ));
    // Enemy spawn timer
    commands.insert_resource(EnemySpawnTimer(Timer::from_seconds(ENEMY_SPAWN_TIME, TimerMode::Repeating)));
}

fn player_movement(
    keyboard_input: Res<Input<KeyCode>>,
    mut query: Query<&mut Transform, With<Player>>,
    time: Res<Time>,
) {
    if let Ok(mut transform) = query.get_single_mut() {
        let mut direction = Vec3::ZERO;
        if keyboard_input.pressed(KeyCode::Left) || keyboard_input.pressed(KeyCode::A) {
            direction.x -= 1.0;
        }
        if keyboard_input.pressed(KeyCode::Right) || keyboard_input.pressed(KeyCode::D) {
            direction.x += 1.0;
        }
        if keyboard_input.pressed(KeyCode::Up) || keyboard_input.pressed(KeyCode::W) {
            direction.y += 1.0;
        }
        if keyboard_input.pressed(KeyCode::Down) || keyboard_input.pressed(KeyCode::S) {
            direction.y -= 1.0;
        }
        let new_pos = transform.translation + direction.normalize_or_zero() * PLAYER_SPEED * time.delta_seconds();
        // Clamp to window
        transform.translation.x = new_pos.x.clamp(-WINDOW_WIDTH / 2.0 + PLAYER_SIZE.x / 2.0, WINDOW_WIDTH / 2.0 - PLAYER_SIZE.x / 2.0);
        transform.translation.y = new_pos.y.clamp(-WINDOW_HEIGHT / 2.0 + PLAYER_SIZE.y / 2.0, WINDOW_HEIGHT / 2.0 - PLAYER_SIZE.y / 2.0);
    }
}

fn player_shooting(
    keyboard_input: Res<Input<KeyCode>>,
    mut commands: Commands,
    query: Query<&Transform, With<Player>>,
) {
    if keyboard_input.just_pressed(KeyCode::Space) {
        if let Ok(player_transform) = query.get_single() {
            commands.spawn((
                SpriteBundle {
                    sprite: Sprite {
                        color: Color::YELLOW,
                        custom_size: Some(BULLET_SIZE),
                        ..default()
                    },
                    transform: Transform::from_translation(player_transform.translation + Vec3::Y * PLAYER_SIZE.y / 2.0),
                    ..default()
                },
                Bullet,
            ));
        }
    }
}

fn bullet_movement(
    mut commands: Commands,
    mut query: Query<(Entity, &mut Transform), With<Bullet>>,
    time: Res<Time>,
) {
    for (entity, mut transform) in query.iter_mut() {
        transform.translation.y += BULLET_SPEED * time.delta_seconds();
        if transform.translation.y > WINDOW_HEIGHT / 2.0 + BULLET_SIZE.y {
            commands.entity(entity).despawn();
        }
    }
}

fn enemy_spawner(
    mut commands: Commands,
    time: Res<Time>,
    mut timer: ResMut<EnemySpawnTimer>,
) {
    if timer.0.tick(time.delta()).just_finished() {
        let mut rng = thread_rng();
        let x = rng.gen_range(-WINDOW_WIDTH / 2.0 + ENEMY_SIZE.x / 2.0..WINDOW_WIDTH / 2.0 - ENEMY_SIZE.x / 2.0);
        commands.spawn((
            SpriteBundle {
                sprite: Sprite {
                    color: Color::RED,
                    custom_size: Some(ENEMY_SIZE),
                    ..default()
                },
                transform: Transform::from_xyz(x, WINDOW_HEIGHT / 2.0 + ENEMY_SIZE.y, 0.0),
                ..default()
            },
            Enemy,
        ));
    }
}

fn enemy_movement(
    mut commands: Commands,
    mut query: Query<(Entity, &mut Transform), With<Enemy>>,
    time: Res<Time>,
) {
    for (entity, mut transform) in query.iter_mut() {
        transform.translation.y -= ENEMY_SPEED * time.delta_seconds();
        if transform.translation.y < -WINDOW_HEIGHT / 2.0 - ENEMY_SIZE.y {
            commands.entity(entity).despawn();
        }
    }
}

fn bullet_enemy_collision(
    mut commands: Commands,
    bullet_query: Query<(Entity, &Transform), With<Bullet>>,
    enemy_query: Query<(Entity, &Transform), With<Enemy>>,
) {
    for (bullet_entity, bullet_transform) in bullet_query.iter() {
        for (enemy_entity, enemy_transform) in enemy_query.iter() {
            let collision = bullet_transform.translation.truncate().distance(enemy_transform.translation.truncate()) < (BULLET_SIZE.y + ENEMY_SIZE.y) / 2.0;
            if collision {
                commands.entity(bullet_entity).despawn();
                commands.entity(enemy_entity).despawn();
                break;
            }
        }
    }
}
