use ggez::{
    event::{EventHandler, MouseButton},
    glam::*,
    graphics::{self, Color, DrawParam, Rect, Text, Drawable},
    Context, GameResult,
};

use crate::constants::{get_screen_width, get_screen_height};
use crate::effects::star::Star;
use crate::game_state::GameStats;


const TITLE_SCALE: f32 = 64.0;
const SUBTITLE_SCALE: f32 = 32.0;
const BUTTON_W: f32 = 200.0;
const BUTTON_H: f32 = 50.0;
const BUTTON_SPACING: f32 = 70.0;

#[derive(Debug, Clone)]
pub enum MenuAction {
    None,
    StartGame,
    SetDifficulty(crate::game_state::Difficulty),
    ShowStats,
    ShowControls,
    Quit,
}

#[derive(Debug)]
pub enum MenuScreen {
    Main,
    DifficultySelect,
    Stats,
    Controls,
    About,
}

#[derive(Debug)]
pub struct MainMenu {
    // Menu state
    current_screen: MenuScreen,
    selected_button: usize,
    action: MenuAction,
    
    // UI elements
    play_btn_rect: Rect,
    stats_btn_rect: Rect,
    controls_btn_rect: Rect,
    about_btn_rect: Rect,
    quit_btn_rect: Rect,
    back_btn_rect: Rect,
    
    // Text elements
    subtitle_text: Text,
    play_text: Text,
    stats_text: Text,
    controls_text: Text,
    about_text: Text,
    quit_text: Text,
    back_text: Text,
    
    // Statistics display
    stats_display: Vec<Text>,
    
    // Controls display
    controls_display: Vec<Text>,
    
    // About display
    about_display: Vec<Text>,
    
    // Visual elements
    time: f32,
    stars: Vec<Star>,
    stats: GameStats,
    pub selected_difficulty: usize,
}

impl MainMenu {
    pub fn new(_ctx: &mut Context) -> Self {
        let stats = GameStats::load();
        
        // Create button rectangles
        let center_x = get_screen_width() * 0.5 - BUTTON_W * 0.5;
        let start_y = get_screen_height() * 0.4;
        
        let play_btn_rect = Rect::new(center_x, start_y, BUTTON_W, BUTTON_H);
        let stats_btn_rect = Rect::new(center_x, start_y + BUTTON_SPACING, BUTTON_W, BUTTON_H);
        let controls_btn_rect = Rect::new(center_x, start_y + BUTTON_SPACING * 2.0, BUTTON_W, BUTTON_H);
        let about_btn_rect = Rect::new(center_x, start_y + BUTTON_SPACING * 3.0, BUTTON_W, BUTTON_H);
        let quit_btn_rect = Rect::new(center_x, start_y + BUTTON_SPACING * 4.0, BUTTON_W, BUTTON_H);
        let back_btn_rect = Rect::new(get_screen_width() * 0.1, get_screen_height() * 0.9, 100.0, 40.0);
        
        // Create text elements
        let subtitle_text = Text::new(graphics::TextFragment::new("Thanks for Reviewing, John!").scale(SUBTITLE_SCALE));
        let play_text = Text::new(graphics::TextFragment::new("PLAY").scale(24.0));
        let stats_text = Text::new(graphics::TextFragment::new("STATISTICS").scale(24.0));
        let controls_text = Text::new(graphics::TextFragment::new("CONTROLS").scale(24.0));
        let about_text = Text::new(graphics::TextFragment::new("ABOUT").scale(24.0));
        let quit_text = Text::new(graphics::TextFragment::new("QUIT").scale(24.0));
        let back_text = Text::new(graphics::TextFragment::new("BACK").scale(20.0));
        
        // Create statistics display
        let stats_display = Self::create_stats_display(&stats);
        
        // Create controls display
        let controls_display = Self::create_controls_display();
        
        // Create about display
        let about_display = Self::create_about_display();
        
        // Create starfield
        let mut rng = rand::thread_rng();
        let mut stars = Vec::with_capacity(crate::constants::STAR_COUNT);
        for _ in 0..crate::constants::STAR_COUNT {
            stars.push(Star::new(&mut rng));
        }
        
        Self {
            current_screen: MenuScreen::Main,
            selected_button: 0,
            action: MenuAction::None,
            play_btn_rect,
            stats_btn_rect,
            controls_btn_rect,
            about_btn_rect,
            quit_btn_rect,
            back_btn_rect,
            subtitle_text,
            play_text,
            stats_text,
            controls_text,
            about_text,
            quit_text,
            back_text,
            stats_display,
            controls_display,
            about_display,
            time: 0.0,
            stars,
            stats,
            selected_difficulty: 1, // Default to Standard
        }
    }
    
