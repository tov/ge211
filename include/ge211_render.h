#pragma once

#include "ge211_color.h"
#include "ge211_forward.h"
#include "ge211_geometry.h"
#include "ge211_window.h"
#include "ge211_util.h"
#include <memory>

namespace ge211 {

namespace detail {

class Renderer
{
public:
    explicit Renderer(const Window&);

    bool is_vsync() const noexcept;

    void set_color(Color);

    void clear();
    void copy(const Texture&, Position);
    void copy(const Texture&, Position, const Transform&);

    // Prepares a texture for rendering with this given renderer, without
    // actually copying it.
    void prepare(const Texture&) const;

    void present() noexcept;

private:
    friend Texture;

    SDL_Renderer* get_raw_() const noexcept;

    static SDL_Renderer* create_renderer_(SDL_Window*);

    delete_ptr<SDL_Renderer> ptr_;
};

// A texture is initially created as a (device-independent) `SDL_Surface`,
// and then turned into an `SDL_Texture` the first time it gets rendered.
// The SDL_Texture is cached and the original `SDL_Surface` is deleted.
class Texture
{
public:
    // An empty texture; don't render this or even ask for its dimensions.
    Texture() noexcept;

    // Takes ownership of the `SDL_Surface*` and will delete it.
    //
    // \preconditions
    //  - The surface is not zero-sized.
    explicit Texture(SDL_Surface*);
    explicit Texture(delete_ptr<SDL_Surface>);

    Dimensions dimensions() const noexcept;

    // Returns nullptr if this `Texture` has been rendered, and can no
    // longer be updated as an `SDL_Surface`.
    SDL_Surface* as_surface() noexcept;

    bool empty() const noexcept;

private:
    friend Renderer;

    struct Impl_
    {
        Impl_(SDL_Surface*) noexcept;
        Impl_(SDL_Texture*) noexcept;

        Impl_(delete_ptr<SDL_Surface>) noexcept;
        Impl_(delete_ptr<SDL_Texture>) noexcept;

        delete_ptr<SDL_Surface> surface_;
        delete_ptr<SDL_Texture> texture_;
        // Invariant:
        //  - Exactly one surface_ and texture_ is non-null.
        //  - Whichever is non-null is non-zero-sized.
        // Note: impl_ below is null for the empty Texture.
    };

    SDL_Texture* get_raw_(const Renderer&) const;

    std::shared_ptr<Impl_> impl_;
};

} // end namespace detail

}
