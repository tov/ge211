#pragma once

#include "ge211_forward.hxx"
#include "ge211_noexcept.hxx"
#include "ge211_util.hxx"

#include <SDL_rect.h>

#include <type_traits>
#include <utility>

namespace ge211
{

/// Geometric objects and their operations.
namespace geometry
{

class Origin_type;

template <class T>
struct Geometry
{
    using Coordinate = T;
    struct Dims;
    struct Posn;
    struct Rect;
};

template <class T>
using Dims = typename Geometry<T>::Dims;

template <class T>
using Posn = typename Geometry<T>::Posn;

template <class T>
using Rect = typename Geometry<T>::Rect;

/// The type of the special value @ref the_origin.
///
/// This type exists only so that we can overload the
/// Posn constructor to construct the origin. See
/// @ref the_origin for examples.
class Origin_type
{
};

/// Represents the dimensions of an object, or more generally,
/// the displacement between two Posn%s. Note that
/// much of the library uses @ref geometry::Dims, which is a
/// type alias for `Dims<int>`.
template<class T>
struct Geometry<T>::Dims
{
    /// The coordinate type for the dimensions. This is an alias of
    /// geometry type parameter `T`.
    using Coordinate = Geometry::Coordinate;

    /// The position type corresponding to this type. This is an
    /// alias of @ref Geometry::Posn.
    using Posn = Geometry::Posn;

    /// The rectangle type corresponding to this type. This is an
    /// alias of @ref Geometry::Rect.
    using Rect = Geometry::Rect;

    Coordinate width;  ///< The width of the object.
    Coordinate height; ///< The height of the object.

    /// Equality for Dims.
    bool operator==(Dims that) const
    {
        return width == that.width && height == that.height;
    }

    /// Disequality for Dims.
    bool operator!=(Dims that) const
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma ide diagnostic ignored "Simplify"
        return !(*this == that);
#pragma clang diagnostic pop
    }

    /// Adds two Dims%es. This is vector addition.
    Dims operator+(Dims that) const
    {
        return {T(width + that.width), T(height + that.height)};
    }

    /// Subtracts two Dims%es. This is vector subtraction.
    Dims operator-(Dims that) const
    {
        return {T(width - that.width), T(height - that.height)};
    }

    /// Multiplies a Dims by a scalar.
    template<class U = T>
    Dims operator*(U scalar) const
    {
        return {T(width * scalar), T(height * scalar)};
    }

    /// Divides a Dims by a scalar.
    template <class U = T>
    Dims operator/(U scalar) const
    {
        return {T(width / scalar), T(height / scalar)};
    }

/// Succinct Dims addition.
    Dims& operator+=(Dims that)
    {
        return *this = *this + that;
    }

    /// Succinct Dims subtraction.
    Dims& operator-=(Dims that)
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
    {
        return *this = *this - that;
    }

    /// Succinct [Dims][1]-scalar multiplication.
    ///
    /// [1]: @ref Dims
    template <class U>
    Dims& operator*=(U scalar)
    {
        return *this = *this * scalar;
    }

    /// Succinct [Dims][1]-scalar division.
    ///
    /// \preconditions
    ///  - `scalar != 0` (when `U` is an integer type)
    ///
    /// [1]: @ref Dims
    template <class U>
    Dims& operator/=(U scalar)
    {
        return *this = *this / scalar;
    }

    /// Alias for @ref Dims::Dims to support explicit conversions
    /// between coordinate types.
    ///
    /// For example:
    ///
    /// ```
    /// auto d1 = ge211::Dims<int>{3, 4};
    /// auto d2 = d1.into<double>();
    /// ```
    template <class U>
    geometry::Dims<U>
    into() const
    {
        return geometry::Dims<U>(U(width), U(height));
    }
};

/// Multiplies a scalar by a Dims.
template<
        class T,
        class U,
        std::enable_if_t<std::is_arithmetic<U>::value, void>
        >
Dims<T> operator*(U scalar, Dims<T> dims)
{
    return dims * scalar;
}


/// A position in the T-valued Cartesian plane. In graphics,
/// the origin is traditionally in the upper left, so the *x* coordinate
/// increases to the right and the *y* coordinate increases downward.
/// Note that much of the library uses geometry::Position, which is a
/// type alias for Posn<int>.
template<class T>
struct Geometry<T>::Posn
{
    /// The coordinate type for the position. This is an alias of
    /// type parameter `T`.
    using Coordinate = Geometry::Coordinate;

