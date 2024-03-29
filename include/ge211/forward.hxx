#pragma once

#include <cstddef>
#include <string>

// Prevent SDL2 from taking over main().
#ifndef SDL_MAIN_HANDLED
  #define SDL_MAIN_HANDLED
#endif

// Forward declarations for some useful SDL2 types.
struct SDL_KeyboardEvent;
union SDL_Event;
struct SDL_Renderer;
struct SDL_RWops;
struct SDL_Surface;
struct SDL_Texture;
struct SDL_Window;
struct Mix_Chunk;
struct _Mix_Music;
typedef struct _Mix_Music Mix_Music;
struct _TTF_Font;
typedef struct _TTF_Font TTF_Font;

// Forward declarations for all ge211 types.
namespace ge211 {

using std::size_t;

class Abstract_game;
class Color;
class Font;
class Sprite_set;
class Window;

template <typename RESULT_TYPE> class Random_source;

namespace audio {

enum class Channel_state;
class Mixer;
class Audio_clip;
class Music_track;
class Sound_effect;
class Sound_effect_handle;

} // end namespace audio

namespace events {

class Key;
enum class Mouse_button;

} // end namespace events

namespace exceptions {

class Exception_base;
class Client_logic_error;
class Session_needed_error;
class Random_source_error;
class Random_source_bounds_error;
class Random_source_empty_stub_error;
class Random_source_unsupported_error;
class Environment_error;
class Ge211_logic_error;
class Late_paint_error;
class Host_error;
class File_open_error;
class Font_load_error;
class Image_load_error;
class Mixer_error;
class Audio_load_error;
class Out_of_channels_error;
class Mixer_not_enabled_error;

} // end namespace exceptions

namespace geometry
{

class Origin_type;
template <typename COORDINATE> struct Dims;
template <typename COORDINATE> struct Posn;
template <typename COORDINATE> struct Rect;
class Transform;

} // end namespace geometry.

namespace sprites {

class Sprite;

class Circle_sprite;
class Image_sprite;
class Multiplexed_sprite;
class Rectangle_sprite;
class Text_sprite;

} // end namespace sprites

namespace time {

class Duration;
class Time_point;

} // end namespace time

/// Internal facilities that you might want to use if you're doing
/// something fancy.
namespace internal {

class Render_sprite;

/// Facilities for logging to the console.
namespace logging {

/* These don't need forward declarations: */
// enum class Log_level;
// class Logger;
// class Log_message;

} // end namespace logging

} // end namespace internal

/// Internal implementation details.
namespace detail {

class Engine;
class File_resource;
class Frame_clock;
struct Placed_sprite;
class Pausable_timer;
class Renderer;
class Session;
class Texture;
class Texture_sprite;
struct Throw_random_source_error;
class Timer;

template <bool>
struct ifstream_opener;

template <typename> struct Param_check;
template <typename> class Bounded_engine;
template <typename> class Unbounded_engine;

} // end namespace detail

// Bring everything but `detail` and `internal` into the ge211
// namespace.

using namespace audio;
using namespace events;
using namespace exceptions;
using namespace geometry;
using namespace sprites;
using namespace time;

}
