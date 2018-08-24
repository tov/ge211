#include "ge211_session.h"
#include "ge211_error.h"

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

    int mix_flags = Mix_Init(0);
    if (mix_flags == 0) {
        warn_sdl() << "Could not initialize audio mixer";
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

    if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY,
                      MIX_DEFAULT_FORMAT,
                      2,
                      4096) < 0)
    {
        warn_sdl() << "Could not open audio channels";
    }

    SDL_StartTextInput();
}

Session::~Session()
{
    SDL_StopTextInput();

    Mix_CloseAudio();

    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
    Mix_Quit();
}

} // end namespace detail

}
