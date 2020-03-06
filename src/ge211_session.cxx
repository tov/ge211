#include "ge211_session.hxx"
#include "ge211_error.hxx"
#include "ge211_util.hxx"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <SDL_ttf.h>

#include <clocale>

namespace ge211 {

namespace detail {

Sdl_session::Sdl_session()
{
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fatal_sdl() << "Could not initialize SDL2";
        exit(1);
    }
}

Sdl_session::~Sdl_session()
{
    SDL_Quit();
}

Mix_session::Mix_session()
        : enabled{0 == Mix_OpenAudio(MIX_DEFAULT_FREQUENCY,
                                     MIX_DEFAULT_FORMAT,
                                     2,
                                     4096)}
{
    if (enabled) {
        int mix_want = MIX_INIT_OGG | MIX_INIT_MP3;
        int mix_have = Mix_Init(mix_want);
        if (mix_have == 0) {
            warn_sdl() << "Could not initialize audio mixer";
        } else if ((mix_have & mix_want) != mix_want) {
            warn_sdl() << "Could not initialize all audio formats";
        }
    } else {
        warn_sdl() << "Could not open audio device";
    }
}

Mix_session::~Mix_session()
{
    if (enabled) {
        Mix_Quit();
        Mix_CloseAudio();
    }
}

Img_session::Img_session()
{
    int img_flags = IMG_INIT_JPG | IMG_INIT_PNG;
    if (IMG_Init(img_flags) != img_flags) {
        fatal_sdl() << "Could not initialize image loading support";
        exit(1);
    }
}

Img_session::~Img_session()
{
    IMG_Quit();
}

Ttf_session::Ttf_session()
{
    if (TTF_Init() < 0) {
        fatal_sdl() << "Could not initialize font rendering support";
        exit(1);
    }
}

Ttf_session::~Ttf_session()
{
    TTF_Quit();
}

Text_input_session::Text_input_session()
{
    SDL_StartTextInput();
}

Text_input_session::~Text_input_session()
{
    SDL_StopTextInput();
}

Session::Session()
{
    setlocale(LC_ALL, "en_US.utf8");
}

} // end namespace detail

}
