#pragma once

#include "ge211_forward.h"
#include "ge211_util.h"
#include "ge211_error.h"

#include <string>
#include <vector>

namespace ge211 {

namespace detail {

std::vector<const char*> get_search_prefixes();

class File_resource
{
public:
    explicit File_resource(const std::string&);

    SDL_RWops* get_raw() const noexcept { return ptr_.get(); }
    SDL_RWops* release() && { return ptr_.release(); }

private:
    static delete_ptr<SDL_RWops> open_rwops_(const std::string&);

    delete_ptr<SDL_RWops> ptr_;
};

} // end namespace detail

/// Represents a font that can be used to render a sprites::Text_sprite.
/// To create a font, you must specify the TrueType font file (`.ttf`) to
/// load, and that file must be in the `Resources/` directory of your
/// project. You can create multiple Font instances for the same font
/// file at different sizes.
///
/// One TTF file, `sans.tff`, is included among %ge211's built-in resources,
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

    TTF_Font* get_raw_() const noexcept { return ptr_.get(); }

    static detail::delete_ptr<TTF_Font>
    load_(const std::string& filename,
          detail::File_resource&& ttf_file,
          int size);

    detail::delete_ptr<TTF_Font> ptr_;
};

}