    fn create_stats_display(stats: &GameStats) -> Vec<Text> {
        vec![
            Text::new(graphics::TextFragment::new(format!("Games Played: {}", stats.total_games_played)).scale(20.0)),
            Text::new(graphics::TextFragment::new(format!("High Score: {}", stats.high_score)).scale(20.0)),
            Text::new(graphics::TextFragment::new(format!("Total Score: {}", stats.total_score)).scale(20.0)),
            Text::new(graphics::TextFragment::new(format!("Total Play Time: {:.1}s", stats.total_play_time)).scale(20.0)),
            Text::new(graphics::TextFragment::new(format!("Max Combo: {}", stats.max_combo)).scale(20.0)),
            Text::new(graphics::TextFragment::new("").scale(20.0)), // Spacer
            Text::new(graphics::TextFragment::new("Enemies Killed:").scale(20.0)),
            Text::new(graphics::TextFragment::new(format!("  Total: {}", stats.total_enemies_killed)).scale(18.0)),
            Text::new(graphics::TextFragment::new(format!("  Normal: {}", stats.normal_enemies_killed)).scale(18.0)),
            Text::new(graphics::TextFragment::new(format!("  Fast: {}", stats.fast_enemies_killed)).scale(18.0)),
            Text::new(graphics::TextFragment::new(format!("  Big: {}", stats.big_enemies_killed)).scale(18.0)),
            Text::new(graphics::TextFragment::new(format!("  Behemoths: {}", stats.behemoths_killed)).scale(18.0)),
            Text::new(graphics::TextFragment::new(format!("  Destroyers: {}", stats.destroyers_killed)).scale(18.0)),
            Text::new(graphics::TextFragment::new(format!("  Carriers: {}", stats.carriers_killed)).scale(18.0)),
            Text::new(graphics::TextFragment::new(format!("  Zeniths: {}", stats.zeniths_killed)).scale(18.0)),
            Text::new(graphics::TextFragment::new(format!("  Bosses: {}", stats.bosses_killed)).scale(18.0)),
        ]
    }
    
    fn create_controls_display() -> Vec<Text> {
        vec![
            Text::new(graphics::TextFragment::new("").scale(20.0)), // Spacer
            Text::new(graphics::TextFragment::new("Movement:").scale(20.0)),
            Text::new(graphics::TextFragment::new("  WASD or Arrow Keys").scale(18.0)),
            Text::new(graphics::TextFragment::new("").scale(20.0)), // Spacer
            Text::new(graphics::TextFragment::new("Combat:").scale(20.0)),
            Text::new(graphics::TextFragment::new("  SPACE - Fire bullets").scale(18.0)),
            Text::new(graphics::TextFragment::new("  Hold SPACE - Charge laser (when available)").scale(18.0)),
        ]
    }
    
