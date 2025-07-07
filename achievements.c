#include "game.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

// =============================================================================
// ACHIEVEMENT SYSTEM
// =============================================================================

// Achievement definitions
static const AchievementDef achievement_definitions[ACHIEVEMENT_COUNT] = {
    // Combat Achievements
    {"First Blood", "Destroy your first enemy", ACHIEVEMENT_BRONZE, false, 0, 1},
    {"Centurion", "Destroy 100 enemies", ACHIEVEMENT_SILVER, false, 0, 100},
    {"Destroyer", "Destroy 1000 enemies", ACHIEVEMENT_GOLD, false, 0, 1000},
    {"Annihilator", "Destroy 5000 enemies", ACHIEVEMENT_PLATINUM, false, 0, 5000},
    
    // Wave Achievements
    {"Veteran", "Reach wave 10", ACHIEVEMENT_BRONZE, false, 0, 10},
    {"Elite", "Reach wave 25", ACHIEVEMENT_SILVER, false, 0, 25},
    {"Legend", "Reach wave 50", ACHIEVEMENT_GOLD, false, 0, 50},
    {"Immortal", "Reach wave 100", ACHIEVEMENT_PLATINUM, false, 0, 100},
    
    // Score Achievements
    {"Rising Star", "Score 10,000 points", ACHIEVEMENT_BRONZE, false, 0, 10000},
    {"High Scorer", "Score 50,000 points", ACHIEVEMENT_SILVER, false, 0, 50000},
    {"Score Master", "Score 100,000 points", ACHIEVEMENT_GOLD, false, 0, 100000},
    {"Point God", "Score 500,000 points", ACHIEVEMENT_PLATINUM, false, 0, 500000},
    
    // Accuracy Achievements
    {"Marksman", "Achieve 75% accuracy in a wave", ACHIEVEMENT_BRONZE, false, 0, 75},
    {"Sharpshooter", "Achieve 90% accuracy in a wave", ACHIEVEMENT_SILVER, false, 0, 90},
    {"Sniper", "Achieve 95% accuracy in a wave", ACHIEVEMENT_GOLD, false, 0, 95},
    {"Perfect Shot", "Achieve 100% accuracy in a wave", ACHIEVEMENT_PLATINUM, false, 0, 100},
    
    // Survival Achievements
    {"Survivor", "Complete 5 waves without dying", ACHIEVEMENT_BRONZE, false, 0, 5},
    {"Resilient", "Complete 10 waves without dying", ACHIEVEMENT_SILVER, false, 0, 10},
    {"Invincible", "Complete 20 waves without dying", ACHIEVEMENT_GOLD, false, 0, 20},
    {"Pacifist Run", "Complete a wave without shooting", ACHIEVEMENT_SPECIAL, false, 0, 1},
    
    // Combo Achievements
    {"Combo Starter", "Achieve a 10x combo", ACHIEVEMENT_BRONZE, false, 0, 10},
    {"Combo Master", "Achieve a 25x combo", ACHIEVEMENT_SILVER, false, 0, 25},
    {"Combo King", "Achieve a 50x combo", ACHIEVEMENT_GOLD, false, 0, 50},
    {"Combo God", "Achieve a 100x combo", ACHIEVEMENT_PLATINUM, false, 0, 100},
    
    // Special Achievements
    {"Untouchable", "Defeat a boss without taking damage", ACHIEVEMENT_GOLD, false, 0, 1},
    {"Wingman", "Rescue a captured ship", ACHIEVEMENT_BRONZE, false, 0, 1},
    {"Arsenal Master", "Unlock all weapons", ACHIEVEMENT_SILVER, false, 0, WEAPON_COUNT},
    {"Bonus Perfectionist", "Perfect score on bonus stage", ACHIEVEMENT_GOLD, false, 0, 1},
    {"Speed Demon", "Complete first 10 waves in under 5 minutes", ACHIEVEMENT_GOLD, false, 0, 300},
    {"Power Hoarder", "Collect 50 power-ups in one game", ACHIEVEMENT_SILVER, false, 0, 50},
    {"Evolution Witness", "See 20 enemy morphings", ACHIEVEMENT_BRONZE, false, 0, 20},
    {"Tactical Analyst", "Observe all 7 AI behaviors", ACHIEVEMENT_BRONZE, false, 0, 7},
    
    // Time-based Achievements
    {"Dedicated", "Play for 1 hour total", ACHIEVEMENT_BRONZE, false, 0, 3600},
    {"Devoted", "Play for 10 hours total", ACHIEVEMENT_SILVER, false, 0, 36000},
    {"Obsessed", "Play for 50 hours total", ACHIEVEMENT_GOLD, false, 0, 180000},
};

