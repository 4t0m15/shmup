#include "game.h"
#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// =============================================================================
// SHADER SYSTEM
// =============================================================================

// Vertex shader source for post-processing effects
static const char* vertex_shader_src = 
    "#version 330\n"
    "in vec3 vertexPosition;\n"
    "in vec2 vertexTexCoord;\n"
    "in vec3 vertexNormal;\n"
    "in vec4 vertexColor;\n"
    "uniform mat4 mvp;\n"
    "out vec2 fragTexCoord;\n"
    "out vec4 fragColor;\n"
    "void main()\n"
    "{\n"
    "    fragTexCoord = vertexTexCoord;\n"
    "    fragColor = vertexColor;\n"
    "    gl_Position = mvp*vec4(vertexPosition, 1.0);\n"
    "}\n";

// Fragment shader for screen distortion effect
static const char* distortion_fragment_src = 
    "#version 330\n"
    "in vec2 fragTexCoord;\n"
    "in vec4 fragColor;\n"
    "uniform sampler2D texture0;\n"
    "uniform float time;\n"
    "uniform float intensity;\n"
    "uniform vec2 screenSize;\n"
    "out vec4 finalColor;\n"
    "void main()\n"
    "{\n"
    "    vec2 uv = fragTexCoord;\n"
    "    vec2 center = vec2(0.5, 0.5);\n"
    "    float dist = distance(uv, center);\n"
    "    float distortion = sin(dist * 20.0 + time * 5.0) * intensity * 0.01;\n"
    "    vec2 direction = normalize(uv - center);\n"
    "    vec2 distortedUV = uv + direction * distortion;\n"
    "    finalColor = texture(texture0, distortedUV) * fragColor;\n"
    "}\n";

// Fragment shader for chromatic aberration
static const char* chromatic_fragment_src = 
    "#version 330\n"
    "in vec2 fragTexCoord;\n"
    "in vec4 fragColor;\n"
    "uniform sampler2D texture0;\n"
    "uniform float intensity;\n"
    "uniform vec2 screenSize;\n"
    "out vec4 finalColor;\n"
    "void main()\n"
    "{\n"
    "    vec2 uv = fragTexCoord;\n"
    "    vec2 center = vec2(0.5, 0.5);\n"
    "    vec2 offset = (uv - center) * intensity * 0.01;\n"
    "    float r = texture(texture0, uv + offset).r;\n"
    "    float g = texture(texture0, uv).g;\n"
    "    float b = texture(texture0, uv - offset).b;\n"
    "    finalColor = vec4(r, g, b, 1.0) * fragColor;\n"
    "}\n";

// Fragment shader for bloom effect
static const char* bloom_fragment_src = 
    "#version 330\n"
    "in vec2 fragTexCoord;\n"
    "in vec4 fragColor;\n"
    "uniform sampler2D texture0;\n"
    "uniform float threshold;\n"
    "uniform float intensity;\n"
    "out vec4 finalColor;\n"
    "void main()\n"
    "{\n"
    "    vec4 color = texture(texture0, fragTexCoord);\n"
    "    float brightness = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));\n"
    "    if(brightness > threshold)\n"
    "    {\n"
    "        finalColor = color * intensity;\n"
    "    }\n"
    "    else\n"
    "    {\n"
    "        finalColor = vec4(0.0, 0.0, 0.0, 1.0);\n"
    "    }\n"
    "}\n";

// Fragment shader for blur effect
static const char* blur_fragment_src = 
    "#version 330\n"
    "in vec2 fragTexCoord;\n"
    "in vec4 fragColor;\n"
    "uniform sampler2D texture0;\n"
    "uniform vec2 direction;\n"
    "uniform float strength;\n"
    "uniform vec2 screenSize;\n"
    "out vec4 finalColor;\n"
    "void main()\n"
    "{\n"
    "    vec2 texelSize = 1.0 / screenSize;\n"
    "    vec4 color = vec4(0.0);\n"
    "    float total = 0.0;\n"
    "    for(int i = -4; i <= 4; i++)\n"
    "    {\n"
    "        float weight = exp(-float(i*i) / (2.0 * strength * strength));\n"
    "        color += texture(texture0, fragTexCoord + direction * texelSize * float(i)) * weight;\n"
    "        total += weight;\n"
    "    }\n"
    "    finalColor = color / total;\n"
    "}\n";

