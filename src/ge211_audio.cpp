#include "ge211_audio.h"
#include "ge211_resource.h"

#include <SDL.h>
#include <SDL_mixer.h>

namespace ge211 {

using namespace detail;

namespace audio {

void Audio_track::play(Duration fade_in)
{
    switch (get_state()) {
        case State::playing:
            // idempotent;
            break;

        case State::halted:
            pos_.unpause();
            pos_.reset();
            this->fade_in(fade_in, 0);
            break;

        case State::paused: {
            Duration point = pos_.elapsed_time();
            pos_.unpause();
            this->fade_in(fade_in, point);
        }
        break;

        case State::fading_out:
            throw Client_logic_error("Audio_track::play: fading out");

        case State::unrouted:
            throw Client_logic_error("Audio_track::play: unloaded");
    }
}

void Audio_track::pause(Duration fade_out)
{
    switch (get_state()) {
        case State::playing:
            pos_.pause();
            this->fade_out(fade_out);
            break;

        case State::halted:
            // allowed (stronger)
            break;

        case State::paused:
            // idempotent
            break;

        case State::fading_out:
            throw Client_logic_error("Audio_track::pause: fading out");

        case State::unrouted:
            throw Client_logic_error("Audio_track::pause: unloaded");
    }
}

void Audio_track::reset()
{
    switch (get_state()) {
        case State::playing:
            throw Client_logic_error("Audio_track::reset: still playing");

        case State::halted:
            pos_.pause();
            pos_.reset();
            break;

        case State::paused:
            pos_.reset();
            break;

        case State::fading_out:
            throw Client_logic_error("Audio_track::reset: fading out");

        case State::unrouted:
            pos_.reset();
            break;
    }
}

Mix_Music* Music_track::load_(const std::string& filename,
                              File_resource&& file_resource)
{
    Mix_Music* raw = Mix_LoadMUS_RW(std::move(file_resource).forget_(), 1);
    if (raw) return raw;

    throw Mixer_error::could_not_load(filename);
}

Music_track::Music_track(const std::string& filename,
                         File_resource&& file_resource)
        : Audio_resource<Mix_Music>(load_(filename, std::move(file_resource)),
                                    &Mix_FreeMusic)
{ }

void Music_track::fade_in(time::Duration dur, time::Duration offset)
{
    Mix_FadeInMusicPos(get_raw_(),
                       1,
                       int(dur.milliseconds()),
                       offset.seconds());
}

void Music_track::fade_out(time::Duration dur)
{
    Mix_FadeOutMusic(int(dur.milliseconds()));
}

Audio_track::State Music_track::poll_state_()
{
    if (get_state() == State::unrouted) return State::unrouted;
    if (Mix_PlayingMusic()) {
        if (Mix_FadingMusic() == MIX_FADING_OUT) return State::fading_out;
        else return State::playing;
    } else {
        if (get_position_().is_paused()) return State::paused;
        else return State::halted;
    }
}

std::unique_ptr<Mixer> Mixer::open_mixer()
{

    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY,
                      MIX_DEFAULT_FORMAT,
                      2,
                      4096) >= 0)
        return std::unique_ptr<Mixer>{new Mixer};
    else
        return {};
}

Mixer::Mixer()
        : current_music_{nullptr}
{
    int music_decoders = Mix_GetNumMusicDecoders();
    info_sdl() << "Number of music decoders is " << music_decoders;
    for (int i = 0; i < music_decoders; ++i) {
        info_sdl() << "  [" << i << "] " << Mix_GetMusicDecoder(i);
    }

    int chunk_decoders = Mix_GetNumChunkDecoders();
    info_sdl() << "Number of chunk decoders is " << chunk_decoders;
    for (int i = 0; i < chunk_decoders; ++i) {
        info_sdl() << "  [" << i << "] " << Mix_GetChunkDecoder(i);
    }
}

Mixer::~Mixer()
{
    Mix_CloseAudio();
}

std::shared_ptr<Music_track> Mixer::load_music(const std::string& filename)
{
    File_resource file_resource{filename};
    std::shared_ptr<Music_track> result(
            new Music_track(filename, std::move(file_resource)));
    return result;
}

void Mixer::route_music(std::shared_ptr<Music_track> track)
{
    if (current_music_) {
        switch (current_music_->get_state()) {
            case State::paused:
            case State::halted:
            case State::unrouted:
                current_music_->state_ = State::unrouted;

            case State::playing:
                throw Client_logic_error("Mixer::route_music: still playing");

            case State::fading_out:
                throw Client_logic_error("Mixer::route_music: fading out");
        }
    }

    current_music_ = track;

    if (current_music_) {
        current_music_->state_ = State::halted;
    }
}

void Mixer::update_state_()
{
    if (current_music_)
        current_music_->update_state_();
}

} // end namespace audio

} // end namespace ge211
