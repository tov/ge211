#pragma once

#include "ge211_forward.hxx"
#include "ge211_noexcept.hxx"
#include "ge211_render.hxx"
#include "ge211_window.hxx"

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

    Abstract_game& game_;
    Window window_;
    detail::Renderer renderer_;
    bool is_focused_ = false;
};

} // end namespace detail

}
