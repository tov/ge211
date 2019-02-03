#include "ge211_audio.h"
#include "ge211_resource.h"

#include <SDL.h>
#include <SDL_mixer.h>

#include <algorithm>
#include <cassert>

namespace ge211 {

using namespace detail;

namespace audio {

static inline int unit_to_volume(double unit_volume)
{
    return int(unit_volume * MIX_MAX_VOLUME);
}

static inline double volume_to_unit(int int_volume)
{
    return int_volume / double(MIX_MAX_VOLUME);
}

std::shared_ptr<Mix_Music> Music_track::load_(const std::string& filename)
{
    File_resource file_resource(filename);
    Mix_Music* raw = Mix_LoadMUS_RW(std::move(file_resource).release(), 1);
    if (raw) return {raw, &Mix_FreeMusic};

    throw Mixer_error::could_not_load(filename);
}

Music_track::Music_track(const std::string& filename, const Mixer&)
        : ptr_{load_(filename)}
{ }

bool Music_track::empty() const
{
    return ptr_ == nullptr;
}

Music_track::operator bool() const
{
    return !empty();
}

std::shared_ptr<Mix_Chunk> Sound_effect::load_(const std::string& filename)
{
    File_resource file_resource(filename);
    Mix_Chunk* raw = Mix_LoadWAV_RW(std::move(file_resource).release(), 1);
    if (raw) return {raw, &Mix_FreeChunk};

    throw Mixer_error::could_not_load(filename);
}

Sound_effect::Sound_effect(const std::string& filename, const Mixer&)
        : ptr_{load_(filename)}
{ }

bool Sound_effect::empty() const
{
    return ptr_ == nullptr;
}

Sound_effect::operator bool() const
{
    return !empty();
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
        : channels_(MIX_CHANNELS)
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

void Mixer::play_music(Music_track music)
{
    attach_music(std::move(music));
    resume_music();
}

void Mixer::attach_music(Music_track music)
{
    switch (music_state_) {
        case State::paused:
        case State::detached:
            break;

        case State::playing:
            throw Client_logic_error("Mixer::attach_music: still playing");

        case State::fading_out:
            throw Client_logic_error("Mixer::attach_music: fading out");
    }

    current_music_ = std::move(music);

    if (current_music_) {
        music_state_ = State::paused;
    } else {
        music_state_ = State::detached;
    }
}

void Mixer::resume_music(Duration fade_in)
{
    switch (music_state_) {
        case State::detached:
            throw Client_logic_error("Mixer::resume_music: no music attached");

        case State::paused:
            Mix_RewindMusic();
            Mix_FadeInMusicPos(current_music_.ptr_.get(),
                               0,
                               int(fade_in.milliseconds()),
                               music_position_.elapsed_time().seconds());
            music_position_.resume();
            music_state_ = State::playing;
            break;

        case State::fading_out:
            throw Client_logic_error("Mixer::resume_music: fading out");

        case State::playing:
            // idempotent
            break;
    }
}

void Mixer::pause_music(Duration fade_out)
{
    switch (music_state_) {
        case State::detached:
            throw Client_logic_error("Mixer::pause_music: no music attached");

        case State::paused:
            // Idempotent
            break;

        case State::fading_out:
            throw Client_logic_error("Mixer::pause_music: fading out");

        case State::playing:
            if (fade_out == Duration(0)) {
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

        case State::detached:
        case State::playing:
        case State::fading_out:
            throw Client_logic_error(
                    "Mixer::rewind_music: must be paused");
    }
}

const Sound_effect& Sound_effect_handle::get_effect() const
{
    return ptr_->effect;
}

Mixer::State Sound_effect_handle::get_state() const
{
    return ptr_->state;
}

int Mixer::find_empty_channel_() const
{
    auto iter = std::find_if(channels_.begin(),
                             channels_.end(),
                             [](const auto& handle) {
                                 return handle.empty();
                             });
    if (iter == channels_.end()) {
        throw Mixer_error::out_of_channels();
    }

    return (int) std::distance(channels_.begin(), iter);
}

void Mixer::poll_channels_()
{
    if (current_music_) {
        if (!Mix_PlayingMusic()) {
            switch (music_state_) {
                case State::detached:
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

    for (int channel = 0; channel < channels_.size(); ++channel) {
        if (channels_[channel] && !Mix_Playing(channel))
        {
            unregister_effect_(channel);
        }
    }
}

Sound_effect_handle
Mixer::play_effect(Sound_effect effect, double volume)
{
    int channel = find_empty_channel_();
    Mix_Volume(channel, unit_to_volume(volume));
    Mix_PlayChannel(channel, effect.ptr_.get(), 0);
    return register_effect_(channel, std::move(effect));
}

void Sound_effect_handle::resume()
{
    switch (ptr_->state) {
        case Mixer::State::detached:
            throw Client_logic_error("Sound_effect_handle::resume: detached");

        case Mixer::State::paused:
            ptr_->state = Mixer::State::playing;
            Mix_Resume(ptr_->channel);
            break;

        case Mixer::State::playing:
            // idempotent
            break;

        case Mixer::State::fading_out:
            throw Client_logic_error("Sound_effect_handle::resume: fading out");
    }
}

void Sound_effect_handle::pause()
{
    switch (ptr_->state) {
        case Mixer::State::detached:
            throw Client_logic_error("Sound_effect_handle::pause: detached");

        case Mixer::State::paused:
            // idempotent
            break;

        case Mixer::State::playing:
            ptr_->state = Mixer::State::paused;
            Mix_Pause(ptr_->channel);
            break;

        case Mixer::State::fading_out:
            throw Client_logic_error("Sound_effect_handle::pause: fading out");
    }
}

void Sound_effect_handle::stop()
{
    switch (ptr_->state) {
        case Mixer::State::detached:
            throw Client_logic_error("Sound_effect_handle::stop: detached");

        case Mixer::State::paused:
            ptr_->mixer.unregister_effect_(ptr_->channel);
            Mix_HaltChannel(ptr_->channel);
            break;

        case Mixer::State::playing:
            ptr_->mixer.unregister_effect_(ptr_->channel);
            Mix_HaltChannel(ptr_->channel);
            break;

        case Mixer::State::fading_out:
            throw Client_logic_error("Sound_effect_handle::stop: fading out");
    }
}

void Mixer::pause_all_effects()
{
    Mix_Pause(-1);

    for (const auto& handle : channels_) {
        if (handle && handle.ptr_->state == State::playing)
            handle.ptr_->state = State::paused;
    }
}

void Mixer::resume_all_effects()
{
    Mix_Resume(-1);

    for (const auto& handle : channels_) {
        if (handle && handle.ptr_->state == State::paused)
            handle.ptr_->state = State::playing;
    }
}

int Mixer::available_effect_channels() const
{
    return available_effect_channels_;
}

Sound_effect_handle
Mixer::register_effect_(int channel, Sound_effect effect)
{
    assert(!channels_[channel]);
    channels_[channel] = Sound_effect_handle(*this, std::move(effect), channel);
    --available_effect_channels_;
    return channels_[channel];
}

void Mixer::unregister_effect_(int channel)
{
    assert(channels_[channel]);
    channels_[channel].ptr_->state = State::detached;
    channels_[channel] = {};
    ++available_effect_channels_;
}

double Mixer::get_music_volume() const
{
    return volume_to_unit(Mix_VolumeMusic(-1));
}

void Mixer::set_music_volume(double unit_value)
{
    Mix_VolumeMusic(unit_to_volume(unit_value));
}

bool Sound_effect_handle::empty() const
{
    return ptr_ == nullptr;
}

Sound_effect_handle::operator bool() const
{
    return !empty();
}

Sound_effect_handle::Sound_effect_handle(Mixer& mixer,
                                         Sound_effect effect,
                                         int channel)
        : ptr_(std::make_shared<Impl_>(mixer, std::move(effect), channel))
{ }

double Sound_effect_handle::get_volume() const
{
    if (ptr_->state == Mixer::State::detached)
        return 0;
    else
        return volume_to_unit(Mix_Volume(ptr_->channel, -1));
}

void Sound_effect_handle::set_volume(double unit_value)
{
    if (ptr_->state != Mixer::State::detached)
        Mix_Volume(ptr_->channel, unit_to_volume(unit_value));
}


} // end namespace audio

} // end namespace ge211
