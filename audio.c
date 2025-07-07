#include "game.h"
#include <raylib.h>
#include <math.h>
#include <stdlib.h>

// =============================================================================
// AUDIO SYSTEM
// =============================================================================

// Initialize audio system
void InitAudioSystem(AudioSystem* audio) {
    if (!audio) return;
    
    InitAudioDevice();
    
    // Initialize sound slots
    for (int i = 0; i < MAX_SOUNDS; i++) {
        audio->sounds[i].loaded = false;
        audio->sounds[i].volume = 1.0f;
        audio->sounds[i].pitch = 1.0f;
    }
    
    // Initialize music slots
    for (int i = 0; i < MAX_MUSIC; i++) {
        audio->music[i].loaded = false;
        audio->music[i].volume = 1.0f;
    }
    
    audio->master_volume = 1.0f;
    audio->sfx_volume = 1.0f;
    audio->music_volume = 1.0f;
    audio->current_music = -1;
    audio->fade_timer = 0.0f;
    audio->fade_duration = 0.0f;
    audio->fade_target_volume = 1.0f;
}

// Cleanup audio system
void CleanupAudioSystem(AudioSystem* audio) {
    if (!audio) return;
    
    // Stop current music before cleanup
    if (audio->current_music >= 0 && audio->current_music < MAX_MUSIC) {
        StopMusicStream(audio->music[audio->current_music].music);
    }
    
    // Unload all sounds
    for (int i = 0; i < MAX_SOUNDS; i++) {
        if (audio->sounds[i].loaded) {
            UnloadSound(audio->sounds[i].sound);
            audio->sounds[i].loaded = false;
        }
    }
    
    // Unload all music (this properly frees the wave data)
    for (int i = 0; i < MAX_MUSIC; i++) {
        if (audio->music[i].loaded) {
            UnloadMusicStream(audio->music[i].music);
            audio->music[i].loaded = false;
        }
    }
    
    CloseAudioDevice();
}

