#pragma once

#include "ge211_forward.hxx"
#include "ge211_doxygen.hxx"
#include "ge211_base.hxx"
#include "ge211_render.hxx"
#include "ge211_time.hxx"
#include "ge211_window.hxx"

namespace ge211 {

namespace detail {

class Engine
{
public:
    // TODO XXX
    void identify(char const*) const;

    explicit Engine(Abstract_game&);

    void run();
    void prepare(const sprites::Sprite&) const;
    Window& get_window() NOEXCEPT_;

    ~Engine();

private:
    void handle_events_(SDL_Event&);
    void paint_sprites_(Sprite_set&);

    detail::Frame_clock& clock_()
    { return game_.clock_; }

#ifdef __EMSCRIPTEN__
    friend void
    em_cycle_callback(void *user_data);
#endif

    Abstract_game& game_;
    Window window_;
    detail::Renderer renderer_;
    bool is_focused_ = false;

    struct State_;
};

} // end namespace detail

}
