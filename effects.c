#include "game.h"
#include <math.h>
#include <stdlib.h>

// =============================================================================
// PARTICLE SYSTEM
// =============================================================================

// Initialize particle system
void InitParticleSystem(ParticleSystem* system) {
    if (!system) return;
    
    for (int i = 0; i < MAX_PARTICLES; i++) {
        system->particles[i].active = false;
        system->particles[i].position = (Vector2){0, 0};
        system->particles[i].velocity = (Vector2){0, 0};
        system->particles[i].color = WHITE;
        system->particles[i].size = 1.0f;
        system->particles[i].life = 0.0f;
        system->particles[i].max_life = 1.0f;
        system->particles[i].type = PARTICLE_SPARK;
    }
    
    system->screen_shake_intensity = 0.0f;
    system->screen_shake_duration = 0.0f;
    system->screen_offset = (Vector2){0, 0};
    system->flash_intensity = 0.0f;
    system->flash_duration = 0.0f;
    system->flash_color = WHITE;
}

// Spawn particle
void SpawnParticle(ParticleSystem* system, Vector2 position, Vector2 velocity, 
                   Color color, float size, float life, ParticleType type) {
    if (!system) return;
    
    // Find inactive particle
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!system->particles[i].active) {
            system->particles[i].active = true;
            system->particles[i].position = position;
            system->particles[i].velocity = velocity;
            system->particles[i].color = color;
            system->particles[i].size = size;
            system->particles[i].life = life;
            system->particles[i].max_life = life;
            system->particles[i].type = type;
            break;
        }
    }
}

// Create explosion effect
void CreateExplosion(ParticleSystem* system, Vector2 position, Color color, int particle_count) {
    if (!system) return;
    
    for (int i = 0; i < particle_count; i++) {
        float angle = (float)i / particle_count * TWO_PI;
        float speed = 50.0f + (rand() % 100);
        Vector2 velocity = {
            cosf(angle) * speed,
            sinf(angle) * speed
        };
        
        Color particle_color = color;
        particle_color.a = 200 + (rand() % 55);
        
        float size = 2.0f + (rand() % 4);
        float life = 0.5f + (rand() % 100) / 200.0f;
        
        SpawnParticle(system, position, velocity, particle_color, size, life, PARTICLE_EXPLOSION);
    }
    
    // Add some sparks
    for (int i = 0; i < particle_count / 2; i++) {
        float angle = (float)rand() / RAND_MAX * TWO_PI;
        float speed = 100.0f + (rand() % 150);
        Vector2 velocity = {
            cosf(angle) * speed,
            sinf(angle) * speed
        };
        
        SpawnParticle(system, position, velocity, YELLOW, 1.0f, 0.3f, PARTICLE_SPARK);
    }
}

// Create bullet trail
void CreateBulletTrail(ParticleSystem* system, Vector2 position, Vector2 velocity, Color color) {
    if (!system) return;
    
    Vector2 trail_velocity = {
        velocity.x * -0.2f + (rand() % 20 - 10),
        velocity.y * -0.2f + (rand() % 20 - 10)
    };
    
    Color trail_color = color;
    trail_color.a = 100;
    
    SpawnParticle(system, position, trail_velocity, trail_color, 1.5f, 0.2f, PARTICLE_TRAIL);
}

// Create power-up effect
void CreatePowerUpEffect(ParticleSystem* system, Vector2 position) {
    if (!system) return;
    
    for (int i = 0; i < 20; i++) {
        float angle = (float)i / 20.0f * TWO_PI;
        float speed = 30.0f + (rand() % 40);
        Vector2 velocity = {
            cosf(angle) * speed,
            sinf(angle) * speed
        };
        
        Color colors[] = {GOLD, YELLOW, ORANGE};
        Color color = colors[rand() % 3];
        
        SpawnParticle(system, position, velocity, color, 2.0f, 1.0f, PARTICLE_SPARKLE);
    }
}