// Generate procedural sound effect
Sound GenerateProceduralSound(SoundType type) {
    Wave wave = {0};
    
    switch (type) {
        case SOUND_PLAYER_SHOOT:
            // Sharp, high-pitched laser sound
            wave = LoadWaveFromMemory(".wav", NULL, 0);  // Will be generated
            wave.frameCount = 4800;  // 0.1 seconds at 48kHz
            wave.sampleRate = 48000;
            wave.sampleSize = 16;
            wave.channels = 1;
            
            // Allocate memory for wave data
            wave.data = malloc(wave.frameCount * wave.channels * (wave.sampleSize / 8));
            short* samples = (short*)wave.data;
            
            for (int i = 0; i < wave.frameCount; i++) {
                float t = (float)i / wave.frameCount;
                float frequency = 800.0f + 400.0f * (1.0f - t);  // Descending frequency
                float amplitude = 0.3f * (1.0f - t) * (1.0f - t);  // Exponential decay
                
                float sample = amplitude * sinf(2.0f * PI * frequency * t);
                samples[i] = (short)(sample * 32767);
            }
            break;
            
        case SOUND_ENEMY_SHOOT:
            // Lower, more menacing sound
            wave.frameCount = 6000;  // 0.125 seconds
            wave.sampleRate = 48000;
            wave.sampleSize = 16;
            wave.channels = 1;
            
            wave.data = malloc(wave.frameCount * wave.channels * (wave.sampleSize / 8));
            samples = (short*)wave.data;
            
            for (int i = 0; i < wave.frameCount; i++) {
                float t = (float)i / wave.frameCount;
                float frequency = 300.0f + 200.0f * sinf(t * PI * 3.0f);  // Warbling
                float amplitude = 0.25f * (1.0f - t * 0.8f);
                
                float sample = amplitude * sinf(2.0f * PI * frequency * t);
                // Add some distortion
                sample = sinf(sample * 3.0f) * 0.7f;
                samples[i] = (short)(sample * 32767);
            }
            break;
            
        case SOUND_EXPLOSION_SMALL:
            // Short burst of noise
            wave.frameCount = 8000;  // ~0.167 seconds
            wave.sampleRate = 48000;
            wave.sampleSize = 16;
            wave.channels = 1;
            
            wave.data = malloc(wave.frameCount * wave.channels * (wave.sampleSize / 8));
            samples = (short*)wave.data;
            
            for (int i = 0; i < wave.frameCount; i++) {
                float t = (float)i / wave.frameCount;
                float amplitude = 0.4f * powf(1.0f - t, 2.0f);
                
                // White noise with some filtering
                float sample = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * amplitude;
                
                // Add some low frequency rumble
                sample += 0.3f * amplitude * sinf(2.0f * PI * 60.0f * t);
                
                samples[i] = (short)(sample * 32767);
            }
            break;
            
        case SOUND_EXPLOSION_LARGE:
            // Longer, deeper explosion
            wave.frameCount = 16000;  // ~0.33 seconds
            wave.sampleRate = 48000;
            wave.sampleSize = 16;
            wave.channels = 1;
            
            wave.data = malloc(wave.frameCount * wave.channels * (wave.sampleSize / 8));
            samples = (short*)wave.data;
            
            for (int i = 0; i < wave.frameCount; i++) {
                float t = (float)i / wave.frameCount;
                float amplitude = 0.6f * powf(1.0f - t, 1.5f);
                
                // Filtered white noise
                float sample = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * amplitude;
                
                // Add multiple frequency components
                sample += 0.4f * amplitude * sinf(2.0f * PI * 40.0f * t);
                sample += 0.3f * amplitude * sinf(2.0f * PI * 80.0f * t);
                sample += 0.2f * amplitude * sinf(2.0f * PI * 120.0f * t);
                
                samples[i] = (short)(sample * 32767);
            }
            break;
            
        case SOUND_POWERUP:
            // Rising arpeggio
            wave.frameCount = 12000;  // 0.25 seconds
            wave.sampleRate = 48000;
            wave.sampleSize = 16;
            wave.channels = 1;
            
            wave.data = malloc(wave.frameCount * wave.channels * (wave.sampleSize / 8));
            samples = (short*)wave.data;
            
            float frequencies[] = {440.0f, 554.37f, 659.25f, 880.0f};  // A4, C#5, E5, A5
            
            for (int i = 0; i < wave.frameCount; i++) {
                float t = (float)i / wave.frameCount;
                int note_index = (int)(t * 4.0f);
                if (note_index >= 4) note_index = 3;
                
                float frequency = frequencies[note_index];
                float amplitude = 0.3f * (1.0f - t * 0.5f);
                
                float sample = amplitude * sinf(2.0f * PI * frequency * t);
                // Add harmonics
                sample += 0.2f * amplitude * sinf(2.0f * PI * frequency * 2.0f * t);
                
                samples[i] = (short)(sample * 32767);
            }
            break;
            
        case SOUND_HIT:
            // Sharp impact sound
            wave.frameCount = 2400;  // 0.05 seconds
            wave.sampleRate = 48000;
            wave.sampleSize = 16;
            wave.channels = 1;
            
            wave.data = malloc(wave.frameCount * wave.channels * (wave.sampleSize / 8));
            samples = (short*)wave.data;
            
            for (int i = 0; i < wave.frameCount; i++) {
                float t = (float)i / wave.frameCount;
                float amplitude = 0.5f * powf(1.0f - t, 3.0f);
                
                // High frequency click with noise
                float sample = amplitude * sinf(2.0f * PI * 1200.0f * t);
                sample += 0.3f * amplitude * ((float)rand() / RAND_MAX * 2.0f - 1.0f);
                
                samples[i] = (short)(sample * 32767);
            }
            break;
            
        case SOUND_MENU_SELECT:
            // Pleasant UI sound
            wave.frameCount = 4800;  // 0.1 seconds
            wave.sampleRate = 48000;
            wave.sampleSize = 16;
            wave.channels = 1;
            
            wave.data = malloc(wave.frameCount * wave.channels * (wave.sampleSize / 8));
            samples = (short*)wave.data;
            
            for (int i = 0; i < wave.frameCount; i++) {
                float t = (float)i / wave.frameCount;
                float amplitude = 0.2f * (1.0f - t);
                
                // Two-tone chime
                float sample = amplitude * sinf(2.0f * PI * 800.0f * t);
                sample += amplitude * sinf(2.0f * PI * 1200.0f * t);
                
                samples[i] = (short)(sample * 32767);
            }
            break;
            
        case SOUND_MENU_MOVE:
            // Subtle navigation sound
            wave.frameCount = 2400;  // 0.05 seconds
            wave.sampleRate = 48000;
            wave.sampleSize = 16;
            wave.channels = 1;
            
            wave.data = malloc(wave.frameCount * wave.channels * (wave.sampleSize / 8));
            samples = (short*)wave.data;
            
            for (int i = 0; i < wave.frameCount; i++) {
                float t = (float)i / wave.frameCount;
                float amplitude = 0.15f * (1.0f - t);
                
                float sample = amplitude * sinf(2.0f * PI * 600.0f * t);
                samples[i] = (short)(sample * 32767);
            }
            break;
    }
    
    Sound sound = LoadSoundFromWave(wave);

    // Explicitly free wave.data after loading the sound to prevent memory leaks.
    // UnloadWave may not free wave.data if it was allocated manually.
    if (wave.data) free(wave.data);

    return sound;
}

