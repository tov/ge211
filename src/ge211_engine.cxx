#include "ge211_engine.hxx"
#include "ge211_base.hxx"
#include "ge211_render.hxx"
#include "ge211_sprites.hxx"

#include <SDL.h>
#include "utf8.h"

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
          window_{game_.initial_window_title(), game_.initial_window_dimensions()},
          renderer_{window_}
{
    game_.engine_ = this;
}

Engine::~Engine()
{
    game_.engine_ = nullptr;
}

void Engine::prepare(const sprites::Sprite& sprite) const
{
    sprite.prepare(renderer_);
}

void Engine::run()
{
    SDL_Event e;
    Sprite_set sprites;

    bool has_vsync = renderer_.is_vsync();

    try {
        game_.on_start();

        while (!game_.quit_) {
            handle_events_(e);
            game_.on_frame(game_.get_prev_frame_length().seconds());
            game_.poll_channels_();
            game_.draw(sprites);

            renderer_.set_color(game_.background_color);
            renderer_.clear();
            paint_sprites_(sprites);

            game_.mark_present_();
            renderer_.present();

            Duration allowed_frame_length =
                    (is_focused_ && has_vsync)?
                    min_frame_length : software_frame_length;

            auto frame_length = game_.frame_start_.elapsed_time();
            if (frame_length < allowed_frame_length) {
                auto duration = allowed_frame_length - frame_length;
                duration.sleep_for();
                game_.mark_frame_();
                internal::logging::debug()
                    << "Software vsync slept for "
                    << duration.seconds() << " s";
            } else {
                game_.mark_frame_();
            }
        }

        game_.on_quit();
    } catch (const Exception_base& e) {
        internal::logging::fatal()
            << "Uncaught exception:\n  "
            << e.what();
        exit(1);
    }
}

void Engine::handle_events_(SDL_Event& e)
{
    while (SDL_PollEvent(&e) != 0) {
        switch (e.type) {
            case SDL_QUIT:
                game_.quit();
                break;

            case SDL_TEXTINPUT: {
                const char* str = e.text.text;
                const char* end = str + std::strlen(str);

                while (str < end) {
                    uint32_t code = utf8::next(str, end);
                    if (code) game_.on_key(Key{code});
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
                if (map_button(e.button.button, button))
                    game_.on_mouse_down(button, {e.button.x, e.button.y});
                break;
            }

            case SDL_MOUSEBUTTONUP: {
                Mouse_button button;
                if (map_button(e.button.button, button))
                    game_.on_mouse_up(button, {e.button.x, e.button.y});
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

                    default:
                        ;
                }
                break;

            default:
                ;
        }
    }
}

void Engine::paint_sprites_(Sprite_set& sprite_set)
{
    auto& vec   = sprite_set.sprites_;
    auto  begin = vec.begin(),
          end   = vec.end();

    std::make_heap(begin, end);

    while (begin != end) {
        std::pop_heap(begin, end--);
        end->render(renderer_);
    }

    vec.clear();
}

Window& Engine::get_window() NOEXCEPT
{
    return window_;
}

} // end namespace detail

}
