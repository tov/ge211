#include "ge211_audio.h"
#include "ge211_resource.h"

#include <SDL.h>
#include <SDL_mixer.h>

#include <algorithm>
#include <cassert>

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
        : effect_tracks_(MIX_CHANNELS, nullptr)
        , effect_states_(MIX_CHANNELS, State::empty)
        , available_effect_channels_(MIX_CHANNELS)
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
        case State::paused:
        case State::empty:
            break;

        case State::playing:
            throw Client_logic_error("Mixer::route_music: still playing");

        case State::fading_out:
            throw Client_logic_error("Mixer::route_music: fading out");
    }

    current_music_ = music;

    if (current_music_) {
        music_state_ = State::paused;
    } else {
        music_state_ = State::empty;
    }
}

void Mixer::unpause_music(Duration fade_in)
{
    switch (music_state_) {
        case State::empty:
            throw Client_logic_error("Mixer::unpause_music: no music attached");

        case State::paused:
            Mix_RewindMusic();
            Mix_FadeInMusicPos(current_music_->get_raw_(),
                               0,
                               int(fade_in.milliseconds()),
                               music_position_.elapsed_time().seconds());
            music_position_.unpause();
            music_state_ = State::playing;
            break;

        case State::fading_out:
            throw Client_logic_error("Mixer::unpause_music: fading out");

        case State::playing:
            // idempotent
            break;
    }
}

void Mixer::pause_music(Duration fade_out)
{
    switch (music_state_) {
        case State::empty:
            throw Client_logic_error("Mixer::pause_music: no music attached");

        case State::paused:
            // Idempotent
            break;

        case State::fading_out:
            throw Client_logic_error("Mixer::pause_music: fading out");

        case State::playing:
            if (fade_out == 0.0) {
                Mix_HaltMusic();
                music_position_.pause();
                music_state_ = State::paused;
            } else {
                Mix_FadeOutMusic(int(fade_out.milliseconds()));
                music_state_ = State::fading_out;
            }
            break;
    }
}

void Mixer::rewind_music()
{
    switch (music_state_) {
        case State::paused:
            music_position_.reset();
            break;

        case State::empty:
        case State::playing:
        case State::fading_out:
            throw Client_logic_error(
                    "Mixer::rewind_music: must be paused");
    }
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
    return effect_tracks_.at(size_t(channel));
}

Mixer::State Mixer::get_effect_state(int channel) const
{
    return effect_states_.at(size_t(channel));
}

int Mixer::find_empty_channel_() const
{
    auto iter = std::find_if(effect_states_.begin(),
                             effect_states_.end(),
                             [](auto state) {
                                 return state == State::empty;
                             });
    if (iter == effect_states_.end()) {
        throw Mixer_error::out_of_channels();
    }

    return (int) std::distance(effect_states_.begin(), iter);
}

void Mixer::poll_channels_()
{
    if (current_music_) {
        if (!Mix_PlayingMusic()) {
            switch (music_state_) {
                case State::empty:
                case State::paused:
                    break;

                case State::playing:
                    music_position_.pause();
                    music_position_.reset();
                    music_state_ = State::paused;
                    break;

                case State::fading_out:
                    music_position_.pause();
                    music_state_ = State::paused;
                    break;
            }
        }
    }

    for (int channel = 0; channel < effect_tracks_.size(); ++channel) {
        if (effect_states_[channel] != State::empty
            && !Mix_Playing(channel))
        {
            unregister_effect_(channel);
        }
    }
}

int Mixer::play_effect(const std::shared_ptr<Effect_track>& effect,
                       Duration fade_in)
{
    int channel = find_empty_channel_();
    register_effect_(channel, effect);

    Mix_FadeInChannel(channel, effect->get_raw_(), 0,
                      int(fade_in.milliseconds()));

    return channel;
}

void Mixer::unpause_effect(int channel)
{
    check_channel_in_bounds_(channel);

    switch (effect_states_[channel]) {
        case State::empty:
            throw Client_logic_error("Mixer::unpause_effect: empty channel");

        case State::paused:
            effect_states_[channel] = State::playing;
            Mix_Resume(channel);

        case State::playing:
            // idempotent
            break;

        case State::fading_out:
            throw Client_logic_error("Mixer::unpause_effect: fading out");
    }
}

void Mixer::pause_effect(int channel)
{
    check_channel_in_bounds_(channel);

    switch (effect_states_[channel]) {
        case State::empty:
            throw Client_logic_error("Mixer::pause_effect: empty channel");

        case State::paused:
            // idempotent
            break;

        case State::playing:
            effect_states_[channel] = State::paused;
            Mix_Pause(channel);
            break;

        case State::fading_out:
            throw Client_logic_error("Mixer::pause_effect: fading out");
    }
}

void Mixer::stop_effect(int channel, Duration fade_out)
{
    check_channel_in_bounds_(channel);

    switch (effect_states_[channel]) {
        case State::empty:
            throw Client_logic_error("Mixer::stop_effect: empty channel");

        case State::paused:
            unregister_effect_(channel);
            Mix_HaltChannel(channel);
            break;

        case State::playing:
            if (fade_out == 0.0) {
                unregister_effect_(channel);
                Mix_HaltChannel(channel);
            } else {
                effect_states_[channel] = State::fading_out;
                Mix_FadeOutChannel(channel, int(fade_out.milliseconds()));
            }

        case State::fading_out:
            throw Client_logic_error("Mixer::stop_effect: fading out");
    }
}

void Mixer::pause_all_effects()
{
    Mix_Pause(-1);

    for (auto& state : effect_states_) {
        if (state == State::playing)
            state = State::paused;
    }
}

void Mixer::unpause_all_effects()
{
    Mix_Resume(-1);

    for (auto& state : effect_states_) {
        if (state == State::paused)
            state = State::playing;
    }
}

void Mixer::check_channel_in_bounds_(int channel) const
{
    if (channel < 0 || channel >= effect_tracks_.size())
        throw Client_logic_error("Mixer: channel out of range");
}

int Mixer::available_effect_channels() const
{
    return available_effect_channels_;
}

void Mixer::register_effect_(int channel,
                             const std::shared_ptr<Effect_track>& effect)
{
    assert(effect_states_[channel] == State::empty);
    effect_states_[channel] = State::playing;
    effect_tracks_[channel] = effect;
    --available_effect_channels_;
}

void Mixer::unregister_effect_(int channel)
{
    assert(effect_states_[channel] != State::empty);
    effect_states_[channel] = State::empty;
    effect_tracks_[channel] = nullptr;
    ++available_effect_channels_;
}

} // end namespace audio

} // end namespace ge211
