#pragma once

#include "forward.hxx"
#include "doxygen.hxx"
#include "util.hxx"

#include <exception>
#include <memory>
#include <sstream>
#include <string>

GE211_REGISTER_TYPE_NAME(ge211::Exception_base);
GE211_REGISTER_TYPE_NAME(ge211::Client_logic_error);
GE211_REGISTER_TYPE_NAME(ge211::Session_needed_error);
GE211_REGISTER_TYPE_NAME(ge211::Random_source_error);
GE211_REGISTER_TYPE_NAME(ge211::Random_source_bounds_error);
GE211_REGISTER_TYPE_NAME(ge211::Random_source_empty_stub_error);
GE211_REGISTER_TYPE_NAME(ge211::Random_source_unsupported_error);
GE211_REGISTER_TYPE_NAME(ge211::Environment_error);
GE211_REGISTER_TYPE_NAME(ge211::Ge211_logic_error);
GE211_REGISTER_TYPE_NAME(ge211::Late_paint_error);
GE211_REGISTER_TYPE_NAME(ge211::Host_error);
GE211_REGISTER_TYPE_NAME(ge211::File_open_error);
GE211_REGISTER_TYPE_NAME(ge211::Font_load_error);
GE211_REGISTER_TYPE_NAME(ge211::Image_load_error);
GE211_REGISTER_TYPE_NAME(ge211::Mixer_error);
GE211_REGISTER_TYPE_NAME(ge211::Audio_load_error);
GE211_REGISTER_TYPE_NAME(ge211::Out_of_channels_error);
GE211_REGISTER_TYPE_NAME(ge211::Mixer_not_enabled_error);

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
    /// Derived classes
    friend Client_logic_error;
    friend Environment_error;

public:
    /// The error message associated with the exception. This
    /// pointer is guaranteed to be good as long as the exception
    /// exists and hasn't been mutated. If you need it for longer,
    /// copy it to a std::string.
    const char *what() const NOEXCEPT override;

private:
    explicit Exception_base(std::string message);

    std::shared_ptr<std::string const> message_;
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
    explicit Client_logic_error(std::string message);
};

/// An exception thrown when the client attempts to perform an
/// action that requires a GE211 session before GE211 starts.
/// For example, GE211 needs to initialize the font subsystem
/// before fonts can be loaded, so the `Font` constructor throws
/// this exception if it’s called too early.
class Session_needed_error : public Client_logic_error
{
    /// Throwers
    friend class detail::Session;

public:
    /// The action that the client attempted that couldn't be
    /// completed without a GE211 session.
    std::string const& attempted_action() const { return action_; }

private:
    explicit Session_needed_error(std::string const& action);

    std::string action_;
};

/// Base class for various errors that can happen when constructing
/// or using a @ref ge211::Random_source.
///
/// \sa `class` @ref Random_source_bounds_error
/// \sa `class` @ref Random_source_unsupported_error
class Random_source_error : public Client_logic_error
{
    /// Derived classes
    friend Random_source_bounds_error;
    friend Random_source_empty_stub_error;
    friend Random_source_unsupported_error;

    explicit Random_source_error(std::string message);
};


/// Thrown when the parameter(s) to a @ref Random_source constructor is
/// out of bounds:
///
///   - For the two-argument constructor (@ref
///     Random_source::Random_source(result_type, result_type))
///     and for @ref Random_source::next_between
///     the given closed interval must be non-empty.
///
///   - For the one-argument constructor for integral @ref result_type
///     (@ref Random_source::Random_source(result_type))
///     the parameter must be positive.
///
///   - For the one-argument constructor for `bool` @ref result_type
///     (@ref Random_source::Random_source(double))
///     the parameter must be in the unit interval [`0.0`, `1.0`].
class Random_source_bounds_error final : public Random_source_error
{
    /// Throwers
    template <typename RESULT_TYPE>
    friend
    struct detail::Param_check;

    friend detail::Throw_random_source_error;

    explicit Random_source_bounds_error(std::string message);
};

/// Thrown when one of the stubbing functions is given an empty container
/// of stub values.
///
///   - \sa @ref Random_source::stub_with(std::vector<result_type>)
///   - \sa @ref Random_source::stub_with(std::initializer_list<result_type>)
class Random_source_empty_stub_error final : public Random_source_error
{
    /// Throwers
    friend detail::Throw_random_source_error;

    explicit Random_source_empty_stub_error(std::string message);
};

/// Thrown when the a @ref Random_source does not support the invoked
/// operation.
///
/// This will happen if you:
///
///   - Call @ref Random_source::next_between or @ref
///     Random_source::next_with_probability on a
///     @ref Random_source that had bounds or a probability supplied to its
///     constructor.
///
///   - Call @ref Random_source::next on a @ref Random_source that had
///     @ref ge211::unbounded passed to its constructor rather than bounds
///     or a probability.
class Random_source_unsupported_error final : public Random_source_error
{
    /// Throwers
    friend detail::Throw_random_source_error;

    explicit Random_source_unsupported_error(std::string msg);
};


/// Thrown by member functions of @ref internal::Render_sprite when
/// the sprite has already been rendered to the screen and can no longer
/// be modified.
class Late_paint_error final : public Client_logic_error
{
    // Throwers
    friend ::ge211::internal::Render_sprite;