// Initialize achievement system
void InitAchievementSystem(AchievementSystem* achievements) {
    if (!achievements) return;
    
    // Copy achievement definitions
    memcpy(achievements->achievements, achievement_definitions, sizeof(achievement_definitions));
    
    achievements->total_achievements = ACHIEVEMENT_COUNT;
    achievements->unlocked_count = 0;
    achievements->notification_timer = 0.0f;
    achievements->current_notification = ACHIEVEMENT_COUNT; // Invalid index
    achievements->show_notification = false;
    
    // Initialize statistics
    achievements->stats.total_enemies_killed = 0;
    achievements->stats.total_score = 0;
    achievements->stats.highest_wave = 0;
    achievements->stats.total_play_time = 0.0f;
    achievements->stats.total_shots_fired = 0;
    achievements->stats.total_shots_hit = 0;
    achievements->stats.highest_combo = 0;
    achievements->stats.bosses_defeated = 0;
    achievements->stats.power_ups_collected = 0;
    achievements->stats.ships_rescued = 0;
    achievements->stats.morphings_witnessed = 0;
    achievements->stats.perfect_bonus_stages = 0;
    achievements->stats.no_death_waves = 0;
    achievements->stats.current_no_death_streak = 0;
    achievements->stats.ai_behaviors_seen = 0;
    achievements->stats.weapons_unlocked = 1; // Start with one weapon
    achievements->stats.pacifist_waves = 0;
    achievements->stats.speed_run_time = 0.0f;
    achievements->stats.games_played = 0;
    
    // Load achievements from file
    LoadAchievements(achievements);
}

// Update achievement system
void UpdateAchievementSystem(AchievementSystem* achievements, GameState* gameState, float delta) {
    if (!achievements || !gameState) return;
    
    // Update play time
    achievements->stats.total_play_time += delta;
    
    // Update notification timer
    if (achievements->show_notification) {
        achievements->notification_timer -= delta;
        if (achievements->notification_timer <= 0.0f) {
            achievements->show_notification = false;
        }
    }
    
    // Check for achievement unlocks
    CheckAchievements(achievements, gameState);
}

