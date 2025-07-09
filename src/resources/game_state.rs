use bevy::prelude::*;

#[derive(Resource, Default)]
pub struct GameStats {
    pub score: u32,
    pub lives: u8,
}

