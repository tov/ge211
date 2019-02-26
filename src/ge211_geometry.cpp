#include "ge211_geometry.h"

#include <cmath>
#include <ge211_geometry.h>


namespace ge211 {

namespace geometry {

Transform::Transform() noexcept
        : crop_{0, 0, 0, 0}
        , rotation_{0}
        , scale_{1.0, 1.0}
        , flip_{false, false}
{ }

Transform Transform::crop(Crop crop) noexcept
{
    return Transform().set_crop(crop);
}

Transform Transform::rotation(double degrees) noexcept
{
    return Transform().set_rotation(degrees);
}

Transform Transform::flip_h() noexcept
{
    return Transform().set_flip_h(true);
}

Transform Transform::flip_v() noexcept
{
    return Transform().set_flip_v(true);
}

Transform Transform::scale(double factor) noexcept
{
    return Transform().set_scale(factor);
}

Transform Transform::scale_x(double factor) noexcept
{
    return Transform().set_scale_x(factor);
}

Transform Transform::scale_y(double factor) noexcept
{
    return Transform().set_scale_y(factor);
}

Transform& Transform::set_rotation(double rotation) noexcept
{
    while (rotation < 0) rotation += 360;
    rotation_ = std::fmod(rotation, 360);
    return *this;
}

Transform& Transform::set_flip_h(bool flip_h) noexcept
{
    flip_.x = flip_h;
    return *this;
}

Transform& Transform::set_flip_v(bool flip_v) noexcept
{
    flip_.y = flip_v;
    return *this;
}

Transform& Transform::set_scale(double scale) noexcept
{
    scale_.width = scale;
    scale_.height = scale;
    return *this;
}

Transform& Transform::set_scale_x(double scale_x) noexcept
{
    scale_.width = scale_x;
    return *this;
}

Transform& Transform::set_scale_y(double scale_y) noexcept
{
    scale_.height = scale_y;
    return *this;
}

Transform& Transform::set_crop(Transform::Side side,
                               std::int16_t amount) noexcept
{
    if (side & Side::top)    crop_.top    = amount;
    if (side & Side::bottom) crop_.bottom = amount;
    if (side & Side::left)   crop_.left   = amount;
    if (side & Side::right)  crop_.right  = amount;
    return *this;
}

Transform& Transform::set_crop(Transform::Crop crop) noexcept
{
    crop_ = crop;
    return *this;
}

double Transform::get_rotation() const noexcept
{
    return rotation_;
}

bool Transform::get_flip_h() const noexcept
{
    return flip_.x;
}

bool Transform::get_flip_v() const noexcept
{
    return flip_.y;
}

double Transform::get_scale_x() const noexcept
{
    return scale_.width;
}

double Transform::get_scale_y() const noexcept
{
    return scale_.height;
}

std::int16_t Transform::get_crop(ge211::Transform::Side side) const noexcept
{
    std::int16_t result = 0;
    if (side & Side::top)    result += crop_.top;
    if (side & Side::bottom) result += crop_.bottom;
    if (side & Side::left)   result += crop_.left;
    if (side & Side::right)  result += crop_.right;
    return result;
}

bool Transform::is_identity() const noexcept
{
    return *this == Transform();
}

Transform Transform::operator*(const Transform& other) const noexcept
{
    Transform result;
    result.set_rotation(rotation_ + other.rotation_);
    result.set_flip_h(flip_.x ^ other.flip_.x);
    result.set_flip_v(flip_.y ^ other.flip_.y);
    result.set_scale_x(scale_.width * other.scale_.width);
    result.set_scale_y(scale_.height * other.scale_.height);
    result.crop_.top = crop_.top + other.crop_.top;
    result.crop_.bottom = crop_.bottom + other.crop_.bottom;
    result.crop_.left = crop_.left + other.crop_.left;
    result.crop_.right = crop_.right + other.crop_.right;
    return result;
}

Transform Transform::inverse() const noexcept
{
    Transform result;
    result.set_rotation(-rotation_);
    result.set_flip_h(flip_.x);
    result.set_flip_v(flip_.y);
    result.set_scale_x(1 / scale_.width);
    result.set_scale_y(1 / scale_.height);
    result.crop_.top *= -1;
    result.crop_.bottom *= -1;
    result.crop_.left *= -1;
    result.crop_.right *= -1;
    return result;
}

Transform::Crop Transform::get_crop() const noexcept
{
    return crop_;
}

bool operator==(const Transform& t1, const Transform& t2) noexcept
{
    return t1.get_rotation() == t2.get_rotation() &&
            t1.get_flip_h() == t2.get_flip_h() &&
            t1.get_flip_v() == t2.get_flip_v() &&
            t1.get_scale_x() == t2.get_scale_x() &&
            t1.get_scale_y() == t2.get_scale_y() &&
            t1.get_crop() == t2.get_crop();
}

bool operator!=(const Transform& t1, const Transform& t2) noexcept
{
    return !(t1 == t2);
}

Transform::Side const Transform::Side::none   {  0 };
Transform::Side const Transform::Side::top    {  1 };
Transform::Side const Transform::Side::bottom {  2 };
Transform::Side const Transform::Side::left   {  4 };
Transform::Side const Transform::Side::right  {  8 };
Transform::Side const Transform::Side::all    { 15 };

bool operator==(Transform::Crop a, Transform::Crop b)
{
return a.top == b.top &&
        a.bottom == b.bottom &&
        a.left == b.left &&
        a.right == b.right;
}

bool operator!=(Transform::Crop a, Transform::Crop b)
{
    return !(a == b);
}

}

}
