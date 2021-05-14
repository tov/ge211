#pragma once

#include "ge211_forward.hxx"
#include "ge211_if_cpp.hxx"
#include "ge211_noexcept.hxx"
#include "ge211_type_traits.hxx"
#include "ge211_util.hxx"

#include <SDL_rect.h>

#include <ostream>
#include <type_traits>
#include <utility>

namespace ge211 {

/// Geometric objects and their operations.
namespace geometry {

/// The type of the special value @ref the_origin.
///
/// This type exists only so that we can overload the
/// Posn constructor to construct the origin. See
/// @ref the_origin for examples.
class origin_type
{
};


/// Represents the dimensions of an object, or more generally,
/// the displacement between two Posn%s. The coordinate type
/// `COORDINATE` may be any arithmetic type.
template <typename COORDINATE>
struct Dims
{
    /// The coordinate type for the dimensions. This is an alias of
    /// geometry type parameter `COORDINATE`.
    using Coordinate = COORDINATE;

    Coordinate width;  ///< The width of the object.
    Coordinate height; ///< The height of the object.

    /// \name Constructors and Conversions
    /// @{

    /// Constructs a dimensions from the given *width* and *height*.
    Dims(Coordinate width, Coordinate height)
            : width{width},
              height{height}
    { }

    /// Default-constructs the zero-sized Dims.
    Dims()
            : Dims{Coordinate{}, Coordinate{}}
    { }

    /// Casts or converts a @ref Dims to a Dims of a different coordinate type.
    /// For example:
    ///
    /// ```
    /// ge211::Dims<int> p1 { 3, 4 };
    /// auto p2 = ge211::Dims<double>(p1);
    /// ```
    template <typename FROM_COORD>
    explicit Dims(const Dims<FROM_COORD> &that)
            : width(that.width),
              height(that.height)
    { }

    /// Explicitly converts a Dims to a different coordinate type.
    ///
    /// For example:
    ///
    /// ```
    /// auto d1 = ge211::Dims<int>{3, 4};
    /// auto d2 = d1.into<double>();
    /// ```
    template <typename TO_COORD>
    ge211::Dims<TO_COORD>
    into() const
    {
        return {TO_COORD(width), TO_COORD(height)};
    }

    /// @}

    /// \name Operators
    /// @{

    /// Equality for Dims.
    bool operator==(Dims that) const
    {
        return width == that.width && height == that.height;
    }

    /// Disequality for Dims.
    bool operator!=(Dims that) const
    {
        return !(*this == that);
    }

    /// Less-than-or-equal for Dims. Determines whether `*this` Dims
    /// fits inside `that` Dims.
    bool operator<=(Dims that) const
    {
        return width <= that.width && height <= that.height;
    }

    /// Greater-than-or-equal for Dims. Determines whether `that` Dims
    /// fits inside `*this` Dims.
    bool operator>=(Dims that) const
    {
        return that <= *this;
    }

    /// Less-than for Dims. True when `*this` Dims fits inside `that` Dims
    /// but they aren't equal.
    bool operator<(Dims that) const
    {
        return *this <= that && *this != that;
    }

    /// Greater-than for Dims. True when `that` Dims fits inside `*this` Dims
    /// but they aren't equal.
    bool operator>(Dims that) const
    {
        return that < *this;
    }

    /// Adds two Dims%es. This is vector addition.
    Dims operator+(Dims that) const
    {
        return {Coordinate(width + that.width),
                Coordinate(height + that.height)};
    }

    /// Subtracts two Dims%es. This is vector subtraction.
    Dims operator-(Dims that) const
    {
        return {Coordinate(width - that.width),
                Coordinate(height - that.height)};
    }

    /// Multiplies two Dims%es. This scales them by multiplying the widths and
    /// heights, pointwise.
    ///
    /// # Example
    ///
    /// ```
    /// ge211::Dims<double> cell_size {50, 25};
    /// ge211::Dims<int>    repetitions {3, 2};
    ///
    /// ge211::Dims<double> actual_result = cell_size * repetitions;
    /// ge211::Dims<double> expected_result {150, 50};
    ///
    /// CHECK( actual_result == expected_result );
    /// ```
    template <
            typename OTHER_COORD,
            typename RESULT_COORD = Multiply_Result<Coordinate, OTHER_COORD>
    >
    Dims<RESULT_COORD>
    operator*(Dims<OTHER_COORD> that) const
    {
        return {RESULT_COORD(width * that.width),
                RESULT_COORD(height * that.height)};
    }

