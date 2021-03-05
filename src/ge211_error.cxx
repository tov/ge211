#include "ge211_error.hxx"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <iostream>

namespace ge211 {

namespace exceptions {

static const char* take_sdl_error()
{
    const char* result = SDL_GetError();
    SDL_ClearError();
    return result;
}

const char* Exception_base::what() const NOEXCEPT
{
    return message_->c_str();
}

Exception_base::Exception_base(const std::string& message)
    : message_{std::make_shared<std::string>(message)}
{ }

Environment_error::Environment_error(const std::string& message)
        : Exception_base(message)
{ }

Client_logic_error::Client_logic_error(const std::string& message)
        : Exception_base(message)
{ }

static std::string build_no_session_message(const std::string& action) {
    std::ostringstream oss;
    oss << "\n\nERROR\n=====\n\n"
        << action << " requires an active GE211 session. GE211 sessions\n"
        << "are managed RAII-style by the ge211::Abstract_game class, so\n"
        << "a session will be active whenever you have an instance of a\n"
        << "class derived from Abstract_game, including within that derived\n"
        << "game class's constructor and member functions.\n";

    return oss.str();
}

Session_needed_error::Session_needed_error(const std::string& action)
        : Client_logic_error(build_no_session_message(action))
        , action_(action)
{ }

static std::string build_late_paint_message(char const* who) {
    std::ostringstream oss;
    oss << "\n\nERROR\n=====\n\n"
        << who
        << ": Cannot paint to a ge211::internal::Render_sprite\n"
           "that has already been rendered.\n";

    return oss.str();
}

Late_paint_error::Late_paint_error(char const* who)
        : Client_logic_error(build_late_paint_message(who))
{ }

static std::string build_sdl_error_message(const std::string& message) {
    const char* reason = take_sdl_error();

    std::ostringstream oss;
    if (message.empty()) {
        oss << "SDL Error";
        if (reason[0]) {
            oss << ": " << reason;
        }
    } else {
        oss << message;
        if (reason[0]) {
            oss << "\n  (reason from SDL: " << reason << ")";
        }
    }

    return oss.str();
}

Host_error::Host_error(const std::string& message)
        : Environment_error{build_sdl_error_message(message)}
{ }

File_error::File_error(const std::string& message)
        : Host_error{message}
{ }

File_error File_error::could_not_open(const std::string& filename)
{
    return File_error("Could not open: " + filename);
}

Font_error::Font_error(const std::string& message)
        : Host_error{message}
{ }

Font_error Font_error::could_not_load(const std::string& filename)
{
    return Font_error("Could not load font: " + filename);
}

Ge211_logic_error::Ge211_logic_error(const std::string& message)
        : Environment_error("Apparent ge211 bug! " + message)
{ }

Image_error::Image_error(const std::string& message)
        : Host_error{message}
{ }

Image_error Image_error::could_not_load(const std::string& filename)
{
    return Image_error("Could not load image: " + filename);
}

Mixer_error::Mixer_error(const std::string& message)
        : Host_error{message}
{ }

Mixer_error Mixer_error::could_not_load(const std::string& filename)
{
    return Mixer_error("Could not load music: " + filename);
}

Mixer_error Mixer_error::out_of_channels()
{
    return Mixer_error("Could not play effect: out of channels");
}

Mixer_error Mixer_error::not_enabled()
{
    return Mixer_error("Mixer is not enabled");
}

}

namespace internal {

namespace logging {

static const char*
log_level_string(Log_level level)
{
    switch (level) {
        case Log_level::debug:
            return "debug";
        case Log_level::info:
            return "info";
        case Log_level::warn:
            return "warn";
        case Log_level::fatal:
            return "fatal";
    }

    // Shouldn't happen, because switch above is exhaustive. But this
    // makes gcc warn less.
    return "<unknown>";
}

Log_message debug(std::string reason)
{
    return Log_message{std::move(reason), Log_level::debug};
}

Log_message info(std::string reason)
{
    return Log_message{std::move(reason), Log_level::info};
}

Log_message warn(std::string reason)
{
    return Log_message{std::move(reason), Log_level::warn};
}

Log_message fatal(std::string reason)
{
    return Log_message{std::move(reason), Log_level::fatal};
}

Logger& Logger::instance() NOEXCEPT
{
    static Logger instance;
    return instance;
}

Log_message::Log_message(std::string reason, Log_level level) NOEXCEPT
        : reason_{std::move(reason)}
        , message_{}
        , active_{level >= Logger::instance().level()}
{
    if (active_)
        message_ << "ge211[" << log_level_string(level) << "]: ";
}

Log_message::Log_message(Log_level level)
        : Log_message{"", level}
{ }

Log_message::~Log_message()
{
    if (active_) {
        std::cerr << message_.str();
        if (!reason_.empty()) std::cerr << "\n  (Reason: " << reason_ << ")";
        std::cerr << std::endl;
    }
}

} // end namespace logging

} // end namespace internal

namespace detail {

using namespace internal::logging;

Log_message info_sdl()
{
    return info(take_sdl_error());
}

Log_message warn_sdl()
{
    return warn(take_sdl_error());
}

Log_message fatal_sdl()
{
    return fatal(take_sdl_error());
}

}  // end namespace detail

}

