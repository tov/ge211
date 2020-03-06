#pragma once

namespace ge211 {

namespace detail {

struct PINNED
{
    PINNED() = default;

    PINNED(PINNED&&) = delete;
    PINNED& operator=(PINNED&&) = delete;
    PINNED(const PINNED&) = delete;
    PINNED& operator=(const PINNED&) = delete;
};

struct Sdl_session : PINNED
{
    Sdl_session();
    ~Sdl_session();
};

struct Mix_session : PINNED
{
    Mix_session();
    ~Mix_session();

    bool enabled;
};

struct Img_session : PINNED
{
    Img_session();
    ~Img_session();
};

struct Ttf_session : PINNED
{
    Ttf_session();
    ~Ttf_session();
};

struct Text_input_session : PINNED
{
    Text_input_session();
    ~Text_input_session();
};

struct Session
{
    Session();

    Sdl_session        sdl;
    Mix_session        mix;
    Img_session        img;
    Ttf_session        ttf;
    Text_input_session text_input;
};

} // end namespace detail

}
