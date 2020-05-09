#include "ge211_event.hxx"

#include <SDL_events.h>
#include "utf8.h"

#include <cctype>

namespace ge211 {

namespace detail {

bool map_button(uint8_t input, Mouse_button& output) NOEXCEPT
{
    switch (input) {
        case SDL_BUTTON_LEFT:
            output = Mouse_button::left;
            return true;
        case SDL_BUTTON_MIDDLE:
            output = Mouse_button::middle;
            return true;
        case SDL_BUTTON_RIGHT:
            output = Mouse_button::right;
            return true;
        default:
            return false;
    }
}

} // end namespace detail

namespace events {

static Key map_key(const SDL_KeyboardEvent& e) NOEXCEPT
{
    if (e.keysym.sym >= 0 && e.keysym.sym < 128) {
        return Key::code(uint32_t(e.keysym.sym));
    } else {
        switch (e.keysym.sym) {
            case SDLK_KP_ENTER:
                return Key::code('\r');
            case SDLK_UP:
                return Key::up();
            case SDLK_DOWN:
                return Key::down();
            case SDLK_LEFT:
                return Key::left();
            case SDLK_RIGHT:
                return Key::right();
            case SDLK_LSHIFT:
            case SDLK_RSHIFT:
                return Key::shift();
            case SDLK_LCTRL:
            case SDLK_RCTRL:
                return Key::control();
            case SDLK_LALT:
            case SDLK_RALT:
                return Key::alt();
            case SDLK_LGUI:
            case SDLK_RGUI:
                return Key::command();
            default:
                return Key();
        }
    }
}

Key::Key(const SDL_KeyboardEvent& e) NOEXCEPT
        : Key{map_key(e)}
{ }

static const char* mouse_button_name(Mouse_button button) NOEXCEPT
{
    switch (button) {
        case Mouse_button::left:
            return "left";
        case Mouse_button::middle:
            return "middle";
        case Mouse_button::right:
            return "right";
    }

    return "<unknown>";
}

std::ostream& operator<<(std::ostream& os, Mouse_button button)
{
    return os << mouse_button_name(button);
}

static const char* key_type_name(Key::Type type) NOEXCEPT
{
    switch (type) {
        case Key::Type::code:
            return "ascii";
        case Key::Type::up:
            return "up";
        case Key::Type::down:
            return "down";
        case Key::Type::left:
            return "left";
        case Key::Type::right:
            return "right";
        case Key::Type::shift:
            return "shift";
        case Key::Type::control:
            return "control";
        case Key::Type::alt:
            return "alt";
        case Key::Type::command:
            return "command";
        case Key::Type::other:
            return "other";
    }

    return "<unknown>";
}

std::ostream& operator<<(std::ostream& os, Key::Type type)
{
    return os << key_type_name(type);
}


std::ostream& operator<<(std::ostream& os, Key key)
{
    if (key.type() == Key::Type::code) {
        if (key.code() < 128 && key.is_textual())
            return os << "Key::code('" << char(key.code()) << "')";
        else
            return os << "Key::code(" << key.code() << ")";
    } else {
        return os << "Key::" << key.type() << "()";
    }
}

bool Key::is_textual() const NOEXCEPT
{
    return type_ == Type::code && !iswcntrl(code_);
}

std::string Key::as_text() const
{
    if (!is_textual()) return std::string{};

    char buffer[4];
    char* end = utf8::append(code_, buffer);
    return std::string(buffer, end);
}

} // end namespace events

}