    fn create_about_display() -> Vec<Text> {
        vec![
            Text::new(graphics::TextFragment::new("Survive and get a high score!").scale(16.0)),
            Text::new(graphics::TextFragment::new("").scale(16.0)), // Spacer
            Text::new(graphics::TextFragment::new("ENEMIES & SCORING:").scale(18.0)),
            Text::new(graphics::TextFragment::new("Normal Enemy (Red) - 10 points").scale(16.0)),
            Text::new(graphics::TextFragment::new("Fast Enemy (Green) - 10 points").scale(16.0)),
            Text::new(graphics::TextFragment::new("Big Enemy (Magenta) - 30 points").scale(16.0)),
            Text::new(graphics::TextFragment::new("Zenith (White) - 50 points").scale(16.0)),
            Text::new(graphics::TextFragment::new("").scale(16.0)), // Spacer
            Text::new(graphics::TextFragment::new("BOSSES:").scale(18.0)),
            Text::new(graphics::TextFragment::new("Destroyer (Red) - 1000 points").scale(16.0)),
            Text::new(graphics::TextFragment::new("Carrier (Purple) - 1500 points").scale(16.0)),
            Text::new(graphics::TextFragment::new("Behemoth (Blue) - 2000 points").scale(16.0)),
            Text::new(graphics::TextFragment::new("").scale(16.0)), // Spacer
            Text::new(graphics::TextFragment::new("COMBOS:").scale(18.0)),
            Text::new(graphics::TextFragment::new("Kill enemies quickly to build combos!").scale(16.0)),
            Text::new(graphics::TextFragment::new("Higher combos = higher score multipliers.").scale(16.0)),
            Text::new(graphics::TextFragment::new("").scale(16.0)), // Spacer
            Text::new(graphics::TextFragment::new("Version: 0.1.0").scale(16.0)),
        ]
    }
    
    pub fn get_action(&mut self) -> MenuAction {
        let action = self.action.clone();
        self.action = MenuAction::None;
        action
    }
    
    pub fn refresh_stats(&mut self) {
        self.stats = GameStats::load();
        self.stats_display = Self::create_stats_display(&self.stats);
    }
    
    fn hsv(h: f32, s: f32, v: f32) -> Color {
        let c = v * s;
        let x = c * (1.0 - ((h / 60.0) % 2.0 - 1.0).abs());
        let m = v - c;
        let (r, g, b) = match h as i32 {
            0..=59   => (c, x, 0.0),
            60..=119 => (x, c, 0.0),
            120..=179=> (0.0, c, x),
            180..=239=> (0.0, x, c),
            240..=299=> (x, 0.0, c),
            _        => (c, 0.0, x),
        };
        Color::new(r + m, g + m, b + m, 1.0)
    }
    
    fn draw_button(&self, canvas: &mut graphics::Canvas, ctx: &Context, rect: &Rect, text: &Text, is_selected: bool) -> GameResult {
        let color = if is_selected { Color::YELLOW } else { Color::WHITE };
        let border_color = if is_selected { Color::YELLOW } else { Color::new(0.5, 0.5, 0.5, 1.0) };
        
        // Draw button border
        let border_mesh = graphics::Mesh::new_rectangle(
            ctx,
            graphics::DrawMode::stroke(2.0),
            *rect,
            border_color,
        )?;
        canvas.draw(&border_mesh, DrawParam::default());
        
        // Draw button text
        canvas.draw(
            text,
            DrawParam::default()
                .dest(Vec2::new(
                    rect.center().x - text.dimensions(ctx).unwrap().w as f32 / 2.0,
                    rect.center().y - text.dimensions(ctx).unwrap().h as f32 / 2.0,
                ))
                .color(color),
        );
        
        Ok(())
    }

