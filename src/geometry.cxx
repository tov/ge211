#include "ge211/geometry.hxx"

#include <cmath>

namespace ge211 {

namespace geometry {

Transform::Transform() NOEXCEPT
        : rotation_{0},
          scale_x_{1.0},
          scale_y_{1.0},
          flip_h_{false},
          flip_v_{false}
{ }

Transform
Transform::rotation(double degrees) NOEXCEPT
{
    return Transform().set_rotation(degrees);
}

Transform
Transform::flip_h() NOEXCEPT
{
    return Transform().set_flip_h(true);
}

Transform
Transform::flip_v() NOEXCEPT
{
    return Transform().set_flip_v(true);
}

Transform
Transform::scale(double factor) NOEXCEPT
{
    return Transform().set_scale(factor);
}

Transform
Transform::scale_x(double factor) NOEXCEPT
{
    return Transform().set_scale_x(factor);
}

Transform
Transform::scale_y(double factor) NOEXCEPT
{
    return Transform().set_scale_y(factor);
}

Transform&
Transform::set_rotation(double rotation) NOEXCEPT
{
    while (rotation < 0) { rotation += 360; }
    rotation_ = std::fmod(rotation, 360);
    return *this;
}

Transform&
Transform::set_flip_h(bool flip_h) NOEXCEPT
{
    flip_h_ = flip_h;
    return *this;
}

Transform&
Transform::set_flip_v(bool flip_v) NOEXCEPT
{
    flip_v_ = flip_v;
    return *this;
}

Transform&
Transform::set_scale(double scale) NOEXCEPT
{
    scale_x_ = scale;
    scale_y_ = scale;
    return *this;
}

Transform&
Transform::set_scale_x(double scale_x) NOEXCEPT
{
    scale_x_ = scale_x;
    return *this;
}

Transform&
Transform::set_scale_y(double scale_y) NOEXCEPT
{
    scale_y_ = scale_y;
    return *this;
}

double
Transform::get_rotation() const NOEXCEPT
{
    return rotation_;
}

bool
Transform::get_flip_h() const NOEXCEPT
{
    return flip_h_;
}

bool
Transform::get_flip_v() const NOEXCEPT
{
    return flip_v_;
}

double
Transform::get_scale_x() const NOEXCEPT
{
    return scale_x_;
}

double
Transform::get_scale_y() const NOEXCEPT
{
    return scale_y_;
}

bool
Transform::is_identity() const NOEXCEPT
{
    return *this == Transform();
}

Transform
Transform::inverse() const NOEXCEPT
{
    Transform result;
    result.set_rotation(360 - rotation_);
    result.set_flip_h(flip_h_);
    result.set_flip_v(flip_v_);
    result.set_scale_x(1 / scale_x_);
    result.set_scale_y(1 / scale_y_);
    return result;
}

Transform
Transform::operator*(Transform const& that) const NOEXCEPT
{
    return Transform{}
            .set_rotation(get_rotation() + that.get_rotation())
            .set_flip_h(get_flip_h() ^ that.get_flip_h())
            .set_flip_v(get_flip_v() ^ that.get_flip_v())
            .set_scale_x(get_scale_x() * that.get_scale_x())
            .set_scale_y(get_scale_y() * that.get_scale_y());
}

bool
Transform::operator==(Transform const& that) const NOEXCEPT
{
    return get_rotation() == that.get_rotation() &&
           get_flip_h() == that.get_flip_h() &&
           get_flip_v() == that.get_flip_v() &&
           get_scale_x() == that.get_scale_x() &&
           get_scale_y() == that.get_scale_y();
}

bool
Transform::operator!=(Transform const& that) const NOEXCEPT
{
    return !(operator==(that));
}

}

}
