#include "ge211_render.h"
#include "ge211_error.h"
#include "ge211_util.h"

#include <SDL.h>

#include <utility>

static inline SDL_RendererFlip&
operator|=(SDL_RendererFlip& f1, SDL_RendererFlip f2)
{
    return f1 = SDL_RendererFlip(f1 | f2);
}

namespace ge211 {

namespace detail {

static const uint32_t renderer_flags_to_try[] = {
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC,
        SDL_RENDERER_SOFTWARE | SDL_RENDERER_PRESENTVSYNC,
        SDL_RENDERER_ACCELERATED,
        SDL_RENDERER_SOFTWARE,
        0,
};

SDL_Renderer* Renderer::create_renderer_(SDL_Window* window)
{
    SDL_Renderer* result;

#if SDL_VIDEO_RENDER_METAL
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "metal");
#endif

    for (auto flags : renderer_flags_to_try) {
        result = SDL_CreateRenderer(window, -1, flags);
        if (result) {
            SDL_SetRenderDrawBlendMode(result, SDL_BLENDMODE_BLEND);
            return result;
        }

        info_sdl()
                << "Could not initialize preferred renderer ("
                << flags << "); trying next.";
    }

    throw Host_error{"Could not initialize renderer"};
}

Renderer::Renderer(const Window& window)
        : ptr_{create_renderer_(window.get_raw_()),
               &SDL_DestroyRenderer}
{ }

bool Renderer::is_vsync() const noexcept
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

SDL_Renderer* Renderer::get_raw_() const noexcept
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

void Renderer::present() noexcept
{
    SDL_RenderPresent(get_raw_());
}

void Renderer::copy(const Texture& texture, Position xy)
{
    auto raw_texture = texture.get_raw_(*this);
    if (!raw_texture) return;

    SDL_Rect dstrect = Rectangle::from_top_left(xy, texture.dimensions());

    int render_result = SDL_RenderCopy(get_raw_(), raw_texture,
                                       nullptr, &dstrect);
    if (render_result < 0) {
        warn_sdl() << "Could not render texture";
    }
}

void Renderer::copy(const Texture& texture,
                    Position xy,
                    const Transform& transform)
{
    auto raw_texture = texture.get_raw_(*this);
    if (!raw_texture) return;

    SDL_Rect dstrect = Rectangle::from_top_left(xy, texture.dimensions());
    dstrect.w = int(dstrect.w * transform.get_scale_x());
    dstrect.h = int(dstrect.h * transform.get_scale_y());

    SDL_RendererFlip flip = SDL_FLIP_NONE;
    if (transform.get_flip_h()) flip |= SDL_FLIP_HORIZONTAL;
    if (transform.get_flip_v()) flip |= SDL_FLIP_VERTICAL;

    int render_result = SDL_RenderCopyEx(get_raw_(), raw_texture,
                                         nullptr, &dstrect,
                                         transform.get_rotation(),
                                         nullptr,
                                         flip);
    if (render_result < 0) {
        warn_sdl() << "Could not render texture";
    }
}

void Renderer::prepare(const Texture& texture) const
{
    texture.get_raw_(*this);
}

Texture::Impl_::Impl_(SDL_Surface* surface) noexcept
        : Impl_{{surface, &SDL_FreeSurface}}
{ }

Texture::Impl_::Impl_(SDL_Texture* texture) noexcept
        : Impl_{{texture, &SDL_DestroyTexture}}
{ }

Texture::Impl_::Impl_(delete_ptr<SDL_Surface> surface) noexcept
        : surface_{std::move(surface)},
          texture_{nullptr, &no_op_deleter}
{ }

Texture::Impl_::Impl_(delete_ptr<SDL_Texture> texture) noexcept
        : surface_{nullptr, &no_op_deleter},
          texture_{std::move(texture)}
{ }

Texture::Texture() noexcept
        : impl_{nullptr}
{ }

Texture::Texture(SDL_Surface* surface)
        : impl_{std::make_shared<Impl_>(surface)}
{ }

Texture::Texture(delete_ptr<SDL_Surface> surface)
        : impl_{std::make_shared<Impl_>(std::move(surface))}
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

Dimensions Texture::dimensions() const noexcept
{
    Dimensions result{0, 0};

    if (impl_->texture_) {
        SDL_QueryTexture(impl_->texture_.get(), nullptr, nullptr,
                         &result.width, &result.height);
    } else if (impl_->surface_) {
        result.width = impl_->surface_->w;
        result.height = impl_->surface_->h;
    }

    return result;
}

SDL_Surface* Texture::as_surface() noexcept
{
    return impl_->surface_.get();
}

bool Texture::empty() const noexcept
{
    return impl_ == nullptr;
}

} // end namespace detail

}