// Update particle system
void UpdateParticleSystem(ParticleSystem* system, float delta) {
    if (!system) return;
    
    // Update particles
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!system->particles[i].active) continue;
        
        Particle* p = &system->particles[i];
        
        // Update position
        p->position.x += p->velocity.x * delta;
        p->position.y += p->velocity.y * delta;
        
        // Update life
        p->life -= delta;
        if (p->life <= 0.0f) {
            p->active = false;
            continue;
        }
        
        // Apply effects based on type
        float life_ratio = p->life / p->max_life;
        
        switch (p->type) {
            case PARTICLE_EXPLOSION:
                // Fade out and slow down
                p->color.a = (unsigned char)(255 * life_ratio);
                p->velocity.x *= 0.95f;
                p->velocity.y *= 0.95f;
                p->velocity.y += 50.0f * delta; // Gravity
                break;
                
            case PARTICLE_SPARK:
                // Fast fade with gravity
                p->color.a = (unsigned char)(255 * life_ratio);
                p->velocity.y += 100.0f * delta; // Gravity
                break;
                
            case PARTICLE_TRAIL:
                // Quick fade
                p->color.a = (unsigned char)(100 * life_ratio);
                p->size *= 0.98f;
                break;
                
            case PARTICLE_SPARKLE:
                // Oscillating alpha
                p->color.a = (unsigned char)(255 * life_ratio * (0.5f + 0.5f * sinf(p->life * 10.0f)));
                p->velocity.y -= 20.0f * delta; // Float upward
                break;
                
            case PARTICLE_SMOKE:
                // Expand and fade
                p->color.a = (unsigned char)(128 * life_ratio);
                p->size += 2.0f * delta;
                p->velocity.y -= 10.0f * delta; // Float upward
                break;
        }
    }
    
    // Update screen shake
    if (system->screen_shake_duration > 0.0f) {
        system->screen_shake_duration -= delta;
        
        if (system->screen_shake_duration <= 0.0f) {
            system->screen_offset = (Vector2){0, 0};
        } else {
            float intensity = system->screen_shake_intensity * (system->screen_shake_duration / 0.2f);
            system->screen_offset.x = (rand() % (int)(intensity * 2)) - intensity;
            system->screen_offset.y = (rand() % (int)(intensity * 2)) - intensity;
        }
    }
    
    // Update screen flash
    if (system->flash_duration > 0.0f) {
        system->flash_duration -= delta;
        if (system->flash_duration <= 0.0f) {
            system->flash_intensity = 0.0f;
        } else {
            system->flash_intensity = system->flash_duration / 0.1f; // 0.1s flash duration
        }
    }
}

// Trigger screen shake
void TriggerScreenShake(ParticleSystem* system, float intensity, float duration) {
    if (!system) return;
    
    system->screen_shake_intensity = intensity;
    system->screen_shake_duration = duration;
}

// Trigger screen flash
void TriggerScreenFlash(ParticleSystem* system, Color color, float duration) {
    if (!system) return;
    
    system->flash_color = color;
    system->flash_duration = duration;
    system->flash_intensity = 1.0f;
}

// Draw particle system
void DrawParticleSystem(const ParticleSystem* system) {
    if (!system) return;
    
    // Draw particles
    for (int i = 0; i < MAX_PARTICLES; i++) {
        if (!system->particles[i].active) continue;
        
        const Particle* p = &system->particles[i];
        
        switch (p->type) {
            case PARTICLE_EXPLOSION:
            case PARTICLE_SPARK:
            case PARTICLE_SPARKLE:
                DrawCircleV(p->position, p->size, p->color);
                break;
                
            case PARTICLE_TRAIL:
                DrawCircleV(p->position, p->size * 0.5f, p->color);
                break;
                
            case PARTICLE_SMOKE:
                DrawCircleV(p->position, p->size, p->color);
                // Add smoke ring
                DrawCircleLinesV(p->position, p->size + 1.0f, (Color){p->color.r, p->color.g, p->color.b, p->color.a / 2});
                break;
        }
    }
    
    // Draw screen flash overlay
    if (system->flash_intensity > 0.0f) {
        Color flash_color = system->flash_color;
        flash_color.a = (unsigned char)(flash_color.a * system->flash_intensity);
        DrawRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, flash_color);
    }
}

// =============================================================================
// ENHANCED VISUAL EFFECTS
// =============================================================================

