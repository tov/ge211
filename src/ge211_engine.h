#pragma once

#include "ge211_forward.h"
#include "ge211_render.h"
#include "ge211_window.h"

namespace ge211 {

namespace detail {

class Engine
{
public:
    explicit Engine(Abstract_game&);

    void run();
    void prepare(const sprites::Sprite&) const;
    Window& get_window() noexcept;

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
