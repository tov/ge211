#include "ge211_resource.hxx"
#include "ge211_error.hxx"
#include "ge211_session.hxx"

#include <SDL.h>
#include <SDL_ttf.h>

#include <ios>
#include <string>

namespace ge211 {

using namespace detail;

namespace {

static const char* search_prefixes[] = {
        GE211_RESOURCES,
        "Resources/",
        "../Resources/",
};

template <class OPENER>
typename OPENER::result_t
open_resource_(std::string const& filename)
{
    std::string path;

    for (auto prefix : search_prefixes) {
        path.clear();
        path += prefix;
        path += filename;

        auto result = OPENER::open(path);
        if (result) return result;
    }

    return OPENER::fail(filename);
}

}  // end anonymous namespace

namespace detail {

template <bool BinaryMode>
struct ifstream_opener
{
    using result_t = std::ifstream;

    static constexpr std::ios_base::openmode
    mode = BinaryMode
            ? std::ios_base::in | std::ios_base::binary
            : std::ios_base::in;

    static result_t open(std::string const& path)
    {
        return result_t(path, mode);
    }

    static result_t fail(std::string const& filename)
    {
        throw ge211::File_error::could_not_open(filename);
    }
};

}  // end namespace detail

std::ifstream
open_resource_file(std::string const& filename)
{
    return open_resource_<ifstream_opener<false>>(filename);
}

std::ifstream
open_binary_resource_file(std::string const& filename)
{
    return open_resource_<ifstream_opener<true>>(filename);
}

namespace detail {

static Owned<SDL_RWops> open_rwops_(const std::string& filename)
{
    struct Opener
    {
        using result_t = Owned<SDL_RWops>;

        static result_t open(std::string const& path)
        {
            return SDL_RWFromFile(path.c_str(), "rb");
        }

        static result_t fail(std::string const&)
        {
            return nullptr;
        }
    };

    return open_resource_<Opener>(filename);
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
