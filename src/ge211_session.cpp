#include "ge211_session.h"
#include "ge211_error.h"
#include "ge211_util.h"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#include <clocale>

namespace ge211 {

namespace detail {

Session::Session()
{
    setlocale(LC_ALL, "en_US.utf8");

    int mix_flags = MIX_INIT_OGG | MIX_INIT_MP3;
    if ((Mix_Init(mix_flags) & mix_flags) != mix_flags) {
        info_sdl() << "Could not pre-initialize audio mixer";
    }

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fatal_sdl() << "Could not initialize graphics";
        exit(1);
    }

    int img_flags = IMG_INIT_JPG | IMG_INIT_PNG;
    if (IMG_Init(img_flags) != img_flags) {
        fatal_sdl() << "Could not initialize image loading support";
        exit(1);
    }

    if (TTF_Init() < 0) {
        fatal_sdl() << "Could not initialize text handling";
        exit(1);
    }

    SDL_StartTextInput();
}

Session::~Session()
{
    SDL_StopTextInput();

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    Mix_Quit();
}

} // end namespace detail

}
