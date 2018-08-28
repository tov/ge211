#include "ge211_audio.h"
#include "ge211_resource.h"

#include <SDL.h>
#include <SDL_mixer.h>

namespace ge211 {

using namespace detail;

namespace audio {

void Music_track::unpause(Duration fade_in)
{
    switch (get_state()) {
        case Mixer_state::playing:
            // idempotent;
            break;

        case Mixer_state::halted:
            pos_.reset();
            pos_.unpause();
            fade_in_(fade_in, 0);
            state_ = Mixer_state::playing;
            break;

        case Mixer_state::paused: {
            Duration point = pos_.elapsed_time();
            pos_.unpause();
            fade_in_(fade_in, point);
            state_ = Mixer_state::playing;
        }
        break;

        case Mixer_state::fading_out:
            throw Client_logic_error("Audio_track::play: fading out");

        case Mixer_state::detached:
            throw Client_logic_error("Audio_track::play: detached");
    }
}

void Music_track::pause(Duration fade_out)
{
    switch (get_state()) {
        case Mixer_state::playing:
            pos_.pause();
            fade_out_(fade_out);
            state_ = Mixer_state::paused;
            break;

        case Mixer_state::halted:
            // allowed (stronger)
            break;

        case Mixer_state::paused:
            // idempotent
            break;

        case Mixer_state::fading_out:
            throw Client_logic_error("Audio_track::pause: fading out");

        case Mixer_state::detached:
            throw Client_logic_error("Audio_track::pause: detached");
    }
}

void Music_track::reset()
{
    switch (get_state()) {
        case Mixer_state::playing:
            throw Client_logic_error("Audio_track::reset: still playing");

        case Mixer_state::halted:
            pos_.pause();
            pos_.reset();
            break;

        case Mixer_state::paused:
            pos_.reset();
            break;

        case Mixer_state::fading_out:
            throw Client_logic_error("Audio_track::reset: fading out");

        case Mixer_state::detached:
            pos_.reset();
            break;
    }
}

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

void Music_track::fade_in_(time::Duration dur, time::Duration offset)
{
    Mix_RewindMusic();
    Mix_FadeInMusicPos(get_raw_(),
                       0,
                       int(dur.milliseconds()),
                       offset.seconds());
}

void Music_track::fade_out_(time::Duration dur)
{
    Mix_FadeOutMusic(int(dur.milliseconds()));
}

void Music_track::poll_state_()
{
    if (state_ == Mixer_state::detached) return;

    if (Mix_PlayingMusic()) {
        if (Mix_FadingMusic() == MIX_FADING_OUT)
            state_ = Mixer_state::fading_out;
        else
            state_ = Mixer_state::playing;
    } else {
        if (pos_.is_paused())
            state_ = Mixer_state::paused;
        else {
            pos_.pause();
            pos_.reset();
            state_ = Mixer_state::halted;
        }
    }
}

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

void Effect_track::pause()
{
    switch (get_state()) {
        case Mixer_state::playing:
            Mix_Pause(channel_);
            state_ = Mixer_state::paused;
            break;

        case Mixer_state::halted:
            throw Client_logic_error("Effect_track::pause: halted");

        case Mixer_state::paused:
            // idempotent
            break;

        case Mixer_state::fading_out:
            throw Client_logic_error("Effect_track::pause: fading out");

        case Mixer_state::detached:
            throw Client_logic_error("Effect_track::pause: detached");
    }
}

void Effect_track::unpause()
{
    switch (get_state()) {
        case Mixer_state::playing:
            // idempotent
            break;

        case Mixer_state::halted:
            throw Client_logic_error("Effect_track::unpause: halted");

        case Mixer_state::paused:
            Mix_Resume(channel_);
            state_ = Mixer_state::playing;
            break;

        case Mixer_state::fading_out:
            throw Client_logic_error("Effect_track::unpause: fading out");

        case Mixer_state::detached:
            throw Client_logic_error("Effect_track::unpause: detached");
    }
}

void Effect_track::fade_halt(time::Duration duration)
{
    switch (get_state()) {
        case Mixer_state::playing:
            Mix_FadeOutChannel(channel_, int(duration.milliseconds()));
            state_ = Mixer_state::fading_out;
            break;

        case Mixer_state::paused:
            Mix_HaltChannel(channel_);
            state_ = Mixer_state::halted;
            break;

        case Mixer_state::halted:
        case Mixer_state::fading_out:
        case Mixer_state::detached:
            // Stronger or idempotent
            break;
    }
}

double Effect_track::get_volume() const
{
    return get_raw_()->volume / double(MIX_MAX_VOLUME);
}

void Effect_track::set_volume(double unit_value)
{
    Mix_VolumeChunk(get_raw_(), int(unit_value * MIX_MAX_VOLUME));
}

void Effect_track::poll_state_()
{
    if (state_ == Mixer_state::detached) return;

    if (Mix_Playing(channel_)) {
        if (Mix_Fading(channel_) == MIX_FADING_OUT)
            state_ = Mixer_state::fading_out;
        else if (Mix_Paused(channel_))
            state_ = Mixer_state::paused;
        else
            state_ = Mixer_state::playing;
    } else {
        state_ = Mixer_state::halted;
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
        , effect_channels_(MIX_CHANNELS, nullptr)
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

void Mixer::play_music(const std::shared_ptr<Music_track>& track,
                       bool start_halted)
{
    if (current_music_) {
        switch (current_music_->get_state()) {
            case Mixer_state::paused:
            case Mixer_state::halted:
            case Mixer_state::detached:
                current_music_->state_ = Mixer_state::detached;
                break;

            case Mixer_state::playing:
                throw Client_logic_error("Mixer::route_music: still playing");

            case Mixer_state::fading_out:
                throw Client_logic_error("Mixer::route_music: fading out");
        }
    }

    current_music_ = track;

    if (current_music_) {
        if (current_music_->pos_.elapsed_time() == 0.0)
            current_music_->state_ = Mixer_state::halted;
        else
            current_music_->state_ = Mixer_state::paused;

        if (!start_halted) current_music_->unpause(0.0);
    }
}

std::shared_ptr<Effect_track> Mixer::load_effect(const std::string& filename)
{
    File_resource file_resource{filename};
    std::shared_ptr<Effect_track> result(
            new Effect_track(filename, std::move(file_resource)));
    return result;
}

int Mixer::number_of_effect_channels() const
{
    return int(effect_channels_.size());
}

void Mixer::play_effect(const std::shared_ptr<Effect_track>& effect_track,
                        int channel,
                        time::Duration fade_in,
                        time::Duration duration)
{
    // If a non-negative channel is specified, we may need to detach an effect
    // from the given channel.
    if (channel >= 0) {
        if (channel >= number_of_effect_channels())
            throw Client_logic_error("Mixer::play_effect: bad channel");

        const auto& old_effect = effect_channels_[channel];
        if (old_effect) {
            switch (old_effect->get_state()) {
                case Mixer_state::playing:
                case Mixer_state::fading_out:
                    throw Client_logic_error(
                            "Mixer::play_effect: channel already in use");

                case Mixer_state::paused:
                case Mixer_state::halted:
                case Mixer_state::detached:
                    old_effect->state_ = Mixer_state::detached;
                    break;
            }
        }
        effect_channels_[channel] = nullptr;
    }

    int new_channel = Mix_FadeInChannelTimed(channel,
                                             effect_track->get_raw_(),
                                             0,
                                             int(fade_in.milliseconds()),
                                             int(duration.milliseconds()));
    if (new_channel < 0) throw Mixer_error::out_of_channels();

    if (channel < 0) {
        const auto& old_effect = effect_channels_[new_channel];
        if (old_effect)
            old_effect->state_ = Mixer_state::detached;
    }

    effect_track->state_ = Mixer_state::playing;
    effect_channels_[new_channel] = effect_track;
}

const std::shared_ptr<Effect_track>&
Mixer::get_effect(int channel) const
{
    if (channel < 0 || channel >= number_of_effect_channels())
        throw Client_logic_error("Mixer::get_effect: channel out of range");

    return effect_channels_[channel];
}

void Mixer::poll_state_()
{
    if (current_music_)
        current_music_->poll_state_();

    for (const auto& effect_track : effect_channels_)
        if (effect_track)
            effect_track->poll_state_();
}

} // end namespace audio

} // end namespace ge211