// Check for achievement unlocks
void CheckAchievements(AchievementSystem* achievements, GameState* gameState) {
    if (!achievements || !gameState) return;
    
    for (int i = 0; i < ACHIEVEMENT_COUNT; i++) {
        if (achievements->achievements[i].unlocked) continue;
        
        bool should_unlock = false;
        
        switch (achievements->achievements[i].id) {
            // Combat achievements
            case ACHIEVEMENT_FIRST_KILL:
            case ACHIEVEMENT_KILL_100:
            case ACHIEVEMENT_KILL_1000:
            case ACHIEVEMENT_KILL_5000:
                should_unlock = achievements->stats.total_enemies_killed >= achievements->achievements[i].target_value;
                break;
                
            // Wave achievements
            case ACHIEVEMENT_WAVE_10:
            case ACHIEVEMENT_WAVE_25:
            case ACHIEVEMENT_WAVE_50:
            case ACHIEVEMENT_WAVE_100:
                should_unlock = achievements->stats.highest_wave >= achievements->achievements[i].target_value;
                break;
                
            // Score achievements
            case ACHIEVEMENT_SCORE_10K:
            case ACHIEVEMENT_SCORE_50K:
            case ACHIEVEMENT_SCORE_100K:
            case ACHIEVEMENT_SCORE_500K:
                should_unlock = achievements->stats.total_score >= achievements->achievements[i].target_value;
                break;
                
            // Accuracy achievements
            case ACHIEVEMENT_ACCURACY_75:
            case ACHIEVEMENT_ACCURACY_90:
            case ACHIEVEMENT_ACCURACY_95:
            case ACHIEVEMENT_PERFECT_ACCURACY:
                if (achievements->stats.total_shots_fired > 0) {
                    float accuracy = (float)achievements->stats.total_shots_hit / achievements->stats.total_shots_fired * 100.0f;
                    should_unlock = accuracy >= achievements->achievements[i].target_value;
                }
                break;
                
            // Survival achievements
            case ACHIEVEMENT_NO_DEATH_WAVE_5:
            case ACHIEVEMENT_NO_DEATH_WAVE_10:
            case ACHIEVEMENT_NO_DEATH_WAVE_20:
                should_unlock = achievements->stats.current_no_death_streak >= achievements->achievements[i].target_value;
                break;
                
            case ACHIEVEMENT_PACIFIST:
                should_unlock = achievements->stats.pacifist_waves >= 1;
                break;
                
            // Combo achievements
            case ACHIEVEMENT_COMBO_10:
            case ACHIEVEMENT_COMBO_25:
            case ACHIEVEMENT_COMBO_50:
            case ACHIEVEMENT_COMBO_100:
                should_unlock = achievements->stats.highest_combo >= achievements->achievements[i].target_value;
                break;
                
            // Special achievements
            case ACHIEVEMENT_BOSS_NO_DAMAGE:
                should_unlock = achievements->stats.bosses_defeated > 0; // Will be set by specific boss defeat logic
                break;
                
            case ACHIEVEMENT_DUAL_FIGHTER:
                should_unlock = achievements->stats.ships_rescued >= 1;
                break;
                
            case ACHIEVEMENT_ALL_WEAPONS:
                should_unlock = achievements->stats.weapons_unlocked >= WEAPON_COUNT;
                break;
                
            case ACHIEVEMENT_PERFECT_BONUS:
                should_unlock = achievements->stats.perfect_bonus_stages >= 1;
                break;
                
            case ACHIEVEMENT_SPEED_RUN:
                should_unlock = achievements->stats.speed_run_time > 0 && achievements->stats.speed_run_time <= 300.0f;
                break;
                
            case ACHIEVEMENT_HOARDER:
                should_unlock = achievements->stats.power_ups_collected >= 50;
                break;
                
            case ACHIEVEMENT_MORPHING_MASTER:
                should_unlock = achievements->stats.morphings_witnessed >= 20;
                break;
                
            case ACHIEVEMENT_AI_OBSERVER:
                should_unlock = achievements->stats.ai_behaviors_seen >= 7;
                break;
                
            // Time-based achievements
            case ACHIEVEMENT_PLAY_TIME_1H:
            case ACHIEVEMENT_PLAY_TIME_10H:
            case ACHIEVEMENT_PLAY_TIME_50H:
                should_unlock = achievements->stats.total_play_time >= achievements->achievements[i].target_value;
                break;
        }
        
        if (should_unlock) {
            UnlockAchievement(achievements, (AchievementID)i);
        }
    }
}

// Unlock an achievement
void UnlockAchievement(AchievementSystem* achievements, AchievementID id) {
    if (!achievements || id >= ACHIEVEMENT_COUNT || achievements->achievements[id].unlocked) return;
    
    achievements->achievements[id].unlocked = true;
    achievements->achievements[id].unlock_time = time(NULL);
    achievements->unlocked_count++;
    
    // Show notification
    ShowAchievementNotification(achievements, id);
    
    // Save achievements
    SaveAchievements(achievements);
    
    // Award points based on tier
    int points = GetAchievementPoints(achievements->achievements[id].tier);
    achievements->stats.total_score += points;
}

// Show achievement notification
void ShowAchievementNotification(AchievementSystem* achievements, AchievementID id) {
    if (!achievements || id >= ACHIEVEMENT_COUNT) return;
    
    achievements->current_notification = id;
    achievements->notification_timer = 5.0f; // Show for 5 seconds
    achievements->show_notification = true;
}

// Get achievement points for tier
int GetAchievementPoints(AchievementTier tier) {
    switch (tier) {
        case ACHIEVEMENT_BRONZE: return 100;
        case ACHIEVEMENT_SILVER: return 250;
        case ACHIEVEMENT_GOLD: return 500;
        case ACHIEVEMENT_PLATINUM: return 1000;
        case ACHIEVEMENT_SPECIAL: return 750;
        default: return 0;
    }
}