    /// Multiplies a Dims by a scalar.
    template <
            typename ARITHMETIC_TYPE,
            typename = std::enable_if_t<Is_Arithmetic<ARITHMETIC_TYPE>>
    >
    Dims operator*(ARITHMETIC_TYPE scalar) const
    {
        return {Coordinate(width * scalar), Coordinate(height * scalar)};
    }

    /// Divides a Dims by a scalar.
    template <
            typename ARITHMETIC_TYPE,
            typename = std::enable_if_t<Is_Arithmetic<ARITHMETIC_TYPE>>
    >
    Dims operator/(ARITHMETIC_TYPE scalar) const
    {
        return {Coordinate(width / scalar), Coordinate(height / scalar)};
    }

    // Negates a Dims. (Multiplies it by -1.)
    Dims operator-() const
    {
        return {-width, -height};
    }

    /// Succinct Dims addition.
    Dims &operator+=(Dims that)
    {
        return *this = *this + that;
    }

    /// Succinct Dims subtraction.
    Dims &operator-=(Dims that)
    {
        return *this = *this - that;
    }

    /// Succinct Dims–Dims multiplication. Scales `*this` by `that`.
    template <
            typename OTHER_COORD
    >
    Dims &operator*=(Dims<OTHER_COORD> that)
    {
        return *this = *this * that;
    }

    /// Succinct [Dims][1]-scalar multiplication.
    ///
    /// [1]: @ref Dims
    template <
            typename ARITHMETIC_TYPE,
            typename = std::enable_if_t<Is_Arithmetic<ARITHMETIC_TYPE>>
    >
    Dims &operator*=(ARITHMETIC_TYPE scalar)
    {
        return *this = *this * scalar;
    }

    /// Succinct [Dims][1]-scalar division.
    ///
    /// \preconditions
    ///  - `scalar != 0` (when `ARITHMETIC_TYPE` is an integer type)
    ///
    /// [1]: @ref Dims
    template <
            typename ARITHMETIC_TYPE,
            typename = std::enable_if_t<Is_Arithmetic<ARITHMETIC_TYPE>>
    >
    Dims &operator/=(ARITHMETIC_TYPE scalar)
    {
        return *this = *this / scalar;
    }

    /// @}
};


/// Multiplies a scalar and a Dims. (This is scalar-vector
/// multiplication.)
template <
        typename COORDINATE,
        typename SCALAR,
        typename = std::enable_if_t<Is_Arithmetic<SCALAR>>
>
Dims<COORDINATE>
operator*(SCALAR scalar, Dims<COORDINATE> dims)
{
    return dims * scalar;
}

/// A position in the `COORDINATE`-valued Cartesian plane, where `COORDINATE`
/// can be any arithmetic type. In graphics, the origin is traditionally in
/// the upper left, so the *x* coordinate increases to the right and the *y*
/// coordinate increases downward.
template <typename COORDINATE>
struct Posn
{
    /// The coordinate type for the position. This is an alias of
    /// type parameter `COORDINATE`.
    using Coordinate = COORDINATE;

    /// The dimensions type corresponding to this type. This is an
    /// alias of @ref ge211::geometry::Dims.
    using Dims_type = Dims<COORDINATE>;

    Coordinate x; ///< The *x* coordinate
    Coordinate y; ///< The *y* coordiante

    /// \name Constructors and Conversions
    /// @{

    /// Constructs a position from the given *x* and *y* coordinates.
    Posn(Coordinate x, Coordinate y)
    NOEXCEPT_(detail::is_nothrow_convertible<Coordinate>())
            : x{x},
              y{y}
    { }

    /// Constructs the origin when given @ref the_origin.
    Posn(origin_type)
    NOEXCEPT_(noexcept(Coordinate{}))
            : Posn(Coordinate{}, Coordinate{})
    { }

