#pragma once

#include "ge211_forward.h"
#include "ge211_time.h"
#include "ge211_util.h"

#include <memory>
#include <vector>

namespace ge211 {

namespace detail {

template <typename T>
class Audio_resource
{
public:
    Audio_resource(T* ptr, void (*deleter)(T*))
            : ptr_{ptr, deleter}
    { }

    Audio_resource(Audio_resource&) = delete;
    Audio_resource(Audio_resource&&) = delete;
    Audio_resource& operator=(Audio_resource&) = delete;
    Audio_resource& operator=(Audio_resource&&) = delete;

protected:
    T* get_raw_() const { return ptr_.get(); }

private:
    delete_ptr<T> ptr_;
};

} // end namespace detail

/// Audio utilities, for playing music and sound effects.
namespace audio {

/// The state of an audio track.
enum class Mixer_state
{
    /// Currently not attached to any mixer.
    detached,
    /// Actively playing.
    playing,
    /// In the process of fading out from playing to paused.
    fading_out,
    /// Attached, not playing, possibly in the middle.
    paused,
    /// Attached, not playing, positioned at beginning.
    halted,
};

/// A music track, which can be attached to the Mixer and played.
class Music_track : private detail::Audio_resource<Mix_Music>
{
public:
    /// Gets the current state of the audio track.
    Mixer_state get_state() const { return state_; }

    /// Pauses this audio track, with the given fade-out duration.
    void pause(time::Duration fade_out = 0);
    /// Unpauses this audio track, with the given fade-in duration.
    void unpause(time::Duration fade_in = 0);
    /// Resets this audio track to the beginning, in halted state.
    void reset();

private:
    // Friends
    friend ge211::audio::Mixer;

    // Member variables
    Mixer_state state_{Mixer_state::detached};
    time::Pausable_timer pos_{true};

    // Private constructor
    Music_track(const std::string& filename, detail::File_resource&&);

    // Private static factory
    static Mix_Music* load_(const std::string& filename,
                            detail::File_resource&&);

    // Interface to Mixer
    void poll_state_();

    // Private helpers
    void fade_in_(time::Duration dur, time::Duration offset);
    void fade_out_(time::Duration dur);
};

/// A sound effect track, which can be attached to a Mixer channel and played.
class Effect_track : private detail::Audio_resource<Mix_Chunk>
{
public:
    /// Gets the current state of the audio track.
    Mixer_state get_state() const { return state_; }

    /// Pauses the sound effect if playing.
    void pause();
    /// Unpauses the sound effect if paused.
    void unpause();
    /// Fades out and halts the effect.
    void fade_halt(time::Duration duration);

    /// Returns the sound effect's volume as a number from 0 to 1.
    double get_volume() const;
    /// Sets the sound effects volume as a number from 0 to 1.
    void set_volume(double unit_value);

private:
    // Friends
    friend ge211::audio::Mixer;

    // Member variables
    Mixer_state state_;
    int channel_; // The channel we're attached to, if any.

    // Private constructor
    Effect_track(const std::string& filename, detail::File_resource&&);

    // Private static factory
    static Mix_Chunk* load_(const std::string& filename,
                            detail::File_resource&&);

    // Interface to Mixer
    void poll_state_();
};

/// The entity that coordinates playing all audio tracks.
class Mixer
{
public:
    /// Loads a new music track, returning a shared pointer to the track.
    std::shared_ptr<Music_track> load_music(const std::string& filename);

    /// Attaches the given music track to this mixer and (by default) starts
    /// playing the track.
    void play_music(const std::shared_ptr<Music_track>&,
                    bool start_halted = false);

    /// Gets the Music_track currently attached to this Mixer, if any.
    const std::shared_ptr<Music_track>& get_music() const
    {
        return current_music_;
    }

    /// Loads a new sound effect track, returning a shared pointer to the track.
    std::shared_ptr<Effect_track> load_effect(const std::string& filename);

    /// Returns the number of channels on which effects can be played.
    int number_of_effect_channels() const;

    /// Attaches the given sound effect track to the given mixer effect
    /// channel, and (by default) starts playing the track.
    void play_effect(const std::shared_ptr<Effect_track>&,
                     int channel = -1,
                     time::Duration fade_in = 0.0,
                     time::Duration duration = 0.0);

    /// Gets the Effect_track currently attached to the given channel.
    const std::shared_ptr<Effect_track>& get_effect(int channel) const;

    /// The mixer cannot be copied.
    Mixer(const Mixer&) = delete;
    /// The mixer cannot be copied.
    Mixer(const Mixer&&) = delete;
    /// The mixer cannot be moved.
    Mixer& operator=(const Mixer&) = delete;
    /// The mixer cannot be moved.
    Mixer& operator=(const Mixer&&) = delete;

    /// Destructor, cleans up the mixer's resources.
    ~Mixer();

private:
    // Only an Abstract_game is allowed to create a mixer. (And if there is
    // more than one Abstract_game at a time, we are in trouble.)
    friend ge211::Abstract_game;
    friend ge211::detail::Engine;

    /// Opens the mixer, if possible, returning nullptr for failure.
    static std::unique_ptr<Mixer> open_mixer();

    /// Private constructor -- should not be called.
    Mixer();

    /// Updates the state of the routed music.
    void poll_state_();

private:
    std::shared_ptr<Music_track> current_music_;
    std::vector<std::shared_ptr<Effect_track>> effect_channels_;
};

} // end namespace audio

} // end namespace ge211