// Generate procedural background music
Music GenerateProceduralMusic(MusicType type) {
    Wave wave = {0};
    
    switch (type) {
        case MUSIC_MENU:
            // Ambient menu music - 30 seconds loop
            wave.frameCount = 48000 * 30;  // 30 seconds at 48kHz
            wave.sampleRate = 48000;
            wave.sampleSize = 16;
            wave.channels = 2;  // Stereo
            
            wave.data = malloc(wave.frameCount * wave.channels * (wave.sampleSize / 8));
            short* samples = (short*)wave.data;
            
            for (int i = 0; i < wave.frameCount; i++) {
                float t = (float)i / wave.sampleRate;
                
                // Slow, atmospheric pad sound
                float freq1 = 220.0f;  // A3
                float freq2 = 329.63f; // E4
                float freq3 = 440.0f;  // A4
                
                float amp = 0.1f;
                float sample_left = amp * (
                    sinf(2.0f * PI * freq1 * t) +
                    0.7f * sinf(2.0f * PI * freq2 * t) +
                    0.5f * sinf(2.0f * PI * freq3 * t)
                ) / 3.0f;
                
                // Add some slow LFO for movement
                float lfo = sinf(2.0f * PI * 0.2f * t);
                sample_left *= (1.0f + 0.1f * lfo);
                
                // Stereo with slight delay
                float sample_right = sample_left;
                if (i > 480) {  // 10ms delay
                    sample_right = samples[(i - 480) * 2] / 32767.0f * 0.8f;
                }
                
                samples[i * 2] = (short)(sample_left * 32767);
                samples[i * 2 + 1] = (short)(sample_right * 32767);
            }
            break;
            
        case MUSIC_GAME:
            // Energetic game music - 60 seconds loop
            wave.frameCount = 48000 * 60;
            wave.sampleRate = 48000;
            wave.sampleSize = 16;
            wave.channels = 2;
            
            wave.data = malloc(wave.frameCount * wave.channels * (wave.sampleSize / 8));
            samples = (short*)wave.data;
            
            // Simple chord progression in 4/4 time
            float bpm = 140.0f;
            float beat_length = 60.0f / bpm;
            
            for (int i = 0; i < wave.frameCount; i++) {
                float t = (float)i / wave.sampleRate;
                float beat = fmodf(t, beat_length * 4.0f) / (beat_length * 4.0f);
                
                // Bass line
                float bass_freq = 110.0f;  // A2
                if (beat > 0.25f && beat < 0.5f) bass_freq = 146.83f;  // D3
                if (beat > 0.5f && beat < 0.75f) bass_freq = 123.47f;   // B2
                
                float bass = 0.15f * sinf(2.0f * PI * bass_freq * t);
                
                // Lead melody (simple arpeggio)
                float lead_freq = 440.0f + 220.0f * sinf(2.0f * PI * 0.5f * t);
                float lead = 0.08f * sinf(2.0f * PI * lead_freq * t);
                
                // Drums (kick on beats 1 and 3)
                float drum = 0.0f;
                float beat_pos = fmodf(t, beat_length);
                if (beat_pos < 0.1f && ((int)(t / beat_length) % 2 == 0)) {
                    drum = 0.2f * powf(1.0f - beat_pos / 0.1f, 2.0f) * 
                           sinf(2.0f * PI * 60.0f * beat_pos);
                }
                
                float sample_left = bass + lead + drum;
                float sample_right = sample_left * 0.9f;  // Slight stereo width
                
                samples[i * 2] = (short)(sample_left * 32767);
                samples[i * 2 + 1] = (short)(sample_right * 32767);
            }
            break;
            
        case MUSIC_BOSS:
            // Intense boss music - 45 seconds loop
            wave.frameCount = 48000 * 45;
            wave.sampleRate = 48000;
            wave.sampleSize = 16;
            wave.channels = 2;
            
            wave.data = malloc(wave.frameCount * wave.channels * (wave.sampleSize / 8));
            samples = (short*)wave.data;
            
            for (int i = 0; i < wave.frameCount; i++) {
                float t = (float)i / wave.sampleRate;
                
                // Aggressive bass
                float bass_freq = 82.41f;  // E2
                float bass = 0.2f * sinf(2.0f * PI * bass_freq * t);
                bass += 0.1f * sinf(2.0f * PI * bass_freq * 2.0f * t);  // Octave
                
                // Dissonant lead
                float lead1 = 0.06f * sinf(2.0f * PI * 659.25f * t);  // E5
                float lead2 = 0.06f * sinf(2.0f * PI * 698.46f * t);  // F5 (dissonant)
                
                // Fast hi-hat pattern
                float hihat = 0.0f;
                if (fmodf(t * 8.0f, 1.0f) < 0.1f) {
                    hihat = 0.05f * ((float)rand() / RAND_MAX);
                }
                
                float sample_left = bass + lead1 + hihat;
                float sample_right = bass + lead2 + hihat * 0.8f;
                
                samples[i * 2] = (short)(sample_left * 32767);
                samples[i * 2 + 1] = (short)(sample_right * 32767);
            }
            break;
    }
    
    Music music = LoadMusicStreamFromMemory(".wav", (unsigned char*)wave.data, 
                                          wave.frameCount * wave.channels * (wave.sampleSize / 8));
    // Explicitly free wave.data after loading the music to prevent memory leaks.
    // If Raylib takes ownership and frees this memory, this is safe; if not, this prevents leaks.
    if (wave.data) free(wave.data);
    return music;
}

