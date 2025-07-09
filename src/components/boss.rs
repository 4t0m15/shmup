use bevy::prelude::*;

#[derive(Component)]
pub struct Boss;

#[derive(Component)]
pub struct BossHealth {
    pub hp: u32,
    pub max_hp: u32,
}
