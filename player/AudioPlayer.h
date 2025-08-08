#pragma once

#include <include.h>

class AudioPlayer {
public:
    void setVolume(float volume) {
        if (sound) ma_sound_set_volume(sound, volume);
    }

    float getVolume() {
        return sound ? ma_sound_get_volume(sound) : 0.0f;
    }

    void setPan(float pan) {
        if (sound) ma_sound_set_pan(sound, pan);
    }

    float getPan() {
        return sound ? ma_sound_get_pan(sound) : 0.0f;
    }

    void setPitch(float pitch) {
        if (sound) ma_sound_set_pitch(sound, pitch);
    }

    float getPitch() {
        return sound ? ma_sound_get_pitch(sound) : 1.0f;
    }

    void setLooping(bool looping) {
        if (sound) ma_sound_set_looping(sound, looping ? MA_TRUE : MA_FALSE);
    }

    bool isLooping() {
        return sound ? ma_sound_is_looping(sound) : false;
    }

    void setPosition(float x, float y, float z) {
        if (sound) ma_sound_set_position(sound, x, y, z);
    }

    ma_vec3f getPosition(float& x, float& y, float& z) {
        ma_vec3f position;
        if (sound) position = ma_sound_get_position(sound);
        return position;
    }

    void setSpatializationEnabled(bool enabled) {
        if (sound) ma_sound_set_spatialization_enabled(sound, enabled ? MA_TRUE : MA_FALSE);
    }

    void setMinDistance(float distance) {
        if (sound) ma_sound_set_min_distance(sound, distance);
    }

    void setMaxDistance(float distance) {
        if (sound) ma_sound_set_max_distance(sound, distance);
    }

    void setAttenuationModel(ma_attenuation_model model) {
        if (sound) ma_sound_set_attenuation_model(sound, model);
    }

    void play() {
        if (sound) ma_sound_start(sound);
    }

    void stop() {
        if (sound) ma_sound_stop(sound);
    }

    void pause() {
        if (sound) ma_sound_stop(sound); // No built-in pause, stopping is the closest
    }

    bool isPlaying() {
        return sound ? ma_sound_is_playing(sound) : false;
    }

    void seekToFrame(ma_uint64 frameIndex) {
        if (sound) ma_sound_seek_to_pcm_frame(sound, frameIndex);
    }

    ma_uint64 getCurrentFrame() {
        ma_uint64 cursor = 0;
        if (sound) ma_sound_get_cursor_in_pcm_frames(sound, &cursor);
        return cursor;
    }

    ma_uint64 getLength() {
        ma_uint64 length = 0;
        if (sound) ma_sound_get_length_in_pcm_frames(sound, &length);
        return length;
    }

    ma_sound* sound = nullptr;
    ma_engine* engine;
};