// Fragment shader for energy field effect
static const char* energy_field_fragment_src = 
    "#version 330\n"
    "in vec2 fragTexCoord;\n"
    "in vec4 fragColor;\n"
    "uniform sampler2D texture0;\n"
    "uniform float time;\n"
    "uniform vec2 screenSize;\n"
    "out vec4 finalColor;\n"
    "void main()\n"
    "{\n"
    "    vec2 uv = fragTexCoord;\n"
    "    vec4 color = texture(texture0, uv);\n"
    "    \n"
    "    // Create energy field pattern\n"
    "    float field1 = sin(uv.x * 50.0 + time * 2.0) * sin(uv.y * 30.0 + time * 1.5);\n"
    "    float field2 = cos(uv.x * 40.0 - time * 1.8) * cos(uv.y * 35.0 - time * 2.2);\n"
    "    float energy = (field1 + field2) * 0.1;\n"
    "    \n"
    "    // Add energy glow\n"
    "    vec3 energyColor = vec3(0.0, 0.8, 1.0) * energy;\n"
    "    finalColor = color + vec4(energyColor, 0.0);\n"
    "}\n";

// Initialize shader system
void InitShaderSystem(ShaderSystem* shaders) {
    if (!shaders) return;
    
    // Initialize all shaders as invalid
    for (int i = 0; i < SHADER_COUNT; i++) {
        shaders->shaders[i].id = 0;
        shaders->shader_loaded[i] = false;
    }
    
    shaders->post_process_enabled = true;
    shaders->bloom_enabled = true;
    shaders->chromatic_aberration_enabled = false;
    shaders->distortion_enabled = false;
    shaders->energy_field_enabled = false;
    
    shaders->bloom_threshold = 0.8f;
    shaders->bloom_intensity = 1.5f;
    shaders->chromatic_intensity = 0.5f;
    shaders->distortion_intensity = 0.3f;
    shaders->blur_strength = 2.0f;
    
    shaders->screen_texture = (RenderTexture2D){0};
    shaders->bloom_texture = (RenderTexture2D){0};
    shaders->temp_texture = (RenderTexture2D){0};
    
    // Load shaders
    LoadAllShaders(shaders);
    
    // Create render textures
    CreateShaderTextures(shaders);
}

// Load all shaders
void LoadAllShaders(ShaderSystem* shaders) {
    if (!shaders) return;
    
    // Load distortion shader
    shaders->shaders[SHADER_DISTORTION] = LoadShaderFromMemory(vertex_shader_src, distortion_fragment_src);
    if (shaders->shaders[SHADER_DISTORTION].id > 0) {
        shaders->shader_loaded[SHADER_DISTORTION] = true;
        shaders->time_loc[SHADER_DISTORTION] = GetShaderLocation(shaders->shaders[SHADER_DISTORTION], "time");
        shaders->intensity_loc[SHADER_DISTORTION] = GetShaderLocation(shaders->shaders[SHADER_DISTORTION], "intensity");
        shaders->screen_size_loc[SHADER_DISTORTION] = GetShaderLocation(shaders->shaders[SHADER_DISTORTION], "screenSize");
    }
    
    // Load chromatic aberration shader
    shaders->shaders[SHADER_CHROMATIC] = LoadShaderFromMemory(vertex_shader_src, chromatic_fragment_src);
    if (shaders->shaders[SHADER_CHROMATIC].id > 0) {
        shaders->shader_loaded[SHADER_CHROMATIC] = true;
        shaders->intensity_loc[SHADER_CHROMATIC] = GetShaderLocation(shaders->shaders[SHADER_CHROMATIC], "intensity");
        shaders->screen_size_loc[SHADER_CHROMATIC] = GetShaderLocation(shaders->shaders[SHADER_CHROMATIC], "screenSize");
    }
    
    // Load bloom shader
    shaders->shaders[SHADER_BLOOM] = LoadShaderFromMemory(vertex_shader_src, bloom_fragment_src);
    if (shaders->shaders[SHADER_BLOOM].id > 0) {
        shaders->shader_loaded[SHADER_BLOOM] = true;
        shaders->threshold_loc[SHADER_BLOOM] = GetShaderLocation(shaders->shaders[SHADER_BLOOM], "threshold");
        shaders->intensity_loc[SHADER_BLOOM] = GetShaderLocation(shaders->shaders[SHADER_BLOOM], "intensity");
    }
    
    // Load blur shader
    shaders->shaders[SHADER_BLUR] = LoadShaderFromMemory(vertex_shader_src, blur_fragment_src);
    if (shaders->shaders[SHADER_BLUR].id > 0) {
        shaders->shader_loaded[SHADER_BLUR] = true;
        shaders->direction_loc[SHADER_BLUR] = GetShaderLocation(shaders->shaders[SHADER_BLUR], "direction");
        shaders->strength_loc[SHADER_BLUR] = GetShaderLocation(shaders->shaders[SHADER_BLUR], "strength");
        shaders->screen_size_loc[SHADER_BLUR] = GetShaderLocation(shaders->shaders[SHADER_BLUR], "screenSize");
    }
    
    // Load energy field shader
    shaders->shaders[SHADER_ENERGY_FIELD] = LoadShaderFromMemory(vertex_shader_src, energy_field_fragment_src);
    if (shaders->shaders[SHADER_ENERGY_FIELD].id > 0) {
        shaders->shader_loaded[SHADER_ENERGY_FIELD] = true;
        shaders->time_loc[SHADER_ENERGY_FIELD] = GetShaderLocation(shaders->shaders[SHADER_ENERGY_FIELD], "time");
        shaders->screen_size_loc[SHADER_ENERGY_FIELD] = GetShaderLocation(shaders->shaders[SHADER_ENERGY_FIELD], "screenSize");
    }
}