// Get achievement progress
float GetAchievementProgress(const AchievementSystem* achievements, AchievementID id) {
    if (!achievements || id >= ACHIEVEMENT_COUNT) return 0.0f;
    
    if (achievements->achievements[id].unlocked) return 1.0f;
    
    float current_value = 0.0f;
    
    switch (id) {
        case ACHIEVEMENT_FIRST_KILL:
        case ACHIEVEMENT_KILL_100:
        case ACHIEVEMENT_KILL_1000:
        case ACHIEVEMENT_KILL_5000:
            current_value = (float)achievements->stats.total_enemies_killed;
            break;
            
        case ACHIEVEMENT_WAVE_10:
        case ACHIEVEMENT_WAVE_25:
        case ACHIEVEMENT_WAVE_50:
        case ACHIEVEMENT_WAVE_100:
            current_value = (float)achievements->stats.highest_wave;
            break;
            
        case ACHIEVEMENT_SCORE_10K:
        case ACHIEVEMENT_SCORE_50K:
        case ACHIEVEMENT_SCORE_100K:
        case ACHIEVEMENT_SCORE_500K:
            current_value = (float)achievements->stats.total_score;
            break;
            
        case ACHIEVEMENT_COMBO_10:
        case ACHIEVEMENT_COMBO_25:
        case ACHIEVEMENT_COMBO_50:
        case ACHIEVEMENT_COMBO_100:
            current_value = (float)achievements->stats.highest_combo;
            break;
            
        case ACHIEVEMENT_PLAY_TIME_1H:
        case ACHIEVEMENT_PLAY_TIME_10H:
        case ACHIEVEMENT_PLAY_TIME_50H:
            current_value = achievements->stats.total_play_time;
            break;
            
        default:
            return 0.0f;
    }
    
    return current_value / achievements->achievements[id].target_value;
}

// Update statistics
void UpdateAchievementStats(AchievementSystem* achievements, StatType stat, int value) {
    if (!achievements) return;
    
    switch (stat) {
        case STAT_ENEMY_KILLED:
            achievements->stats.total_enemies_killed += value;
            break;
        case STAT_WAVE_REACHED:
            if (value > achievements->stats.highest_wave) {
                achievements->stats.highest_wave = value;
            }
            break;
        case STAT_SCORE_ADDED:
            achievements->stats.total_score += value;
            break;
        case STAT_SHOT_FIRED:
            achievements->stats.total_shots_fired += value;
            break;
        case STAT_SHOT_HIT:
            achievements->stats.total_shots_hit += value;
            break;
        case STAT_COMBO_ACHIEVED:
            if (value > achievements->stats.highest_combo) {
                achievements->stats.highest_combo = value;
            }
            break;
        case STAT_BOSS_DEFEATED:
            achievements->stats.bosses_defeated += value;
            break;
        case STAT_POWER_UP_COLLECTED:
            achievements->stats.power_ups_collected += value;
            break;
        case STAT_SHIP_RESCUED:
            achievements->stats.ships_rescued += value;
            break;
        case STAT_MORPHING_WITNESSED:
            achievements->stats.morphings_witnessed += value;
            break;
        case STAT_PERFECT_BONUS:
            achievements->stats.perfect_bonus_stages += value;
            break;
        case STAT_DEATH_OCCURRED:
            achievements->stats.current_no_death_streak = 0;
            break;
        case STAT_WAVE_COMPLETED:
            achievements->stats.current_no_death_streak += value;
            if (achievements->stats.current_no_death_streak > achievements->stats.no_death_waves) {
                achievements->stats.no_death_waves = achievements->stats.current_no_death_streak;
            }
            break;
        case STAT_WEAPON_UNLOCKED:
            achievements->stats.weapons_unlocked += value;
            break;
        case STAT_PACIFIST_WAVE:
            achievements->stats.pacifist_waves += value;
            break;
        case STAT_AI_BEHAVIOR_SEEN:
            achievements->stats.ai_behaviors_seen |= (1 << value); // Use bitfield for unique behaviors
            break;
        case STAT_GAME_STARTED:
            achievements->stats.games_played += value;
            achievements->stats.speed_run_time = 0.0f; // Reset speed run timer
            break;
        case STAT_SPEED_RUN_COMPLETE:
            achievements->stats.speed_run_time = (float)value;
            break;
    }
}