    /// Constructs a position from a Dims, which gives the
    /// displacement of the position from the origin.
    explicit Posn(Dims_type dims)
    NOEXCEPT_(detail::is_nothrow_convertible<Coordinate>())
            : Posn{dims.width, dims.height}
    { }

    /// Casts or converts a @ref Posn to a Posn of a different coordinate type.
    /// For example:
    ///
    /// ```
    /// ge211::Posn<int> p1 { 3, 4 };
    /// auto p2 = ge211::Posn<double>(p1);
    /// ```
    template <typename FROM_COORD>
    explicit Posn(const Posn<FROM_COORD> &that)
            : x(that.x),
              y(that.y)
    { }

    /// Explicitly converts a Posn to another coordinate type.
    ///
    /// For example:
    ///
    /// ```
    /// auto p1 = ge211::Posn<int>{3, 4};
    /// auto p2 = p1.into<double>();
    /// ```
    template <typename TO_COORD>
    ge211::Posn<TO_COORD>
    into() const
    {
        return {TO_COORD(x), TO_COORD(y)};
    }

    /// @}

    /// \name Operators
    /// @{

    /// Equality for positions.
    bool operator==(Posn that) const
    NOEXCEPT_(detail::is_nothrow_comparable<Coordinate>())
    {
        return x == that.x && y == that.y;
    }

    /// Disequality for positions.
    bool operator!=(Posn p2) const
    NOEXCEPT_(detail::is_nothrow_comparable<Coordinate>())
    {
        return !(*this == p2);
    }

    /// Translates a position by some displacement. This is the same as
    /// @ref Posn::down_right_by(Dims_type) const.
    Posn operator+(Dims_type dims) const
    NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>())
    {
        return down_right_by(dims);
    }

    /// Translates a position by the opposite of some displacement. This is
    /// the same as @ref Posn::up_left_by(Dims_type) const.
    Posn operator-(Dims_type dims) const
    NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>())
    {
        return up_left_by(dims);
    }

    /// Subtracts two Posn%s, yields a Dims.
    Dims_type operator-(Posn that) const
    NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>())
    {
        return {x - that.x, y - that.y};
    }

    /// Succinct position translation.
    Posn &operator+=(Dims_type dims)
    NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>())
    {
        return *this = *this + dims;
    }

    /// Succinct position translation.
    Posn &operator-=(Dims_type dims)
    NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>())
    {
        return *this = *this - dims;
    }

    /// @}

    /// \name Shifting member functions
    /// @{

    /// Constructs the position that is above this position by the given
    /// amount.
    Posn up_by(Coordinate dy) const
    NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>())
    {
        return {x, y - dy};
    }

    /// Constructs the position that is below this position by the given
    /// amount.
    Posn down_by(Coordinate dy) const
    NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>())
    {
        return {x, y + dy};
    }

    /// Constructs the position that is to the left of this position by
    /// the given amount.
    Posn left_by(Coordinate dx) const
    NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>())
    {
        return {x - dx, y};
    }

    /// Constructs the position that is to the right of this position by
    /// the given amount.
    Posn right_by(Coordinate dx) const
    NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>())
    {
        return {x + dx, y};
    }

    /// Constructs the position that is above and left of this position
    /// by the given dimensions.
    Posn up_left_by(Dims_type dims) const
    NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>())
    {
        return {x - dims.width, y - dims.height};
    }

    /// Constructs the position that is above and right of this position
    /// by the given dimensions.
    Posn up_right_by(Dims_type dims) const
    NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>())
    {
        return {x + dims.width, y - dims.height};
    }

    /// Constructs the position that is below and left of this position
    /// by the given dimensions.
    Posn down_left_by(Dims_type dims) const
    NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>())
    {
        return {x - dims.width, y + dims.height};
    }

    /// Constructs the position that is below and right of this position
    /// by the given dimensions.
    Posn down_right_by(Dims_type dims) const
    NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>())
    {
        return {x + dims.width, y + dims.height};
    }

    /// @}
};


/// Represents a positioned rectangle.
template <typename COORDINATE>
struct Rect
{
    /// The coordinate type for the rectangle. This is an alias of
    /// type parameter `COORDINATE`.
    using Coordinate = COORDINATE;

    /// The dimensions type corresponding to this type. This is an
    /// alias of @ref ge211::geometry::Dims.
    using Dims_type = Dims<Coordinate>;

