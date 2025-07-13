/// Get the primary monitor's resolution
pub fn get_primary_monitor_resolution() -> (u32, u32) {
    // For now, return a default resolution to avoid event loop conflicts
    // The actual resolution will be detected by ggez when the window is created
    (1920, 1080)
}

/// Get all available monitors and their resolutions
pub fn get_all_monitors() -> Vec<(String, u32, u32)> {
    // For now, return a default monitor to avoid event loop conflicts
    vec![("Default Monitor".to_string(), 1920, 1080)]
}

/// Print monitor information for debugging
pub fn print_monitor_info() {
    println!("Using default monitor resolution: 1920x1080");
    println!("Note: Actual resolution will be detected by ggez when window is created");
} 