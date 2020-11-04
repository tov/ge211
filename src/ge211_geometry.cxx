#include "ge211_geometry.hxx"

#include <cmath>

namespace ge211 {

namespace geometry {

Transform::Transform() NOEXCEPT
        : rotation_{0}, scale_x_{1.0}, scale_y_{1.0},
          flip_h_{false}, flip_v_{false}
{ }

Transform Transform::rotation(double degrees) NOEXCEPT
{
    return Transform().set_rotation(degrees);
}

Transform Transform::flip_h() NOEXCEPT
{
    return Transform().set_flip_h(true);
}

Transform Transform::flip_v() NOEXCEPT
{
    return Transform().set_flip_v(true);
}

Transform Transform::scale(double factor) NOEXCEPT
{
    return Transform().set_scale(factor);
}

Transform Transform::scale_x(double factor) NOEXCEPT
{
    return Transform().set_scale_x(factor);
}

Transform Transform::scale_y(double factor) NOEXCEPT
{
    return Transform().set_scale_y(factor);
}

Transform& Transform::set_rotation(double rotation) NOEXCEPT
{
    while (rotation < 0) rotation += 360;
    rotation_ = std::fmod(rotation, 360);
    return *this;
}

Transform& Transform::set_flip_h(bool flip_h) NOEXCEPT
{
    flip_h_ = flip_h;
    return *this;
}

Transform& Transform::set_flip_v(bool flip_v) NOEXCEPT
{
    flip_v_ = flip_v;
    return *this;
}

Transform& Transform::set_scale(double scale) NOEXCEPT
{
    scale_x_ = scale;
    scale_y_ = scale;
    return *this;
}

Transform& Transform::set_scale_x(double scale_x) NOEXCEPT
{
    scale_x_ = scale_x;
    return *this;
}

Transform& Transform::set_scale_y(double scale_y) NOEXCEPT
{
    scale_y_ = scale_y;
    return *this;
}

double Transform::get_rotation() const NOEXCEPT
{
    return rotation_;
}

bool Transform::get_flip_h() const NOEXCEPT
{
    return flip_h_;
}

bool Transform::get_flip_v() const NOEXCEPT
{
    return flip_v_;
}

double Transform::get_scale_x() const NOEXCEPT
{
    return scale_x_;
}

double Transform::get_scale_y() const NOEXCEPT
{
    return scale_y_;
}

bool Transform::is_identity() const NOEXCEPT
{
    return *this == Transform();
}

Transform Transform::operator*(const Transform& other) const NOEXCEPT
{
    Transform result;
    result.set_rotation(rotation_ + other.rotation_);
    result.set_flip_h(flip_h_ ^ other.flip_h_);
    result.set_flip_v(flip_v_ ^ other.flip_v_);
    result.set_scale_x(scale_x_ * other.scale_x_);
    result.set_scale_y(scale_y_ * other.scale_y_);
    return result;
}

Transform Transform::inverse() const NOEXCEPT
{
    Transform result;
    result.set_rotation(360 - rotation_);
    result.set_flip_h(flip_h_);
    result.set_flip_v(flip_v_);
    result.set_scale_x(1 / scale_x_);
    result.set_scale_y(1 / scale_y_);
    return result;
}

bool Transform::operator==(Transform const& other) const NOEXCEPT
{
    return get_rotation() == other.get_rotation() &&
           get_flip_h() == other.get_flip_h() &&
           get_flip_v() == other.get_flip_v() &&
           get_scale_x() == other.get_scale_x() &&
           get_scale_y() == other.get_scale_y();
}

bool Transform::operator!=(Transform const& other) const NOEXCEPT
{
    return !(*this == other);
}

}

}