// Create render textures for shader effects
void CreateShaderTextures(ShaderSystem* shaders) {
    if (!shaders) return;
    
    shaders->screen_texture = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    shaders->bloom_texture = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
    shaders->temp_texture = LoadRenderTexture(SCREEN_WIDTH, SCREEN_HEIGHT);
}

// Update shader system
void UpdateShaderSystem(ShaderSystem* shaders, float delta_time) {
    if (!shaders) return;
    
    shaders->shader_time += delta_time;
    
    // Update shader uniforms
    Vector2 screen_size = {(float)SCREEN_WIDTH, (float)SCREEN_HEIGHT};
    
    for (int i = 0; i < SHADER_COUNT; i++) {
        if (!shaders->shader_loaded[i]) continue;
        
        // Update time uniform
        if (shaders->time_loc[i] != -1) {
            SetShaderValue(shaders->shaders[i], shaders->time_loc[i], &shaders->shader_time, SHADER_UNIFORM_FLOAT);
        }
        
        // Update screen size uniform
        if (shaders->screen_size_loc[i] != -1) {
            SetShaderValue(shaders->shaders[i], shaders->screen_size_loc[i], &screen_size, SHADER_UNIFORM_VEC2);
        }
    }
    
    // Update specific shader parameters
    if (shaders->shader_loaded[SHADER_DISTORTION]) {
        SetShaderValue(shaders->shaders[SHADER_DISTORTION], shaders->intensity_loc[SHADER_DISTORTION], 
                      &shaders->distortion_intensity, SHADER_UNIFORM_FLOAT);
    }
    
    if (shaders->shader_loaded[SHADER_CHROMATIC]) {
        SetShaderValue(shaders->shaders[SHADER_CHROMATIC], shaders->intensity_loc[SHADER_CHROMATIC], 
                      &shaders->chromatic_intensity, SHADER_UNIFORM_FLOAT);
    }
    
    if (shaders->shader_loaded[SHADER_BLOOM]) {
        SetShaderValue(shaders->shaders[SHADER_BLOOM], shaders->threshold_loc[SHADER_BLOOM], 
                      &shaders->bloom_threshold, SHADER_UNIFORM_FLOAT);
        SetShaderValue(shaders->shaders[SHADER_BLOOM], shaders->intensity_loc[SHADER_BLOOM], 
                      &shaders->bloom_intensity, SHADER_UNIFORM_FLOAT);
    }
    
    if (shaders->shader_loaded[SHADER_BLUR]) {
        SetShaderValue(shaders->shaders[SHADER_BLUR], shaders->strength_loc[SHADER_BLUR], 
                      &shaders->blur_strength, SHADER_UNIFORM_FLOAT);
    }
}

