use ggez::{conf::{WindowMode, WindowSetup}, event, GameResult};

mod constants;
mod game_state;
mod input;
mod entities;
mod effects;
mod weapons;

use crate::game_state::GameState;

fn main() -> GameResult {
    let cb = ggez::ContextBuilder::new("wipshmup", "ggez")
        .window_setup(WindowSetup::default().title("Shoot 'Em Up"))
        .window_mode(
            WindowMode::default()
                .dimensions(constants::SCREEN_WIDTH, constants::SCREEN_HEIGHT)
                .resizable(false),
        );
    let (ctx, event_loop) = cb.build()?;
    let state = GameState::new();
    event::run(ctx, event_loop, state)
} 