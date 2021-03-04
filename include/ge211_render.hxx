#pragma once

#include "ge211_color.hxx"
#include "ge211_forward.hxx"
#include "ge211_geometry.hxx"
#include "ge211_noexcept.hxx"
#include "ge211_window.hxx"
#include "ge211_util.hxx"

#include <SDL_render.h>
#include <SDL_surface.h>

#include <memory>

namespace ge211 {

namespace detail {

using Uniq_SDL_Renderer = delete_ptr<SDL_Renderer, &SDL_DestroyRenderer>;
using Uniq_SDL_Surface  = delete_ptr<SDL_Surface, &SDL_FreeSurface>;
using Uniq_SDL_Texture  = delete_ptr<SDL_Texture, &SDL_DestroyTexture>;

class Renderer
{
public:
    explicit Renderer(const Window&);

    bool is_vsync() const NOEXCEPT;

    void set_color(Color);

    void clear();
    void copy(const Texture&, Posn<int>);
    void copy(const Texture&, Posn<int>, const Transform&);

    // Prepares a texture for rendering with this given renderer, without
    // actually copying it.
    void prepare(const Texture&) const;

    void present() NOEXCEPT;

private:
    friend Texture;

    Borrowed<SDL_Renderer> get_raw_() const NOEXCEPT;

    static Owned<SDL_Renderer> create_renderer_(Borrowed<SDL_Window>);

    Uniq_SDL_Renderer ptr_;
};

// A texture is initially created as a (device-independent) `SDL_Surface`,
// and then turned into an `SDL_Texture` the first time it gets rendered.
// The SDL_Texture is cached and the original `SDL_Surface` is deleted.
class Texture
{
public:
    // An empty texture; don't render this or even ask for its dimensions.
    Texture() NOEXCEPT;

    // Takes ownership of the `SDL_Surface` and will delete it.
    //
    // \preconditions
    //  - The surface is not zero-sized.
    explicit Texture(Owned<SDL_Surface> surface);
    explicit Texture(Uniq_SDL_Surface);

    Dims<int> dimensions() const NOEXCEPT;

    // Returns nullptr if this `Texture` has been rendered, and can no
    // longer be updated as an `SDL_Surface`.
    Borrowed<SDL_Surface> raw_surface() NOEXCEPT;

    bool empty() const NOEXCEPT;

private:
    friend Renderer;

    struct Impl_
    {
        Impl_(Owned<SDL_Surface>) NOEXCEPT;
        Impl_(Owned<SDL_Texture>) NOEXCEPT;

        Impl_(Uniq_SDL_Surface) NOEXCEPT;
        Impl_(Uniq_SDL_Texture) NOEXCEPT;

        Uniq_SDL_Surface surface_;
        Uniq_SDL_Texture texture_;
        // Invariant:
        //  - Exactly one surface_ and texture_ is non-null.
        //  - Whichever is non-null is non-zero-sized.
        // Note: impl_ below is null for the empty Texture.
    };

    Borrowed<SDL_Texture> get_raw_(const Renderer&) const;

    std::shared_ptr<Impl_> impl_;
};

} // end namespace detail

}