// Begin shader rendering
void BeginGameShaderMode(ShaderSystem* shaders) {
    if (!shaders || !shaders->post_process_enabled) return;
    
    BeginTextureMode(shaders->screen_texture);
}

// End shader rendering and apply effects
void EndGameShaderMode(ShaderSystem* shaders) {
    if (!shaders || !shaders->post_process_enabled) return;
    
    EndTextureMode();
    
    // Apply post-processing effects
    ApplyPostProcessing(shaders);
}

// Apply post-processing effects
void ApplyPostProcessing(ShaderSystem* shaders) {
    if (!shaders) return;
    
    Texture2D current_texture = shaders->screen_texture.texture;
    
    // Apply bloom effect
    if (shaders->bloom_enabled && shaders->shader_loaded[SHADER_BLOOM]) {
        current_texture = ApplyBloomEffect(shaders, current_texture);
    }
    
    // Apply chromatic aberration
    if (shaders->chromatic_aberration_enabled && shaders->shader_loaded[SHADER_CHROMATIC]) {
        current_texture = ApplyChromaticAberration(shaders, current_texture);
    }
    
    // Apply distortion
    if (shaders->distortion_enabled && shaders->shader_loaded[SHADER_DISTORTION]) {
        current_texture = ApplyDistortion(shaders, current_texture);
    }
    
    // Apply energy field effect
    if (shaders->energy_field_enabled && shaders->shader_loaded[SHADER_ENERGY_FIELD]) {
        current_texture = ApplyEnergyField(shaders, current_texture);
    }
    
    // Draw final result to screen
    Rectangle source = {0, 0, (float)current_texture.width, -(float)current_texture.height};
    Rectangle dest = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    DrawTexturePro(current_texture, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
}

// Apply bloom effect
Texture2D ApplyBloomEffect(ShaderSystem* shaders, Texture2D input_texture) {
    if (!shaders || !shaders->shader_loaded[SHADER_BLOOM] || !shaders->shader_loaded[SHADER_BLUR]) {
        return input_texture;
    }
    
    // Extract bright areas
    BeginTextureMode(shaders->bloom_texture);
    BeginShaderMode(shaders->shaders[SHADER_BLOOM]);
    Rectangle source = {0, 0, (float)input_texture.width, -(float)input_texture.height};
    Rectangle dest = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    DrawTexturePro(input_texture, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
    EndShaderMode();
    EndTextureMode();
    
    // Blur the bright areas (horizontal)
    BeginTextureMode(shaders->temp_texture);
    BeginShaderMode(shaders->shaders[SHADER_BLUR]);
    Vector2 horizontal_dir = {1.0f, 0.0f};
    SetShaderValue(shaders->shaders[SHADER_BLUR], shaders->direction_loc[SHADER_BLUR], 
                  &horizontal_dir, SHADER_UNIFORM_VEC2);
    source = (Rectangle){0, 0, (float)shaders->bloom_texture.texture.width, -(float)shaders->bloom_texture.texture.height};
    DrawTexturePro(shaders->bloom_texture.texture, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
    EndShaderMode();
    EndTextureMode();
    
    // Blur the bright areas (vertical)
    BeginTextureMode(shaders->bloom_texture);
    BeginShaderMode(shaders->shaders[SHADER_BLUR]);
    Vector2 vertical_dir = {0.0f, 1.0f};
    SetShaderValue(shaders->shaders[SHADER_BLUR], shaders->direction_loc[SHADER_BLUR], 
                  &vertical_dir, SHADER_UNIFORM_VEC2);
    source = (Rectangle){0, 0, (float)shaders->temp_texture.texture.width, -(float)shaders->temp_texture.texture.height};
    DrawTexturePro(shaders->temp_texture.texture, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
    EndShaderMode();
    EndTextureMode();
    
    // Combine original with bloom
    BeginTextureMode(shaders->temp_texture);
    ClearBackground(BLANK);
    // Draw original
    source = (Rectangle){0, 0, (float)input_texture.width, -(float)input_texture.height};
    DrawTexturePro(input_texture, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
    // Add bloom with additive blending
    BeginBlendMode(BLEND_ADDITIVE);
    source = (Rectangle){0, 0, (float)shaders->bloom_texture.texture.width, -(float)shaders->bloom_texture.texture.height};
    DrawTexturePro(shaders->bloom_texture.texture, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
    EndBlendMode();
    EndTextureMode();
    
    return shaders->temp_texture.texture;
}

// Apply chromatic aberration
Texture2D ApplyChromaticAberration(ShaderSystem* shaders, Texture2D input_texture) {
    if (!shaders || !shaders->shader_loaded[SHADER_CHROMATIC]) {
        return input_texture;
    }
    
    BeginTextureMode(shaders->temp_texture);
    BeginShaderMode(shaders->shaders[SHADER_CHROMATIC]);
    Rectangle source = {0, 0, (float)input_texture.width, -(float)input_texture.height};
    Rectangle dest = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    DrawTexturePro(input_texture, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
    EndShaderMode();
    EndTextureMode();
    
    return shaders->temp_texture.texture;
}

// Apply distortion effect
Texture2D ApplyDistortion(ShaderSystem* shaders, Texture2D input_texture) {
    if (!shaders || !shaders->shader_loaded[SHADER_DISTORTION]) {
        return input_texture;
    }
    
    BeginTextureMode(shaders->temp_texture);
    BeginShaderMode(shaders->shaders[SHADER_DISTORTION]);
    Rectangle source = {0, 0, (float)input_texture.width, -(float)input_texture.height};
    Rectangle dest = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    DrawTexturePro(input_texture, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
    EndShaderMode();
    EndTextureMode();
    
    return shaders->temp_texture.texture;
}

// Apply energy field effect
Texture2D ApplyEnergyField(ShaderSystem* shaders, Texture2D input_texture) {
    if (!shaders || !shaders->shader_loaded[SHADER_ENERGY_FIELD]) {
        return input_texture;
    }
    
    BeginTextureMode(shaders->temp_texture);
    BeginShaderMode(shaders->shaders[SHADER_ENERGY_FIELD]);
    Rectangle source = {0, 0, (float)input_texture.width, -(float)input_texture.height};
    Rectangle dest = {0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
    DrawTexturePro(input_texture, source, dest, (Vector2){0, 0}, 0.0f, WHITE);
    EndShaderMode();
    EndTextureMode();
    
    return shaders->temp_texture.texture;
}

// Enable/disable shader effects
void SetShaderEffect(ShaderSystem* shaders, ShaderType type, bool enabled) {
    if (!shaders) return;
    
    switch (type) {
        case SHADER_BLOOM:
            shaders->bloom_enabled = enabled;
            break;
        case SHADER_CHROMATIC:
            shaders->chromatic_aberration_enabled = enabled;
            break;
        case SHADER_DISTORTION:
            shaders->distortion_enabled = enabled;
            break;
        case SHADER_ENERGY_FIELD:
            shaders->energy_field_enabled = enabled;
            break;
        default:
            break;
    }
}

// Set shader parameters
void SetShaderParameter(ShaderSystem* shaders, ShaderType type, const char* param_name, float value) {
    if (!shaders) return;
    
    switch (type) {
        case SHADER_BLOOM:
            if (strcmp(param_name, "threshold") == 0) {
                shaders->bloom_threshold = value;
            } else if (strcmp(param_name, "intensity") == 0) {
                shaders->bloom_intensity = value;
            }
            break;
        case SHADER_CHROMATIC:
            if (strcmp(param_name, "intensity") == 0) {
                shaders->chromatic_intensity = value;
            }
            break;
        case SHADER_DISTORTION:
            if (strcmp(param_name, "intensity") == 0) {
                shaders->distortion_intensity = value;
            }
            break;
        case SHADER_BLUR:
            if (strcmp(param_name, "strength") == 0) {
                shaders->blur_strength = value;
            }
            break;
        default:
            break;
    }
}

// Trigger temporary shader effects
void TriggerShaderHitEffect(ShaderSystem* shaders) {
    if (!shaders) return;
    
    // Temporarily increase chromatic aberration
    shaders->chromatic_intensity = 2.0f;
    shaders->chromatic_aberration_enabled = true;
    
    // Start distortion effect
    shaders->distortion_intensity = 1.0f;
    shaders->distortion_enabled = true;
}

void TriggerShaderExplosionEffect(ShaderSystem* shaders) {
    if (!shaders) return;
    
    // Increase bloom and add distortion
    shaders->bloom_intensity = 3.0f;
    shaders->distortion_intensity = 2.0f;
    shaders->distortion_enabled = true;
}

void TriggerShaderPowerUpEffect(ShaderSystem* shaders) {
    if (!shaders) return;
    
    // Enable energy field effect
    shaders->energy_field_enabled = true;
}

// Reset shader effects to normal
void ResetShaderEffects(ShaderSystem* shaders, float delta_time) {
    if (!shaders) return;
    
    // Gradually reduce effect intensities
    if (shaders->chromatic_intensity > 0.5f) {
        shaders->chromatic_intensity -= delta_time * 2.0f;
        if (shaders->chromatic_intensity <= 0.5f) {
            shaders->chromatic_intensity = 0.5f;
            shaders->chromatic_aberration_enabled = false;
        }
    }
    
    if (shaders->distortion_intensity > 0.3f) {
        shaders->distortion_intensity -= delta_time * 3.0f;
        if (shaders->distortion_intensity <= 0.3f) {
            shaders->distortion_intensity = 0.3f;
            shaders->distortion_enabled = false;
        }
    }
    
    if (shaders->bloom_intensity > 1.5f) {
        shaders->bloom_intensity -= delta_time * 2.0f;
        if (shaders->bloom_intensity <= 1.5f) {
            shaders->bloom_intensity = 1.5f;
        }
    }
}

// Cleanup shader system
void CleanupShaderSystem(ShaderSystem* shaders) {
    if (!shaders) return;
    
    // Unload shaders
    for (int i = 0; i < SHADER_COUNT; i++) {
        if (shaders->shader_loaded[i]) {
            UnloadShader(shaders->shaders[i]);
            shaders->shader_loaded[i] = false;
        }
    }
    
    // Unload render textures
    if (shaders->screen_texture.id > 0) {
        UnloadRenderTexture(shaders->screen_texture);
    }
    if (shaders->bloom_texture.id > 0) {
        UnloadRenderTexture(shaders->bloom_texture);
    }
    if (shaders->temp_texture.id > 0) {
        UnloadRenderTexture(shaders->temp_texture);
    }
}

// Draw shader debug UI
void DrawShaderDebugUI(const ShaderSystem* shaders) {
    if (!shaders) return;
    
    int ui_x = 10;
    int ui_y = 150;
    int line_height = 20;
    
    DrawText("SHADER EFFECTS", ui_x, ui_y, 18, WHITE);
    ui_y += line_height + 5;
    
    DrawText(TextFormat("Post-Process: %s", shaders->post_process_enabled ? "ON" : "OFF"), 
             ui_x, ui_y, 14, shaders->post_process_enabled ? GREEN : RED);
    ui_y += line_height;
    
    DrawText(TextFormat("Bloom: %s (%.1f)", shaders->bloom_enabled ? "ON" : "OFF", shaders->bloom_intensity), 
             ui_x, ui_y, 14, shaders->bloom_enabled ? GREEN : RED);
    ui_y += line_height;
    
    DrawText(TextFormat("Chromatic: %s (%.1f)", shaders->chromatic_aberration_enabled ? "ON" : "OFF", shaders->chromatic_intensity), 
             ui_x, ui_y, 14, shaders->chromatic_aberration_enabled ? GREEN : RED);
    ui_y += line_height;
    
    DrawText(TextFormat("Distortion: %s (%.1f)", shaders->distortion_enabled ? "ON" : "OFF", shaders->distortion_intensity), 
             ui_x, ui_y, 14, shaders->distortion_enabled ? GREEN : RED);
    ui_y += line_height;
    
    DrawText(TextFormat("Energy Field: %s", shaders->energy_field_enabled ? "ON" : "OFF"), 
             ui_x, ui_y, 14, shaders->energy_field_enabled ? GREEN : RED);
}