    /// The dimensions type corresponding to this type. This is an
    /// alias of @ref Geometry::Dims.
    using Dims = Geometry::Dims;

    /// The rectangle type corresponding to this type. This is an
    /// alias of @ref Geometry::Rect.
    using Rect = Geometry::Rect;

    Coordinate x; ///< The *x* coordinate
    Coordinate y; ///< The *y* coordiante

    /// \name Constructors and Conversions
    /// @{

    /// Constructs a position from the given *x* and *y* coordinates.
    Posn(Coordinate x, Coordinate y)
    NOEXCEPT_(detail::is_nothrow_convertible<T>())
            : x{x}, y{y}
    { }

    /// Constructs the origin when given @ref the_origin.
    Posn(Origin_type)
    NOEXCEPT_(detail::is_nothrow_convertible<int, T>())
            : Posn(0, 0)
    { }

    /// Constructs a position from a Dims, which gives the
    /// displacement of the position from the origin.
    explicit Posn(Dims dims)
    NOEXCEPT_(detail::is_nothrow_convertible<T>())
            : Posn{dims.width, dims.height}
    { }

    /// Constructs a @ref Posn from a Posn of another coordinate type.
    /// For example:
    ///
    /// ```
    /// ge211::Posn<int> p1 { 3, 4 };
    /// auto p2 = ge211::Posn<double>(p1);
    /// ```
    template <class U>
    explicit Posn(const U& that)
            : x(that.x)
            , y(that.y)
    { }

    /// Converts a Posn to another coordinate type.
    /// For example:
    ///
    /// ```
    /// auto p1 = ge211::Posn<int>{3, 4};
    /// auto p2 = p1.into<double>();
    /// ```
    template<class U>
    geometry::Posn<U>
    into() const
    NOEXCEPT_(detail::is_nothrow_convertible<T, U>())
    {
        return {U(x), U(y)};
    }

    /// @}

    /// \name Operators
    /// @{

    /// Equality for positions.
    bool operator==(Posn that) const
    NOEXCEPT_(detail::is_nothrow_comparable<T>())
    {
        return x == that.x && y == that.y;
    }

    /// Disequality for positions.
    bool operator!=(Posn p2) const
    NOEXCEPT_(detail::is_nothrow_comparable<T>())
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma ide diagnostic ignored "Simplify"
        return !(*this == p2);
#pragma clang diagnostic pop
    }

    /// Translates a position by some displacement. This is the same as
    /// @ref Posn::down_right_by(Dims) const.
    Posn operator+(Dims dims) const
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
    {
        return down_right_by(dims);
    }

    /// Translates a position by the opposite of some displacement. This is
    /// the same as @ref Posn::up_left_by(Dims) const.
    Posn operator-(Dims dims) const
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
    {
        return up_left_by(dims);
    }

    /// Subtracts two Posn%s, yields a Dims.
    Dims operator-(Posn that) const
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
    {
        return {x - that.x, y - that.y};
    }

    /// Succinct position translation.
    Posn& operator+=(Dims dims)
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
    {
        return *this = *this + dims;
    }

    /// Succinct position translation.
    Posn& operator-=(Dims dims)
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
    {
        return *this = *this - dims;
    }

    /// @}

    /// \name Shifting member functions
    /// @{

    /// Constructs the position that is above this position by the given
    /// amount.
    Posn up_by(Coordinate dy) const
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
    {
        return {x, y - dy};
    }

    /// Constructs the position that is below this position by the given
    /// amount.
    Posn down_by(Coordinate dy) const
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
    {
        return {x, y + dy};
    }

    /// Constructs the position that is to the left of this position by
    /// the given amount.
    Posn left_by(Coordinate dx) const
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
    {
        return {x - dx, y};
    }

    /// Constructs the position that is to the right of this position by
    /// the given amount.
    Posn right_by(Coordinate dx) const
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
    {
        return {x + dx, y};
    }

    /// Constructs the position that is above and left of this position
    /// by the given dimensions.
    Posn up_left_by(Dims dims) const
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
    {
        return {x - dims.width, y - dims.height};
    }

    /// Constructs the position that is above and right of this position
    /// by the given dimensions.
    Posn up_right_by(Dims dims) const
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
    {
        return {x + dims.width, y - dims.height};
    }