// Draw enhanced bullet with trail effect
void DrawEnhancedBullet(Vector2 position, Color color, bool is_player_bullet, float trail_alpha) {
    if (is_player_bullet) {
        // Player bullets: bright with energy effect
        float bullet_length = 12.0f;
        float bullet_width = 4.0f;
        
        // Draw energy trail
        for (int i = 0; i < 3; i++) {
            Color trail_color = color;
            trail_color.a = (unsigned char)(trail_alpha * (100 - i * 30));
            DrawRectangle(
                (int)(position.x - bullet_width / 2.0f),
                (int)(position.y - bullet_length / 2.0f + i * 2),
                (int)bullet_width,
                (int)(bullet_length - i * 2),
                trail_color
            );
        }
        
        // Draw main body
        DrawRectangle(
            (int)(position.x - bullet_width / 2.0f),
            (int)(position.y - bullet_length / 2.0f),
            (int)bullet_width,
            (int)bullet_length,
            color
        );
        
        // Draw bright tip
        DrawRectangle(
            (int)(position.x - bullet_width / 4.0f),
            (int)(position.y - bullet_length / 2.0f),
            (int)(bullet_width / 2.0f),
            (int)(bullet_length / 3.0f),
            WHITE
        );
        
        // Draw glow effect
        DrawCircle((int)position.x, (int)position.y, bullet_width + 2, (Color){color.r, color.g, color.b, 40});
        
    } else {
        // Enemy bullets: menacing with dark energy
        float bullet_length = 10.0f;
        float bullet_width = 3.0f;
        
        // Draw dark energy trail
        Color dark_trail = {color.r / 2, color.g / 2, color.b / 2, (unsigned char)(trail_alpha * 60)};
        DrawRectangle(
            (int)(position.x - bullet_width / 2.0f),
            (int)(position.y - bullet_length / 2.0f + 2),
            (int)bullet_width,
            (int)(bullet_length + 4),
            dark_trail
        );
        
        // Draw main body
        DrawRectangle(
            (int)(position.x - bullet_width / 2.0f),
            (int)(position.y - bullet_length / 2.0f),
            (int)bullet_width,
            (int)bullet_length,
            color
        );
        
        // Draw menacing glow
        DrawCircle((int)position.x, (int)position.y, bullet_width + 1, (Color){255, 100, 100, 30});
    }
}

