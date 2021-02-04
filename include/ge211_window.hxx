#pragma once

#include "ge211_forward.hxx"
#include "ge211_geometry.hxx"
#include "ge211_noexcept.hxx"
#include "ge211_util.hxx"

#include <SDL_version.h>
#include <SDL_video.h>

#include <string>

namespace ge211 {

/// Provides access to the game window and its properties.
class Window
{
public:
    /// Returns the current dimensions of this window.
    Dims<int> get_dimensions() const NOEXCEPT;
    /// Changes the size of the window. Throws exceptions::Environment_error
    /// if the dimensions are negative or outside the allowable range.
    void set_dimensions(Dims<int>);

    /// Gets the position of the upper-left corner of the window with
    /// respect to the upper-left corner of the screen.
    Posn<int> get_position() const NOEXCEPT;
    /// Sets the position of the upper-left corner of the window with
    /// respect to the upper-left corner of the screen.
    void set_position(Posn<int>);
    /// A special value to pass to set_position(Posn<int>) to center
    /// the window on the screen.
    static const Posn<int> centered;

#if SDL_VERSION_ATLEAST(2, 0, 5)
    /// Returns whether the user can resize this window.
    bool get_resizeable() const NOEXCEPT;
    /// Changes whether the user can resize this window.
    void set_resizeable(bool) NOEXCEPT;
#endif

    /// Returns whether the program is in fullscreen mode.
    bool get_fullscreen() const NOEXCEPT;
    /// Sets whether the program should be in fullscreen mode. Throws
    /// exceptions::Host_error if change fails.
    void set_fullscreen(bool);

    /// Returns the maximum dimensions for a non-fullscreen window.
    /// This is the size of the screen, minus space reserved for the
    /// system (such as the Windows taskbar or Mac menu and dock).
    Dims<int> max_window_dimensions() const NOEXCEPT;

    /// Returns the maximum dimensions for a fullscreen window. Call
    /// this before switching to fullscreen mode, since if you fullscreen
    /// a smaller window, the video mode may change.
    static Dims<int> max_fullscreen_dimensions() NOEXCEPT;

private:
    friend class detail::Engine;
    friend class detail::Renderer;

    Window(const std::string&, Dims<int> dim);

    Borrowed<SDL_Window> get_raw_() const NOEXCEPT { return ptr_.get(); }
    uint32_t get_flags_() const NOEXCEPT;

    detail::delete_ptr<SDL_Window, &SDL_DestroyWindow> ptr_;
};

}
