#include "ge211_resource.hxx"
#include "ge211_error.hxx"

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

std::vector<const char*> get_search_prefixes()
{
    using namespace std;
    return vector<const char*>(begin(search_prefixes),
                               end(search_prefixes));
}

delete_ptr<SDL_RWops> File_resource::open_rwops_(const std::string& filename)
{
    std::string path;

    for (auto prefix : search_prefixes) {
        path.clear();
        path += prefix;
        path += filename;

        auto rwops = SDL_RWFromFile(path.c_str(), "rb");
        if (rwops) return make_delete_ptr(rwops, SDL_RWclose);

        info_sdl() << "File_resource: could not load";
    }

    throw File_error::could_not_open(filename);
}

File_resource::File_resource(const std::string& filename)
        : ptr_{open_rwops_(filename)}
{ }

} // end namespace detail

delete_ptr<TTF_Font> Font::load_(const std::string& filename,
                                 File_resource&& file,
                                 int size)
{
    TTF_Font* result = TTF_OpenFontRW(std::move(file).release(), 1, size);
    if (result) return {result, &TTF_CloseFont};

    throw Font_error::could_not_load(filename);
}

Font::Font(const std::string& filename, int size)
        : ptr_{nullptr, &no_op_deleter}
{
    File_resource fr(filename);
    ptr_ = load_(filename, std::move(fr), size);
}

}