    /// The position type corresponding to this type. This is an
    /// alias of @ref ge211::geometry::Posn.
    using Posn_type = Posn<Coordinate>;

    Coordinate x;         ///< The *x* coordinate of the upper-left vertex.
    Coordinate y;         ///< The *y* coordinate of the upper-left vertex.
    Coordinate width;     ///< The width of the rectangle in pixels.
    Coordinate height;    ///< The height of the rectangle in pixels.

    /// \name Construction and Conversion
    /// @{

    /// Constructs a rectangle given the *x* and *y* coordinates of its
    /// top left corner, and its width and height.
    Rect(Coordinate x, Coordinate y, Coordinate width, Coordinate height)
            : x{x},
              y{y},
              width{width},
              height{height}
    { }

    /// Default-constructs the zero-sized Rect at the origin.
    Rect()
            : Rect{Coordinate{}, Coordinate{}, Coordinate{}, Coordinate{}}
    { }

    /// Casts or converts a @ref Rect to a Rect of a different coordinate type.
    /// For example:
    ///
    /// ```
    /// ge211::Posn<int> p1 { 3, 4 };
    /// auto p2 = ge211::Posn<double>(p1);
    /// ```
    template <typename FROM_COORD>
    explicit Rect(const Rect<FROM_COORD> &that)
            : x(that.x),
              y(that.y),
              width(that.width),
              height(that.height)
    { }

    /// Explicitly converts a Rect to another coordinate type.
    ///
    /// For example:
    ///
    /// ```
    /// auto r1 = ge211::Rect<int>{-1, -1, 2, 2};
    /// auto r2 = r1.into<double>();
    /// ```
    template <typename TO_COORD>
    ge211::Rect<TO_COORD>
    into() const
    {
        return {TO_COORD(x), TO_COORD(y), TO_COORD(width), TO_COORD(height)};
    }

    /// @}

    /// \name Operators
    /// @{

    /// Equality for rectangles. Note that this is naïve, in that it considers
    /// empty rectangles with different positions to be different.
    bool operator==(const Rect &that) const
    NOEXCEPT_(detail::is_nothrow_comparable<Coordinate>())
    {
        return x == that.x &&
               y == that.y &&
               width == that.width &&
               height == that.height;
    }

    /// Disequality for rectangles.
    bool operator!=(const Rect &that) const
    NOEXCEPT_(detail::is_nothrow_comparable<Coordinate>())
    {
        return !(*this == that);
    }

    /// @}

    /// \name Dims and positions
    /// @{

    /// The dimensions of the rectangle. Equivalent to
    /// `Dims<Coordinate>{rect.width, rect.height}`.
    Dims_type dimensions() const
    NOEXCEPT_(detail::is_nothrow_convertible<Coordinate>())
    {
        return {width, height};
    }

    /// The position of the top left vertex.
    Posn_type top_left() const
    NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>())
    {
        return {x, y};
    }

    /// The position of the top right vertex.
    Posn_type top_right() const
    NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>())
    {
        return top_left().right_by(width);
    }

    /// The position of the bottom left vertex.
    Posn_type bottom_left() const
    NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>())
    {
        return top_left().down_by(height);
    }

    /// The position of the bottom right vertex.
    Posn_type bottom_right() const
    NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>())
    {
        return top_left().down_right_by(dimensions());
    }

    /// The position of the center of the rectangle.
    Posn_type center() const
    NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>() &&
              detail::has_nothrow_division<Coordinate>())
    {
        return top_left().down_right_by(dimensions() / Coordinate(2));
    }

    /// @}

    /// \name Static factory functions
    /// @{

    /// Creates a Rect given the position of its top left vertex
    /// and its dimensions.
    static Rect from_top_left(Posn_type tl, Dims_type dims)
    NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>())
    {
        return {tl.x, tl.y, dims.width, dims.height};
    }

    /// Creates a Rect given the position of its top right vertex
    /// and its dimensions.
    static Rect from_top_right(Posn_type tr, Dims_type dims)
    NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>())
    {
        return from_top_left(tr.left_by(dims.width), dims);
    }

    /// Creates a Rect given the position of its bottom left vertex
    /// and its dimensions.
    static Rect from_bottom_left(Posn_type bl, Dims_type dims)
    NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>())
    {
        return from_top_left(bl.up_by(dims.height), dims);
    }

    /// Creates a Rect given the position of its bottom right vertex
    /// and its dimensions.
    static Rect from_bottom_right(Posn_type br, Dims_type dims)
    NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>())
    {
        return from_top_left(br.up_left_by(dims), dims);
    }

    /// Creates a Rect given the position of its center
    /// and its dimensions.
    static Rect from_center(Posn_type center, Dims_type dims)
    NOEXCEPT_(detail::has_nothrow_arithmetic<Coordinate>())
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
    friend ::ge211::internal::Render_sprite;
    friend class detail::Renderer;

    /// Converts this rectangle to an internal SDL rectangle.
    operator SDL_Rect() const
    NOEXCEPT_(detail::is_nothrow_convertible<COORDINATE, int>())
    {
        SDL_Rect result;
        result.x = static_cast<int>(x);
        result.y = static_cast<int>(y);
        result.w = static_cast<int>(width);
        result.h = static_cast<int>(height);
        return result;
    }
};

