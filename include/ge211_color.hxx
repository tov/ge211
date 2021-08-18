#pragma once

#include "ge211_forward.hxx"
#include "ge211_doxygen.hxx"

#include <SDL_pixels.h>
#include <cstdint>

namespace ge211 {

/// For representing colors.
///
/// A color has red, green, blue, and alpha (opacity) components,
/// each of which is an integer from 0 to 255, inclusive.
/// The most common way to construct a color is to pass these
/// components to the constructor
/// Color(uint8_t, uint8_t, uint8_t, uint8_t). The components can
/// also be passed as unit-interval `double`s to
/// Color::from_rgba(double, double, double, double).
///
/// It's also possible to construct a color via alternative color
/// models @ref HSLA and @ref HSVA, and then convert those to RGBA.
class Color
{
public:
    /// \name Constructors and factories
    /// @{

    /// Constructs the transparent color.
    constexpr Color() NOEXCEPT_
            : Color{0, 0, 0, 0}
    { }

    /// Constructs the color with the given components.
    ///
    /// Components are integers from 0 to 255, inclusive. If `alpha` is not
    /// provided, it defaults to fully opaque.
    constexpr Color(uint8_t red,
                    uint8_t green,
                    uint8_t blue,
                    uint8_t alpha = 255) NOEXCEPT_
            : red_{red}, green_{green}, blue_{blue}, alpha_{alpha}
    { }

    /// Constructs a color with the given components.
    ///
    /// Components are doubles from 0.0. to 1.0, inclusive.
    /// If `alpha` is not given, it defaults to 1.0, meaning fully opaque.
    static Color from_rgba(double red, double green, double blue,
                           double alpha = 1.0) NOEXCEPT_;

    /// @}

    /// \name Named colors
    /// @{

    /// Solid white.
    static constexpr Color white() NOEXCEPT_
    {
        return {255, 255, 255};
    }

    /// Solid black.
    static constexpr Color black() NOEXCEPT_
    {
        return {0, 0, 0};
    }

    /// Solid red.
    static Color medium_red() NOEXCEPT_
    { return from_hsla(0, .5, .5); }
    /// Solid green.
    static Color medium_green() NOEXCEPT_
    { return from_hsla(120, .5, .5); }
    /// Solid blue.
    static Color medium_blue() NOEXCEPT_
    { return from_hsla(240, .5, .5); }
    /// Solid cyan.
    static Color medium_cyan() NOEXCEPT_
    { return from_hsla(180, .5, .5); }
    /// Solid magenta.
    static Color medium_magenta() NOEXCEPT_
    { return from_hsla(300, .5, .5); }
    /// Solid yellow.
    static Color medium_yellow() NOEXCEPT_
    { return from_hsla(60, .5, .5); }

    /// @}

    /// \name RGBA-model getters
    /// @{

    /// Gets the red component of a color.
    uint8_t red() const NOEXCEPT_ { return red_; };
    /// Gets the green component of a color.
    uint8_t green() const NOEXCEPT_ { return green_; };
    /// Gets the blue component of a color.
    uint8_t blue() const NOEXCEPT_ { return blue_; };
    /// Gets the alpha (opacity) component of a color.
    uint8_t alpha() const NOEXCEPT_ { return alpha_; };


    /// \name Transformations
    /// @{

    /// Produces a blend of this color and `that`, with higher
    /// `weight` (between 0 and 1) increasing the level of `that`.
    Color blend(double weight, Color that) const NOEXCEPT_;

    /// Returns the inverse of a color.
    Color invert() const NOEXCEPT_;

    /// Returns a color by rotating the hue, leaving the other properties
    /// constant.
    Color rotate_hue(double degrees) const NOEXCEPT_;

    /// Produces a tint by lightening the color. The `amount` must be
    /// between 0 and 1, where 0 leaves the color the same, and 1 produces
    /// white.
    Color lighten(double unit_amount) const NOEXCEPT_;

