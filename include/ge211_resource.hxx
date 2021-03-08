#pragma once

#include "ge211_forward.hxx"
#include "ge211_noexcept.hxx"
#include "ge211_util.hxx"
#include "ge211_error.hxx"

#include <SDL_rwops.h>

// Copied from SDL_ttf.h in order to avoid getting including all of
// SDL.h from the GE211 headers.
extern "C" {
extern DECLSPEC void SDLCALL TTF_CloseFont(TTF_Font*);
}

#include <fstream>
#include <string>
#include <vector>

namespace ge211 {

/// Opens a file in the `Resources/` directory for input in text mode.
///
/// Throws @ref exceptions::File_error if the file cannot be opened.
std::ifstream
open_resource_file(std::string const& filename);

/// Opens a file in the `Resources/` directory for input in binary mode.
///
/// Throws @ref exceptions::File_error if the file cannot be opened.
std::ifstream
open_binary_resource_file(std::string const& filename);

namespace detail {

class File_resource
{
public:
    explicit File_resource(const std::string&);

    Borrowed<SDL_RWops> get_raw() const NOEXCEPT { return ptr_.get(); }

    Owned<SDL_RWops> release() && { return ptr_.release(); }

private:
    static void close_rwops_(Owned<SDL_RWops>);

    delete_ptr<SDL_RWops, &close_rwops_> ptr_;
};

} // end namespace detail

/// Represents a font that can be used to render a sprites::Text_sprite.
/// To create a font, you must specify the TrueType font file (`.ttf`) to
/// load, and that file must be in the `Resources/` directory of your
/// project. You can create multiple Font instances for the same font
/// file at different sizes.
///
/// One TTF file, `sans.ttf`, is included among %ge211's built-in resources,
/// and can always be used even if you haven't added any fonts yourself.
///
/// Note that Font%s cannot be constructed until the text subsystem is
/// initialized. The text subsystem is initialized with the rest of ge211
/// when the Abstract_game underlying your game struct or class is
/// constructed. Thus, you cannot create namespace level (or global)
/// Font%s. The usual place to define Font%s is as member variables in
/// your game struct, since member variables of a derived class are
/// initialized after the base class is initialized.
class Font
{
public:
    /// Loads a font from the specified TrueType font file, at the specified
    /// size.
    Font(const std::string& filename, int size);

private:
    friend Text_sprite;

    Borrowed<TTF_Font> get_raw_() const NOEXCEPT { return ptr_.get(); }

    detail::delete_ptr<TTF_Font, &TTF_CloseFont, true> ptr_;
};

}
