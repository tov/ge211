#include "ge211/error.hxx"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <iostream>

using std::ostringstream;
using std::string;

using std::move;
using std::make_shared;

namespace ge211 {

namespace exceptions {

static const char *
take_sdl_error()
{
    const char *result = SDL_GetError();
    SDL_ClearError();
    return result;
}

const char *
Exception_base::what() const NOEXCEPT
{
    return message_->c_str();
}

Exception_base::Exception_base(string message)
        : message_{make_shared<string>(move(message))}
{ }

Environment_error::Environment_error(string message)
        : Exception_base(move(message))
{ }

Client_logic_error::Client_logic_error(string message)
        : Exception_base(move(message))
{ }

static string
build_no_session_message(string const& action)
{
    ostringstream oss;
    oss << "\n\nERROR\n=====\n\n"
        << action << " requires an active GE211 session. GE211 sessions\n"
        << "are managed RAII-style by the ge211::Abstract_game class, so\n"
        << "a session will be active whenever you have an instance of a\n"
        << "class derived from Abstract_game, including within that derived\n"
        << "game class's constructor and member functions.\n";

    return oss.str();
}

Session_needed_error::Session_needed_error(string const& action)
        : Client_logic_error(build_no_session_message(action)),
          action_(action)
{ }

Random_source_error::Random_source_error(string msg)
        : Client_logic_error(move(msg))
{ }

Random_source_bounds_error::Random_source_bounds_error(string msg)
        : Random_source_error(move(msg))
{ }

Random_source_empty_stub_error::Random_source_empty_stub_error(string msg)
        : Random_source_error{move(msg)}
{ }

Random_source_unsupported_error::Random_source_unsupported_error(string msg)
        : Random_source_error{move(msg)}
{ }


Late_paint_error::Late_paint_error(char const *who)
        : Client_logic_error{build_message_(who)}
{ }

string
Late_paint_error::build_message_(char const *who)
{
    ostringstream oss;

    oss << "\n\nERROR\n=====\n\n"
        << who
        << ": Cannot paint to a ge211::internal::Render_sprite\n"
           "that has already been rendered.\n";

    return oss.str();
}


static string
build_sdl_error_message(string const& message)
{
    const char *reason = take_sdl_error();

    ostringstream oss;
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

Host_error::Host_error(string const& message)
        : Environment_error{build_sdl_error_message(message)}
{ }

File_open_error::File_open_error(string const& filename)
        : Host_error{build_message_(filename)},
          filename_{filename}
{ }

string
File_open_error::build_message_(string const& filename)
{
    return "Could not open file: " + filename;
}

Font_load_error::Font_load_error(string const& filename)
        : Host_error{build_message_(filename)},
          filename_{filename}
{ }

string
Font_load_error::build_message_(string const& filename)
{
    return "Could not load font: " + filename;
}

Ge211_logic_error::Ge211_logic_error(string const& message)
        : Environment_error("Apparent ge211 bug! " + message)
{ }

Image_load_error::Image_load_error(string const& filename)
        : Host_error{build_message_(filename)},
          filename_(filename)
{ }

string
Image_load_error::build_message_(string const& filename)
{
    return "Could not load image: " + filename;
}

Audio_load_error::Audio_load_error(string const& filename)
        : Mixer_error{build_message_(filename)}
{ }

Mixer_error::Mixer_error(string const& problem)
        : Host_error{build_message_(problem)}
{ }

string
Mixer_error::build_message_(string const& problem)
{
    return "[Mixer] " + problem;
}

string
Audio_load_error::build_message_(string const& filename)
{
    return "Could not load audio: " + filename;
}

Out_of_channels_error::Out_of_channels_error()
        : Mixer_error{"Could not play effect: out of channels"}
{ }

Mixer_not_enabled_error::Mixer_not_enabled_error()
        : Mixer_error{"Mixer is not enabled"}
{ }

}

namespace internal {

namespace logging {

static const char *
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

Log_message
debug(string reason)
{
    return Log_message{move(reason), Log_level::debug};
}

Log_message
info(string reason)
{
    return Log_message{move(reason), Log_level::info};
}

Log_message
warn(string reason)
{
    return Log_message{move(reason), Log_level::warn};
}

Log_message
fatal(string reason)
{
    return Log_message{move(reason), Log_level::fatal};
}

Logger&
Logger::instance() NOEXCEPT
{
    static Logger instance;
    return instance;
}

Log_message::Log_message(string reason, Log_level level) NOEXCEPT
        : reason_{move(reason)},
          message_{},
          active_{level >= Logger::instance().level()}
{
    if (active_) {
        message_ << "ge211[" << log_level_string(level) << "]: ";
    }
}

Log_message::Log_message(Log_level level)
        : Log_message{"", level}
{ }

Log_message::~Log_message()
{
    if (active_) {
        std::cerr << message_.str();
        if (!reason_.empty()) {
            std::cerr << "\n  (Reason: " << reason_ << ")";
        }
        std::cerr << std::endl;
    }
}

} // end namespace logging

} // end namespace internal

namespace detail {

using namespace internal::logging;

Log_message
info_sdl()
{
    return info(take_sdl_error());
}

Log_message
warn_sdl()
{
    return warn(take_sdl_error());
}

Log_message
fatal_sdl()
{
    return fatal(take_sdl_error());
}

}  // end namespace detail

}

