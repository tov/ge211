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

/// Audio facilities, for playing music and sound effects.
///
/// All audio facilities are accessed via the Mixer, which is accessed
/// via the get_mixer() const member function of Abstract_game. If the
/// Mixer is present (and it may not be), then it can be used to load
/// audio files as Music_track@s and Effect_track@s. The former is for
/// playing continuous background music, whereas the latter is for
/// adding sound effects. See the Mixer documentation for more.
namespace audio {

/// A music track, which can be attached to the Mixer and played.
///
/// The constructor for this class is private, and consequently it cannot be
/// constructed directly; instead, one should call the
/// Mixer::load_music(const std::string&) member function of the Mixer, which
/// returns a std::shared_ptr to a Music_track.
///
/// Note that Music_track has no public member functions. However, a
/// music track can be passed to these Mixer member functions to play it:
///
///  - Mixer::play_music(const std::shared_ptr<Music_track>&, Duration)
///  - Mixer::attach_music(const std::shared_ptr<Music_track>&)
///
/// Note also that the mixer can only play one music track at at time.
class Music_track : private detail::Audio_resource<Mix_Music>
{
private:
    // Friends
    friend ge211::audio::Mixer;

    // Private constructor
    Music_track(const std::string& filename, detail::File_resource&&);

    // Private static factory
    static Mix_Music* load_(const std::string& filename,
                            detail::File_resource&&);
};

/// A sound effect track, which can be attached to a Mixer channel and played.
///
/// The constructor for this class is private, and consequently it cannot be
/// constructed directly; instead, one should call the
/// Mixer::load_effect(const std::string&) member function of the Mixer, which
/// returns a std::shared_ptr to a Effect_track.
///
/// Note that Effect_track has few public member functions. However, an
/// effect track can be passed to the Mixer member function
/// Mixer::play_effect(const std::shared_ptr<Effect_track>&, Duration)
/// to play it.
class Sound_effect : private detail::Audio_resource<Mix_Chunk>
{
public:
    /// Returns the sound effect's volume as a number from 0.0 to 1.0.
    /// Initially this will be 1.0, but you can lower it with
    /// Effect_track::set_volume(double).
    double get_volume() const;
    /// Sets the sound effects volume as a number from 0.0 to 1.0.
    void set_volume(double unit_value);

private:
    // Friends
    friend ge211::audio::Mixer;

    // Private constructor
    Sound_effect(const std::string& filename, detail::File_resource&&);

    // Private static factory
    static Mix_Chunk* load_(const std::string& filename,
                            detail::File_resource&&);
};

/// The entity that coordinates playing all audio tracks.
class Mixer
{
public:
    /// The state of an audio channel.
    enum class State
    {
        /// No track is attached to the channel.
        empty,
        /// Actively playing.
        playing,
        /// In the process of fading out from playing to paused (for music) or
        /// to halted and detached (for sound effects).
        fading_out,
        /// Attached but not playing.
        paused,
    };

    /// \name Playing music
    ///@{

    /// Loads a new music track, returning a shared pointer to the track.
    std::shared_ptr<Music_track> load_music(const std::string& filename);

    /// Attaches the given music track to this mixer and starts it playing.
    void play_music(const std::shared_ptr<Music_track>&,
                    Duration fade_in = 0.0);

    /// Attaches the given music track to this mixer. Give `nullptr` to detach
    /// the current track, if any.
    ///
    /// **PRECONDITIONS**: It is an error to attach music when other music is
    /// playing or fading out.
    void attach_music(const std::shared_ptr<Music_track>&);

    /// Plays the currently attached music from the current saved position,
    /// fading in if requested.
    void unpause_music(Duration fade_in = 0.0);
    /// Pauses the currently attached music, fading out if requested.
    void pause_music(Duration fade_out = 0.0);
    /// Rewinds the music to the beginning. This is only valid when the music
    /// is paused.
    void rewind_music();

    /// Gets the Music_track currently attached to this Mixer, if any.
    const std::shared_ptr<Music_track>& get_music() const
    {
        return current_music_;
    }

    /// Returns the current state of the attached music, if any.
    State get_music_state() const
    {
        return music_state_;
    }

    ///@}

    /// \name Playing sound effects
    ///@{

    /// Loads a new sound effect track, returning a shared pointer to the track.
    std::shared_ptr<Sound_effect> load_effect(const std::string& filename);

    /// How many effect channels are current unattached?
    int available_effect_channels() const;

    /// Attaches the given effect track to a channel of this mixer, starting
    /// the effect playing and returning the channel.
    int play_effect(const std::shared_ptr<Sound_effect>&,
                    Duration fade_in = 0.0);

    /// Pauses the effect on the given channel.
    void pause_effect(int channel);
    /// Unpauses the effect on the given channel.
    void unpause_effect(int channel);
    /// Stops the effect from playing, and unregisters it when finished.
    void stop_effect(int channel, Duration fade_out = 0.0);

    /// Gets the Effect_track currently attached to the given channel.
    const std::shared_ptr<Sound_effect>& get_effect(int channel) const;

    /// Gets the Effect_track currently attached to the given channel.
    State get_effect_state(int channel) const;

    /// Pauses all currently-playing effects.
    void pause_all_effects();
    /// Unpauses all currently-paused effects.
    void unpause_all_effects();

    ///@}

    ///\name Constructors, assignment operators, and destructor
    ///@{

    /// The mixer cannot be copied.
    Mixer(const Mixer&) = delete;
    /// The mixer cannot be moved.
    Mixer(const Mixer&&) = delete;
    /// The mixer cannot be copied.
    Mixer& operator=(const Mixer&) = delete;
    /// The mixer cannot be moved.
    Mixer& operator=(const Mixer&&) = delete;

    /// Destructor, to clean up the mixer's resources.
    ~Mixer();

    ///@}

private:
    // Only an Abstract_game is allowed to create a mixer. (And if there is
    // more than one Abstract_game at a time, we are in trouble.)
    friend ge211::Abstract_game;
    friend ge211::detail::Engine;

    /// Opens the mixer, if possible, returning nullptr for failure.
    static std::unique_ptr<Mixer> open_mixer();

    /// Private constructor -- should not be called.
    Mixer();

    /// Updates the state of the channels.
    void poll_channels_();

    /// Returns the index of an empty channel, or throws if all are full.
    int find_empty_channel_() const;

    /// Registers an effect with a channel.
    void register_effect_(int channel,
                          const std::shared_ptr<Sound_effect>& effect);
    /// Unregisters the effect associated with a channel.
    void unregister_effect_(int channel);

    /// Asserts that the given channel value is in bounds.
    void check_channel_in_bounds_(int channel) const;

private:
    std::shared_ptr<Music_track> current_music_;
    State music_state_{State::empty};
    Pausable_timer music_position_{true};

    std::vector<std::shared_ptr<Sound_effect>> effect_tracks_;
    std::vector<State> effect_states_;
    int available_effect_channels_;
};

} // end namespace audio

} // end namespace ge211