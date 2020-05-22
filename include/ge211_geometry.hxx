#pragma once

#include "ge211_forward.hxx"
#include "ge211_noexcept.hxx"
#include "ge211_util.hxx"

#include <SDL_rect.h>

#include <type_traits>
#include <utility>

namespace ge211 {

/// Geometric objects and their operations.
namespace geometry {

/// The type of the special value @ref the_origin.
///
/// This type exists only so that we can overload the
/// Basic_position constructor to construct the origin. See
/// @ref the_origin for examples.
class Origin_type { };

/// Represents the dimensions of an object, or more generally,
/// the displacement between two Basic_position%s. Note that
/// much of the library uses @ref Dimensions, which is a
/// type alias for `Basic_dimensions<int>`.
template <class T>
struct Basic_dimensions
{
    /// The coordinate type for the dimensions. This is an alias of
    /// type parameter `T`.
    using Coordinate = T;

    Coordinate width;  ///< The width of the object.
    Coordinate height; ///< The height of the object.

    /// Converts a Basic_dimensions to another coordinate type.
    /// For example:
    ///
    /// ```
    /// auto d1 = ge211::Basic_dimensions<int>{3, 4};
    /// auto d2 = d1.into<double>();
    /// ```
    template <class U>
    Basic_dimensions<U> into() const
        NOEXCEPT_(detail::is_nothrow_convertible<Coordinate, U>())
    {
        return {U(width), U(height)};
    }

