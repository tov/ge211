#pragma once

#include "ge211_audio.hxx"
#include "ge211_color.hxx"
#include "ge211_error.hxx"
#include "ge211_event.hxx"
#include "ge211_forward.hxx"
#include "ge211_geometry.hxx"
#include "ge211_noexcept.hxx"
#include "ge211_random.hxx"
#include "ge211_resource.hxx"
#include "ge211_session.hxx"
#include "ge211_time.hxx"

#include <memory>
#include <string>

namespace ge211 {

/** This is the abstract base class for deriving games.
 *
 * To create a new game, you must define a new struct or class that derives
 * from Abstract_game and includes the state for your game, including
 * any model, view (sprites), and controller state. Then you must
 * override various handler functions to specify the behavior of your game.
 * If nothing else, the Abstract_game::draw(Sprite_set&) function must be
 * overridden to specify how to draw your game.
 *
 * For example, here is a game that creates one rectangular
 * [Sprite](@ref sprites::Sprite) and renders it on the screen:
 *
 * ```cpp
 * #include <ge211.hxx>
 *
 * const ge211::Dims<int> dimensions {300, 200};
 * const ge211::Color     color {200, 0, 0};     // red
 *
 * struct My_game : ge211::Abstract_game
 * {
 *     void draw(ge211::Sprite_set& sprites) override;
 *
 *     ge211::Rectangle_sprite rect{dimensions, color};
 * };
 *
 * void My_game::draw(ge211::Sprite_set& sprites)
 * {
 *     sprites.add_sprite(rect, ge211::Posn<int>(100, 100));
 * }
 *
 * int main()
 * {
 *     My_game().run();
 * }
 * ```
 *
 * Note that sprites must be created outside draw(Sprite_set&), because they
 * need to continue to exist after that function returns. Thus, the usual place
 * to define sprites is as member variables of your game struct or class. In
 * more advanced cases, you may store sprites in a view class.
 *
 * Here is a game that creates one circular sprite and renders it wherever
 * the mouse goes:
 *
 * ```cpp
 * #include <ge211.hxx>
 *
 * struct My_game2 : ge211::Abstract_game
 * {
 *     // Constructs a new game with the given cursor size.
 *     explicit My_game2(int cursor_size);
 *
 *     ///
 *     /// Function members
 *     ///
 *
 *     // Called by the framework whenever the user moves the mouse. We
 *     // override this function so we can react to that event.
 *     void on_mouse_move(ge211::Posn<int> mouse) override;
 *
 *     // Called by the framework to find out what sprites to render on
 *     // the screen. It passes our function an empty Sprite_set, which
 *     // we add sprite(s) to with Sprite_set::add_sprite.
 *     void draw(ge211::Sprite_set& sprites) override;
 *
 *     ///
 *     /// Data members
 *     ///
 *
 *     // The most recent position of the mouse.
 *     ge211::Posn<int> last_mouse {0, 0};
 *
 *     // The circle sprite to render where the mouse is. This is
 *     // initialized with a radius and a color in the My_game2
 *     // constructor.
 *     ge211::Circle_sprite cursor;
 * };
 *
 * // Initializes cursor, a Circle_sprite, with the requested radius
 * // and a predetermined color.
 * My_game2::My_game2(int cursor_size)
 *         : cursor(cursor_size, ge211::Color::medium_blue())
 * { }
 *
 * // Saves the most recent most position in `last_mouse` each time the
 * // mouse moves.
 * void My_game2::on_mouse_move(ge211::Posn<int> mouse)
 * {
 *     last_mouse = mouse;
 * }
 *
 * // Places the Circle_sprite at the most recent mouse position.
 * void My_game2::draw(ge211::Sprite_set& sprites)
 * {
 *     // If the mouse should be the center of the sprite, where do we
 *     // want the top left to be?
 *     ge211::Posn<int> top_left =
 *         last_mouse.up_left_by(cursor.dimensions() / 2);
 *
 *     sprites.add_sprite(cursor, top_left);
 * }
 *
 * // Constructs an instance of our game and then run it.
 * int main()
 * {
 *     My_game2 game(10);
 *     game.run();
 * }
 * ```
 */
class Abstract_game
{
public:

    /// Runs the game. Usually the way to use this is to create an instance of
    /// your game class in `main` and then call run() on it.
    void run();

    /// The default background color of the window, if not changed by the
    /// derived class. To change the background color, assign the protected
    /// member variable Abstract_game::background_color from the
    /// draw(Sprite_set&) or on_start() functions.
    static const Color default_background_color;

    /// The default initial window title. You can change this in a derived class
    /// by overriding the initial_window_title() const member function.
    static const char* const default_window_title;

    /// The default window dimensions, in pixels. You can change this in a
    /// derived class by overriding the initial_window_dimensions() const member
    /// function.
    static const Dims<int> default_window_dimensions;

    /// Polymorphic classes should have virtual destructors.
    virtual ~Abstract_game() {}

protected:
    /// \name Functions to be overridden by clients
    ///@{

    /// You must override this function in the derived class to specify how
    /// to draw your scene. This function is called by the game engine once
    /// per frame, after handling events. It is passed a Sprite_set; add
    /// sprites to the Sprite_set to have them rendered to the screen.
    ///
    /// Note that the sprites added to the Sprite_set cannot be local
    /// variables owned by the draw(Sprite_set&) function itself, as
    /// they must continue to live after the function returns. For this
    /// reason, they are usually stored as members in the game class, or
    /// in a data structure that is a member of the game class.
    virtual void draw(Sprite_set&) = 0;

