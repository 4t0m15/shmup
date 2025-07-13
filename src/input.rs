use ggez::{Context, input::keyboard};

#[derive(Default, Debug)]
pub struct InputState {
    pub held: std::collections::HashSet<keyboard::KeyCode>,
}

impl InputState {
    pub fn poll(&mut self, ctx: &Context) {
        self.held.clear();
        for key in ctx.keyboard.pressed_keys() {
            self.held.insert(*key);
        }
    }

    #[inline]
    pub fn pressed(&self, key: keyboard::KeyCode) -> bool {
        self.held.contains(&key)
    }
} 