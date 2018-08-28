#include "ge211_audio.h"
#include "ge211_resource.h"

#include <SDL.h>
#include <SDL_mixer.h>

namespace ge211 {

using namespace detail;

namespace audio {

delete_ptr<Mix_Music> Mixer::load_music_(const std::string& filename)
{
    for (auto prefix : get_search_prefixes()) {
        std::string path;
        path += prefix;
        path += filename;
        Mix_Music* raw = Mix_LoadMUS(path.c_str());

        if (raw) return {raw, Mix_FreeMusic};
    }

    throw Mixer_error::could_not_load(filename);
}

Mixer::Mixer() noexcept
        : music_ptr_{nullptr, &no_op_deleter}
{ }

Mixer::State Mixer::get_music_state() const
{
    if (! music_ptr_) {
        return State::unloaded;
    } else if (Mix_PlayingMusic()) {
        if (Mix_PausedMusic()) {
            return State::paused;
        } else {
            return State::playing;
        }
    } else {
        return State::halted;
    }
}

void Mixer::load_music(const std::string& filename)
{
    switch (get_music_state()) {
        case State::unloaded:
            music_ptr_ = load_music_(filename);
            break;

        default:
            throw Client_logic_error("Music is already loaded");
    }
}

void Mixer::play_music()
{
    switch (get_music_state()) {
        case State::halted:
            if (Mix_FadeInMusicPos(music_ptr_.get(), 1, 500, 0.0) < 0) {
                throw Mixer_error("Could not play music.");
            }
            break;

        case State::playing:
            // Idempotent
            break;

        case State::paused:
            Mix_ResumeMusic();
            break;

        case State::unloaded:
            throw Client_logic_error("No music loaded");
    }
}

void Mixer::pause_music()
{
    switch (get_music_state()) {
        case State::halted:
            // Okay.
            break;

        case State::playing:
            Mix_PauseMusic();
            break;

        case State::paused:
            // Idempotent.
            break;

        case State::unloaded:
            throw Client_logic_error("No music loaded");
    }
}

void Mixer::stop_music()
{
    switch (get_music_state()) {
        case State::halted:
            // Idempotent
            break;

        case State::playing:
            Mix_FadeOutMusic(500);
            break;

        case State::paused:
            Mix_HaltMusic();
            break;

        case State::unloaded:
            throw Client_logic_error("No music loaded");
    }
}

void Mixer::unload_music()
{
    switch (get_music_state()) {
        case State::unloaded:
            // Okay, idempotent
            break;

        case State::halted:
            music_ptr_ = {nullptr, &no_op_deleter};
            break;

        default:
            throw Client_logic_error("Cannot unload playing music");
    }
}
} // end namespace audio

} // end namespace ge211