    /// Constructs the position that is below and left of this position
    /// by the given dimensions.
    Posn down_left_by(Dims dims) const
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
    {
        return {x - dims.width, y + dims.height};
    }

    /// Constructs the position that is below and right of this position
    /// by the given dimensions.
    Posn down_right_by(Dims dims) const
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
    {
        return {x + dims.width, y + dims.height};
    }

    /// @}
};


/// Represents a positioned rectangle.
template<class T>
struct Geometry<T>::Rect
{
    /// The coordinate type for the rectangle. This is an alias of
    /// type parameter `T`.
    using Coordinate = Geometry::Coordinate;

    /// The dimensions type corresponding to this type. This is an
    /// alias of @ref Geometry::Dims.
    using Dims = Geometry::Dims;

    /// The position type corresponding to this type. This is an
    /// alias of @ref Geometry::Posn.
    using Posn = Geometry::Posn;

    Coordinate x;         ///< The *x* coordinate of the upper-left vertex.
    Coordinate y;         ///< The *y* coordinate of the upper-left vertex.
    Coordinate width;     ///< The width of the rectangle in pixels.
    Coordinate height;    ///< The height of the rectangle in pixels.

    /// \name Conversions
    /// @{

    /// Converts a Rect to another coordinate type.
    ///
    /// For example:
    ///
    /// ```
    /// auto r1 = ge211::Rect<int>{-1, -1, 2, 2};
    /// auto r2 = r1.into<double>();
    /// ```
    template<typename U>
    geometry::Rect<U>
    into() const
    NOEXCEPT_(detail::is_nothrow_convertible<T, U>())
    {
        return {U(x), U(y), U(width), U(height)};
    }

    /// Alias for Rect::into to support casting between
    /// coordinate types.
    ///
    /// For example:
    ///
    /// ```
    /// ge211::Rect<int>    r1{-1, -1, 2, 2};
    /// ge211::Rect<double> r2(p1);
    /// ```
    template <class U>
    explicit operator
    geometry::Rect<U>() const
    NOEXCEPT_(noexcept(into<U>()))
    {
        return into<U>();
    }

    /// @}

    /// \name Operators
    /// @{

    /// Equality for rectangles. Note that this is naïve, in that it considers
    /// empty rectangles with different positions to be different.
    bool operator==(const Rect& that) const
    NOEXCEPT_(detail::is_nothrow_comparable<T>())
    {
        return x == that.x &&
               y == that.y &&
               width == that.width &&
               height == that.height;
    }

    /// Disequality for rectangles.
    bool operator!=(const Rect &that) const
    NOEXCEPT_(detail::is_nothrow_comparable<T>())
    {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"
#pragma ide diagnostic ignored "Simplify"
        return !(*this == that);
#pragma clang diagnostic pop
    }

    /// @}

    /// \name Dimensions and positions
    /// @{

    /// The dimensions of the rectangle. Equivalent to
    /// `Dims<T>{rect.width, rect.height}`.
    Dims dimensions() const
    NOEXCEPT_(detail::is_nothrow_convertible<T>())
    {
        return {width, height};
    }

    /// The position of the top left vertex.
    Posn top_left() const
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
    {
        return {x, y};
    }

    /// The position of the top right vertex.
    Posn top_right() const
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
    {
        return top_left().right_by(width);
    }

    /// The position of the bottom left vertex.
    Posn bottom_left() const
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
    {
        return top_left().down_by(height);
    }

    /// The position of the bottom right vertex.
    Posn bottom_right() const
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
    {
        return top_left().down_right_by(dimensions());
    }

    /// The position of the center of the rectangle.
    Posn center() const
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>() &&
              detail::has_nothrow_division<T>())
    {
        return top_left().down_right_by(dimensions() / Coordinate(2));
    }

    /// @}

    /// \name Static factory functions
    /// @{

    /// Creates a Rect given the position of its top left vertex
    /// and its dimensions.
    static Rect from_top_left(Posn tl, Dims dims)
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
    {
        return {tl.x, tl.y, dims.width, dims.height};
    }

    /// Creates a Rect given the position of its top right vertex
    /// and its dimensions.
    static Rect from_top_right(Posn tr, Dims dims)
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
    {
        return from_top_left(tr.left_by(dims.width), dims);
    }

    /// Creates a Rect given the position of its bottom left vertex
    /// and its dimensions.
    static Rect from_bottom_left(Posn bl, Dims dims)
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
    {
        return from_top_left(bl.up_by(dims.height), dims);
    }

    /// Creates a Rect given the position of its bottom right vertex
    /// and its dimensions.
    static Rect from_bottom_right(Posn br, Dims dims)
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
    {
        return from_top_left(br.up_left_by(dims), dims);
    }

    /// Creates a Rect given the position of its center
    /// and its dimensions.
    static Rect from_center(Posn center, Dims dims)
    NOEXCEPT_(detail::has_nothrow_arithmetic<T>())
    {
        return from_top_left(center.up_left_by(dims / Coordinate(2)), dims);
    }

    /// @}

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
    NOEXCEPT_(detail::is_nothrow_convertible<T, int>())
    {
        SDL_Rect result;
        result.x = static_cast<int>(x);
        result.y = static_cast<int>(y);
        result.w = static_cast<int>(width);
        result.h = static_cast<int>(height);
        return result;
    }
};

