#include "game.h"
#include <math.h>
#include <stdlib.h>
#include <time.h>

// Function to seed the random number generator
void SeedRandomGenerator(GameState* gameState) {
    // Seed the random number generator with current time
    // This ensures different random sequences each game
    gameState->random_seed = (unsigned int)time(NULL);
    srand(gameState->random_seed);
}

// Helper functions for movement patterns
Vector2 GameVector2Lerp(Vector2 start, Vector2 end, float t) {
    Vector2 result;
    result.x = start.x + t * (end.x - start.x);
    result.y = start.y + t * (end.y - start.y);
    return result;
}

Vector2 BezierQuad(Vector2 start, Vector2 control, Vector2 end, float t) {
    Vector2 result;
    float inv_t = 1.0f - t;
    result.x = inv_t * inv_t * start.x + 2 * inv_t * t * control.x + t * t * end.x;
    result.y = inv_t * inv_t * start.y + 2 * inv_t * t * control.y + t * t * end.y;
    return result;
}

// Enhanced movement pattern calculations with aggression scaling
Vector2 CalculateMovementPattern(Enemy* enemy, float delta) {
    Vector2 new_pos = enemy->position;
    float speed_multiplier = enemy->aggression_multiplier;
    
    switch (enemy->pattern) {
        case PATTERN_STRAIGHT: {
            // Simple straight line movement with aggression scaling
            Vector2 target = (enemy->state == ENTERING) ? enemy->formation_pos : enemy->attack_start;
            float dx = target.x - enemy->position.x;
            float dy = target.y - enemy->position.y;
            float distance = sqrtf(dx * dx + dy * dy);
            
            if (distance > 0) {
                float speed = (enemy->state == ENTERING) ? 
                    ENEMY_FORMATION_SPEED * speed_multiplier : 
                    ENEMY_ATTACK_SPEED * speed_multiplier;
                new_pos.x += (dx / distance) * speed * delta;
                new_pos.y += (dy / distance) * speed * delta;
            }
            break;
        }
        
        case PATTERN_ARC: {
            // Arc movement with faster progression for aggressive enemies
            enemy->pattern_progress += delta * 0.5f * speed_multiplier;
            if (enemy->pattern_progress > 1.0f) enemy->pattern_progress = 1.0f;
            
            Vector2 start = enemy->entry_start;
            Vector2 target = (enemy->state == ENTERING) ? enemy->formation_pos : enemy->attack_start;
            
            // Create arc with control point
            Vector2 control = {
                (start.x + target.x) / 2.0f + enemy->pattern_param * 100.0f,
                (start.y + target.y) / 2.0f - 50.0f
            };
            
            new_pos = BezierQuad(start, control, target, enemy->pattern_progress);
            break;
        }
        
        case PATTERN_SPIRAL: {
            // Faster spiral for aggressive enemies
            enemy->pattern_progress += delta * 2.0f * speed_multiplier;
            float radius = 50.0f * (1.0f - enemy->pattern_progress * 0.5f);
            float angle = enemy->pattern_progress * TWO_PI * 2.0f;
            
            Vector2 center = (enemy->state == ENTERING) ? enemy->formation_pos : enemy->attack_start;
            new_pos.x = center.x + cosf(angle) * radius;
            new_pos.y = center.y + sinf(angle) * radius + enemy->pattern_progress * 100.0f * speed_multiplier;
            break;
        }
        
        case PATTERN_SWIRL: {
            // Mirrored swirl pattern
            enemy->pattern_progress += delta * 1.5f;
            float radius = 40.0f + sinf(enemy->pattern_progress * PI) * 20.0f;
            float angle = enemy->pattern_progress * TWO_PI + enemy->pattern_param * PI;
            
            Vector2 center = (enemy->state == ENTERING) ? enemy->formation_pos : enemy->attack_start;
            new_pos.x = center.x + cosf(angle) * radius * enemy->pattern_param;
            new_pos.y = center.y + sinf(angle) * radius * 0.5f + enemy->pattern_progress * 80.0f;
            break;
        }
        
        case PATTERN_LOOP: {
            // Loop pattern for diving attacks
            enemy->pattern_progress += delta * 1.0f;
            if (enemy->pattern_progress > 1.0f) enemy->pattern_progress = 1.0f;
            
            Vector2 loop_center = {
                enemy->attack_start.x + (enemy->position.x - enemy->attack_start.x) * 0.5f,
                enemy->attack_start.y + 100.0f
            };
            
            new_pos = BezierQuad(
                loop_center,
                (Vector2){ loop_center.x + LOOP_RADIUS, loop_center.y },
                (Vector2){ loop_center.x, loop_center.y + LOOP_RADIUS },
                enemy->pattern_progress
            );
            break;
        }
        
        case PATTERN_BEAM: {
            // Straight beam attack for boss
            enemy->pattern_progress += delta * 0.8f;
            Vector2 target = { enemy->attack_start.x, SCREEN_HEIGHT + 100 };
            new_pos = GameVector2Lerp(enemy->attack_start, target, enemy->pattern_progress);
            break;
        }
        
        case PATTERN_CURVE: {
            // Curved attack pattern
            enemy->pattern_progress += delta * 1.2f;
            float curve_strength = sinf(enemy->pattern_progress * PI * 2.0f) * 80.0f;
            Vector2 target = { enemy->attack_start.x, SCREEN_HEIGHT + 50 };
            Vector2 linear = GameVector2Lerp(enemy->attack_start, target, enemy->pattern_progress);
            new_pos.x = linear.x + curve_strength * enemy->pattern_param;
            new_pos.y = linear.y;
            break;
        }
        
        case PATTERN_ZIGZAG: {
            // New zigzag pattern
            enemy->pattern_progress += delta * 1.5f * speed_multiplier;
            float zigzag_amplitude = 60.0f;
            float zigzag_frequency = 4.0f;
            
            Vector2 target = { enemy->attack_start.x, SCREEN_HEIGHT + 50 };
            Vector2 linear = GameVector2Lerp(enemy->attack_start, target, enemy->pattern_progress);
            new_pos.x = linear.x + sinf(enemy->pattern_progress * PI * zigzag_frequency) * zigzag_amplitude;
            new_pos.y = linear.y;
            break;
        }
        
        case PATTERN_SINE_WAVE: {
            // New sine wave pattern
            enemy->pattern_progress += delta * 1.0f * speed_multiplier;
            float wave_amplitude = 40.0f;
            float wave_frequency = 3.0f;
            
            Vector2 target = { enemy->attack_start.x, SCREEN_HEIGHT + 50 };
            Vector2 linear = GameVector2Lerp(enemy->attack_start, target, enemy->pattern_progress);
            new_pos.x = linear.x + sinf(enemy->pattern_progress * PI * wave_frequency) * wave_amplitude * enemy->pattern_param;
            new_pos.y = linear.y;
            break;
        }
        
        case PATTERN_FIGURE_EIGHT: {
            // New figure-eight pattern
            enemy->pattern_progress += delta * 0.8f * speed_multiplier;
            float radius = 50.0f;
            float angle = enemy->pattern_progress * TWO_PI * 2.0f;
            
            Vector2 center = (enemy->state == ENTERING) ? enemy->formation_pos : enemy->attack_start;
            new_pos.x = center.x + sinf(angle) * radius;
            new_pos.y = center.y + sinf(angle * 2.0f) * radius * 0.5f + enemy->pattern_progress * 80.0f;
            break;
        }
    }
    
    return new_pos;
}