    /// Alias for Basic_dimensions::into to support explicit conversions
    /// between coordinate types.
    ///
    /// For example:
    ///
    /// ```
    /// ge211::Basic_dimensions<int> d1{3, 4};
    /// ge211::Basic_dimensions<double> d2(d1);
    /// ```
    template <class U>
    explicit operator Basic_dimensions<U>() const
    NOEXCEPT_(noexcept(into<U>()))
    {
        return into<U>();
    }
};

/// Type alias for the most common use of Basic_dimensions, which is
/// with a coordinate type of `int`.
using Dimensions = Basic_dimensions<int>;

/// Equality for Basic_dimensions.
template <class T>
bool operator==(Basic_dimensions<T> a, Basic_dimensions<T> b)
    NOEXCEPT_(detail::is_nothrow_comparable<T>())
{
    return a.width == b.width && a.height == b.height;
}

/// Disequality for Basic_dimensions.
template <class T>
bool operator!=(Basic_dimensions<T> a, Basic_dimensions<T> b)
    NOEXCEPT_(detail::is_nothrow_comparable<T>())
{
    return !(a == b);
}

/// Adds two Basic_dimensions%es. This is vector addition.
template<class T>
Basic_dimensions<T> operator+(Basic_dimensions<T> d1,
                              Basic_dimensions<T> d2)
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
{
    return {d1.width + d2.width, d1.height + d2.height};
}

/// Subtracts two Basic_dimensions%es. This is vector subtraction.
template <class T>
Basic_dimensions<T> operator-(Basic_dimensions<T> d1,
                              Basic_dimensions<T> d2)
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
{
    return {d1.width - d2.width, d1.height - d2.height};
}

/// Multiplies a Basic_dimensions by a scalar.
template <class T>
Basic_dimensions<T> operator*(Basic_dimensions<T> d1, T s2)
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
{
    return {d1.width * s2, d1.height * s2};
}

/// Multiplies a Basic_dimensions by a scalar.
template <class T>
Basic_dimensions<T> operator*(T s1, Basic_dimensions<T> d2)
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
{
    return d2 * s1;
}

/// Multiplies a Basic_dimensions by a `double`. This is vector-scalar
/// multiplication. If the result components would be fractional, they are
/// truncated.
///
/// This function is disabled if `T` is `double`, as there is another
/// function for that.
template <class T,
          class = std::enable_if_t<!std::is_same<T, double>::value, void>>
Basic_dimensions<T> operator*(Basic_dimensions<T> d1, double s2)
    NOEXCEPT_(detail::has_nothrow_arithmetic<T, double>())
{
    return {static_cast<T>(d1.width * s2),
            static_cast<T>(d1.height * s2)};
}

/// Multiplies a Basic_dimensions by a `double`. This is vector-scalar
/// multiplication. If the result components would be fractional, they are
/// truncated.
///
/// This function is disabled if `T` is `double`, as there is another
/// function for that.
template <class T,
          class = std::enable_if_t<!std::is_same<T, double>::value, void>>
Basic_dimensions<T> operator*(double s1, Basic_dimensions<T> d2)
    NOEXCEPT_(detail::has_nothrow_arithmetic<double, T>())
{
    return d2 * s1;
}

/// Divides a Basic_dimensions by a `T`. This is vector-scalar
/// division. If the result components would be fractional, they are
/// truncated.
///
/// \preconditions
///  - `z` cannot be `0` if `T` is an integral type.
template <class T>
Basic_dimensions<T> operator/(Basic_dimensions<T> d1, T s2)
    NOEXCEPT_(detail::has_nothrow_division<T>())
{
    return {d1.width / s2, d1.height / s2};
}

/// Divides a Basic_dimensions by an `double`. This is vector-scalar
/// division. If the result components would be fractional, they are
/// truncated.
///
/// This function is disabled if `T` is `double`, as there is another
/// function for that.
template <class T,
          class = std::enable_if_t<!std::is_same<T, double>::value, void>>
Basic_dimensions<T> operator/(Basic_dimensions<T> d1, double s2)
    NOEXCEPT_(detail::has_nothrow_arithmetic<T, double>())
{
    return d1 * (1 / s2);
}

/// Succinct Basic_dimensions addition.
template <class T>
Basic_dimensions<T>& operator+=(Basic_dimensions<T>& d1,
                                Basic_dimensions<T> d2)
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
{
    return d1 = d1 + d2;
}

/// Succinct Basic_dimensions subtraction.
template <class T>
Basic_dimensions<T>& operator-=(Basic_dimensions<T>& d1,
                                Basic_dimensions<T> d2)
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
{
    return d1 = d1 - d2;
}

/// Succinct [Basic_dimensions][1]-scalar multiplication.
///
/// [1]: @ref Basic_dimensions
template <class T>
Basic_dimensions<T>& operator*=(Basic_dimensions<T>& d1, T s2)
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
{
    return d1 = d1 * s2;
}

/// Succinct [Basic_dimensions][1]-scalar multiplication.
///
/// [1]: @ref Basic_dimensions
template <class T>
Basic_dimensions<T>& operator*=(Basic_dimensions<T>& d1, double s2)
    NOEXCEPT_(detail::has_nothrow_arithmetic<T, double>())
{
    return d1 = d1 * s2;
}

/// Succinct [Basic_dimensions][1]-scalar division.
///
/// \preconditions
///  - `s2 != 0`
///
/// [1]: @ref Basic_dimensions
template <class T>
Basic_dimensions<T>& operator/=(Basic_dimensions<T>& d1, T s2)
    NOEXCEPT_(detail::has_nothrow_division<T>())
{
    return d1 = d1 / s2;
}

/// Succinct [Basic_dimensions][1]-scalar division.
///
/// [1]: @ref Basic_dimensions
template <class T>
Basic_dimensions<T>& operator/=(Basic_dimensions<T>& d1, double s2)
    NOEXCEPT_(detail::has_nothrow_division<T, double>())
{
    return d1 = d1 / s2;
}

/// A position in the T-valued Cartesian plane. In graphics,
/// the origin is traditionally in the upper left, so the *x* coordinate
/// increases to the right and the *y* coordinate increases downward.
/// Note that much of the library uses geometry::Position, which is a
/// type alias for Basic_position<int>.
template <class T>
struct Basic_position
{
    /// The coordinate type for the position. This is an alias of
    /// type parameter `T`.
    using Coordinate = T;
    /// A dimensions type having the same coordinate type as this position
    /// type.
    using Dimensions = Basic_dimensions<Coordinate>;