/// An iterator over the `Posn<T>`s of a `Rect<T>`.
///
/// Iterates in column-major order.
template<class T>
class Geometry<T>::Rect::iterator
        : public std::iterator<
                std::input_iterator_tag,
                Posn const>
{
public:

    /// Returns the current `Position` of this iterator.
    Posn operator*() const
    {
        return current_;
    }

    /// Returns a pointer to the current `Position` of this iterator.
    Posn const* operator->() const
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
    friend Rect;

    iterator(Posn current, Coordinate y_begin, Coordinate y_end)
            : current_(current),
              y_begin_(y_begin),
              y_end_(y_end)
    { }

    Posn       current_;
    Coordinate y_begin_;
    Coordinate y_end_;
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
    bool   flip_h_;
    bool   flip_v_;
};

/// Equality for `Transform`s.
bool operator==(const Transform&, const Transform&) NOEXCEPT;

/// Disequality for `Transform`s.
bool operator!=(const Transform&, const Transform&) NOEXCEPT;

/// Gets implicitly converted to `Posn<T>(0, 0)`
/// for any coordinate type `T`.
///
/// Examples:
///
/// ```
/// ge211::Posn<float> p0 = the_origin;
/// ```
///
/// ```
/// return ge211::Rectangle::from_top_left(the_origin, {w, h});
/// ```
//;
constexpr Origin_type the_origin;

/// Constructs a Dims given the width and height.
///
/// Unlike the constructor, this function can infer the coordinate type
/// from its arguments. For example:
///
/// ```
/// auto near = make_dims(5, 7);   // Dims<int>
/// auto far  = make_dims(5, 7e9); // Dims<double>
/// ```
template <class T>
Dims<T> make_dims(T x, T y)
{
    return {x, y};
}

/// Constructs a Posn given the `x` and `y` coordinates.
///
/// Unlike the constructor, this function can infer the coordinate type
/// from its arguments. For example:
///
/// ```
/// auto here  = make_posn( 0, 0);    // Posn<int>
/// auto there = make_posn(-5, 7e9);  // Posn<double>
/// ```
template <class T>
Posn<T> make_posn(T x, T y)
{
    return {x, y};
}


/// Constructs a Rect given its member variables.
///
/// It takes them in the same order as the constructor: the `x` and
/// `y` coordinates followed by the width and height.
///
/// Unlike the constructor, this function can infer the coordinate type
/// from its arguments. For example:
///
/// ```
/// // infers Rect<double>:
/// auto unit_square = make_rect(0., 0., 1., 1.);
///
/// // infers Rect<int>:
/// auto big_reversi = make_rect(0, 0, 16, 16);
/// ```
template <class T>
Rect<T> make_rect(T x, T y, T width, T height)
{
    return {x, y, width, height};
}

} // end namespace geometry.

using namespace geometry;

} // end namespace ge211

// specializations in std:
namespace std
{

/// Template specialization to define hashing of
/// @ref ge211::geometry::Posn,
/// which allows storing them in a @ref std::unordered_set, or using
/// them as keys in a @ref std::unordered_map.
/* XXX TODO */
/*
template <class T>
struct hash<ge211::geometry::Posn<T>>
{
    /// Hashes a Posn<T>, provided that T is hashable.
    std::size_t operator()(ge211::geometry::Posn<T> pos) const
    NOEXCEPT
    {
        return hash_t_(pos.x) * 31 ^ hash_t_(pos.y);
    }

private:
    std::hash<T> hash_t_;
};
 */

} // end namespace std

