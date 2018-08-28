#include "ge211_audio.h"
#include "ge211_resource.h"

#include <SDL.h>
#include <SDL_mixer.h>

namespace ge211 {

using namespace detail;

namespace audio {

//void Music_track::unpause(Duration fade_in)
//{
//    switch (get_state()) {
//        case Channel_state::playing:
//            // idempotent;
//            break;
//
//        case Channel_state::halted:
//            pos_.reset();
//            pos_.unpause();
//            fade_in_(fade_in, 0);
//            state_ = Channel_state::playing;
//            break;
//
//        case Channel_state::paused: {
//            Duration point = pos_.elapsed_time();
//            pos_.unpause();
//            fade_in_(fade_in, point);
//            state_ = Channel_state::playing;
//        }
//        break;
//
//        case Channel_state::fading_out:
//            throw Client_logic_error("Audio_track::play: fading out");
//
//        case Channel_state::detached:
//            throw Client_logic_error("Audio_track::play: detached");
//    }
//}
//
//void Music_track::pause(Duration fade_out)
//{
//    switch (get_state()) {
//        case Channel_state::playing:
//            pos_.pause();
//            fade_out_(fade_out);
//            state_ = Channel_state::paused;
//            break;
//
//        case Channel_state::halted:
//            // allowed (stronger)
//            break;
//
//        case Channel_state::paused:
//            // idempotent
//            break;
//
//        case Channel_state::fading_out:
//            throw Client_logic_error("Audio_track::pause: fading out");
//
//        case Channel_state::detached:
//            throw Client_logic_error("Audio_track::pause: detached");
//    }
//}
//
//void Music_track::reset()
//{
//    switch (get_state()) {
//        case Channel_state::playing:
//            throw Client_logic_error("Audio_track::reset: still playing");
//
//        case Channel_state::halted:
//            pos_.pause();
//            pos_.reset();
//            break;
//
//        case Channel_state::paused:
//            pos_.reset();
//            break;
//
//        case Channel_state::fading_out:
//            throw Client_logic_error("Audio_track::reset: fading out");
//
//        case Channel_state::detached:
//            pos_.reset();
//            break;
//    }
//}

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

//void Music_track::fade_in_(time::Duration dur, time::Duration offset)
//{
//    Mix_RewindMusic();
//    Mix_FadeInMusicPos(get_raw_(),
//                       0,
//                       int(dur.milliseconds()),
//                       offset.seconds());
//}
//
//void Music_track::fade_out_(time::Duration dur)
//{
//    Mix_FadeOutMusic(int(dur.milliseconds()));
//}

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

//void Effect_track::pause()
//{
//    switch (get_state()) {
//        case Channel_state::playing:
//            Mix_Pause(channel_);
//            state_ = Channel_state::paused;
//            break;
//
//        case Channel_state::halted:
//            throw Client_logic_error("Effect_track::pause: halted");
//
//        case Channel_state::paused:
//            // idempotent
//            break;
//
//        case Channel_state::fading_out:
//            throw Client_logic_error("Effect_track::pause: fading out");
//
//        case Channel_state::detached:
//            throw Client_logic_error("Effect_track::pause: detached");
//    }
//}

//void Effect_track::unpause()
//{
//    switch (get_state()) {
//        case Channel_state::playing:
//            // idempotent
//            break;
//
//        case Channel_state::halted:
//            throw Client_logic_error("Effect_track::unpause: halted");
//
//        case Channel_state::paused:
//            Mix_Resume(channel_);
//            state_ = Channel_state::playing;
//            break;
//
//        case Channel_state::fading_out:
//            throw Client_logic_error("Effect_track::unpause: fading out");
//
//        case Channel_state::detached:
//            throw Client_logic_error("Effect_track::unpause: detached");
//    }
//}
//
//void Effect_track::fade_halt(time::Duration duration)
//{
//    switch (get_state()) {
//        case Channel_state::playing:
//            Mix_FadeOutChannel(channel_, int(duration.milliseconds()));
//            state_ = Channel_state::fading_out;
//            break;
//
//        case Channel_state::paused:
//            Mix_HaltChannel(channel_);
//            state_ = Channel_state::halted;
//            break;
//
//        case Channel_state::halted:
//        case Channel_state::fading_out:
//        case Channel_state::detached:
//            // Stronger or idempotent
//            break;
//    }
//}

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
    if (music_state_ != Channel_state::paused) {
        throw Client_logic_error("Mixer::rewind_music: must be paused");
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

//void Mixer::play_effect(const std::shared_ptr<Effect_track>& effect_track,
//                        int channel,

//                        time::Duration duration)
//{
//    // If a non-negative channel is specified, we may need to detach an effect
//    // from the given channel.
//    if (channel >= 0) {
//        if (channel >= number_of_effect_channels())
//            throw Client_logic_error("Mixer::play_effect: bad channel");
//
//        const auto& old_effect = effect_channels_[channel];
//        if (old_effect) {
//            switch (old_effect->get_state()) {
//                case Channel_state::playing:
//                case Channel_state::fading_out:
//                    throw Client_logic_error(
//                            "Mixer::play_effect: channel already in use");
//
//                case Channel_state::paused:
//                case Channel_state::halted:
//                case Channel_state::detached:
//                    old_effect->state_ = Channel_state::detached;
//                    break;
//            }
//        }
//        effect_channels_[channel] = nullptr;
//    }
//
//    int new_channel = Mix_FadeInChannelTimed(channel,
//                                             effect_track->get_raw_(),
//                                             0,
//                                             int(fade_in.milliseconds()),
//                                             int(duration.milliseconds()));
//    if (new_channel < 0) throw Mixer_error::out_of_channels();
//
//    if (channel < 0) {
//        const auto& old_effect = effect_channels_[new_channel];
//        if (old_effect)
//            old_effect->state_ = Channel_state::detached;
//    }
//
//    effect_track->state_ = Channel_state::playing;
//    effect_channels_[new_channel] = effect_track;
//}

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
        if (!Mix_PlayingMusic() && !music_position_.is_paused()) {
            music_position_.pause();
            music_state_ = Channel_state::paused;
        }
    }

    for (int channel = 0; channel < effect_channels_.size(); ++channel) {
        if (!Mix_Playing(channel)) {
            effect_states_[channel] = Channel_state::paused;
        }
    }
}

} // end namespace audio

} // end namespace ge211