    /// Produces a shade by darkening the color. The `unit_amount` must be
    /// between 0 and 1, where 0 leaves the color the same, and 1 produces
    /// black.
    Color darken(double unit_amount) const NOEXCEPT_;

    /// Produces a fuller tone by saturating the color. The `unit_amount` must
    /// be between 0 and 1, where 0 leaves the color the same, and 1
    /// produces a fully saturated color.
    Color saturate(double unit_amount) const NOEXCEPT_;

    /// Produces a weaker tone by desaturating the color. The `unit_amount`
    /// must be between 0 and 1, where 0 leaves the color the same, and
    /// 1 produces gray at the same lightness as the original color.
    Color desaturate(double unit_amount) const NOEXCEPT_;

    /// Increases opacity of the color. The `unit_amount` must
    /// be between 0 and 1, where 0 leaves the color the same, and 1
    /// produces a fully opaque color.
    Color fade_in(double unit_amount) const NOEXCEPT_;

    /// Decreases opacity of the color. The // `unit_amount`
    /// must be between 0 and 1, where 0 leaves the color the same, and
    /// 1 produces full transparency.
    Color fade_out(double unit_amount) const NOEXCEPT_;

    /// @}

    /// \name Alternative color models
    /// @{

    /// Representation for the hue-saturation-lightness-alpha color
    /// model. See [Wikipedia](https://en.wikipedia.org/wiki/HSL_and_HSV)
    /// for details on color models.
    struct HSLA
    {
        /// The hue in degrees from 0 to 360. 0° (and 360°) is red,
        /// 120° is green, and 240° is blue.
        double hue;

        /// The fullness of the color, from 0.0 (grey) to 1.0 (fully
        /// saturated).
        double saturation;

        /// The lightness of the color, from 0.0 (black) to 1.0 (white).
        double lightness;

        /// The opacity of the color, from 0.0 (fully transparent) to
        /// 1.0 (fully opaque).
        double alpha;

        /// Constructs a hue-saturation-lightness-alpha color from its
        /// unit interval components.
        HSLA(double hue, double saturation,
             double lightness, double alpha = 1.0) NOEXCEPT_;

        /// Converts color to the RGBA color model.
        Color to_rgba() const NOEXCEPT_;

        /// \name Transformations
        /// @{

        /// Returns a color by rotating the hue, leaving the other properties

        HSLA rotate_hue(double degrees) const NOEXCEPT_;
        /// Produces a fuller tone by saturating the color. The `unit_amount` must
        /// be between 0 and 1, where 0 leaves the color the same, and 1
        /// produces a fully saturated color.
        HSLA saturate(double unit_amount) const NOEXCEPT_;
        /// Produces a weaker tone by desaturating the color. The `unit_amount`
        /// must be between 0 and 1, where 0 leaves the color the same, and
        /// 1 produces gray at the same lightness as the original color.
        HSLA desaturate(double unit_amount) const NOEXCEPT_;
        /// Produces a tint by lightening the color. The `amount` must be
        /// between 0 and 1, where 0 leaves the color the same, and 1 produces
        /// white.
        HSLA lighten(double unit_amount) const NOEXCEPT_;
        /// Produces a shade by darkening the color. The `unit_amount` must be
        /// between 0 and 1, where 0 leaves the color the same, and 1 produces
        /// black.
        HSLA darken(double unit_amount) const NOEXCEPT_;
        /// Increases opacity of the color. The `unit_amount` must
        /// be between 0 and 1, where 0 leaves the color the same, and 1
        /// produces a fully opaque color.
        HSLA fade_in(double unit_amount) const NOEXCEPT_;
        /// Decreases opacity of the color. The // `unit_amount`
        /// must be between 0 and 1, where 0 leaves the color the same, and
        /// 1 produces full transparency.
        HSLA fade_out(double unit_amount) const NOEXCEPT_;

        /// @}
    };

