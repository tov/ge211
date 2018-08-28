#pragma once

#include "ge211_forward.h"
#include "ge211_time.h"
#include "ge211_util.h"

#include <memory>

namespace ge211 {

namespace detail {

template <typename T>
class Audio_resource
{
public:
    Audio_resource(T* ptr, void (*deleter)(T*))
            : ptr_{ptr, deleter}
    { }

protected:
    T* get_raw_() const { return ptr_.get(); }

private:
    delete_ptr<T> ptr_;
};

} // end namespace detail

namespace audio {

class Audio_track
{
public:
    enum class State
    {
        unrouted, halted, playing, fading_out, paused,
    };

    State get_state() const { return state_; }

    void play(time::Duration fade_in = 0);
    void pause(time::Duration fade_out = 0);
    void reset();

    ~Audio_track()
    { }

protected:
    virtual void fade_in(time::Duration dur, time::Duration offset) = 0;
    virtual void fade_out(time::Duration dur) = 0;

    virtual State poll_state_() = 0;
    const time::Pausable_timer& get_position_() { return pos_; }

private:
    friend class ge211::audio::Mixer;

    void update_state_()
    {
        state_ = poll_state_();
    }

    State state_{State::unrouted};
    time::Pausable_timer pos_{true};
};

class Music_track
        : public Audio_track
        , private detail::Audio_resource<Mix_Music>
{
protected:
    void fade_in(time::Duration dur, time::Duration offset) override;
    void fade_out(time::Duration dur) override;
    State poll_state_() override;

private:
    friend ge211::audio::Mixer;

    Music_track(const std::string& filename, detail::File_resource&&);

    static Mix_Music* load_(const std::string& filename,
                            detail::File_resource&&);
};

class Mixer
{
public:
    using State = Audio_track::State;

    std::shared_ptr<Music_track> load_music(const std::string& filename);
    void route_music(std::shared_ptr<Music_track>);

    Mixer(const Mixer&) = delete;
    Mixer(const Mixer&&) = delete;
    Mixer& operator=(const Mixer&) = delete;
    Mixer& operator=(const Mixer&&) = delete;

    /// Destructor, cleans up the mixer's resources.
    ~Mixer();

private:
    // Only an Abstract_game is allowed to create a mixer. (And if there is
    // more than one Abstract_game at a time, we are in trouble.)
    friend class ge211::Abstract_game;
    friend class ge211::detail::Engine;

    /// Opens the mixer, if possible, returning nullptr for failure.
    static std::unique_ptr<Mixer> open_mixer();

    /// Private constructor -- should not be called.
    Mixer();

    /// Updates the state of the routed music.
    void update_state_();

private:
    std::shared_ptr<Music_track> current_music_;
};

} // end namespace audio

} // end namespace ge211