use ggez::{conf::{WindowMode, WindowSetup}, event, GameResult};

mod constants;
mod game_state;
mod input;
mod entities;
mod effects;
mod weapons;
mod main_menu;
mod monitor;

use crate::game_state::GameState;
use crate::main_menu::{MainMenu, MenuAction};
use crate::constants::{init_scaling, get_scaling};
use crate::monitor::{get_primary_monitor_resolution, print_monitor_info};

#[derive(Debug)]
enum GameScreen {
    Menu(MainMenu),
    Game(GameState),
}

impl event::EventHandler for GameScreen {
    fn update(&mut self, ctx: &mut ggez::Context) -> GameResult {
        // Remove set_mode call; only set window mode at startup
        match self {
            GameScreen::Menu(menu) => menu.update(ctx),
            GameScreen::Game(game) => game.update(ctx),
        }
    }

    fn draw(&mut self, ctx: &mut ggez::Context) -> GameResult {
        match self {
            GameScreen::Menu(menu) => menu.draw(ctx),
            GameScreen::Game(game) => game.draw(ctx),
        }
    }

    fn mouse_button_up_event(
        &mut self,
        ctx: &mut ggez::Context,
        button: ggez::event::MouseButton,
        x: f32,
        y: f32,
    ) -> GameResult {
        match self {
            GameScreen::Menu(menu) => {
                menu.mouse_button_up_event(ctx, button, x, y)?;
                
                // Check for menu actions
                match menu.get_action() {
                    MenuAction::SetDifficulty(difficulty) => {
                        *self = GameScreen::Game(GameState::new_with_difficulty(ctx, difficulty));
                    }
                    MenuAction::Quit => {
                        ctx.request_quit();
                    }
                    _ => {}
                }
                Ok(())
            }
            GameScreen::Game(game) => game.mouse_button_up_event(ctx, button, x, y),
        }
    }

    fn key_down_event(
        &mut self,
        ctx: &mut ggez::Context,
        input: ggez::input::keyboard::KeyInput,
        repeat: bool,
    ) -> GameResult {
        match self {
            GameScreen::Menu(menu) => {
                menu.key_down_event(ctx, input, repeat)?;
                
                // Check for menu actions after key events
                match menu.get_action() {
                    MenuAction::SetDifficulty(difficulty) => {
                        *self = GameScreen::Game(GameState::new_with_difficulty(ctx, difficulty));
                    }
                    MenuAction::Quit => {
                        ctx.request_quit();
                    }
                    _ => {}
                }
                Ok(())
            }
            GameScreen::Game(game) => {
                game.key_down_event(ctx, input, repeat)?;
                
                // Check if player wants to return to menu (ESC key)
                if let Some(ggez::input::keyboard::KeyCode::Escape) = input.keycode {
                    if game.game_over {
                        // Return to menu when game is over
                        *self = GameScreen::Menu(MainMenu::new(ctx));
                    }
                }
                Ok(())
            }
        }
    }

    fn key_up_event(
        &mut self,
        ctx: &mut ggez::Context,
        input: ggez::input::keyboard::KeyInput,
    ) -> GameResult {
        match self {
            GameScreen::Menu(menu) => menu.key_up_event(ctx, input),
            GameScreen::Game(game) => game.key_up_event(ctx, input),
        }
    }
}

fn main() -> GameResult {
    // Detect primary monitor resolution
    let (detected_width, detected_height) = get_primary_monitor_resolution();
    print_monitor_info();
    
    println!("Setting window to detected resolution: {}x{}", detected_width, detected_height);
    
    let cb = ggez::ContextBuilder::new("wipshmup", "ggez")
        .window_setup(WindowSetup::default().title("WIPSHMUP - Space Shooter"))
        .window_mode(
            WindowMode::default()
                .dimensions(detected_width as f32, detected_height as f32)
                .fullscreen_type(ggez::conf::FullscreenType::Desktop)
                .borderless(true)
                .resizable(false),
        );
    let (mut ctx, event_loop) = cb.build()?;

    // Get actual screen dimensions and initialize scaling
    let (actual_width, actual_height) = ctx.gfx.drawable_size();
    init_scaling(actual_width, actual_height);
    
    // Update global screen dimensions
    unsafe {
        crate::constants::SCREEN_WIDTH = actual_width;
        crate::constants::SCREEN_HEIGHT = actual_height;
    }
    
    let scaling = get_scaling();
    println!("Screen dimensions: {}x{}", actual_width, actual_height);
    println!("Scale factor: {:.2}", scaling.scale_factor);

    // Start with the main menu
    let menu = MainMenu::new(&mut ctx);
    let game_screen = GameScreen::Menu(menu);
    event::run(ctx, event_loop, game_screen)
} 