    Coordinate x; ///< The *x* coordinate
    Coordinate y; ///< The *y* coordiante

    /// \name Constructors
    /// @{

    /// Constructs a position from the given *x* and *y* coordinates.
    Basic_position(Coordinate x, Coordinate y)
        NOEXCEPT_(detail::is_nothrow_convertible<T>())
            : x{x}, y{y}
    { }

    /// Constructs the origin when given @ref the_origin.
    Basic_position(Origin_type)
    NOEXCEPT_(detail::is_nothrow_convertible<int, T>())
            : Basic_position(0, 0)
    { }

    /// Constructs a position from a Basic_dimensions, which gives the
    /// displacement of the position from the origin.
    explicit
    Basic_position(Dimensions dims)
    NOEXCEPT_(detail::is_nothrow_convertible<T>())
            : Basic_position(dims.width, dims.height)
    { }

    /// Converts a Basic_position to another coordinate type.
    /// For example:
    ///
    /// ```
    /// auto p1 = ge211::Basic_position<int>{3, 4};
    /// auto p2 = p1.into<double>();
    /// ```
    template <class U>
    Basic_position<U> into() const
        NOEXCEPT_(detail::is_nothrow_convertible<T, U>())
    {
        return {U(x), U(y)};
    }

    /// Alias for Basic_position::into to support casting between
    /// coordinate types.
    ///
    /// For example:
    ///
    /// ```
    /// ge211::Basic_position<int>    p1(3, 4);
    /// ge211::Basic_position<double> p2(p1);
    /// ```
    template <class U>
    explicit operator Basic_position<U>() const
    NOEXCEPT_(noexcept(into<U>()))
    {
        return into<U>();
    }

    /// @}

    /// \name Shifting member functions
    /// @{

    /// Constructs the position that is above this position by the given
    /// amount.
    Basic_position up_by(Coordinate dy) const
        NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
    {
        return {x, y - dy};
    }

    /// Constructs the position that is below this position by the given
    /// amount.
    Basic_position down_by(Coordinate dy) const
        NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
    {
        return {x, y + dy};
    }

    /// Constructs the position that is to the left of this position by
    /// the given amount.
    Basic_position left_by(Coordinate dx) const
        NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
    {
        return {x - dx, y};
    }

    /// Constructs the position that is to the right of this position by
    /// the given amount.
    Basic_position right_by(Coordinate dx) const
        NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
    {
        return {x + dx, y};
    }

    /// Constructs the position that is above and left of this position
    /// by the given dimensions.
    Basic_position up_left_by(Dimensions dims) const
        NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
    {
        return {x - dims.width, y - dims.height};
    }

    /// Constructs the position that is above and right of this position
    /// by the given dimensions.
    Basic_position up_right_by(Dimensions dims) const
        NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
    {
        return {x + dims.width, y - dims.height};
    }

    /// Constructs the position that is below and left of this position
    /// by the given dimensions.
    Basic_position down_left_by(Dimensions dims) const
        NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
    {
        return {x - dims.width, y + dims.height};
    }

    /// Constructs the position that is below and right of this position
    /// by the given dimensions.
    Basic_position down_right_by(Dimensions dims) const
        NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
    {
        return {x + dims.width, y + dims.height};
    }

