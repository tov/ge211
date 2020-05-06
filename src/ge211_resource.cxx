#include "ge211_resource.hxx"
#include "ge211_error.hxx"
#include "ge211_session.hxx"

#include <SDL.h>
#include <SDL_ttf.h>

#include <string>

namespace ge211 {

using namespace detail;

static const char* search_prefixes[] = {
        GE211_RESOURCES,
        "Resources/",
        "../Resources/",
};

namespace detail {

static Owned<SDL_RWops> open_rwops_(const std::string& filename)
{
    std::string path;

    for (auto prefix : search_prefixes) {
        path.clear();
        path += prefix;
        path += filename;

        auto rwops = SDL_RWFromFile(path.c_str(), "rb");
        if (rwops) return rwops;

        info_sdl() << "File_resource: could not load";
    }

    return nullptr;
}

File_resource::File_resource(const std::string& filename)
        : ptr_(open_rwops_(filename))
{
    if (!ptr_)
        throw File_error::could_not_open(filename);
}

void File_resource::close_rwops_(Owned<SDL_RWops> ptr)
{
    SDL_RWclose(ptr);
}

} // end namespace detail

static Owned<TTF_Font> open_ttf_(const std::string& filename, int size)
{
    return TTF_OpenFontRW(File_resource(filename).release(), 1, size);
}

Font::Font(const std::string& filename, int size)
        : ptr_(open_ttf_(filename, size))
{
    Session::check_session("Font loading");

    if (!ptr_)
        throw Font_error::could_not_load(filename);
}

}