    fn draw_difficulty_select(&self, canvas: &mut graphics::Canvas, ctx: &Context) -> GameResult {
        let title = Text::new(graphics::TextFragment::new("SELECT DIFFICULTY").scale(40.0));
        canvas.draw(
            &title,
            DrawParam::default()
                .dest(Vec2::new(get_screen_width() * 0.5 - title.dimensions(ctx).unwrap().w as f32 / 2.0, 100.0))
                .color(Color::YELLOW),
        );
        let difficulties = [
            ("Goober", Color::from_rgb(120, 200, 255)),
            ("Standard", Color::from_rgb(255, 255, 255)),
            ("Ultra-Violence", Color::from_rgb(255, 80, 80)),
            ("Not when, how", Color::from_rgb(255, 0, 255)),
        ];
        let start_y = 200.0;
        let spacing = 70.0;
        for (i, (label, color)) in difficulties.iter().enumerate() {
            let text = Text::new(graphics::TextFragment::new(*label).scale(32.0));
            let is_selected = self.selected_difficulty == i;
            let draw_color = if is_selected { *color } else { Color::from_rgb(180, 180, 180) };
            canvas.draw(
                &text,
                DrawParam::default()
                    .dest(Vec2::new(get_screen_width() * 0.5 - text.dimensions(ctx).unwrap().w as f32 / 2.0, start_y + i as f32 * spacing))
                    .color(draw_color),
            );
        }
        // Back button
        self.draw_button(canvas, ctx, &self.back_btn_rect, &self.back_text, false)?;
        Ok(())
    }
}

impl EventHandler for MainMenu {
    fn update(&mut self, _ctx: &mut Context) -> GameResult {
        self.time += 1.0 / 60.0;
        for star in &mut self.stars {
            star.update(1.0 / 60.0);
        }
        Ok(())
    }

