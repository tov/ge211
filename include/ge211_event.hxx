#pragma once

#include "ge211_forward.hxx"
#include "ge211_error.hxx"
#include "ge211_doxygen.hxx"

#include <sstream>
#include <string>

namespace ge211 {

/// Types for representing mouse and keyboard events.
namespace events {

/// A representation of a mouse button. This is used by
/// Abstract_game::on_mouse_down(Mouse_button, Posn<int>) and
/// Abstract_game::on_mouse_up(Mouse_button, Posn<int>) to specify
/// which mouse button was depressed or released.
enum class Mouse_button
{
    /// The primary mouse button. This is an ordinary click.
    left,
    /// The tertiary mouse button. If you don’t have an actual mouse
    /// with three buttons then you might not have this.
    middle,
    /// The secondary mouse button. On a track pad you might do this
    /// with control-click or a two-finger tap.
    right,
};

/// Prints a #Mouse_button on a std::ostream. This function prints a
/// representation suitable for debugging, but probably not suitable for
/// end users.
std::ostream& operator<<(std::ostream&, Mouse_button);

} // end namespace events

namespace detail {

// Attempts to convert an SDL mouse button code to a ge211 Mouse_button.
// Returns true on success, or false if the SDL mouse button code does
// not correspond to left, middle, or right.
bool map_button(uint8_t, Mouse_button&) NOEXCEPT_;

// Unicode constants.
static char32_t const lowest_unicode_surrogate = 0xD800;
static char32_t const highest_unicode_surrogate = 0xDFFF;
static char32_t const highest_unicode_code_point = 0x10FFFF;

// Checks for valid Unicode code points.
inline bool is_valid_unicode(char32_t code)
{
    return code < lowest_unicode_surrogate ||
            (code > highest_unicode_surrogate &&
                    code <= highest_unicode_code_point);
}

} // end namespace detail

namespace events {

/// Represents a key on the keyboard.
///
/// The easiest way to detect a key is to create the Key apply you want
/// to detect and then compare for equality. For example, you can create
/// the up arrow Key with `Key::up()`, or the uppercase `M` key with
/// `Key::code('M')`.
///
/// Here's an example of key handling:
///
/// ```cpp
/// void on_key(Key key) override
/// {
///     if (key == Key::code('q'))
///         quit();
///     else if (key == Key::up())
///         move_up();
///     else if (key == Key::down())
///         move_down();
/// }
/// ```
///
/// Another way to recognize a key is to look at its two properties:
///
///  - type() const — will be the apply Key::Type::code if the key
///    represents a Unicode character (code point), or some other apply
///    of the Key::Type enumeration for non-Unicode keys.
///
///  - code() const — will be the Unicode code point apply of the key if
///    type() const is Key::Type::code, and will be `0` otherwise.
///
/// Here is an example distinguishing several keys by switching on their
/// properties:
///
/// ```cpp
/// void on_key(Key key) override
/// {
///     switch (key.type()) {
///     case Key::Type::up:
///         move_up();
///         break;
///     case Key::Type::down:
///         move_down();
///         break;
///     case Key::Type::code: {
///         char c = key.code();
///         switch (c) {
///         case '\b':
///             backspace();
///             break;
///         case '\r':
///             enter();
///             break;
///         default:
///             add_to_buffer(c);
///         }
///         break;
///    }
/// }
/// ```
///
/// Currently this type supports keys that deliver Unicode values using
/// whatever input method is supported by your operating system, as well
/// as the arrow keys and modifier keys shift, control, option/alt, and
/// command/meta. If you need to handle other keys, contact me and I will
/// add them.
class Key
{
public:
    /// \name Constructor and factories
    /// @{

    /// Constructs the empty key, with type Key::Type::other.
    Key() NOEXCEPT_ : Key{Type::other} { }

    /// Constructs a key with the given Unicode code point code.
    /// Throws exceptions::Client_logic_error if `c` is not a valid
    /// Unicode code point. Valid code points are from 0 to
    /// 0x10FFFF, [except for 0xD800 to
    /// 0xDFFF](https://en.wikipedia.org/wiki/Universal_Character_Set_characters#Surrogates).
    static Key code(char32_t c)
    {
        if (detail::is_valid_unicode(c))
            return Key{c};

        throw Client_logic_error{"Not a valid Unicode code point"};
    }

    /// Constructs the up arrow key.
    static Key up() NOEXCEPT_ { return Key{Type::up}; };

    /// Constructs the down arrow key.
    static Key down() NOEXCEPT_ { return Key{Type::down}; };

    /// Constructs the left arrow key.
    static Key left() NOEXCEPT_ { return Key{Type::left}; };

    /// Constructs the right arrow key.
    static Key right() NOEXCEPT_ { return Key{Type::right}; };

    /// Constructs the shift key.
    static Key shift() NOEXCEPT_ { return Key{Type::shift}; };

    /// Constructs the control key.
    static Key control() NOEXCEPT_ { return Key{Type::control}; };

    /// Constructs the alt (or option) key.
    static Key alt() NOEXCEPT_ { return Key{Type::alt}; };

    /// Constructs the command (or meta) key.
    static Key command() NOEXCEPT_ { return Key{Type::command}; };

    /// Constructs an invalid or unknown key. This returns the same
    /// apply as the default constructor Key().
    static Key other() NOEXCEPT_ { return Key{Type::other}; }

    /// @}

    /// The possible types of keys.
    enum class Type
    {
        /// Indicates a key with an Unicode apply, which can be gotten
        /// with Key::code() const.
        code,
        /// The up arrow key.
        up,
        /// The down arrow key.
        down,
        /// The left arrow key.
        left,
        /// The right arrow key.
        right,
        /// The shift key.
        shift,
        /// The control key
        control,
        /// The alt or option key
        alt,
        /// The command or meta key
        command,
        /// Any other, unknown or invalid key.
        other,
    };

    /// The type of the key.
    Type type() const NOEXCEPT_ { return type_; }

    /// The Unicode code point of the key, if it has one.
    char32_t code() const NOEXCEPT_ { return code_; }

    /// Does the key represent printable text? This is true for some but not
    /// all Type::code keys. It's never true for other types of keys.
    bool is_textual() const NOEXCEPT_;

    /// Returns a representation of the key's code as a std::string. This could
    /// be useful if you want to capture typing text, rather than game control,
    /// because concatenating a string to a string is easier than concatenating
    /// the `char32_t` code() const to a string, when that could be an
    /// arbitrary Unicode code point.
    ///
    /// The result of this function is only meaningful when
    /// is_textual() const returns true.
    std::string as_text() const;

private:
    explicit Key(Type type) NOEXCEPT_
            : type_{type},
              code_{0}
    { }

    explicit Key(char32_t c) NOEXCEPT_
            : type_{Type::code},
              code_{c}
    { }

    friend class detail::Engine;
    explicit Key(SDL_KeyboardEvent const&) NOEXCEPT_;

    Type type_;
    char32_t code_;
};

/// Equality for keys.
inline bool operator==(Key a, Key b) NOEXCEPT_
{
    return a.type() == b.type() && a.code() == b.code();
}

/// Disequality for keys.
inline bool operator!=(Key a, Key b) NOEXCEPT_
{
    return !(a == b);
}

/// Prints a Key::Type on a std::ostream. This function prints a representation
/// suitable for debugging, but probably not suitable for end users.
std::ostream& operator<<(std::ostream&, Key::Type);

/// Prints a Key on a std::ostream. This function prints a representation
/// suitable for debugging, but probably not suitable for end users.
std::ostream& operator<<(std::ostream&, Key);

} // end namespace events

}