// Load sound effect
bool LoadSoundEffect(AudioSystem* audio, SoundType type) {
    if (!audio || type >= SOUND_COUNT) return false;
    
    if (audio->sounds[type].loaded) {
        UnloadSound(audio->sounds[type].sound);
    }
    
    audio->sounds[type].sound = GenerateProceduralSound(type);
    audio->sounds[type].loaded = true;
    audio->sounds[type].volume = 1.0f;
    audio->sounds[type].pitch = 1.0f;
    
    return true;
}

// Load music
bool LoadMusicTrack(AudioSystem* audio, MusicType type) {
    if (!audio || type >= MUSIC_COUNT) return false;
    
    if (audio->music[type].loaded) {
        UnloadMusicStream(audio->music[type].music);
    }
    
    audio->music[type].music = GenerateProceduralMusic(type);
    audio->music[type].loaded = true;
    audio->music[type].volume = 1.0f;
    
    return true;
}

// Play sound effect
void PlaySoundEffect(AudioSystem* audio, SoundType type, float volume, float pitch) {
    if (!audio || type >= SOUND_COUNT || !audio->sounds[type].loaded) return;
    
    float final_volume = volume * audio->sounds[type].volume * audio->sfx_volume * audio->master_volume;
    float final_pitch = pitch * audio->sounds[type].pitch;
    
    SetSoundVolume(audio->sounds[type].sound, final_volume);
    SetSoundPitch(audio->sounds[type].sound, final_pitch);
    PlaySound(audio->sounds[type].sound);
}

// Play music
void PlayMusicTrack(AudioSystem* audio, MusicType type) {
    if (!audio || type >= MUSIC_COUNT || !audio->music[type].loaded) return;
    
    // Stop current music if playing
    if (audio->current_music >= 0 && audio->current_music < MUSIC_COUNT) {
        StopMusicStream(audio->music[audio->current_music].music);
    }
    
    audio->current_music = type;
    PlayMusicStream(audio->music[type].music);
    SetMusicVolume(audio->music[type].music, 
                   audio->music[type].volume * audio->music_volume * audio->master_volume);
}

// Stop music
void StopMusic(AudioSystem* audio) {
    if (!audio || audio->current_music < 0) return;
    
    StopMusicStream(audio->music[audio->current_music].music);
    audio->current_music = -1;
}

// Fade music volume
void FadeMusic(AudioSystem* audio, float target_volume, float duration) {
    if (!audio || audio->current_music < 0) return;
    
    audio->fade_target_volume = target_volume;
    audio->fade_duration = duration;
    audio->fade_timer = duration;
}