    fn draw(&mut self, ctx: &mut Context) -> GameResult {
        let mut canvas = graphics::Canvas::from_frame(ctx, Color::BLACK);

        // Draw starfield
        for star in &self.stars {
            let pulse = (star.pulse_timer.sin() * 0.3 + 0.7) as f32;
            let color = Color::new(1.0, 1.0, 1.0, 0.7 * pulse * star.brightness);
            let star_mesh = graphics::Mesh::new_circle(
                ctx,
                graphics::DrawMode::fill(),
                star.position,
                1.5,
                0.1,
                color,
            )?;
            canvas.draw(&star_mesh, DrawParam::default());
        }

        match self.current_screen {
            MenuScreen::Main => {
                // Rainbow title
                let title = "WIPSHMUP";
                let mut x_cursor = get_screen_width() * 0.5 - (TITLE_SCALE * 0.5 * title.len() as f32 * 0.6);
                for (i, ch) in title.chars().enumerate() {
                    let glyph = Text::new(graphics::TextFragment::new(ch.to_string()).scale(TITLE_SCALE));
                    let hue = (self.time * 180.0 + i as f32 * 30.0) % 360.0;
                    canvas.draw(
                        &glyph,
                        DrawParam::default()
                            .dest(Vec2::new(x_cursor, get_screen_height() * 0.15))
                            .color(Self::hsv(hue, 1.0, 1.0)),
                    );
                    x_cursor += TITLE_SCALE * 0.6;
                }

                // Subtitle
                canvas.draw(
                    &self.subtitle_text,
                    DrawParam::default()
                        .dest(Vec2::new(get_screen_width() * 0.5 - self.subtitle_text.dimensions(ctx).unwrap().w as f32 / 2.0, get_screen_height() * 0.25))
                        .color(Color::from_rgb(255, 208, 0)),
                );

                // Draw buttons
                self.draw_button(&mut canvas, ctx, &self.play_btn_rect, &self.play_text, self.selected_button == 0)?;
                self.draw_button(&mut canvas, ctx, &self.stats_btn_rect, &self.stats_text, self.selected_button == 1)?;
                self.draw_button(&mut canvas, ctx, &self.controls_btn_rect, &self.controls_text, self.selected_button == 2)?;
                self.draw_button(&mut canvas, ctx, &self.about_btn_rect, &self.about_text, self.selected_button == 3)?;
                self.draw_button(&mut canvas, ctx, &self.quit_btn_rect, &self.quit_text, self.selected_button == 4)?;
            }
            MenuScreen::DifficultySelect => {
                self.draw_difficulty_select(&mut canvas, ctx)?;
            }
            MenuScreen::Stats => {
                // Title
                let title = Text::new(graphics::TextFragment::new("STATISTICS").scale(40.0));
                canvas.draw(
                    &title,
                    DrawParam::default()
                        .dest(Vec2::new(get_screen_width() * 0.5 - title.dimensions(ctx).unwrap().w as f32 / 2.0, 50.0))
                        .color(Color::YELLOW),
                );
                
                // Draw stats
                let mut y_offset = 120.0;
                for stat_text in &self.stats_display {
                    canvas.draw(
                        stat_text,
                        DrawParam::default()
                            .dest(Vec2::new(get_screen_width() * 0.5 - stat_text.dimensions(ctx).unwrap().w as f32 / 2.0, y_offset))
                            .color(Color::WHITE),
                    );
                    y_offset += 30.0;
                }
                
                // Back button
                self.draw_button(&mut canvas, ctx, &self.back_btn_rect, &self.back_text, false)?;
            }
            
            MenuScreen::Controls => {
                // Title
                let title = Text::new(graphics::TextFragment::new("CONTROLS").scale(40.0));
                canvas.draw(
                    &title,
                    DrawParam::default()
                        .dest(Vec2::new(get_screen_width() * 0.5 - title.dimensions(ctx).unwrap().w as f32 / 2.0, 50.0))
                        .color(Color::YELLOW),
                );
                
                // Draw controls
                let mut y_offset = 120.0;
                for control_text in &self.controls_display {
                    canvas.draw(
                        control_text,
                        DrawParam::default()
                            .dest(Vec2::new(get_screen_width() * 0.5 - control_text.dimensions(ctx).unwrap().w as f32 / 2.0, y_offset))
                            .color(Color::WHITE),
                    );
                    y_offset += 30.0;
                }
                
                // Back button
                self.draw_button(&mut canvas, ctx, &self.back_btn_rect, &self.back_text, false)?;
            }
            
            MenuScreen::About => {
                // Title
                let title = Text::new(graphics::TextFragment::new("ABOUT").scale(40.0));
                canvas.draw(
                    &title,
                    DrawParam::default()
                        .dest(Vec2::new(get_screen_width() * 0.5 - title.dimensions(ctx).unwrap().w as f32 / 2.0, 50.0))
                        .color(Color::YELLOW),
                );
                
                // Draw about content
                let mut y_offset = 100.0;
                for about_text in &self.about_display {
                    canvas.draw(
                        about_text,
                        DrawParam::default()
                            .dest(Vec2::new(get_screen_width() * 0.5 - about_text.dimensions(ctx).unwrap().w as f32 / 2.0, y_offset))
                            .color(Color::WHITE),
                    );
                    y_offset += 22.0; // Reduced spacing for smaller text
                }
                
                // Back button
                self.draw_button(&mut canvas, ctx, &self.back_btn_rect, &self.back_text, false)?;
            }
        }

        canvas.finish(ctx)?;
        Ok(())
    }

