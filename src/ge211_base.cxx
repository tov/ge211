#include "ge211_base.hxx"
#include "ge211_engine.hxx"
#include "ge211_error.hxx"

#include <SDL.h>

namespace ge211 {

using namespace detail;

// Storage for constexprs, just in case.
const Dims<int> Abstract_game::default_window_dimensions{800, 600};
const char* const Abstract_game::default_window_title = "ge211 window";
const Color Abstract_game::default_background_color = Color::black();

Dims<int> Abstract_game::initial_window_dimensions() const
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

void Abstract_game::quit() NOEXCEPT
{
    quit_ = true;
}

Window& Abstract_game::get_window() const
{
    if (engine_) return engine_->get_window();

    throw Client_logic_error{"Abstract_game::window: Window does not exist "
                             "until engine is initialized"};
}

void Abstract_game::prepare(const sprites::Sprite& sprite) const
{
    if (engine_)
        engine_->prepare(sprite);
    else {
        internal::logging::warn()
            << "Abstract_game::prepare: Could not prepare sprite "
            << "because engine is not initialized";
    }
}

void Abstract_game::poll_channels_()
{
    if (mixer_.is_forced())
        mixer_->poll_channels_();
}

void Abstract_game::on_key_down(Key key)
{
    if (key.code() == '\u001B') quit();
}

} // end namespace ge211