// Update audio system
void UpdateAudioSystem(AudioSystem* audio, float delta) {
    if (!audio) return;
    
    // Update music stream
    if (audio->current_music >= 0 && audio->current_music < MUSIC_COUNT) {
        UpdateMusicStream(audio->music[audio->current_music].music);
        
        // Handle music fading
        if (audio->fade_timer > 0.0f) {
            audio->fade_timer -= delta;
            if (audio->fade_timer <= 0.0f) {
                audio->fade_timer = 0.0f;
                audio->music[audio->current_music].volume = audio->fade_target_volume;
            } else {
                float progress = 1.0f - (audio->fade_timer / audio->fade_duration);
                float current_vol = audio->music[audio->current_music].volume;
                audio->music[audio->current_music].volume = 
                    current_vol + (audio->fade_target_volume - current_vol) * progress;
            }
            
            SetMusicVolume(audio->music[audio->current_music].music,
                          audio->music[audio->current_music].volume * 
                          audio->music_volume * audio->master_volume);
        }
        
        // Loop music if it ends
        if (!IsMusicStreamPlaying(audio->music[audio->current_music].music)) {
            PlayMusicStream(audio->music[audio->current_music].music);
        }
    }
}

// Set volume levels
void SetGameMasterVolume(AudioSystem* audio, float volume) {
    if (!audio) return;
    audio->master_volume = Clamp(volume, 0.0f, 1.0f);
}

void SetGameSFXVolume(AudioSystem* audio, float volume) {
    if (!audio) return;
    audio->sfx_volume = Clamp(volume, 0.0f, 1.0f);
}

void SetGameMusicVolume(AudioSystem* audio, float volume) {
    if (!audio) return;
    audio->music_volume = Clamp(volume, 0.0f, 1.0f);
    
    // Update current music volume
    if (audio->current_music >= 0 && audio->current_music < MUSIC_COUNT) {
        SetMusicVolume(audio->music[audio->current_music].music,
                      audio->music[audio->current_music].volume * 
                      audio->music_volume * audio->master_volume);
    }
}

// Helper function for clamping values
float Clamp(float value, float min, float max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
}

// Initialize all audio assets
void InitAllAudioAssets(AudioSystem* audio) {
    if (!audio) return;
    
    // Load all sound effects
    for (int i = 0; i < SOUND_COUNT; i++) {
        LoadSoundEffect(audio, (SoundType)i);
    }
    
    // Load all music tracks
    for (int i = 0; i < MUSIC_COUNT; i++) {
        LoadMusicTrack(audio, (MusicType)i);
    }
}

// Play contextual sound effects
void PlayGameSound(AudioSystem* audio, GameSoundContext context, float intensity) {
    if (!audio) return;
    
    switch (context) {
        case GAME_SOUND_PLAYER_SHOOT:
            PlaySoundEffect(audio, SOUND_PLAYER_SHOOT, 0.7f, 1.0f + (rand() % 20 - 10) / 100.0f);
            break;
            
        case GAME_SOUND_ENEMY_SHOOT:
            PlaySoundEffect(audio, SOUND_ENEMY_SHOOT, 0.5f, 1.0f + (rand() % 30 - 15) / 100.0f);
            break;
            
        case GAME_SOUND_ENEMY_HIT:
            PlaySoundEffect(audio, SOUND_HIT, 0.6f * intensity, 1.0f + (rand() % 40 - 20) / 100.0f);
            break;
            
        case GAME_SOUND_PLAYER_HIT:
            PlaySoundEffect(audio, SOUND_HIT, 0.8f, 0.8f);
            break;
            
        case GAME_SOUND_ENEMY_DESTROY_SMALL:
            PlaySoundEffect(audio, SOUND_EXPLOSION_SMALL, 0.7f, 1.0f);
            break;
            
        case GAME_SOUND_ENEMY_DESTROY_LARGE:
            PlaySoundEffect(audio, SOUND_EXPLOSION_LARGE, 0.9f, 1.0f);
            break;
            
        case GAME_SOUND_POWERUP:
            PlaySoundEffect(audio, SOUND_POWERUP, 0.8f, 1.0f);
            break;
            
        case GAME_SOUND_MENU_MOVE:
            PlaySoundEffect(audio, SOUND_MENU_MOVE, 0.6f, 1.0f);
            break;
            
        case GAME_SOUND_MENU_SELECT:
            PlaySoundEffect(audio, SOUND_MENU_SELECT, 0.7f, 1.0f);
            break;
    }
}