    /// @}
};

/// Type alias for the most common use of Basic_position, which is with
/// a coordinate type of `int`.
using Position = Basic_position<int>;

/// Equality for positions.
template <class T>
bool operator==(Basic_position<T> p1, Basic_position<T> p2)
    NOEXCEPT_(detail::is_nothrow_comparable<T>())
{
    return p1.x == p2.x && p1.y == p2.y;
}

/// Disequality for positions.
template <class T>
bool operator!=(Basic_position<T> p1, Basic_position<T> p2)
    NOEXCEPT_(detail::is_nothrow_comparable<T>())
{
    return !(p1 == p2);
}

/// Translates a position by some displacement. This is the same as
/// @ref Basic_position::down_right_by(Dimensions) const.
template <class T>
Basic_position<T> operator+(Basic_position<T> p1, Basic_dimensions<T> d2)
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
{
    return p1.down_right_by(d2);
}

/// Translates a position by some displacement.
template <class T>
Basic_position<T> operator+(Basic_dimensions<T> d1, Basic_position<T> p2)
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
{
    return p2.down_right_by(d1);
}

/// Translates a position by the opposite of some displacement. This is
/// the same as @ref Basic_position::up_left_by(Dimensions) const.
template <class T>
Basic_position<T> operator-(Basic_position<T> p1, Basic_dimensions<T> d2)
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
{
    return p1.up_left_by(d2);
}

/// Translates a position by the opposite of some displacement.
template <class T>
Basic_dimensions<T> operator-(Basic_position<T> p1, Basic_position<T> p2)
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
{
    return {p1.x - p2.x, p1.y - p2.y};
}

/// Succinct position translation.
template <class T>
Basic_position<T>& operator+=(Basic_position<T>& p1, Basic_dimensions<T> d2)
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
{
    return p1 = p1 + d2;
}

/// Succinct position translation.
template <class T>
Basic_position<T>& operator-=(Basic_position<T>& p1, Basic_dimensions<T> d2)
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
{
    return p1 = p1 - d2;
}

/// Represents a positioned rectangle.
template <class T>
struct Basic_rectangle
{
    /// The coordinate type for the rectangle. This is an alias of
    /// type parameter `T`.
    using Coordinate = T;
    /// A dimensions type having the same coordinate type as this rectangle
    /// type.
    using Dimensions = Basic_dimensions<Coordinate>;
    /// A position type having the same coordinate type as this rectangle
    /// type.
    using Position = Basic_position<Coordinate>;

    Coordinate x;         ///< The *x* coordinate of the upper-left vertex.
    Coordinate y;         ///< The *y* coordinate of the upper-left vertex.
    Coordinate width;     ///< The width of the rectangle in pixels.
    Coordinate height;    ///< The height of the rectangle in pixels.

    /// Converts a Basic_rectangle to another coordinate type.
    ///
    /// For example:
    ///
    /// ```
    /// auto r1 = ge211::Basic_rectangle<int>{-1, -1, 2, 2};
    /// auto r2 = r1.into<double>();
    /// ```
    template<typename U>
    Basic_rectangle<U> into() const
        NOEXCEPT_(detail::is_nothrow_convertible<Coordinate, U>())
    {
        return {U(x), U(y), U(width), U(height)};
    }

    /// Alias for Basic_rectangle::into to support casting between
    /// coordinate types.
    ///
    /// For example:
    ///
    /// ```
    /// ge211::Basic_rectangle<int>    r1{-1, -1, 2, 2};
    /// ge211::Basic_rectangle<double> r2(p1);
    /// ```
    template <class U>
    explicit operator Basic_rectangle<U>() const
    NOEXCEPT_(noexcept(into<U>()))
    {
        return into<U>();
    }

    /// Creates a Basic_rectangle given the position of its top left vertex
    /// and its dimensions.
    static Basic_rectangle from_top_left(Position tl, Dimensions dims)
        NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>())
    {
        return {tl.x, tl.y, dims.width, dims.height};
    }

    /// Creates a Basic_rectangle given the position of its top right vertex
    /// and its dimensions.
    static Basic_rectangle from_top_right(Position tr, Dimensions dims)
        NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>())
    {
        return from_top_left(tr.left_by(dims.width), dims);
    }

