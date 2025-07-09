use bevy::prelude::*;
use crate::components::background::Background;

const BG_SCROLL_SPEED: f32 = 60.0;

pub fn spawn_background(mut commands: Commands, asset_server: Res<AssetServer>) {
    // Placeholder: use a solid color or a simple texture if available
    let texture_handle = asset_server.load("sprites/background.png");
    commands.spawn((
        SpriteBundle {
            texture: texture_handle,
            transform: Transform::from_xyz(0.0, 0.0, 0.0),
            ..default()
        },
        Background,
    ));
}

pub fn scroll_background(
    time: Res<Time>,
    mut query: Query<&mut Transform, With<Background>>,
) {
    for mut transform in &mut query {
        transform.translation.y -= BG_SCROLL_SPEED * time.delta_secs();
        // Loop background (simple wrap)
        if transform.translation.y < -320.0 {
            transform.translation.y += 640.0;
        }
    }
}
