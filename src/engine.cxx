#include "ge211/engine.hxx"
#include "ge211/base.hxx"
#include "ge211/render.hxx"
#include "ge211/sprites.hxx"

#include "utf8.h"

#include <SDL.h>

#include <algorithm>
#include <cstring>

namespace ge211 {

namespace detail {

// Used to control the frame rate if we have to fallback to
// software rendering, or for when the window is hidden (and
// vsync stops working).
static const int software_fps = 60;
static const Duration software_frame_length = Duration(1) / software_fps;
static const Duration min_frame_length = software_frame_length / 2;

Engine::Engine(Abstract_game& game)
        : game_{game},
          window_{
                  game_.initial_window_title(),
                  game_.initial_window_dimensions(),
          },
          renderer_{window_}
{
    game_.engine_ = this;
}

Engine::~Engine()
{
    game_.engine_ = nullptr;
}

void
Engine::prepare(const sprites::Sprite& sprite) const
{
    sprite.prepare(renderer_);
}

struct Engine::State_
{
    Engine& engine;
    bool has_vsync;
    SDL_Event event{};
    Sprite_set sprite_set{};

    explicit State_(Engine& engine);

    bool run_cycle();
};

Engine::State_::State_(Engine& engine)
        : engine(engine),
          has_vsync(engine.renderer_.is_vsync())
{ }

bool
Engine::State_::run_cycle()
{
    auto& game = engine.game_;
    auto& clock = game.clock_;
    auto& renderer = engine.renderer_;

    clock.mark_frame();
    auto frame_length = game.clock_.prev_frame_length();

    engine.handle_events_(event);
    game.on_frame(frame_length.seconds());

    if (game.quit_) {
        return false;
    }

    game.poll_channels_();
    game.draw(sprite_set);

    renderer.set_color(game.background_color);
    renderer.clear();
    engine.paint_sprites_(sprite_set);

    Duration allowed_frame_length =
            (engine.is_focused_ && has_vsync) ?
            min_frame_length : software_frame_length;

    if (frame_length < allowed_frame_length) {
        auto duration = allowed_frame_length - frame_length;
        duration.sleep_for_();
        internal::logging::debug()
                << "Software vsync slept for "
                << duration.seconds() << " s";
    }

    clock.mark_present();
    renderer.present();

    return true;
}

void
Engine::run()
{
    try {
        State_ state(*this);
        game_.on_start();
        while (state.run_cycle()) { }
        game_.on_quit();
    }

    catch (const Exception_base& e) {
        internal::logging::fatal()
                << "Uncaught exception:\n  "
                << e.what();
        exit(1);
    }
}

void
Engine::handle_events_(SDL_Event& e)
{
    while (SDL_PollEvent(&e) != 0) {
        switch (e.type) {
        case SDL_QUIT:
            game_.quit();
            break;

        case SDL_TEXTINPUT: {
            const char *str = e.text.text;
            const char *end = str + std::strlen(str);

            while (str < end) {
                uint32_t code = utf8::next(str, end);
                if (code) { game_.on_key(Key{code}); }
            }

            break;
        }

        case SDL_KEYDOWN: {
            Key key(e.key);
            if (!e.key.repeat) {
                game_.on_key_down(key);
            }
            if (!key.is_textual()) {
                game_.on_key(key);
            }
            break;
        }

        case SDL_KEYUP:
            game_.on_key_up(Key{e.key});
            break;

        case SDL_MOUSEBUTTONDOWN: {
            Mouse_button button;
            if (map_button(e.button.button, button)) {
                game_.on_mouse_down(button, {e.button.x, e.button.y});
            }
            break;
        }

        case SDL_MOUSEBUTTONUP: {
            Mouse_button button;
            if (map_button(e.button.button, button)) {
                game_.on_mouse_up(button, {e.button.x, e.button.y});
            }
            break;
        }

        case SDL_MOUSEMOTION:
            game_.on_mouse_move({e.motion.x, e.motion.y});
            break;

        case SDL_WINDOWEVENT:
            switch (e.window.event) {
            case SDL_WINDOWEVENT_FOCUS_GAINED:
                is_focused_ = true;
                break;

            case SDL_WINDOWEVENT_FOCUS_LOST:
                is_focused_ = false;
                break;

            default:;
            }
            break;

        default:;
        }
    }
}

void
Engine::paint_sprites_(Sprite_set& sprite_set)
{
    auto& vec = sprite_set.sprites_;
    auto begin = vec.begin(),
            end = vec.end();

    std::make_heap(begin, end);

    while (begin != end) {
        std::pop_heap(begin, end--);
        end->render(renderer_);
    }

    vec.clear();
}

Window&
Engine::get_window() NOEXCEPT_
{
    return window_;
}

} // end namespace detail

}