    /// Creates a Basic_rectangle given the position of its bottom left vertex
    /// and its dimensions.
    static Basic_rectangle from_bottom_left(Position bl, Dimensions dims)
        NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>())
    {
        return from_top_left(bl.up_by(dims.height), dims);
    }

    /// Creates a Basic_rectangle given the position of its bottom right vertex
    /// and its dimensions.
    static Basic_rectangle from_bottom_right(Position br, Dimensions dims)
        NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>())
    {
        return from_top_left(br.up_left_by(dims), dims);
    }

    /// Creates a Basic_rectangle given the position of its center
    /// and its dimensions.
    static Basic_rectangle from_center(Position center, Dimensions dims)
        NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>())
    {
        return from_top_left(center.up_left_by(dims / Coordinate(2)), dims);
    }

    /// The dimensions of the rectangle. Equivalent to
    /// `Basic_dimensions{rect.width, rect.height}`.
    Dimensions dimensions() const
        NOEXCEPT_(detail::is_nothrow_convertible<Coordinate>())
    {
        return {width, height};
    }

    /// The position of the top left vertex.
    Position top_left() const
        NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>())
    {
        return {x, y};
    }

    /// The position of the top right vertex.
    Position top_right() const
        NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>())
    {
        return top_left().right_by(width);
    }

    /// The position of the bottom left vertex.
    Position bottom_left() const
        NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>())
    {
        return top_left().down_by(height);
    }

    /// The position of the bottom right vertex.
    Position bottom_right() const
        NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>())
    {
        return top_left().down_right_by(dimensions());
    }

    /// The position of the center of the rectangle.
    Position center() const
        NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>() &&
                  detail::has_nothrow_division<Coordinate>())
    {
        return top_left().down_right_by(dimensions() / Coordinate(2));
    }

    class iterator;

    /// Returns an `iterator` to the top left corner of this rectangle.
    iterator begin() const
    {
        return {top_left(), y, y + height};
    }

    /// Returns an `iterator` one past the end of this rectangle.
    iterator end() const
    {
        return {top_left().right_by(width), y, y + height};
    }

private:
    friend Circle_sprite;
    friend class detail::Render_sprite;
    friend class detail::Renderer;

    /// Converts this rectangle to an internal SDL rectangle.
    operator SDL_Rect() const
        NOEXCEPT_(detail::is_nothrow_convertible<Coordinate, int>())
    {
        SDL_Rect result;
        result.x = static_cast<int>(x);
        result.y = static_cast<int>(y);
        result.w = static_cast<int>(width);
        result.h = static_cast<int>(height);
        return result;
    }
};

/// Type alias for the most common use of Basic_rectangle, which is with
/// a coordinate type of `int`.
using Rectangle = Basic_rectangle<int>;

/// Equality for rectangles. Note that this is naïve, in that it considers
/// empty rectangles with different positions to be different.
template <class T>
bool operator==(const Basic_rectangle<T>& r1,
                const Basic_rectangle<T>& r2)
    NOEXCEPT_(detail::is_nothrow_comparable<T>())
{
return r1.x == r2.x &&
        r1.y == r2.y &&
        r1.width == r2.width &&
        r1.height == r2.height;
}

/// Disequality for rectangles.
template <class T>
bool operator!=(const Basic_rectangle<T> &r1,
                const Basic_rectangle<T> &r2)
    NOEXCEPT_(detail::is_nothrow_comparable<T>())
{
    return !(r1 == r2);
}

/// An iterator over the `Basic_position<T>`s of a `Basic_rectangle<T>`.
///
/// Iterates in column-major order.
template <class T>
class Basic_rectangle<T>::iterator
        : public std::iterator<
                std::input_iterator_tag,
                typename Basic_rectangle<T>::Position const >
{
public:

    /// Returns the current `Position` of this iterator.
    Position operator*() const
    {
        return current_;
    }

    /// Returns a pointer to the current `Position` of this iterator.
    Position const* operator->() const
    {
        return &current_;
    }

    /// Pre-increments, advancing this iterator to the next `Position`.
    iterator& operator++()
    {
        if (++current_.y >= y_end_) {
            ++current_.x;
            current_.y = y_begin_;
        }

        return *this;
    }

    /// Pre-decrements, retreating this iterator to the previous `Position`.
    iterator& operator--()
    {
        if (current_.y == y_begin_) {
            current_.y = y_end_;
            --current_.x;
        }

        --current_.y;

        return *this;
    }

    /// Post-increments, advancing this iterator to the next `Position`.
    iterator operator++(int)
    {
        iterator result(*this);
        ++*this;
        return result;
    }

    /// Post-decrements, retreating this iterator to the previous `Position`.
    iterator operator--(int)
    {
        iterator result(*this);
        --*this;
        return result;
    }

    /// Compares whether two iterators are equal. Considers only the current
    /// position, not the bounds of the stripe we're iterating through.
    bool operator==(iterator const& that) const
    {
        return **this == *that;
    }

    /// Iterator inequality.
    bool operator!=(iterator const& that) const
    {
        return !(*this == that);
    }

private:
    friend Basic_rectangle<T>;

    iterator(Position current, Coordinate y_begin, Coordinate y_end)
            : current_(current)
            , y_begin_(y_begin)
            , y_end_(y_end)
    { }

    Position    current_;
    Coordinate  y_begin_;
    Coordinate  y_end_;
};