    /// Constructs a color given the hue, saturation, lightness, and alpha.
    ///
    /// \param hue in degrees, from 0.0 to 360.0
    /// \param saturation from 0.0 to 1.0
    /// \param lightness from 0.0 to 1.0
    /// \param alpha opacity, from 0.0 to 1.0
    /// \return the color
    static Color from_hsla(double hue, double saturation, double lightness,
                           double alpha = 1) NOEXCEPT_;

    /// Converts a color to the hue-saturation-lightness (HSL) color model.
    HSLA to_hsla() const NOEXCEPT_;
    /// Representation for the hue-saturation-apply-alpha color
    /// model. See [Wikipedia](https://en.wikipedia.org/wiki/HSL_and_HSV)
    /// for details on color models.
    struct HSVA
    {
        /// The hue in degrees from 0 to 360. 0° (and 360°) is red,
        /// 120° is green, and 240° is blue.
        double hue;

        /// The fullness of the color, from 0,0 (grey) to 1.0 (fully
        /// saturated).
        double saturation;

        /// The brightness of the color, from 0.0 (black) to 1.0 (fully
        /// colored).
        double value;

        /// The opacity of the color, from 0.0 (fully transparent) to
        /// 1.0 (fully opaque).
        double alpha;

        /// Constructs a hue-saturation-apply-alpha color from its
        /// unit interval components.
        HSVA(double hue, double saturation,
             double value, double alpha = 1.0) NOEXCEPT_;

        /// Converts color to the RGBA color model.
        Color to_rgba() const NOEXCEPT_;

        /// \name Transformations
        /// @{

        /// Returns a color by rotating the hue, leaving the other properties
        /// constant.
        HSVA rotate_hue(double degrees) const NOEXCEPT_;
        /// Produces a fuller tone by saturating the color. The `unit_amount` must
        /// be between 0 and 1, where 0 leaves the color the same, and 1
        /// produces a fully saturated color.
        HSVA saturate(double unit_amount) const NOEXCEPT_;
        /// Produces a weaker tone by desaturating the color. The `unit_amount`
        /// must be between 0 and 1, where 0 leaves the color the same, and
        /// 1 produces gray at the same lightness as the original color.
        HSVA desaturate(double unit_amount) const NOEXCEPT_;
        /// Produces a brighter color by increasing the apply. The `unit_amount`
        /// must be between 0 and 1, where 0 leaves the color the same, and
        /// 1 produces a fully bright color.
        HSVA revalue(double unit_amount) const NOEXCEPT_;
        /// Produces a darker color by decreasing the apply. The `unit_amount`
        /// must be between 0 and 1, where 0 leaves the color the same, and
        /// 1 produces black.
        HSVA devalue(double unit_amount) const NOEXCEPT_;
        /// Increases opacity of the color. The `unit_amount` must
        /// be between 0 and 1, where 0 leaves the color the same, and 1
        /// produces a fully opaque color.
        HSVA fade_in(double unit_amount) const NOEXCEPT_;
        /// Decreases opacity of the color. The // `unit_amount`
        /// must be between 0 and 1, where 0 leaves the color the same, and
        /// 1 produces full transparency.
        HSVA fade_out(double unit_amount) const NOEXCEPT_;

        /// @}
    };

    /// Constructs a color given the hue, saturation, apply, and alpha.
    ///
    /// \param hue in degrees, from 0.0 to 360.0
    /// \param saturation from 0.0 to 1.0
    /// \param value from 0.0 to 1.0
    /// \param alpha opacity, from 0.0 to 1.0
    /// \return the color
    static Color from_hsva(double hue, double saturation, double value,
                           double alpha = 1) NOEXCEPT_;

    /// Converts a color to the hue-saturation-apply (HSV) color model.
    HSVA to_hsva() const NOEXCEPT_;

    /// @}

private:
    uint8_t red_;
    uint8_t green_;
    uint8_t blue_;
    uint8_t alpha_;

    friend Text_sprite;
    friend ::ge211::internal::Render_sprite;

    SDL_Color to_sdl_() const NOEXCEPT_;
    uint32_t to_sdl_(const SDL_PixelFormat*) const NOEXCEPT_;
};

}

