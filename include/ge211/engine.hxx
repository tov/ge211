#pragma once

#include "forward.hxx"
#include "doxygen.hxx"
#include "base.hxx"
#include "render.hxx"
#include "time.hxx"
#include "window.hxx"

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

    Abstract_game& game_;
    Window window_;
    detail::Renderer renderer_;
    bool is_focused_ = false;

    struct State_;
};

} // end namespace detail

}
