#pragma once

#include "ge211_color.hxx"
#include "ge211_forward.hxx"
#include "ge211_geometry.hxx"
#include "ge211_noexcept.hxx"
#include "ge211_time.hxx"
#include "ge211_render.hxx"
#include "ge211_resource.hxx"

#include <vector>
#include <sstream>

namespace ge211 {

/// Sprites are images that can be rendered to the screen. This namespace
/// defines a base Sprite class that declares common sprite operations,
/// and four specific types of sprites with different purposes.
namespace sprites {

/// A sprite is an image that knows how to render itself to the screen
/// at a given location, under a particular transformation. You cannot
/// create a Sprite object directly, but must create one of its derived
/// classes, such as @ref Image_sprite or @ref Rectangle_sprite. You can
/// find out any sprite's dimensions with the @ref Sprite::dimensions()
/// const member function. Specific derived classes of Sprite, such as
/// @ref Rectangle_sprite, may have more specific operations.
///
/// \sa @ref Circle_sprite
/// \sa @ref Image_sprite
/// \sa @ref Rectangle_sprite
/// \sa @ref Text_sprite
///
/// \internal
/// A sprite is anything with dimensions that knows how to render itself
/// at a given position. Derived classes need to implement both the
/// public `dimensions` function and the private `render` function.
class Sprite
{
public:
    /// Returns the current dimensions of this Sprite.
    ///
    /// When deriving from Sprite to implement your own kind of sprite
    /// (or when deriving from Multiplexed_sprite), you need to make sure
    /// that this returns the *maximum* dimensions that the sprite could
    /// render at. For animated sprites, it's usually best if all the
    /// frames have the same dimensions anyway.
    virtual Dims<int> dimensions() const = 0;

    virtual ~Sprite() {}

private:
    friend class detail::Engine;
    friend struct detail::Placed_sprite;
    friend Multiplexed_sprite;

    virtual void render(detail::Renderer&,
                        Posn<int>,
                        Transform const&) const = 0;

    virtual void prepare(detail::Renderer const&) const {}
};

} // end namespace sprites

namespace detail {

// A `Texture_sprite` is a `Sprite` that can be rendered by copying
// a texture. Instead of specifying how to render themselves directly,
// derived classes must specify how to get a `Texture` representing
// the sprite. The dimensions of the sprite are the dimensions of the
// resulting texture. The return type of `Texture const&` means that
// `get_texture_` cannot just create and return a texture, but must
// store it somewhere. This is because `Texture`s will usually be
// cached. (Otherwise, you wouldn't use a `Texture_sprite`.)
class Texture_sprite : public Sprite
{
public:
    Dims<int> dimensions() const override;

private:
    void render(detail::Renderer&, Posn<int>, Transform const&) const override;
    void prepare(detail::Renderer const&) const override;

    virtual Texture const& get_texture_() const = 0;
};

} // end namespace detail

namespace internal {

/// A `Render_sprite` works by allowing its derived classes to render
/// themselves pixel-by-pixel onto an [`SDL_Surface`☛]. Then it converts
/// the rendered surface into a `Texture`, which it caches.
///
/// The constructor of the derived class should pass the required
/// dimensions to the `Render_sprite` constructor. Then, in its own
/// constructor, use @ref fill_surface(), @ref fill_rectangle(), and
/// @ref set_pixel() to paint the desired sprite image to the
/// surface. Or for direct access to the underlying [`SDL_Surface`☛], use
/// @ref raw_surface().
///
/// [`SDL_Surface`☛]: https://wiki.libsdl.org/SDL_Surface
class Render_sprite : public detail::Texture_sprite
{
protected:
    /// Constructs a Render_sprite with the given pixel dimensions.
    ///
    /// \preconditions
    ///  - Both dimensions are positive.
    explicit Render_sprite(Dims<int>);

    /// Returns whether we can paint to this Render_sprite.
    ///
    /// If this sprite has already been rendered to the screen then this
    /// function returns `false`. When the result is `false`, then calling
    /// any of @ref fill_surface(), @ref fill_rectangle(), @ref
    /// set_pixel(), or @ref raw_surface() will throw an
    /// @ref exceptions::Late_paint_error exception.
    bool can_paint() const;

