#include "ge211_color.hxx"

#include <algorithm>
#include <cmath>
#include <tuple>


namespace ge211 {

template<class T, class U>
static T weighted_average(T t, double weight, U u) NOEXCEPT
{
    auto   f1     = static_cast<double>(t);
    auto   f2     = static_cast<double>(u);
    double result = (1 - weight) * f1 + weight * f2;
    return T(result);
}

template<class Whole, class Field, class Goal>
static Whole adjust_field(Whole result, Field Whole::*field,
                          double weight, Goal goal) NOEXCEPT
{
    result.*field = weighted_average(result.*field, weight, goal);
    return result;
}

Color
Color::from_rgba(double red, double green, double blue, double alpha) NOEXCEPT
{
    return Color{uint8_t(255 * red),
                 uint8_t(255 * green),
                 uint8_t(255 * blue),
                 uint8_t(255 * alpha)};
}

// Creates a color from the HSL/HSV-style hue, the chroma, an adjustment
// for brightness, and the alpha.
//
// From https://en.wikipedia.org/wiki/HSL_and_HSV
static Color from_hcma(double hue,
                       double C,
                       double m,
                       double alpha) NOEXCEPT
{
    double H6 = std::fmod(hue, 360.0) / 60.0;
    double X  = C * (1 - std::fabs(std::fmod(H6, 2) - 1));

    double r1 = 0, g1 = 0, b1 = 0;

    if (H6 <= 1) {
        r1 = C;
        g1 = X;
    } else if (H6 <= 2) {
        r1 = X;
        g1 = C;
    } else if (H6 <= 3) {
        g1 = C;
        b1 = X;
    } else if (H6 <= 4) {
        g1 = X;
        b1 = C;
    } else if (H6 <= 5) {
        b1 = C;
        r1 = X;
    } else {
        b1 = X;
        r1 = C;
    }

    return Color::from_rgba(r1 + m, g1 + m, b1 + m, alpha);
}

Color Color::from_hsla(double hue, double saturation, double lightness,
                       double alpha) NOEXCEPT
{
    double C = (1 - std::fabs(2 * lightness - 1)) * saturation;
    double m = lightness - 0.5 * C;
    return from_hcma(hue, C, m, alpha);
}

Color Color::from_hsva(double hue, double saturation, double value,
                       double alpha) NOEXCEPT
{
    double C = value * saturation;
    double m = value - C;
    return from_hcma(hue, C, m, alpha);
}

Color Color::blend(double weight, Color that) const NOEXCEPT
{
    return Color{
            weighted_average(red(), weight, that.red()),
            weighted_average(green(), weight, that.green()),
            weighted_average(blue(), weight, that.blue()),
            weighted_average(alpha(), weight, that.alpha())
    };
}

Color Color::invert() const NOEXCEPT
{
    return Color{uint8_t(~red_), uint8_t(~blue_), uint8_t(~green_), alpha_};
}

Color Color::rotate_hue(double degrees) const NOEXCEPT
{
    return to_hsva().rotate_hue(degrees).to_rgba();
}

Color Color::lighten(double unit_amount) const NOEXCEPT
{
    return to_hsla().lighten(unit_amount).to_rgba();
}

Color Color::darken(double unit_amount) const NOEXCEPT
{
    return to_hsla().darken(unit_amount).to_rgba();
}

Color Color::saturate(double unit_amount) const NOEXCEPT
{
    return to_hsla().saturate(unit_amount).to_rgba();
}

Color Color::desaturate(double unit_amount) const NOEXCEPT
{
    return to_hsla().desaturate(unit_amount).to_rgba();
}

static std::tuple<double, double, double, double> to_HCMm(Color color) NOEXCEPT
{
    double R = color.red() / 255.0;
    double G = color.green() / 255.0;
    double B = color.blue() / 255.0;

    double M = std::max(R, std::max(G, B));
    double m = std::min(R, std::min(G, B));
    double C = M - m;

    double H6 =
                   (M == R) ? std::fmod((G - B) / C, 6) :
                   (M == G) ? (B - R) / C + 2 :
                   (R - G) / C + 4;

    double H = 60 * H6;

    return std::make_tuple(H, C, M, m);
}

Color::HSLA Color::to_hsla() const NOEXCEPT
{
    double H, C, M, m;
    std::tie(H, C, M, m) = to_HCMm(*this);

    double L = (M + m) / 2;
    double S = (L == 1) ? 0 : C / (1 - std::fabs(2 * L - 1));

    return {H, S, L, alpha() / 255.0};
}

Color::HSVA Color::to_hsva() const NOEXCEPT
{
    double H, C, M, m;
    std::tie(H, C, M, m) = to_HCMm(*this);

    double V = M;
    double S = V == 0 ? 0 : C / V;

    return {H, S, V, alpha() / 255.0};
}

SDL_Color Color::to_sdl_() const NOEXCEPT
{
    SDL_Color result;
    result.a = alpha_;
    result.r = red_;
    result.g = green_;
    result.b = blue_;
    return result;
}

uint32_t Color::to_sdl_(const SDL_PixelFormat* format) const NOEXCEPT
{
    return SDL_MapRGBA(format, red_, green_, blue_, alpha_);
}

Color Color::fade_in(double unit_amount) const NOEXCEPT
{
    return adjust_field(*this, &Color::alpha_, unit_amount, 255);
}

Color Color::fade_out(double unit_amount) const NOEXCEPT
{
    return adjust_field(*this, &Color::alpha_, unit_amount, 0);
}

Color::HSLA::HSLA(double hue,
                  double saturation,
                  double lightness,
                  double alpha) NOEXCEPT
        : hue{hue}
        , saturation{saturation}
        , lightness{lightness}
        , alpha{alpha}
{ }

Color Color::HSLA::to_rgba() const NOEXCEPT
{
    return Color::from_hsla(hue, saturation, lightness, alpha);
}

Color::HSLA Color::HSLA::rotate_hue(double degrees) const NOEXCEPT
{
    auto result = *this;
    result.hue = std::fmod(result.hue + degrees, 360);
    return result;
}

Color::HSLA Color::HSLA::saturate(double unit_amount) const NOEXCEPT
{
    return adjust_field(*this, &HSLA::saturation, unit_amount, 1.0);
}

Color::HSLA Color::HSLA::desaturate(double unit_amount) const NOEXCEPT
{
    return adjust_field(*this, &HSLA::saturation, unit_amount, 0.0);
}

Color::HSLA Color::HSLA::lighten(double unit_amount) const NOEXCEPT
{
    return adjust_field(*this, &HSLA::lightness, unit_amount, 1.0);
}

Color::HSLA Color::HSLA::darken(double unit_amount) const NOEXCEPT
{
    return adjust_field(*this, &HSLA::lightness, unit_amount, 0.0);
}

Color::HSLA Color::HSLA::fade_in(double unit_amount) const NOEXCEPT
{
    return adjust_field(*this, &HSLA::alpha, unit_amount, 1.0);
}

Color::HSLA Color::HSLA::fade_out(double unit_amount) const NOEXCEPT
{
    return adjust_field(*this, &HSLA::alpha, unit_amount, 0.0);
}

Color Color::HSVA::to_rgba() const NOEXCEPT
{
    return Color::from_hsva(hue, saturation, value, alpha);
}

Color::HSVA::HSVA(double hue, double saturation,
                  double value, double alpha) NOEXCEPT
        : hue(hue)
        , saturation(saturation)
        , value(value)
        , alpha(alpha)
{ }

Color::HSVA Color::HSVA::rotate_hue(double degrees) const NOEXCEPT
{
    auto result = *this;
    result.hue = std::fmod(result.hue + degrees, 360);
    return result;
}

Color::HSVA Color::HSVA::saturate(double unit_amount) const NOEXCEPT
{
    return adjust_field(*this, &HSVA::saturation, unit_amount, 1.0);
}

Color::HSVA Color::HSVA::desaturate(double unit_amount) const NOEXCEPT
{
    return adjust_field(*this, &HSVA::saturation, unit_amount, 0.0);
}

Color::HSVA Color::HSVA::revalue(double unit_amount) const NOEXCEPT
{
    return adjust_field(*this, &HSVA::value, unit_amount, 1.0);
}

Color::HSVA Color::HSVA::devalue(double unit_amount) const NOEXCEPT
{
    return adjust_field(*this, &HSVA::value, unit_amount, 0.0);
}

Color::HSVA Color::HSVA::fade_in(double unit_amount) const NOEXCEPT
{
    return adjust_field(*this, &HSVA::alpha, unit_amount, 1.0);
}

Color::HSVA Color::HSVA::fade_out(double unit_amount) const NOEXCEPT
{
    return adjust_field(*this, &HSVA::alpha, unit_amount, 0.0);
}

}