    /// Called by the game engine once per frame. The parameter is the duration
    /// of the previous frame in seconds. Override this function to react to time
    /// passing in order to implement animation.
    virtual void on_frame(double last_frame_seconds) {
        (void) last_frame_seconds;
    }

    /// Called by the game engine for each keypress. This uses the system's
    /// repeating behavior, so the user holding down a key can result in multiple
    /// events being delivered. To find out exactly when keys go down and up,
    /// override on_key_down(Key) and on_key_up(Key) instead.
    virtual void on_key(Key) { }

    /// Called by the game engine each time a key is depressed.
    /// Note that this function is delivered the actual key pressed, not the
    /// character that would be generated. For example, if shift is down
    /// and the *5 / %* key is pressed, the delivered key code is `'5'`, not
    /// `'%'`. Similarly, letter keys deliver only lowercase key codes. If
    /// you want key presses interpreted as characters, override on_key(Key)
    /// instead.
    ///
    /// The default behavior of this function, if not overridden, is to quit
    /// if the escape key (code 27) is pressed.
    virtual void on_key_down(Key);

    /// Called by the game engine each time a key is released. This delivers
    /// the same raw key code as on_key_down(Key).
    virtual void on_key_up(Key) { }

    /// Called by the game engine each time a mouse button is depressed.
    virtual void on_mouse_down(Mouse_button, Posn<int>) { }

    /// Called by the game engine each time a mouse button is released.
    virtual void on_mouse_up(Mouse_button, Posn<int>) { }

    /// Called by the game engine each time the mouse moves.
    virtual void on_mouse_move(Posn<int>) { }

    /// Called by the game engine after initializing the game but before
    /// commencing the event loop. You can do this to perform initialization
    /// tasks such as preparing sprites::Sprite%s with
    /// prepare(const Sprite&) const.
    virtual void on_start() { }

    /// Called by the game engine after exiting the event loop but before
    /// the game instance is destroyed. Overriding the function cannot be
    /// used to show anything to the user, since no more rendering will be
    /// performed. It could, however, be used to save a file or shutdown
    /// a network connection.
    ///
    /// Note that this function is called only if the game exits
    /// normally, by calling quit(), or by the user telling
    /// the OS to quit the program. It is not called on exceptions or
    /// errors.
    virtual void on_quit() { }

    /// Override this function to specify the initial dimensions of the
    /// game's window.
    /// This is only called by the engine once at startup.
    virtual Dims<int> initial_window_dimensions() const;

    /// Override this function to specify the initial title of the game.
    /// This is only called by the engine once at startup.
    virtual std::string initial_window_title() const;

    ///@}

    /// \name Functions to be called by clients
    ///@{

    /// Causes the event loop to quit after the current frame finishes.
    void quit() NOEXCEPT;

    /// Gets the Window that the game is running in. This can be used to query
    /// its size, change its title, etc.
    ///
    /// exceptions::Client_logic_error will be thrown if this function is
    /// called before the window is created by `run()`.
    Window& get_window() const;

    /// Gets access to the audio mixer, which can be used to play
    /// music and sound effects.
    Mixer& mixer() const
    {
        return *mixer_;
    }

    /// Gets the time point at which the current frame started. This can be
    /// used to measure intervals between events, though it might be better
    /// to use a time::Timer or time::Pausable_timer.
    Time_point get_frame_start_time() const NOEXCEPT
    { return frame_start_.start_time(); }

    /// Returns the duration of the frame right before the frame currently
    /// running. See time::Duration for information on how to use the result.
    Duration get_prev_frame_length() const NOEXCEPT
    { return prev_frame_length_; }

    /// Returns an approximation of the current frame rate in Hz.
    /// Typically we synchronize the frame rate with the video controller, but
    /// accessing it might be useful for diagnosing performance problems.
    double get_frame_rate() const NOEXCEPT
    { return fps_; }

    /// Returns an approximation of the current machine load due to GE211.
    double get_load_percent() const NOEXCEPT
    { return load_; }

    /// Prepares a sprites::Sprite for rendering, without actually including it
    /// in the scene. The first time a sprites::Sprite is rendered, it ordinarily
    /// has to be converted and transferred to video memory. This function
    /// performs that conversion and transfer eagerly instead of waiting
    /// for it to happen the first time the sprites::Sprite is used. Careful use of
    /// preparation can be used to control when pauses happen and make other
    /// parts of the game smoother. The easiest thing is often to prepare
    /// all sprites you intend to use from an overridden `on_start()`
    /// function.
    void prepare(const sprites::Sprite&) const;

    ///@}

    /// Assign this member variable to change the window's background color
    /// in subsequent frames. The usual place to assign this variable is from
    /// your overridden on_start() and/or draw(Sprite_set&) functions.
    Color background_color = default_background_color;

private:
    friend class detail::Engine;

    void mark_present_() NOEXCEPT;
    void mark_frame_() NOEXCEPT;

    void poll_channels_();

    detail::Session session_;
    detail::lazy_ptr<Mixer> mixer_;
    detail::Engine* engine_ = nullptr;

    bool quit_ = false;

    Timer          frame_start_;
    Duration       prev_frame_length_;

    double         fps_            {0};
    double         load_           {100};

    int            sample_counter_ {0};
    Timer          real_time_;
    Pausable_timer busy_time_;
};

}
