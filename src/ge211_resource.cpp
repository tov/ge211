#include "ge211_resource.h"
#include "ge211_error.h"

#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#include <string>

namespace ge211 {

using namespace detail;

static const char* search_prefixes[] = {
        "Resources/",
        "../Resources/",
        GE211_RESOURCES
};

namespace detail {

static void close_rwops(SDL_RWops* rwops)
{
    SDL_RWclose(rwops);
}

delete_ptr<SDL_RWops> File_resource::open_rwops_(const std::string& filename)
{
    for (auto prefix : search_prefixes) {
        std::string path;
        path += prefix;
        path += filename;
        SDL_RWops* rwops = SDL_RWFromFile(path.c_str(), "rb");
        if (rwops) return {rwops, &close_rwops};
    }

    throw File_error::could_not_open(filename);
}

File_resource::File_resource(const std::string& filename)
        : ptr_{open_rwops_(filename)}
{ }

} // end namespace detail

delete_ptr<TTF_Font> Font::load_(const std::string& filename,
                                 File_resource& file,
                                 int size)
{
    TTF_Font* result = TTF_OpenFontRW(file.get_raw_(), 0, size);
    if (result) return {result, &TTF_CloseFont};

    throw Font_error::could_not_load(filename);
}

Font::Font(const std::string& filename, int size)
        : file_{filename},
          ptr_{load_(filename, file_, size)}
{ }

delete_ptr<Mix_Music> Mixer::load_music_(const std::string& filename)
{
    for (auto prefix : search_prefixes) {
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
            if (Mix_PlayMusic(music_ptr_.get(), 1) < 0) {
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
            Mix_HaltMusic();
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

}