// Draw achievement notification
void DrawAchievementNotification(const AchievementSystem* achievements) {
    if (!achievements || !achievements->show_notification || achievements->current_notification >= ACHIEVEMENT_COUNT) return;
    
    const Achievement* achievement = &achievements->achievements[achievements->current_notification];
    
    // Calculate animation
    float fade_time = 1.0f;
    float alpha = 1.0f;
    if (achievements->notification_timer < fade_time) {
        alpha = achievements->notification_timer / fade_time;
    } else if (achievements->notification_timer > 4.0f) {
        alpha = (5.0f - achievements->notification_timer) / fade_time;
    }
    
    // Draw notification box
    int box_width = 350;
    int box_height = 80;
    int box_x = SCREEN_WIDTH - box_width - 20;
    int box_y = 20;
    
    Color bg_color = {0, 0, 0, (unsigned char)(180 * alpha)};
    Color border_color = GetTierColor(achievement->tier);
    border_color.a = (unsigned char)(255 * alpha);
    
    DrawRectangle(box_x, box_y, box_width, box_height, bg_color);
    DrawRectangleLines(box_x, box_y, box_width, box_height, border_color);
    
    // Draw achievement icon (simple colored square for now)
    int icon_size = 40;
    int icon_x = box_x + 10;
    int icon_y = box_y + 20;
    Color icon_color = GetTierColor(achievement->tier);
    icon_color.a = (unsigned char)(255 * alpha);
    DrawRectangle(icon_x, icon_y, icon_size, icon_size, icon_color);
    
    // Draw text
    Color text_color = {255, 255, 255, (unsigned char)(255 * alpha)};
    DrawText("ACHIEVEMENT UNLOCKED!", icon_x + icon_size + 10, box_y + 5, 14, text_color);
    DrawText(achievement->name, icon_x + icon_size + 10, box_y + 25, 16, text_color);
    DrawText(achievement->description, icon_x + icon_size + 10, box_y + 45, 12, text_color);
    
    // Draw points
    int points = GetAchievementPoints(achievement->tier);
    DrawText(TextFormat("+%d pts", points), box_x + box_width - 60, box_y + box_height - 20, 12, text_color);
}

// Draw achievement menu
void DrawAchievementMenu(const AchievementSystem* achievements, int selected_index) {
    if (!achievements) return;
    
    ClearBackground(BLACK);
    
    // Draw title
    DrawText("ACHIEVEMENTS", SCREEN_WIDTH / 2 - 80, 30, 24, WHITE);
    DrawText(TextFormat("Unlocked: %d/%d", achievements->unlocked_count, achievements->total_achievements), 
             SCREEN_WIDTH / 2 - 60, 60, 16, WHITE);
    
    // Calculate completion percentage
    float completion = (float)achievements->unlocked_count / achievements->total_achievements * 100.0f;
    DrawText(TextFormat("Completion: %.1f%%", completion), SCREEN_WIDTH / 2 - 50, 80, 16, WHITE);
    
    // Draw achievement list
    int start_y = 120;
    int items_per_page = 15;
    int start_index = (selected_index / items_per_page) * items_per_page;
    
    for (int i = 0; i < items_per_page && start_index + i < ACHIEVEMENT_COUNT; i++) {
        int achievement_index = start_index + i;
        const Achievement* achievement = &achievements->achievements[achievement_index];
        
        int y = start_y + i * 20;
        Color text_color = WHITE;
        Color tier_color = GetTierColor(achievement->tier);
        
        if (achievement_index == selected_index) {
            DrawRectangle(10, y - 2, SCREEN_WIDTH - 20, 18, (Color){50, 50, 50, 100});
            text_color = YELLOW;
        }
        
        if (achievement->unlocked) {
            // Draw checkmark
            DrawText("✓", 15, y, 16, GREEN);
            DrawText(achievement->name, 40, y, 14, text_color);
            DrawText(achievement->description, 250, y, 12, GRAY);
            DrawText(GetTierName(achievement->tier), SCREEN_WIDTH - 100, y, 12, tier_color);
        } else {
            // Draw progress
            float progress = GetAchievementProgress(achievements, (AchievementID)achievement_index);
            DrawText("○", 15, y, 16, GRAY);
            DrawText(achievement->name, 40, y, 14, GRAY);
            DrawText(achievement->description, 250, y, 12, DARKGRAY);
            DrawText(TextFormat("%.1f%%", progress * 100.0f), SCREEN_WIDTH - 100, y, 12, GRAY);
        }
    }
    
    // Draw navigation hint
    DrawText("Use UP/DOWN to navigate, ESC to return", 10, SCREEN_HEIGHT - 30, 14, LIGHTGRAY);
}

// Get tier color
Color GetTierColor(AchievementTier tier) {
    switch (tier) {
        case ACHIEVEMENT_BRONZE: return (Color){205, 127, 50, 255};
        case ACHIEVEMENT_SILVER: return (Color){192, 192, 192, 255};
        case ACHIEVEMENT_GOLD: return (Color){255, 215, 0, 255};
        case ACHIEVEMENT_PLATINUM: return (Color){229, 228, 226, 255};
        case ACHIEVEMENT_SPECIAL: return (Color){138, 43, 226, 255};
        default: return WHITE;
    }
}

