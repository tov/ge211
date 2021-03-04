#include "ge211_render.hxx"
#include "ge211_error.hxx"
#include "ge211_util.hxx"

#include <SDL.h>

#include <utility>

static inline SDL_RendererFlip&
operator|=(SDL_RendererFlip& f1, SDL_RendererFlip f2)
{
    return f1 = SDL_RendererFlip(f1 | f2);
}

namespace ge211
{

namespace detail
{

namespace
{

struct Renderer_flag
{
    uint32_t value;
    const char* description;
};

#pragma push_macro("RF")
#define RF(E) Renderer_flag{(E), #E}

static const Renderer_flag renderer_flags_to_try[] = {
        RF(SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC),
        RF(SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC),
        RF(SDL_RENDERER_ACCELERATED),
        RF(SDL_RENDERER_SOFTWARE),
        RF(0),
};

#pragma pop_macro("RF")

} // end anonymous namespace

SDL_Renderer* Renderer::create_renderer_(SDL_Window* window)
{
    SDL_Renderer* result;

#if SDL_VIDEO_RENDER_METAL
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");
#endif

    for (auto flag : renderer_flags_to_try) {
        result = SDL_CreateRenderer(window, -1, flag.value);
        if (result) {
            SDL_SetRenderDrawBlendMode(result, SDL_BLENDMODE_BLEND);
            return result;
        }

        info_sdl()
                << "Could not initialize renderer ("
                << flag.description << "); trying next.";
    }

    return nullptr;
}

Renderer::Renderer(const Window& window)
        : ptr_{create_renderer_(window.get_raw_())}
{
    if (!ptr_)
        throw Host_error{"Could not initialize renderer."};
}

bool Renderer::is_vsync() const NOEXCEPT
{
    SDL_RendererInfo info;
    SDL_GetRendererInfo(get_raw_(), &info);
    return (info.flags & SDL_RENDERER_PRESENTVSYNC) != 0;
}

void Renderer::clear()
{
    if (SDL_RenderClear(get_raw_()))
        throw Host_error{"Could not clear window"};
}

SDL_Renderer* Renderer::get_raw_() const NOEXCEPT
{
    return ptr_.get();
}

void Renderer::set_color(Color color)
{
    if (SDL_SetRenderDrawColor(
            get_raw_(),
            color.red(), color.green(), color.blue(), color.alpha()))
        throw Host_error{"Could not set renderer color"};
}

void Renderer::present() NOEXCEPT
{
    SDL_RenderPresent(get_raw_());
}

void Renderer::copy(const Texture& texture, Posn<int> xy)
{
    auto raw_texture = texture.get_raw_(*this);
    if (!raw_texture) return;

    SDL_Rect dstrect = Rect<int>::from_top_left(xy, texture.dimensions());

    int render_result = SDL_RenderCopy(get_raw_(), raw_texture,
                                       nullptr, &dstrect);
    if (render_result < 0) {
        warn_sdl() << "Could not render texture";
    }
}

void Renderer::copy(const Texture& texture,
                    Posn<int> xy,
                    const Transform& transform)
{
    auto raw_texture = texture.get_raw_(*this);
    if (!raw_texture) return;

    SDL_Rect dstrect = Rect<int>::from_top_left(xy, texture.dimensions());
    dstrect.w = int(dstrect.w * transform.get_scale_x());
    dstrect.h = int(dstrect.h * transform.get_scale_y());

    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if (transform.get_flip_h()) flip |= SDL_FLIP_HORIZONTAL;
    if (transform.get_flip_v()) flip |= SDL_FLIP_VERTICAL;

    auto rotation = transform.get_rotation();

    int render_result;
    if (rotation == 0 && flip == SDL_FLIP_NONE) {
        render_result = SDL_RenderCopy(
                get_raw_(), raw_texture,
                nullptr, &dstrect);
    } else {
        render_result = SDL_RenderCopyEx(
                get_raw_(), raw_texture,
                nullptr, &dstrect,
                transform.get_rotation(), nullptr,
                flip);
    }

    if (render_result < 0) {
        warn_sdl() << "Could not render texture";
    }
}

void Renderer::prepare(const Texture& texture) const
{
    texture.get_raw_(*this);
}

Texture::Impl_::Impl_(Owned<SDL_Surface> surface) NOEXCEPT
        : surface_(surface)
{ }

Texture::Impl_::Impl_(Owned<SDL_Texture> texture) NOEXCEPT
        : texture_(texture)
{ }

Texture::Impl_::Impl_(Uniq_SDL_Surface surface) NOEXCEPT
        : surface_(std::move(surface))
{ }

Texture::Impl_::Impl_(Uniq_SDL_Texture texture) NOEXCEPT
        : texture_(std::move(texture))
{ }

Texture::Texture() NOEXCEPT
{ }

Texture::Texture(Owned<SDL_Surface> surface)
        : Texture(Uniq_SDL_Surface(surface))
{ }

Texture::Texture(Uniq_SDL_Surface surface)
        : impl_(std::make_shared<Impl_>(std::move(surface)))
{ }

SDL_Texture* Texture::get_raw_(const Renderer& renderer) const
{
    if (impl_->texture_) return impl_->texture_.get();

    if (!impl_->surface_) return nullptr;

    SDL_Texture* raw = SDL_CreateTextureFromSurface(renderer.get_raw_(),
                                                    impl_->surface_.get());
    if (raw) {
        *impl_ = Impl_(raw);
        return raw;
    }

    throw Host_error{"Could not create texture from surface"};
}

Dims<int> Texture::dimensions() const NOEXCEPT
{
    Dims<int> result{0, 0};

    if (impl_->texture_) {
        SDL_QueryTexture(impl_->texture_.get(), nullptr, nullptr,
                         &result.width, &result.height);
    } else if (impl_->surface_) {
        result.width  = impl_->surface_->w;
        result.height = impl_->surface_->h;
    }

    return result;
}

Borrowed<SDL_Surface> Texture::raw_surface() NOEXCEPT
{
    return impl_->surface_.get();
}

bool Texture::empty() const NOEXCEPT
{
    return impl_ == nullptr;
}

} // end namespace detail

}