    explicit Late_paint_error(char const *who);

    static std::string build_message_(char const *who);
};

/// Indicates that an error was encountered by the game engine or
/// in the client's environment.
/// This could indicate a problem with your video driver,
/// a missing file, or even a bug in %ge211 itself. The derived
/// classes indicate more precisely the nature of the condition.
class Environment_error : public Exception_base
{
    /// Derived classes
    friend Ge211_logic_error;
    friend Host_error;

    /// Throwers
    friend Window;

    explicit Environment_error(std::string message);
};

/// Indicates a condition unexpected by %ge211, that appears
/// to break its invariants. This may very well indicate a bug
/// in %ge211. Please report it if you see one of these.
class Ge211_logic_error : public Environment_error
{
    /// Throwers
    friend Mixer;
    friend Text_sprite;

    explicit Ge211_logic_error(std::string const& message);
};

/// Indicates an exception from the host environment being
/// passed along by %ge211. The host environment is usually
/// SDL2, so these exceptions may include a reason provided
/// by SDL2.
class Host_error : public Environment_error
{
    /// Derived classes
    friend File_open_error;
    friend Font_load_error;
    friend Image_load_error;
    friend Mixer_error;

    /// Throwers
    friend Text_sprite;
    friend Window;
    friend ::ge211::internal::Render_sprite;
    friend detail::Renderer;
    friend detail::Texture;

    explicit Host_error(std::string const& extra_message = "");
};

/// Indicates an error opening a file.
class File_open_error final : public Host_error
{
    /// Throwers
    friend class detail::File_resource;

    template <bool> friend
    struct detail::ifstream_opener;

public:
    /// Returns the name of the file that could not be opened.
    std::string const& filename() const { return filename_; }

private:
    std::string filename_;

    explicit File_open_error(std::string const& filename);

    static std::string build_message_(std::string const& filename);
};

/// Indicates an error loading a font from an already-open file.
class Font_load_error final : public Host_error
{
    /// Throwers
    friend Font;

public:
    /// Returns the filename of the font that could not be loaded.
    std::string const& filename() const { return filename_; }

private:
    std::string filename_;

    explicit Font_load_error(std::string const& filename);

    static std::string build_message_(std::string const& filename);
};

/// Indicates an error loading an image from an already-open file.
class Image_load_error final : public Host_error
{
    /// Throwers
    friend Image_sprite;

public:
    /// Returns the filename of the image that could not be loaded.
    std::string const& filename() const { return filename_; }

private:
    std::string filename_;

    explicit Image_load_error(std::string const& filename);

    static std::string build_message_(std::string const& filename);
};

/// Indicates an error in the mixer, which could include the inability to
/// understand an audio file format.
///
/// The specific error determines which derived class is thrown:
///
/// \sa `class` @ref Audio_load_error
/// \sa `class` @ref Out_of_channels_error
/// \sa `class` @ref Mixer_not_enabled_error
class Mixer_error : public Host_error
{
    /// Derived classes
    friend class Audio_load_error;

    friend class Out_of_channels_error;

    friend class Mixer_not_enabled_error;

    explicit Mixer_error(std::string const& problem);

    std::string build_message_(std::string const& problem);
};

/// Thrown when an audio clip could not be loaded from a successfully opened
/// file. This could indicate that the audio format is not supported or
/// not understood.
class Audio_load_error final : public Mixer_error
{
    /// Throwers
    friend Audio_clip;

public:
    /// Returns the filename of the audio clip that could not be loaded.
    std::string const& filename() const { return filename_; }

private:
    std::string filename_;

    explicit Audio_load_error(std::string const& filename);

    std::string build_message_(std::string const& filename);
};

/// Thrown when a sound effect cannot be played because all of the
/// mixer's channels are already in use.
class Out_of_channels_error final : public Mixer_error
{
    /// Throwers
    friend Mixer;

    Out_of_channels_error();
};

/// Thrown when audio cannot be loaded because the mixer is not
/// available. This could happen if GE211 (via SDL2) cannot recognize
/// or access your sound card.
class Mixer_not_enabled_error final : public Mixer_error
{
    /// Throwers
    friend Mixer;

    Mixer_not_enabled_error();
};

} // end namespace exceptions

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
    explicit Log_message(
            std::string reason,
            Log_level level = Log_level::debug) NOEXCEPT;

    /// Appends more text to this Log_message.
    template <typename STREAM_INSERTABLE>
    Log_message& operator<<(STREAM_INSERTABLE const& value)
    {
        if (active_) { message_ << value; }
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
Log_message
debug(std::string reason = "");

/// Returns a info-level log message.
Log_message
info(std::string reason = "");

/// Returns a warn-level log message.
Log_message
warn(std::string reason = "");

/// Returns a fatal-level log message.
Log_message
fatal(std::string reason = "");

} // end namespace logging

} // end namespace internal

namespace detail {

using Log_message = internal::logging::Log_message;

// These functions generate log messages with the reason set to the
// cause of SDL2's most recent message.

Log_message
info_sdl();
Log_message
warn_sdl();
Log_message
fatal_sdl();

} // end namespace detail

}