    fn mouse_button_up_event(&mut self, _ctx: &mut Context, button: MouseButton, x: f32, y: f32) -> GameResult {
        if button == MouseButton::Left {
            match self.current_screen {
                MenuScreen::Main => {
                    if self.play_btn_rect.contains([x, y]) {
                        self.current_screen = MenuScreen::DifficultySelect;
                    } else if self.stats_btn_rect.contains([x, y]) {
                        self.current_screen = MenuScreen::Stats;
                        self.refresh_stats();
                    } else if self.controls_btn_rect.contains([x, y]) {
                        self.current_screen = MenuScreen::Controls;
                    } else if self.about_btn_rect.contains([x, y]) {
                        self.current_screen = MenuScreen::About;
                    } else if self.quit_btn_rect.contains([x, y]) {
                        self.action = MenuAction::Quit;
                    }
                }
                MenuScreen::DifficultySelect => {
                    let start_y = 200.0;
                    let spacing = 70.0;
                    let mouse_y = y;
                    for i in 0..4 {
                        let text = Text::new(graphics::TextFragment::new("").scale(32.0));
                        let text_y = start_y + i as f32 * spacing;
                        let text_h = 40.0;
                        if mouse_y >= text_y && mouse_y <= text_y + text_h {
                            self.selected_difficulty = i;
                            self.action = MenuAction::SetDifficulty(match i {
                                0 => crate::game_state::Difficulty::Goober,
                                1 => crate::game_state::Difficulty::Standard,
                                2 => crate::game_state::Difficulty::UltraViolence,
                                3 => crate::game_state::Difficulty::NotWhenHow,
                                _ => crate::game_state::Difficulty::Standard,
                            });
                        }
                    }
                    if self.back_btn_rect.contains([x, y]) {
                        self.current_screen = MenuScreen::Main;
                    }
                }
                MenuScreen::Stats | MenuScreen::Controls | MenuScreen::About => {
                    if self.back_btn_rect.contains([x, y]) {
                        self.current_screen = MenuScreen::Main;
                    }
                }
            }
        }
        Ok(())
    }

    fn key_down_event(&mut self, _ctx: &mut Context, input: ggez::input::keyboard::KeyInput, _repeat: bool) -> GameResult {
        match self.current_screen {
            MenuScreen::Main => {
                match input.keycode {
                    Some(ggez::input::keyboard::KeyCode::Up) => {
                        if self.selected_button > 0 {
                            self.selected_button -= 1;
                        }
                    }
                    Some(ggez::input::keyboard::KeyCode::Down) => {
                        if self.selected_button < 4 { // Updated to 4 for new buttons
                            self.selected_button += 1;
                        }
                    }
                    Some(ggez::input::keyboard::KeyCode::Return) => {
                        match self.selected_button {
                            0 => self.current_screen = MenuScreen::DifficultySelect,
                            1 => {
                                self.current_screen = MenuScreen::Stats;
                                self.refresh_stats();
                            }
                            2 => self.current_screen = MenuScreen::Controls,
                            3 => self.current_screen = MenuScreen::About,
                            4 => self.action = MenuAction::Quit,
                            _ => {}
                        }
                    }
                    _ => {}
                }
            }
            MenuScreen::DifficultySelect => {
                match input.keycode {
                    Some(ggez::input::keyboard::KeyCode::Up) => {
                        if self.selected_difficulty > 0 {
                            self.selected_difficulty -= 1;
                        }
                    }
                    Some(ggez::input::keyboard::KeyCode::Down) => {
                        if self.selected_difficulty < 3 {
                            self.selected_difficulty += 1;
                        }
                    }
                    Some(ggez::input::keyboard::KeyCode::Return) => {
                        self.action = MenuAction::SetDifficulty(match self.selected_difficulty {
                            0 => crate::game_state::Difficulty::Goober,
                            1 => crate::game_state::Difficulty::Standard,
                            2 => crate::game_state::Difficulty::UltraViolence,
                            3 => crate::game_state::Difficulty::NotWhenHow,
                            _ => crate::game_state::Difficulty::Standard,
                        });
                    }
                    Some(ggez::input::keyboard::KeyCode::Escape) => {
                        self.current_screen = MenuScreen::Main;
                    }
                    _ => {}
                }
            }
            MenuScreen::Stats | MenuScreen::Controls | MenuScreen::About => {
                if let Some(ggez::input::keyboard::KeyCode::Escape) = input.keycode {
                    self.current_screen = MenuScreen::Main;
                }
            }
        }
        Ok(())
    }

    fn key_up_event(&mut self, _ctx: &mut Context, _input: ggez::input::keyboard::KeyInput) -> GameResult {
        Ok(())
    }
} 