/// An iterator over the `Posn<COORDINATE>`s of a `Rect<COORDINATE>`.
///
/// Iterates in column-major order.
template <typename COORDINATE>
class Rect<COORDINATE>::iterator
        : public std::iterator<std::input_iterator_tag, const Posn_type>
{
public:
    /// Returns the current Posn of this iterator.
    Posn_type operator*() const
    {
        return current_;
    }

    /// Returns a pointer to the current Posn of this iterator.
    Posn_type const *operator->() const
    {
        return &current_;
    }

    /// Pre-increments, advancing this iterator to the next Posn.
    iterator &operator++()
    {
        if (++current_.y >= y_end_) {
            ++current_.x;
            current_.y = y_begin_;
        }

        return *this;
    }

    /// Pre-decrements, retreating this iterator to the previous Posn.
    iterator &operator--()
    {
        if (current_.y == y_begin_) {
            current_.y = y_end_;
            --current_.x;
        }

        --current_.y;

        return *this;
    }

    /// Post-increments, advancing this iterator to the next Posn.
    iterator operator++(int)
    {
        iterator result(*this);
        ++*this;
        return result;
    }

    /// Post-decrements, retreating this iterator to the previous Posn.
    iterator operator--(int)
    {
        iterator result(*this);
        --*this;
        return result;
    }

    /// Compares whether two iterators are equal. Considers only the current
    /// position, not the bounds of the stripe we're iterating through.
    bool operator==(iterator const &that) const
    {
        return **this == *that;
    }

    /// Iterator inequality.
    bool operator!=(iterator const &that) const
    {
        return !(*this == that);
    }

private:
    friend Rect;

    iterator(Posn_type current, Coordinate y_begin, Coordinate y_end)
            : current_(current),
              y_begin_(y_begin),
              y_end_(y_end)
    { }

    Posn_type current_;
    Coordinate y_begin_;
    Coordinate y_end_;
};


/// A rendering transformation, which can scale, flip, and rotate.
/// A Transform can be given to
/// Sprite_set::add_sprite(const Sprite&, Posn<int>, int, const Transform&)
/// to specify how a [Sprite](@ref sprites::Sprite) should be rendered.
///
/// To construct a transform that does just one thing, you can use one of
/// the static factory functions:
///
///   - @ref Transform::rotation(double)
///   - @ref Transform::flip_h()
///   - @ref Transform::flip_v()
///   - @ref Transform::scale(double)
///   - @ref Transform::scale_x(double)
///   - @ref Transform::scale_y(double)
///
/// It is also possible to modify a transform with the setter functions
/// such as @ref set_rotation(double) and @ref set_scale(double). This
/// can be used to configure a transform that does more than one thing:
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
    Transform &set_rotation(double) NOEXCEPT;

    /// Modifies this transform to determine whether to flip horizontally.
    Transform &set_flip_h(bool) NOEXCEPT;

    /// Modifies this transform to determine whether to flip vertically.
    Transform &set_flip_v(bool) NOEXCEPT;

    /// Modifies this transform to scale the sprite by the given amount in
    /// both dimensions. This overwrites the effect of previous calls to
    /// set_scale_x(double) and set_scale_y(double).
    Transform &set_scale(double) NOEXCEPT;

    /// Modifies this transform to scale the sprite horizontally. This
    /// overwrites the effect of previous calls to `set_scale(double)`
    /// as well as itself.
    Transform &set_scale_x(double) NOEXCEPT;

    /// Modifies this transform to scale the sprite vertically. This
    /// overwrites the effect of previous calls to `set_scale(double)`
    /// as well as itself.
    Transform &set_scale_y(double) NOEXCEPT;

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
    Transform operator*(const Transform& other) const NOEXCEPT;

    /// Returns the inverse of this transform. Composing a transform with its
    /// inverse should result in the identity transformation, though because
    /// floating point is approximate, is_identity() const may not actually
    /// answer `true`.
    Transform inverse() const NOEXCEPT;

    /// @}

    /// \name Comparing transforms
    /// @{

    /// Equality for `Transform`s.
    bool operator==(const Transform& other) const NOEXCEPT;

    /// Disequality for `Transform`s.
    bool operator!=(const Transform& other) const NOEXCEPT;

    /// @}