// Get tier name
const char* GetTierName(AchievementTier tier) {
    switch (tier) {
        case ACHIEVEMENT_BRONZE: return "Bronze";
        case ACHIEVEMENT_SILVER: return "Silver";
        case ACHIEVEMENT_GOLD: return "Gold";
        case ACHIEVEMENT_PLATINUM: return "Platinum";
        case ACHIEVEMENT_SPECIAL: return "Special";
        default: return "Unknown";
    }
}

// Save achievements to file
void SaveAchievements(const AchievementSystem* achievements) {
    if (!achievements) return;

    FILE* file = fopen("achievements.dat", "wb");
    if (!file) {
        // Optionally log: fprintf(stderr, "Error: Could not open achievements.dat for writing.\n");
        return;
    }
    int version = 1;
    if (fwrite(&version, sizeof(int), 1, file) != 1) goto fail;
    for (int i = 0; i < ACHIEVEMENT_COUNT; i++) {
        if (fwrite(&achievements->achievements[i].unlocked, sizeof(bool), 1, file) != 1) goto fail;
        if (fwrite(&achievements->achievements[i].unlock_time, sizeof(time_t), 1, file) != 1) goto fail;
    }
    if (fwrite(&achievements->stats, sizeof(AchievementStats), 1, file) != 1) goto fail;
    if (fclose(file) != 0) {
        // Optionally log: fprintf(stderr, "Warning: Failed to close achievements.dat after writing.\n");
    }
    return;
fail:
    // Optionally log: fprintf(stderr, "Error: Failed to write achievements.dat.\n");
    fclose(file);
}

// Load achievements from file
void LoadAchievements(AchievementSystem* achievements) {
    if (!achievements) return;

    FILE* file = fopen("achievements.dat", "rb");
    if (!file) return;
    int version;
    if (fread(&version, sizeof(int), 1, file) != 1 || version != 1) {
        fclose(file);
        return;
    }
    for (int i = 0; i < ACHIEVEMENT_COUNT; i++) {
        if (fread(&achievements->achievements[i].unlocked, sizeof(bool), 1, file) != 1) break;
        if (fread(&achievements->achievements[i].unlock_time, sizeof(time_t), 1, file) != 1) break;
        if (achievements->achievements[i].unlocked) {
            achievements->unlocked_count++;
        }
    }
    if (fread(&achievements->stats, sizeof(AchievementStats), 1, file) != 1) {
        // Optionally log: fprintf(stderr, "Warning: Failed to read achievement stats.\n");
    }
    if (fclose(file) != 0) {
        // Optionally log: fprintf(stderr, "Warning: Failed to close achievements.dat after reading.\n");
    }
}

// Reset all achievements
void ResetAchievements(AchievementSystem* achievements) {
    if (!achievements) return;
    
    for (int i = 0; i < ACHIEVEMENT_COUNT; i++) {
        achievements->achievements[i].unlocked = false;
        achievements->achievements[i].unlock_time = 0;
    }
    
    achievements->unlocked_count = 0;
    memset(&achievements->stats, 0, sizeof(AchievementStats));
    achievements->stats.weapons_unlocked = 1; // Start with one weapon
    
    SaveAchievements(achievements);
}

// Get total achievement score
int GetTotalAchievementScore(const AchievementSystem* achievements) {
    if (!achievements) return 0;
    
    int total = 0;
    for (int i = 0; i < ACHIEVEMENT_COUNT; i++) {
        if (achievements->achievements[i].unlocked) {
            total += GetAchievementPoints(achievements->achievements[i].tier);
        }
    }
    return total;
}

// Check if player is eligible for special rewards
bool IsEligibleForSpecialReward(const AchievementSystem* achievements, SpecialRewardType reward) {
    if (!achievements) return false;
    
    switch (reward) {
        case REWARD_WEAPON_UNLOCK:
            return achievements->unlocked_count >= 5;
        case REWARD_LIFE_BONUS:
            return achievements->unlocked_count >= 10;
        case REWARD_SCORE_MULTIPLIER:
            return achievements->unlocked_count >= 15;
        case REWARD_SPECIAL_ABILITY:
            return achievements->unlocked_count >= 20;
        default:
            return false;
    }
}