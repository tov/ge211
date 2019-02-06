#include "ge211_base.h"
#include "ge211_engine.h"
#include "ge211_error.h"

#include <SDL.h>

namespace ge211 {

using namespace detail;

// Storage for constexprs, just in case.
const Dimensions Abstract_game::default_window_dimensions{800, 600};
const char* const Abstract_game::default_window_title = "ge211 window";
const Color Abstract_game::default_background_color = Color::black();

// How many frames to run before calculating the frame rate.
static int const frames_per_sample = 60;

Dimensions Abstract_game::initial_window_dimensions() const
{
    return default_window_dimensions;
}

std::string Abstract_game::initial_window_title() const
{
    return default_window_title;
}

void Abstract_game::run()
{
    Engine(*this).run();
}

void Abstract_game::quit() noexcept
{
    quit_ = true;
}

Window& Abstract_game::get_window() const
{
    if (engine_) return engine_->get_window();

    throw Client_logic_error{"Abstract_game::window: Window does not exist "
                             "until engine is initialized"};
}

Random& Abstract_game::get_random() const noexcept
{
    return rng_;
}

audio::Mixer* Abstract_game::get_mixer() const noexcept
{
    return mixer_.get();
}

void Abstract_game::prepare(const sprites::Sprite& sprite) const
{
    if (engine_)
        engine_->prepare(sprite);
    else {
        warn() << "Abstract_game::prepare: Could not prepare sprite "
               << "because engine is not initialized";
    }
}

void Abstract_game::mark_frame_() noexcept
{
    prev_frame_length_ = frame_start_.reset();

    if (! (fps_sample_count_ = (fps_sample_count_ + 1) % frames_per_sample))
        fps_ = frames_per_sample / fps_sample_start_.reset().seconds();
}

void Abstract_game::on_key_down(Key key)
{
    if (key.code() == '\u001B') quit();
}

} // end namespace ge211