// Draw enhanced enemy with visual effects
void DrawEnhancedEnemy(const Enemy* enemy, float pulse_factor) {
    if (!enemy || !enemy->active) return;
    
    Color enemy_color = GREEN;
    float enemy_size = ENEMY_SIZE;
    
    switch (enemy->type) {
        case BOSS:
            enemy_color = PURPLE;
            enemy_size = BOSS_SIZE;
            break;
        case ESCORT:
            enemy_color = ORANGE;
            break;
        case FLAGSHIP:
            enemy_color = GOLD;
            enemy_size = FLAGSHIP_SIZE;
            break;
        case HOSTILE_SHIP:
            enemy_color = MAROON;
            break;
        default:
            enemy_color = GREEN;
            break;
    }
    
    // Add pulsing effect based on health
    if (enemy->health == 1) {
        float pulse = 0.8f + 0.2f * pulse_factor;
        enemy_color.r = (unsigned char)(enemy_color.r * pulse);
        enemy_color.g = (unsigned char)(enemy_color.g * pulse);
        enemy_color.b = (unsigned char)(enemy_color.b * pulse);
    }
    
    // Draw morphing effect
    if (enemy->state == MORPHING) {
        float morph_progress = 1.0f - (enemy->morph_timer / MORPH_DURATION);
        enemy_color.a = (unsigned char)(255 * (0.5f + 0.5f * sinf(morph_progress * PI * 8.0f)));
        
        // Draw morphing energy rings
        for (int i = 0; i < 3; i++) {
            float ring_radius = enemy_size / 2.0f + i * 5.0f + morph_progress * 10.0f;
            Color ring_color = {255, 255, 0, (unsigned char)(100 - i * 30)};
            DrawCircleLinesV(enemy->position, ring_radius, ring_color);
        }
    }
    
    // Draw enemy body with enhanced visuals
    DrawCircleV(enemy->position, enemy_size / 2.0f + 1, (Color){0, 0, 0, 100}); // Shadow
    DrawCircleV(enemy->position, enemy_size / 2.0f, enemy_color);
    
    // Draw health indicator for damaged enemies
    if (enemy->health > 1) {
        float health_ratio = (float)enemy->health / 
            ((enemy->type == BOSS) ? 5 : (enemy->type == ESCORT) ? 2 : 1);
        Color health_color = (health_ratio > 0.5f) ? GREEN : (health_ratio > 0.25f) ? YELLOW : RED;
        
        DrawRectangle(
            (int)(enemy->position.x - enemy_size / 2.0f),
            (int)(enemy->position.y - enemy_size / 2.0f - 8),
            (int)(enemy_size * health_ratio),
            3,
            health_color
        );
        DrawRectangleLines(
            (int)(enemy->position.x - enemy_size / 2.0f),
            (int)(enemy->position.y - enemy_size / 2.0f - 8),
            (int)enemy_size,
            3,
            WHITE
        );
    }
    
    // Draw tractor beam with enhanced visuals
    if (enemy->tractor_active) {
        // Rotating beam effect
        for (int i = 0; i < 8; i++) {
            float angle = enemy->tractor_angle + (i * PI / 4.0f);
            Vector2 beam_end = {
                enemy->tractor_center.x + cosf(angle) * TRACTOR_BEAM_RANGE,
                enemy->tractor_center.y + sinf(angle) * TRACTOR_BEAM_RANGE
            };
            
            Color beam_color = {255, 255, 0, (unsigned char)(50 + 30 * sinf(enemy->tractor_angle * 2.0f))};
            DrawLineEx(enemy->tractor_center, beam_end, 2.0f, beam_color);
        }
        
        // Central beam circle
        DrawCircleLinesV(enemy->tractor_center, TRACTOR_BEAM_RANGE, YELLOW);
        DrawCircleLinesV(enemy->tractor_center, TRACTOR_BEAM_RANGE * 0.7f, (Color){255, 255, 0, 100});
    }
    
    // Draw AI behavior indicator (debug)
    #ifdef DEBUG_AI
    const char* behavior_names[] = {
        "FORM", "AGGR", "FLNK", "SWRM", "EVAD", "CORD", "DEFN"
    };
    if (enemy->ai_behavior < 7) {
        DrawText(behavior_names[enemy->ai_behavior], 
                (int)(enemy->position.x - 10), 
                (int)(enemy->position.y + enemy_size / 2.0f + 5), 
                10, WHITE);
    }
    #endif
}

// Create hit effect
void CreateHitEffect(ParticleSystem* system, Vector2 position, bool is_enemy_hit) {
    if (!system) return;
    
    if (is_enemy_hit) {
        // Enemy hit - orange/red sparks
        for (int i = 0; i < 8; i++) {
            float angle = (float)rand() / RAND_MAX * TWO_PI;
            float speed = 30.0f + (rand() % 50);
            Vector2 velocity = {cosf(angle) * speed, sinf(angle) * speed};
            
            Color colors[] = {ORANGE, RED, YELLOW};
            Color color = colors[rand() % 3];
            
            SpawnParticle(system, position, velocity, color, 2.0f, 0.3f, PARTICLE_SPARK);
        }
        
        // Screen shake for enemy hits
        TriggerScreenShake(system, 2.0f, 0.1f);
    } else {
        // Player hit - blue/white sparks with stronger effects
        for (int i = 0; i < 15; i++) {
            float angle = (float)rand() / RAND_MAX * TWO_PI;
            float speed = 50.0f + (rand() % 100);
            Vector2 velocity = {cosf(angle) * speed, sinf(angle) * speed};
            
            Color colors[] = {SKYBLUE, WHITE, BLUE};
            Color color = colors[rand() % 3];
            
            SpawnParticle(system, position, velocity, color, 3.0f, 0.5f, PARTICLE_SPARK);
        }
        
        // Strong screen shake and flash for player hits
        TriggerScreenShake(system, 8.0f, 0.3f);
        TriggerScreenFlash(system, (Color){255, 100, 100, 100}, 0.2f);
    }
}