#include "ge211_session.hxx"
#include "ge211_error.hxx"
#include "ge211_util.hxx"

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <clocale>

namespace ge211 {

namespace detail {

Sdl_session::Sdl_session()
{
    SDL_SetMainReady();

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        fatal_sdl() << "Could not initialize SDL2";
        exit(1);
    }
}

Sdl_session::~Sdl_session()
{
    SDL_Quit();
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
    ++session_count_;
}

Session::~Session()
{
    --session_count_;
}

std::atomic<int> Session::session_count_{0};

void Session::check_session(const char* action)
{
    if (session_count_ <= 0)
        throw Session_needed_error(action);
}

} // end namespace detail

}