    /// Fills the whole surface with the given color.
    ///
    /// Typically this will only be called from a derived class's
    /// constructor.
    ///
    /// \precondition
    /// Throws @ref exceptions::Late_paint_error if `!`@ref can_paint().
    void fill_surface(Color);

    /// Fills the given rectangle in the given color.
    ///
    /// Typically this will only be called from a derived class's
    /// constructor.
    ///
    /// \precondition
    /// Throws @ref exceptions::Late_paint_error if `!`@ref can_paint().
    void fill_rectangle(Rect<int>, Color);

    /// Sets one pixel to the given color.
    ///
    /// Typically this will only be called from a derived class's
    /// constructor.
    ///
    /// \precondition
    /// Throws @ref exceptions::Late_paint_error if `!`@ref can_paint().
    void set_pixel(Posn<int>, Color);

    /// Gains access to the underlying [`SDL_Surface`☛].
    ///
    /// Typically this will only be called from a derived class's
    /// constructor. Never returns null.
    ///
    /// \precondition
    /// Throws @ref exceptions::Late_paint_error if `!`@ref can_paint().
    ///
    /// [`SDL_Surface`☛]: https://wiki.libsdl.org/SDL_Surface
    Borrowed<SDL_Surface> raw_surface();

private:
    detail::Texture texture_;

    detail::Texture const& get_texture_() const override;

    void fill_rectangle_(Rect<int>, Color, char const* who);

    Borrowed<SDL_Surface> raw_surface_(char const* who);

    static detail::Uniq_SDL_Surface
    create_surface_(Dims<int>);
};

} // end namespace internal

namespace sprites {

/// A Sprite that renders as a solid rectangle.
class Rectangle_sprite : public internal::Render_sprite
{
public:
    /// Constructs a rectangle sprite from required Dims
    /// and an optional Color, which defaults to white.
    ///
    /// \preconditions
    ///  - both dimensions must be positive
    explicit Rectangle_sprite(Dims<int>, Color = Color::white());

    /// Changes the color of this rectangle sprite.
    void recolor(Color);
};

/// A Sprite that renders as a solid circle.
class Circle_sprite : public internal::Render_sprite
{
public:
    /// Constructs a circle sprite from its radius and optionally
    /// a Color, which defaults to white. Note that when positioned,
    /// the reference point is the upper-left corner of the bounding
    /// box of the sprite, not the center of the circle.
    ///
    /// \preconditions
    ///  - radius must be positive
    explicit Circle_sprite(int radius, Color = Color::white());

    /// Changes the color of this circle sprite.
    void recolor(Color);

private:
    int radius_() const;
};

/// A Sprite that displays a bitmap image.
class Image_sprite : public detail::Texture_sprite
{
public:
    /// Constructs an image sprite, given the filename of the
    /// image to display. The image must be saved in the project's
    /// `Resources/` directory. Many image formats are supported,
    /// including JPEG, PNG, GIF, BMP, etc.
    explicit Image_sprite(std::string const& filename);

private:
    detail::Texture const& get_texture_() const override;

    static detail::Texture load_texture_(std::string const& filename);

    detail::Texture texture_;
};

/// A Sprite that displays text.
class Text_sprite : public detail::Texture_sprite
{
public:
    /// Constructs an empty text sprite.
    ///
    /// This is useful when you don't yet know the message at the point
    /// where the sprite is created. It's an error to pass the an
    /// empty Text_sprite to
    /// Sprite_set::add_sprite(Sprite const&, Posn<int>, int, Transform), but
    /// you can use Text_sprite::reconfigure(Builder const&) to make
    /// it non-empty.
    ///
    /// \example
    ///
    /// ```
    /// struct View
    /// {
    ///     void draw(ge211::Sprite_set& set,
    ///               std::string const& name,
    ///               int                score);
    ///
    ///     ge211::Font sans30{"sans.ttf", 30};
    ///     ge211::Text_sprite name_sprite;
    ///     ge211::Text_sprite score_sprite;
    /// }
    ///
    /// void View::draw(ge211::Sprite_set& set,
    ///                 std::string const& name,
    ///                 int                score)
    /// {
    ///     ge211::Text_sprite::Builder name_builder(sans30);
    ///     name_builder.color(NAME_COLOR) << name;
    ///     name_sprite.reconfigure(name_builder);
    ///     set.add_sprite(name_sprite, NAME_POSITION);
    ///
    ///     ge211::Text_sprite::Builder score_builder(sans30);
    ///     score_builder.color(SCORE_COLOR) << score;
    ///     score_sprite.reconfigure(score_builder);
    ///     set.add_sprite(score_sprite, SCORE_POSITION);
    /// }
    /// ```
    Text_sprite();

