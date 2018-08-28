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
enum class Channel_state
{
    /// No track is attached to this channel.
    empty,
    /// Actively playing.
    playing,
    /// In the process of fading out from playing to paused.
    fading_out,
    /// Not playing.
    paused,
};

/// A music track, which can be attached to the Mixer and played.
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
class Effect_track : private detail::Audio_resource<Mix_Chunk>
{
public:
    /// Returns the sound effect's volume as a number from 0 to 1.
    double get_volume() const;
    /// Sets the sound effects volume as a number from 0 to 1.
    void set_volume(double unit_value);

private:
    // Friends
    friend ge211::audio::Mixer;

    // Private constructor
    Effect_track(const std::string& filename, detail::File_resource&&);

    // Private static factory
    static Mix_Chunk* load_(const std::string& filename,
                            detail::File_resource&&);
};

/// The entity that coordinates playing all audio tracks.
class Mixer
{
public:
    /// \name Playing music
    ///@{

    /// Loads a new music track, returning a shared pointer to the track.
    std::shared_ptr<Music_track> load_music(const std::string& filename);

    /// Attaches the given music track to this mixer and starts it playing.
    void play_music(const std::shared_ptr<Music_track>&,
                    time::Duration fade_in = 0.0);

    /// Attaches the given music track to this mixer. Give `nullptr` to detach
    /// the current track, if any.
    ///
    /// **PRECONDITIONS**: It is an error to attach music when other music is
    /// playing or fading out.
    void attach_music(const std::shared_ptr<Music_track>&);

    /// Plays the currently attached music from the current saved position,
    /// fading in if requested.
    void unpause_music(time::Duration fade_in = 0.0);
    /// Pauses the currently attached music, fading out if requested.
    void pause_music(time::Duration fade_out = 0.0);
    /// Rewinds the music to the beginning. This is only valid when the music
    // is paused.
    void rewind_music();

    /// Gets the Music_track currently attached to this Mixer, if any.
    const std::shared_ptr<Music_track>& get_music() const
    {
        return current_music_;
    }

    /// Returns the current state of the attached music, if any.
    Channel_state get_music_state() const
    {
        return music_state_;
    }

    ///@}

    /// \name Playing sound effects
    ///@{

    /// Loads a new sound effect track, returning a shared pointer to the track.
    std::shared_ptr<Effect_track> load_effect(const std::string& filename);

    /// Attaches the given effect track to a channel of this mixer, starting
    /// the effect playing and returning the channel.
    int play_effect(const std::shared_ptr<Effect_track>&,
                    time::Duration fade_in = 0.0);

    /// Attaches the given effect track to a channel of this mixer, returning
    /// the channel.
    int attach_effect(const std::shared_ptr<Effect_track>&);

    /// Detaches the effect that is attached to the given channel.
    void detach_effect(int channel);

    /// Plays the effect on the given channel.
    void start_effect(int channel, time::Duration fade_in = 0.0);
    /// Pauses the effect on the given channel.
    void pause_effect(int channel);
    /// Unpauses the effect on the given channel.
    void unpause_effect(int channel);
    /// Stops the effect from playing.
    void stop_effect(int channel, time::Duration fade_out = 0.0);

    /// Gets the Effect_track currently attached to the given channel.
    const std::shared_ptr<Effect_track>& get_effect(int channel) const;

    /// Gets the Effect_track currently attached to the given channel.
    Channel_state get_effect_state(int channel) const;

    ///@}

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

    /// Updates the state of the attached music and sound effects.
    void poll_state_();

private:
    std::shared_ptr<Music_track> current_music_;
    Channel_state music_state_{Channel_state::empty};
    time::Pausable_timer music_position_{true};

    std::vector<std::shared_ptr<Effect_track>> effect_channels_;
    std::vector<Channel_state> effect_states_;
};

} // end namespace audio

} // end namespace ge211