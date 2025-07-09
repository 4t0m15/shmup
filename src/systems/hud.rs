use bevy::*;
use crate::resources::game_state::GameStats;

#[derive(Component)]
pub struct HudText;

#[derive(Component)]
pub struct BossHealthBar;

pub fn setup_hud(mut commands: Commands, asset_server: Res<AssetServer>) {
    commands.spawn((
        TextBundle {
            text: Text::from_section(
                "Score: 0   Lives: 3",
                TextStyle {
                    font: asset_server.load("fonts/FiraSans-Bold.ttf"),
                    font_size: 32.0,
                    color: Color::WHITE,
                },
            ),
            style: Style {
                position_type: PositionType::Absolute,
                position: UiRect { left: Val::Px(16.0), top: Val::Px(8.0), ..default() },
                ..default()
            },
            ..default()
        },
        HudText,
    ));
}

pub fn setup_boss_health_bar(mut commands: Commands) {
    commands.spawn((
        NodeBundle {
            style: Style {
                size: Size::new(Val::Px(320.0), Val::Px(16.0)),
                position_type: PositionType::Absolute,
                position: UiRect { left: Val::Px(80.0), top: Val::Px(48.0), ..default() },
                ..default()
            },
            background_color: BackgroundColor(Color::srgb(0.5, 0.0, 0.0)),
            ..default()
        },
        BossHealthBar,
    ));
}

pub fn update_hud(stats: Res<GameStats>, mut query: Query<&mut Text, With<HudText>>) {
    if let Ok(mut text) = query.get_single_mut() {
        if let Some(section) = text.sections.get_mut(0) {
            section.value = format!("Score: {}   Lives: {}", stats.score, stats.lives);
        }
    }
}

pub fn update_boss_health_bar(
    boss_query: Query<&crate::components::boss::BossHealth>,
    mut query: Query<&mut Style, With<BossHealthBar>>,
) {
    if let (Ok(boss_hp), Ok(mut style)) = (boss_query.get_single(), query.get_single_mut()) {
        let percent = boss_hp.hp as f32 / boss_hp.max_hp as f32;
        style.size.width = Val::Px(320.0 * percent);
    } else if let Ok(mut style) = query.get_single_mut() {
        style.size.width = Val::Px(0.0);
    }
}