/// A rendering transform, which can scale, flip, and rotate. A Transform
/// can be given to
/// Sprite_set::add_sprite(const Sprite&, Position, int, const Transform&)
/// to specify how a sprites::Sprite should be rendered.
///
/// To construct a transform that does just one thing, you can use one of
/// the static factory functions:
///
///   - Transform::rotation(double)
///   - Transform::flip_h()
///   - Transform::flip_v()
///   - Transform::scale(double)
///   - Transform::scale_x(double)
///   - Transform::scale_y(double)
///
/// It is also possible to modify a transform with the setter functions
/// such as set_rotation(double) and set_scale(double). This can be used
/// to configure a transform that does more than one thing:
///
/// ```
/// ge211::Transform my_transform =
///     ge211::Transform{}
///         .set_flip_h(true)
///         .set_flip_v(true)
///         .scale_x(2);
/// ```
///
class Transform
{
public:
    /// \name Constructor and factory functions
    /// @{

    /// Constructs the identity transform, which has no effect.
    Transform() NOEXCEPT;

    /// Constructs a rotating transform, given the rotation in degrees
    /// clockwise.
    static Transform rotation(double) NOEXCEPT;

    /// Constructs a transform that flips the sprite horizontally.
    static Transform flip_h() NOEXCEPT;

    /// Constructs a transform that flips the sprite vertically.
    static Transform flip_v() NOEXCEPT;

    /// Constructs a transform that scales the sprite in both dimensions.
    static Transform scale(double) NOEXCEPT;

    /// Constructs a transform that scales the sprite in the *x* dimension.
    static Transform scale_x(double) NOEXCEPT;

    /// Constructs a transform that scales the sprite in the *y* dimension.
    static Transform scale_y(double) NOEXCEPT;

    /// @}

    /// \name Setters
    /// @{

    /// Modifies this transform to have the given rotation, in degrees.
    Transform& set_rotation(double) NOEXCEPT;
    /// Modifies this transform to determine whether to flip horizontally.
    Transform& set_flip_h(bool) NOEXCEPT;
    /// Modifies this transform to determine whether to flip vertically.
    Transform& set_flip_v(bool) NOEXCEPT;
    /// Modifies this transform to scale the sprite by the given amount in
    /// both dimensions. This overwrites the effect of previous calls to
    /// set_scale_x(double) and set_scale_y(double).
    Transform& set_scale(double) NOEXCEPT;
    /// Modifies this transform to scale the sprite horizontally. This
    /// overwrites the effect of previous calls to `set_scale(double)`
    /// as well as itself.
    Transform& set_scale_x(double) NOEXCEPT;
    /// Modifies this transform to scale the sprite vertically. This
    /// overwrites the effect of previous calls to `set_scale(double)`
    /// as well as itself.
    Transform& set_scale_y(double) NOEXCEPT;

    /// @}

    /// \name Getters
    /// @{