    /// Constructs a white text sprite with the given text and font.
    ///
    /// For more control (color, wrapping, turning off anti-aliasing),
    /// use the Builder API instead.
    ///
    /// While it is okay to construct a text sprite with no text, it
    /// cannot be rendered into a scene. Use empty() const to check
    /// if you haven't kept track.
    ///
    /// \example
    ///
    /// ```
    /// struct View
    /// {
    ///     ge211::Font        sans72      {"sans.ttf", 72},
    ///                        sans30      {"sans.ttf", 30};
    ///
    ///     ge211::Text_sprite title       {"My Fun Game", sans72},
    ///                        score_label {"Score:", sans30},
    ///                        level_label {"Level:", sans30},
    ///                        score_value,
    ///                        level_value;
    /// };
    /// ```
    Text_sprite(std::string const&, Font const&);

    /// Is this Text_sprite empty? (If so, you shouldn't try to use
    /// it.)
    bool empty() const;

    /// Is this Text_sprite non-empty (and thus renderable)?
    explicit operator bool() const;

    // Defined below.
    class Builder;

    /// Resets this text sprite with the configuration from the given Builder.
    void reconfigure(Builder const&);

private:
    explicit Text_sprite(Builder const&);

    void assert_initialized_() const;

    detail::Texture const& get_texture_() const override;

    static detail::Texture create_texture(Builder const&);

    detail::Texture texture_;
};

/// Builder-style API for configuring and constructing Text_sprite%s.
///
/// The idea is that a Text_sprite::Builder allows configuring a
/// Text_sprite in detail before actually constructing it.
///
/// \example
///
/// ```
/// struct View
/// {
///     ge211::Font sans{"sans.ttf", 16},
///     ge211::Text_sprite percentage_sprite;
///     double cached_value = NAN;
///
///     void update_percentage(double unit_value);
///     …
/// };
///
/// void View::update_percentage(double unit_value)
/// {
///     if (unit_value == cached_value) return;
///
///     ge211::Text_sprite::Builder builder(sans);
///
///     if (unit_value < 0.5) {
///         builder.color(Color::medium_green());
///     } else {
///         builder.color(Color::medium_red());
///     }
///
///     builder << std::setprecision(1) << std::fixed
///             << 100 * unit_value << '%';
///
///     percentage_sprite.reconfigure(builder);
///     cached_value = unit_value;
/// }
/// ```
class Text_sprite::Builder
{
public:
    /// \name Constructor and builder
    /// @{

    /// Constructs a new Text_sprite::Builder with the given Font.
    explicit Builder(Font const&);

    /// Builds the configured Text_sprite.
    Text_sprite build() const;

    /// @}

    /// \name Builder-style setters
    /// @{

    /// Adds to the builder's message. This takes any printable type
    /// and prints it à la `operator<<`. Returns the builder, for call
    /// chaining.
    template <typename PRINTABLE>
    Builder& add_message(PRINTABLE const& value)
    {
        message_ << value;
        return *this;
    }

    /// Adds to the builder's message. This is an alias for
    /// @ref add_message(const PRINTABLE&).
    ///
    /// For example:
    ///
    /// ```cpp
    /// Text_sprite position_sprite(Posn<int> position)
    /// {
    ///     Builder builder(font);
    ///     builder << "(" << position.x << ", " << position.y << ")";
    ///     return builder.build();
    /// }
    /// ```
    template <typename PRINTABLE>
    Builder& operator<<(PRINTABLE const& value)
    {
        return add_message(value);
    }

    /// Replaces the configured message with the given message.
    /// Returns a reference to the Builder for call chaining.
    Builder& message(std::string const&);
    /// Sets font to use.
    /// Returns a reference to the Builder for call chaining.
    Builder& font(Font const&);
    /// Sets the color to use.
    /// Returns a reference to the Builder for call chaining.
    Builder& color(Color);
    /// Sets whether to use anti-aliasing. Anti-aliasing, on by default, makes
    /// text smoother but can make it take longer to render.
    /// Returns a reference to the Builder for call chaining.
    Builder& antialias(bool);
    /// Sets the pixel width for wrapping the text. If set to 0, the text does not
    /// wrap at all. Newlines in the text will produces newlines in the output
    /// only if wrapping is on (non-zero).
    /// Returns a reference to the Builder for call chaining.
    Builder& word_wrap(int);

