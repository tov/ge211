#pragma once

#include "ge211_forward.h"
#include "ge211_util.h"

namespace ge211 {

namespace audio {

class Mixer
{
public:
    Mixer() noexcept;

    enum class State
    {
        unloaded, halted, paused, playing
    };

    State get_music_state() const;

    void load_music(const std::string& filename);
    void play_music();
    void pause_music();
    void stop_music();
    void unload_music();

private:
    static detail::delete_ptr<Mix_Music>
    load_music_(const std::string&);

    detail::delete_ptr<Mix_Music> music_ptr_;
};

} // end namespace audio

} // end namespace ge211