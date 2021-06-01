#pragma once

#include "ge211_forward.hxx"
#include "ge211_noexcept.hxx"
#include "ge211_base.hxx"
#include "ge211_render.hxx"
#include "ge211_time.hxx"
#include "ge211_window.hxx"

#ifdef __EMSCRIPTEN__
# include <emscripten/emscripten.h>
# include <emscripten/html5.h>
#endif

namespace ge211 {

namespace detail {

class Engine
{
public:
    explicit Engine(Abstract_game&);

    void run();
    void prepare(const sprites::Sprite&) const;
    Window& get_window() NOEXCEPT;

    ~Engine();

private:
    void handle_events_(SDL_Event&);
    void paint_sprites_(Sprite_set&);

    detail::Frame_clock& clock_()
    { return game_.clock_; }

#ifdef __EMSCRIPTEN__
    friend bool
    em_cycle_callback(double millis, void* user_data);
#endif

    Abstract_game& game_;
    Window window_;
    detail::Renderer renderer_;
    bool is_focused_ = false;

    struct Cycle_state_;
};

} // end namespace detail

}