    /// @}

    /// \name Getters
    /// @{

    /// Gets the configured message.
    std::string message() const;
    /// Gets the font that will be used.
    Font const& font() const;
    /// Gets the color that will be used.
    Color color() const;
    /// Gets whether anti-aliasing will be used.
    bool antialias() const;
    /// Gets the wrapping width that will be used.
    int word_wrap() const;

    /// @}

private:
    std::ostringstream message_;
    const Font* font_;
    Color color_;
    bool antialias_;
    uint32_t word_wrap_;
};

/// A Sprite that allows switching between other sprites based on the
/// time at rendering.
class Multiplexed_sprite : public Sprite
{
public:
    /// Resets the age of the sprite to 0.
    void reset();

protected:
    /// Override this to specify what sprite to render, based on the
    /// age of this sprite. This can be used to implement animation.
    virtual const Sprite& select_(Duration age) const = 0;

private:
    void render(detail::Renderer& renderer, Posn<int> position,
                Transform const& transform) const override;

    Timer since_;
};

} // end namespace sprites

namespace detail {

struct Placed_sprite
{
    const Sprite* sprite;
    Posn<int> xy;
    int z;
    Transform transform;

    Placed_sprite(Sprite const&, Posn<int>, int, Transform const&) NOEXCEPT;

    void render(Renderer&) const;
};

bool operator<(Placed_sprite const&, Placed_sprite const&) NOEXCEPT;

} // end namespace detail

/// A collection of positioned [Sprite](@ref ge211::sprites::Sprite)s
/// ready to be rendered to the screen. Each time @ref
/// Abstract_game::draw(Sprite_set&) is called by the game engine, it is
/// given an empty @ref Sprite_set, and it must add every [Sprite] that
/// should appear on the screen to that @ref Sprite_set. Each [Sprite]
/// is added with an *x–y* coordinates (@ref Posn<int>) and a *z*
/// coordinate that determines stacking order. Each sprite may have a
/// [Transform] applied as well.
///
/// \sa Sprite_set::add_sprite(Sprite const&, Posn<int>, int, Transform const&)
/// \sa `class` [Sprite]
/// \sa `class` [Transform]
///
/// [Sprite]: @ref ge211::sprites::Sprite
/// [Transform]: @ref ge211::geometry::Transform
class Sprite_set
{
public:
    /// Adds the given sprite to the sprite set to render it in the next frame.
    ///
    /// \param sprite The sprite to render in the window.
    /// \param xy The (*x*, *y*) coordinates of the sprite, given as the
    /// position of its top-left corner relative to the top-left
    /// corner of the window.
    /// \param z (*optional*, defaults to 0) The *z* coordinate, which
    /// determines the relative layering of all the sprites in the window.
    /// The layering of sprites placed with the same *z* is unspecified and
    /// not related to the order in which they were added, so you may need to
    /// specify *z* if sprites overlap and you care which appears in front.
    /// \param transform (*optional*, defaults to the identity transform)
    /// A [Transform] allows scaling, flipping, and rotating the [Sprite]
    /// when it is rendered.
    /// \return a reference to this same Sprite_set
    ///
    /// \ownership
    ///
    /// This function borrows the sprite and retains a reference to it in the
    /// the Sprite_set, which is then used to render the sprite after
    /// Abstract_game::draw(Sprite_set&) returns. This means that any sprites
    /// added to the Sprite_set need to be owned by some other object that
    /// outlives the current call to @ref Abstract_game::draw.
    ///
    /// \sa `struct` [Posn]
    /// \sa `class` [Transform]
    /// \sa `class` [Sprite]
    ///
    /// [Posn]: @ref ge211::geometry::Posn
    /// [Transform]: @ref ge211::geometry::Transform
    /// [Sprite]: @ref ge211::sprites::Sprite
    Sprite_set& add_sprite(Sprite const& sprite,
                           Posn<int> xy,
                           int z = 0,
                           Transform const& transform = Transform());

private:
    friend class detail::Engine;

    Sprite_set();
    std::vector<detail::Placed_sprite> sprites_;
};

}
