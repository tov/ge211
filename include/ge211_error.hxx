#pragma once

#include "ge211_forward.hxx"
#include "ge211_noexcept.hxx"

#include <exception>
#include <memory>
#include <sstream>
#include <string>

namespace ge211 {

/// An exception hierarchy for %ge211 to report errors.
namespace exceptions {

/// The root of the %ge211 exception hierarchy. Exceptions thrown by
/// %ge211 are derived from Exception_base.
///
/// The constructor of Exception_base is private, which means that
/// you cannot construct it directly, nor can you derive from it.
/// However, its derived class Client_logic_error has a public
/// constructor, so you can use it as you wish.
class Exception_base : public std::exception
{
public:
    /// The error message associated with the exception. This
    /// pointer is guaranteed to be good as long as the exception
    /// exists and hasn't been mutated. If you need it for longer,
    /// copy it to a std::string.
    const char* what() const NOEXCEPT override;

private:
    explicit Exception_base(const std::string& message);

    /// Derived classes
    friend Client_logic_error;
    friend Environment_error;

    std::shared_ptr<const std::string> message_;
};

/// An exception that indicates that a logic error was performed
/// by the client. For example, a Client_logic_error is thrown by
/// Abstract_game::get_window() const if that function is called
/// before the Window has been created. Client code may throw or
/// derive from Client_logic_error as well.
class Client_logic_error : public Exception_base
{
public:
    /// Constructs the error, given the provided error message.
    explicit Client_logic_error(const std::string& message);
};

/// An exception thrown when the client attempts to perform an
/// action that requires a GE211 session before GE211 starts.
/// For example, GE211 needs to initialize the font subsystem
/// before fonts can be loaded, so the `Font` constructor throws
/// this exception if it’s called too early.
class Session_needed_error : public Client_logic_error
{
public:
    /// The action that the client attempted that couldn't be
    /// completed without a GE211 session.
    const std::string& attempted_action() const { return action_; }

private:
    friend class detail::Session;

    explicit Session_needed_error(const std::string& action);

    std::string action_;
};

/// Thrown by member functions of @ref internal::Render_sprite when
/// the sprite has already been rendered to the screen and can no longer
/// be modified.
class Late_paint_error final : public Client_logic_error
{
    explicit Late_paint_error(char const* who);

    // Throwers
    friend ::ge211::internal::Render_sprite;
};

/// Indicates that an error was encountered by the game engine or
/// in the client's environment.
/// This could indicate a problem with your video driver,
/// a missing file, or even a bug in %ge211 itself. The derived
/// classes indicate more precisely the nature of the condition.
class Environment_error : public Exception_base
{
    explicit Environment_error(const std::string& message);

    /// Throwers
    friend Window;

    /// Derived classes
    friend Ge211_logic_error;
    friend Host_error;
};

/// Indicates a condition unexpected by %ge211, that appears
/// to break its invariants. This may very well indicate a bug
/// in %ge211. Please report it if you see one of these.
class Ge211_logic_error : public Environment_error
{
    explicit Ge211_logic_error(const std::string& message);

    /// Throwers
    friend Mixer;
    friend Text_sprite;
};

/// Indicates an exception from the host environment being
/// passed along by %ge211. The host environment is usually
/// SDL2, so these exceptions may include a reason provided
/// by SDL2.
class Host_error : public Environment_error
{
    explicit Host_error(const std::string& extra_message = "");

    /// Derived classes
    friend File_error;
    friend Font_error;
    friend Image_error;
    friend Mixer_error;

    /// Throwers
    friend Text_sprite;
    friend Window;
    friend ::ge211::internal::Render_sprite;
    friend class detail::Renderer;
    friend class detail::Texture;
};

/// Indicates an error opening a file.
class File_error final : public Host_error
{
    explicit File_error(const std::string& message);
    static File_error could_not_open(const std::string& filename);

    /// Throwers
    friend class detail::File_resource;

    template <bool>
    friend struct detail::ifstream_opener;
};

/// Indicates an error loading a font front an already-open file.
class Font_error final : public Host_error
{
    explicit Font_error(const std::string& message);
    static Font_error could_not_load(const std::string& filename);

    /// Thrower
    friend Font;
};

/// Indicates an error loading an image from an already-open file.
class Image_error final : public Host_error
{
    explicit Image_error(const std::string& message);
    static Image_error could_not_load(const std::string& filename);

    /// Thrower
    friend Image_sprite;
};

/// Indicates an error in the mixer, which could include the inability to
/// understand an audio file format.
class Mixer_error : public Host_error
{
    Mixer_error(const std::string& message);
    static Mixer_error could_not_load(const std::string& filename);
    static Mixer_error out_of_channels();
    static Mixer_error not_enabled();

    /// Thrower
    friend Mixer;
    friend Audio_clip;
    friend Music_track;
    friend Sound_effect;
};

} // end namespace exception

namespace internal {

namespace logging {

/// How serious is this log message?
enum class Log_level
{
    /// extra debugging information
    debug,
    /// helpful information
    info,
    /// non-fatal but concerning conditions
    warn,
    /// serious errors
    fatal,
};

/// Right now a Logger just keeps track of the current log
/// level. There's only one Logger (Singleton Pattern).
class Logger
{
public:
    /// Returns the log level of this logger.
    Log_level level() const NOEXCEPT { return level_; }

    /// Changes the log level of this logger.
    void level(Log_level level) NOEXCEPT { level_ = level; }

    /// Returns the one and only logger instance.
    static Logger& instance() NOEXCEPT;

private:
    Logger() NOEXCEPT = default;

    Log_level level_ = Log_level::warn;
};

/// A Log_message accumulates information and then prints it all at
/// once when it's about to be destroyed.
class Log_message
{
public:
    /// Construct a new Log_message with the given log level. The
    /// default log level is Log_level::debug.
    explicit Log_message(Log_level level = Log_level::debug);

    /// Construct a new Log_message with the given log level and
    /// cause. The default log level is Log_level::debug.
    explicit Log_message(std::string reason,
                         Log_level level = Log_level::debug) NOEXCEPT;

    /// Appends more text to this Log_message.
    template <typename STREAM_INSERTABLE>
    Log_message& operator<<(STREAM_INSERTABLE const& value)
    {
        if (active_) message_ << value;
        return *this;
    }

    /// A Log_message has important work to do when it's destroyed.
    virtual ~Log_message();

    /// A Log_message cannot be copied, since that would cause it to
    /// print twice.
    Log_message(const Log_message&) = delete;

    /// A Log_message cannot be copied, since that would cause it to
    /// print twice.
    Log_message& operator=(const Log_message&) = delete;

    /// A log message can be moved. The source of the move becomes
    /// inactive, meaning it won't print anything when destroyed.
    Log_message(Log_message&&) = default;

    /// A log message can be move-assigned. The source of the move
    /// becomes inactive, meaning it won't print anything when
    /// destroyed.
    Log_message& operator=(Log_message&&) = default;

private:
    std::string reason_;
    std::ostringstream message_;
    bool active_;
};

/// Returns a debug-level log message.
Log_message debug(std::string reason = "");

/// Returns a info-level log message.
Log_message info(std::string reason = "");

/// Returns a warn-level log message.
Log_message warn(std::string reason = "");

/// Returns a fatal-level log message.
Log_message fatal(std::string reason = "");

} // end namespace logging

} // end namespace internal

namespace detail {

using Log_message = internal::logging::Log_message;

// These functions generate log messages with the reason set to the
// cause of SDL2's most recent message.

Log_message info_sdl();
Log_message warn_sdl();
Log_message fatal_sdl();

} // end namespace detail

}