    /// Returns the rotation that will be applied to the sprite.
    double get_rotation() const NOEXCEPT;
    /// Returns whether the sprite will be flipped horizontally.
    bool get_flip_h() const NOEXCEPT;
    /// Returns whether the sprite will be flipped vertically.
    bool get_flip_v() const NOEXCEPT;
    /// Returns how much the sprite will be scaled horizontally.
    double get_scale_x() const NOEXCEPT;
    /// Returns how much the sprite will be scaled vertically.
    double get_scale_y() const NOEXCEPT;

    /// @}

    /// \name Combining transforms
    /// @{

    /// Is this transformation the identity transformation that does nothing?
    /// Because floating point is approximate, this may answer `false` for
    /// transforms that are nearly the identity. But it should answer `true`
    /// for any transform constructed by the default constructor Transform().
    bool is_identity() const NOEXCEPT;

    /// Composes two transforms to combine both of their effects.
    Transform operator*(const Transform&) const NOEXCEPT;

    /// Returns the inverse of this transform. Composing a transform with its
    /// inverse should result in the identity transformation, though because
    /// floating point is approximate, is_identity() const may not actually
    /// answer `true`.
    Transform inverse() const NOEXCEPT;

    /// @}

private:
    double rotation_;
    double scale_x_;
    double scale_y_;
    bool flip_h_;
    bool flip_v_;
};

/// Equality for `Transform`s.
bool operator==(const Transform&, const Transform&) NOEXCEPT;
/// Disequality for `Transform`s.
bool operator!=(const Transform&, const Transform&) NOEXCEPT;

/// Gets implicitly converted to `Basic_position<T>(0, 0)`
/// for any coordinate type `T`.
///
/// Examples:
///
/// ```
/// ge211::Basic_position<float> p0 = the_origin;
/// ```
///
/// ```
/// return ge211::Rectangle::from_top_left(the_origin, {w, h});
/// ```
//;
constexpr Origin_type the_origin;

/// Constructs a Basic_dimensions given the width and height.
///
/// Unlike the constructor, this function can infer the coordinate type
/// from its arguments. For example:
///
/// ```
/// auto near = make_dimensions(5, 7);   // Basic_dimensions<int>
/// auto far  = make_dimensions(5, 7e9); // Basic_dimensions<double>
/// ```
template <class T>
Basic_dimensions<T> make_dimensions(T x, T y)
{
    return {x, y};
}

/// Constructs a Basic_position given the `x` and `y` coordinates.
///
/// Unlike the constructor, this function can infer the coordinate type
/// from its arguments. For example:
///
/// ```
/// auto here  = make_position( 0, 0);    // Basic_position<int>
/// auto there = make_position(-5, 7e9);  // Basic_position<double>
/// ```
template <class T>
Basic_position<T> make_position(T x, T y)
{
    return {x, y};
}


/// Constructs a Basic_rectangle given its member variables.
///
/// It takes them in the same order as the constructor: the `x` and
/// `y` coordinates followed by the width and height.
///
/// Unlike the constructor, this function can infer the coordinate type
/// from its arguments. For example:
///
/// ```
/// // infers Basic_rectangle<double>:
/// auto unit_square = make_rectangle(0., 0., 1., 1.);
///
/// // infers Basic_rectangle<int>:
/// auto big_reversi = make_rectangle(0, 0, 16, 16);
/// ```
template <class T>
Basic_rectangle<T> make_rectangle(T x, T y, T width, T height)
{
    return {x, y, width, height};
}

} // end namespace geometry.

} // end namespace ge211

// specializations in std:
namespace std
{

/// Template specialization to define hashing of
/// @ref ge211::geometry::Basic_position,
/// which allows storing them in a @ref std::unordered_set, or using
/// them as keys in a @ref std::unordered_map.
template <class T>
struct hash<ge211::Basic_position<T>>
{
    /// Hashes a Basic_position<T>, provided that T is hashable.
    std::size_t operator()(ge211::Basic_position<T> pos) const NOEXCEPT
    {
        return hash_t_(pos.x) * 31 ^ hash_t_(pos.y);
    }

private:
    std::hash<T> hash_t_;
};

} // end namespace std
