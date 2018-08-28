#include "ge211_audio.h"
#include "ge211_resource.h"

#include <SDL.h>
#include <SDL_mixer.h>

namespace ge211 {

using namespace detail;

namespace audio {

Mix_Music* Music_track::load_(const std::string& filename,
                              File_resource&& file_resource)
{
    Mix_Music* raw = Mix_LoadMUS_RW(std::move(file_resource).release(), 1);
    if (raw) return raw;

    throw Mixer_error::could_not_load(filename);
}

Music_track::Music_track(const std::string& filename,
                         File_resource&& file_resource)
        : Audio_resource<Mix_Music>(load_(filename, std::move(file_resource)),
                                    &Mix_FreeMusic)
{ }

Mix_Chunk* Effect_track::load_(const std::string& filename,
                               detail::File_resource&& file_resource)
{
    Mix_Chunk* raw = Mix_LoadWAV_RW(std::move(file_resource).release(), 1);
    if (raw) return raw;

    throw Mixer_error::could_not_load(filename);
}

Effect_track::Effect_track(const std::string& filename,
                           detail::File_resource&& file_resource)
        : Audio_resource<Mix_Chunk>(load_(filename, std::move(file_resource)),
                                    &Mix_FreeChunk)
{ }

double Effect_track::get_volume() const
{
    return get_raw_()->volume / double(MIX_MAX_VOLUME);
}

void Effect_track::set_volume(double unit_value)
{
    Mix_VolumeChunk(get_raw_(), int(unit_value * MIX_MAX_VOLUME));
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
        : effect_channels_(MIX_CHANNELS, nullptr)
        , effect_states_(MIX_CHANNELS, Channel_state::empty)
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

void Mixer::play_music(const std::shared_ptr<Music_track>& music,
                       Duration fade_in)
{
    attach_music(music);
    unpause_music(fade_in);
}

void Mixer::attach_music(const std::shared_ptr<Music_track>& music)
{
    switch (music_state_) {
        case Channel_state::paused:
        case Channel_state::empty:
        case Channel_state::halted:
            break;

        case Channel_state::playing:
            throw Client_logic_error("Mixer::route_music: still playing");

        case Channel_state::fading_out:
            throw Client_logic_error("Mixer::route_music: fading out");
    }

    current_music_ = music;

    if (current_music_) {
        music_state_ = Channel_state::paused;
    } else {
        music_state_ = Channel_state::empty;
    }
}

void Mixer::unpause_music(Duration fade_in)
{
    switch (music_state_) {
        case Channel_state::empty:
            throw Client_logic_error("Mixer::unpause_music: no music attached");

        case Channel_state::halted:
        case Channel_state::paused:
            Mix_RewindMusic();
            Mix_FadeInMusicPos(current_music_->get_raw_(),
                               0,
                               int(fade_in.milliseconds()),
                               music_position_.elapsed_time().seconds());
            music_position_.unpause();
            music_state_ = Channel_state::playing;
            break;

        case Channel_state::fading_out:
            throw Client_logic_error("Mixer::unpause_music: fading out");

        case Channel_state::playing:
            // idempotent
            break;
    }
}

void Mixer::pause_music(Duration fade_out)
{
    switch (music_state_) {
        case Channel_state::empty:
            throw Client_logic_error("Mixer::pause_music: no music attached");

        case Channel_state::halted:
        case Channel_state::paused:
            // Idempotent
            break;

        case Channel_state::fading_out:
            throw Client_logic_error("Mixer::pause_music: fading out");

        case Channel_state::playing:
            if (fade_out == 0.0) {
                Mix_HaltMusic();
                music_position_.pause();
                music_state_ = Channel_state::paused;
            } else {
                Mix_FadeOutMusic(int(fade_out.milliseconds()));
                music_state_ = Channel_state::fading_out;
            }
            break;
    }
}

void Mixer::rewind_music()
{
    switch (music_state_) {
        case Channel_state::paused:
        case Channel_state::halted:
            break;

        case Channel_state::empty:
        case Channel_state::playing:
        case Channel_state::fading_out:
            throw Client_logic_error(
                    "Mixer::rewind_music: must be paused or halted");
    }

    music_position_.reset();
}

std::shared_ptr<Effect_track> Mixer::load_effect(const std::string& filename)
{
    File_resource file_resource{filename};
    std::shared_ptr<Effect_track> result(
            new Effect_track(filename, std::move(file_resource)));
    return result;
}

const std::shared_ptr<Effect_track>& Mixer::get_effect(int channel) const
{
    return effect_channels_.at(size_t(channel));
}

Channel_state Mixer::get_effect_state(int channel) const
{
    return effect_states_.at(size_t(channel));
}

void Mixer::poll_state_()
{
    if (current_music_) {
        if (!Mix_PlayingMusic()) {
            switch (music_state_) {
                case Channel_state::empty:
                case Channel_state::paused:
                case Channel_state::halted:
                    break;

                case Channel_state::playing:
                    music_position_.pause();
                    music_position_.reset();
                    music_state_ = Channel_state::halted;
                    break;

                case Channel_state::fading_out:
                    music_position_.pause();
                    music_state_ = Channel_state::paused;
                    break;
            }
        }
    }

    for (int channel = 0; channel < effect_channels_.size(); ++channel) {
        if (effect_states_[channel] != Channel_state::empty
            && !Mix_Playing(channel))
        {
            effect_states_[channel] = Channel_state::halted;
        }
    }
}

} // end namespace audio

} // end namespace ge211