private:
    double rotation_;
    double scale_x_;
    double scale_y_;
    bool flip_h_;
    bool flip_v_;
};


/// Gets implicitly converted to `Posn<COORDINATE>(0, 0)`
/// for any coordinate type `COORDINATE`.
///
/// Examples:
///
/// ```
/// ge211::Posn<float> p0 = the_origin;
/// ```
///
/// ```
/// return ge211::Rect<float>::from_top_left(the_origin, {w, h});
/// ```
//;
constexpr origin_type the_origin;


/// \name Factory functions
/// @{

/// Constructs a Dims given the width and height.
///
/// Unlike the constructor, this function can infer the coordinate type
/// from its arguments. For example:
///
/// ```
/// auto near = make_dims(5, 7);   // Dims<int>
/// auto far  = make_dims(5, 7e9); // Dims<double>
/// ```
template <typename COORDINATE>
Dims<COORDINATE>
make_dims(COORDINATE x, COORDINATE y)
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
template <typename COORDINATE>
Posn<COORDINATE>
make_posn(COORDINATE x, COORDINATE y)
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
template <typename COORDINATE>
Rect<COORDINATE>
make_rect(COORDINATE x, COORDINATE y, COORDINATE width, COORDINATE height)
{
    return {x, y, width, height};
}

/// @}

/// \name Stream-insertion operators
/// @{

/// Formats a @ref Dims on an output stream.
template <typename COORDINATE>
std::ostream& operator<<(std::ostream& out, Dims<COORDINATE> dims)
{
    return out
            << "Dims<" << detail::name_of_type<COORDINATE>() << ">("
            << dims.width << ", " << dims.height << ")";
}

/// Formats a @ref Posn on an output stream.
template <typename COORDINATE>
std::ostream& operator<<(std::ostream& out, Posn<COORDINATE> p)
{
    return out
            << "Posn<" << detail::name_of_type<COORDINATE>() << ">("
            << p.x << ", " << p.y << ")";
}

/// Formats a @ref Rect on an output stream.
template <typename COORDINATE>
std::ostream& operator<<(std::ostream& out, Rect<COORDINATE> rect)
{
    return out
            << "Rect<" << detail::name_of_type<COORDINATE>() << ">("
            << rect.x     << ", " << rect.y      << ", "
            << rect.width << ", " << rect.height << ")";
}

/// @}

} // end namespace geometry.

} // end namespace ge211

// specializations in std:
namespace std {

/// Template specialization to define hashing of
/// @ref ge211::geometry::Posn%s, which allows storing them as members of an
/// @ref std::unordered_set or as keys of an
/// @ref std::unordered_map.
template <typename COORDINATE>
struct hash<ge211::Posn<COORDINATE>>
{
    /// Hashes a Posn<COORDINATE>, provided that COORDINATE is hashable.
    std::size_t operator()(ge211::Posn<COORDINATE> pos) const
    NOEXCEPT
    {
        return hash_t_(pos.x) * 31 ^ hash_t_(pos.y);
    }

private:
    std::hash<COORDINATE> hash_t_;
};

} // end namespace std

