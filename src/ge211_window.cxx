#include "ge211_window.hxx"
#include "ge211_error.hxx"

#include <SDL.h>

namespace ge211 {

using namespace detail;

Window::Window(const std::string& title, Dims<int> dim)
        : ptr_{SDL_CreateWindow(title.c_str(),
                                SDL_WINDOWPOS_UNDEFINED,
                                SDL_WINDOWPOS_UNDEFINED,
                                dim.width,
                                dim.height,
                                SDL_WINDOW_SHOWN)}
{
    if (!ptr_)
        throw Host_error{"Could not create window"};
}

uint32_t Window::get_flags_() const NOEXCEPT
{
    return SDL_GetWindowFlags(get_raw_());
}

Dims<int> Window::get_dimensions() const NOEXCEPT
{
    Dims<int> result{0, 0};
    SDL_GetWindowSize(get_raw_(), &result.width, &result.height);
    return result;
}

void Window::set_dimensions(Dims<int> dims)
{
    SDL_SetWindowSize(get_raw_(), dims.width, dims.height);

    if (get_dimensions() != dims)
        throw Environment_error{"Window::set_dimensions: out of range"};
}

#if SDL_VERSION_ATLEAST(2, 0, 5)
bool Window::get_resizeable() const NOEXCEPT
{
    return (get_flags_() & SDL_WINDOW_RESIZABLE) != 0;
}

void Window::set_resizeable(bool resizable) NOEXCEPT
{
    SDL_SetWindowResizable(get_raw_(), resizable? SDL_TRUE : SDL_FALSE);
}
#endif

Posn<int> Window::get_position() const NOEXCEPT
{
    Posn<int> result{0, 0};
    SDL_GetWindowPosition(get_raw_(), &result.x, &result.y);
    return result;
}

void Window::set_position(Posn<int> position)
{
    SDL_SetWindowPosition(get_raw_(), position.x, position.y);
}

const Posn<int> Window::centered{SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED};

bool Window::get_fullscreen() const NOEXCEPT
{
    return (get_flags_() & SDL_WINDOW_FULLSCREEN) != 0;
}

void Window::set_fullscreen(bool fullscreen)
{
    uint32_t flags = fullscreen? SDL_WINDOW_FULLSCREEN : 0;

    if (SDL_SetWindowFullscreen(get_raw_(), flags) < 0)
        throw Host_error{"Window::set_fullscreen: failed"};
}

Dims<int> Window::max_fullscreen_dimensions() NOEXCEPT
{
    SDL_Rect rect;
    SDL_GetDisplayBounds(0, &rect);
    return {rect.w, rect.h};
}

Dims<int> Window::max_window_dimensions() const NOEXCEPT
{
    int top, left, bottom, right;
    SDL_GetWindowBordersSize(get_raw_(), &top, &left, &bottom, &right);

    SDL_Rect rect;
    SDL_GetDisplayUsableBounds(0, &rect);

    return {rect.w - left - right, rect.h - top - bottom};
}

}
