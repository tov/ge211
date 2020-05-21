#include <ge211.hxx>

#include <cmath>
#include <iomanip>
#include <vector>
#include <utility>

using namespace ge211;
using namespace std;

// MODEL CONSTANTS

Dimensions const scene_dimensions{1024, 768};
Basic_dimensions<double> const gravity_acceleration{0, 120}; // px/s^2
int const min_launch_speed{350}; // px/s
int const max_launch_speed{500}; // px/s
int const max_launch_angle{30}; // degrees from vertical
double const fuse_seconds{2};
int const min_stars{40};
int const max_stars{400};
int const min_star_speed{10}; // px/s
int const max_star_speed{100}; // px/s
double const burn_seconds{2};
int const number_of_colors{12};

// VIEW CONSTANTS

int const mortar_radius = 5;
Color const mortar_color{255, 255, 127, 80};
int const star_radius = 2;

// MODEL DATA DEFINITIONS

struct Projectile
{
    using Position = Basic_position<double>;
    using Velocity = Basic_dimensions<double>;

    Position position;
    Velocity velocity;

    void update(double dt);

    /// Creates a Projectile with the given Position and a random velocity
    /// within the given speed range and angle range.
    static Projectile random(
            Random&,
            Position,
            double min_speed, double max_speed,
            double min_degrees, double max_degrees);
};

struct Firework
{
    enum class Stage { mortar, stars, done };

    Stage stage;
    Projectile mortar;
    vector<Projectile> stars;
    int star_color;
    double stage_time;

    unsigned update(double dt);
    static Firework random(Random&, Projectile::Position);
};

struct Model
{
    vector<Firework> fireworks;

    unsigned update(double dt);
    void add_random(Random&, Projectile::Position);
};

// VIEW DATA DEFINITIONS

struct View
{
    View(Mixer&);

    Font sans{"sans.ttf", 30};
    Text_sprite fps;
    Text_sprite load;
    Circle_sprite mortar{mortar_radius, mortar_color};
    vector<Circle_sprite> stars;
    Sound_effect pop;
};

// MAIN STRUCT AND FUNCTION

struct Fireworks : Abstract_game
{
    // Constructor
    Fireworks();

    // Model
    Model model;

    // View
    View view;
    Dimensions initial_window_dimensions() const override;
    void draw(Sprite_set& sprites) override;
    void draw_fireworks(Sprite_set& sprites) const;
    void draw_stats(Sprite_set& sprites);

    // Controller
    bool is_paused = false;
    void on_key(Key key) override;
    void on_mouse_up(Mouse_button button, Position position) override;
    void on_frame(double dt) override;
};

int main()
{
    Fireworks{}.run();
}

// FUNCTION DEFINITIONS FOR MODEL

void Projectile::update(double dt)
{
    position += velocity * dt;
    velocity += gravity_acceleration * dt;
}

Projectile
Projectile::random(Random& rng, Position position,
                   double min_speed, double max_speed,
                   double min_degrees, double max_degrees)
{
    double speed = rng.between(min_speed, max_speed);
    double radians = M_PI / 180 * rng.between(min_degrees, max_degrees);
    return {position, {speed * cos(radians), speed * sin(radians)}};
}

unsigned Firework::update(double dt)
{
    unsigned explosions = 0;

    switch (stage) {
        case Stage::mortar:
            if ((stage_time -= dt) <= 0) {
                for (Projectile& star : stars) {
                    star.position = mortar.position;
                    star.velocity += mortar.velocity;
                }
                stage_time = burn_seconds;
                stage = Stage::stars;
                ++explosions;
            } else {
                mortar.update(dt);
            }
            break;

        case Stage::stars:
            if ((stage_time -= dt) <= 0) {
                stage = Stage::done;
            } else {
                for (Projectile& star : stars) {
                    star.update(dt);
                }
            }
            break;

        case Stage::done:
            break;
    }

    return explosions;
}

Firework Firework::random(Random& rng, Projectile::Position p0)
{
    Projectile mortar = Projectile::random(rng, p0,
                                           min_launch_speed, max_launch_speed,
                                           -90 - max_launch_angle,
                                           -90 + max_launch_angle);

    vector<Projectile> stars;

    int star_count = rng.between(min_stars, max_stars);
    for (int i = 0; i < star_count; ++i) {
        Projectile star = Projectile::random(rng, {0, 0},
                                             min_star_speed, max_star_speed,
                                             0, 360);
        stars.push_back(star);
    }

    int star_color = rng.up_to(number_of_colors);

    return Firework{Stage::mortar, mortar, stars, star_color, fuse_seconds};
}

unsigned Model::update(double dt)
{
    unsigned explosions = 0;

    for (Firework& firework : fireworks)
        explosions += firework.update(dt);

    size_t i = 0;
    while (i < fireworks.size()) {
        if (fireworks[i].stage == Firework::Stage::done) {
            fireworks[i] = move(fireworks.back());
            fireworks.pop_back();
        } else {
            ++i;
        }
    }

    return explosions;
}

void Model::add_random(Random& rng, Projectile::Position position0)
{
    fireworks.push_back(Firework::random(rng, position0));
}

// FUNCTION DEFINITIONS FOR VIEW

View::View(Mixer& mixer)
{
    double hue = 1.0;
    double dhue = 360.0 / number_of_colors;

    for (int i = 0; i < number_of_colors; ++i) {
        stars.emplace_back(star_radius, Color::from_hsla(hue, .75, .75, .75));
        hue += dhue;
    }

    pop.try_load("pop.ogg", mixer);
}

Dimensions Fireworks::initial_window_dimensions() const
{
    return scene_dimensions;
}

void Fireworks::draw(Sprite_set& sprites)
{
    draw_fireworks(sprites);
    draw_stats(sprites);
}

void Fireworks::draw_fireworks(Sprite_set& sprites) const
{
    for (Firework const& firework : model.fireworks) {
        switch (firework.stage) {
            case Firework::Stage::mortar:
                sprites.add_sprite(view.mortar,
                                   Position(firework.mortar.position));
                break;

            case Firework::Stage::stars:
                for (Projectile const& star : firework.stars) {
                    sprites.add_sprite(view.stars[firework.star_color],
                                       Position(star.position));
                }
                break;

            // Shouldn't ever happen:
            case Firework::Stage::done:
                break;
        }
    }
}

void Fireworks::draw_stats(Sprite_set& sprites)
{
    Dimensions const margin {20, 10};

    view.fps.reconfigure(Text_sprite::Builder(view.sans)
                                 << setprecision(3)
                                 << get_frame_rate());
    view.load.reconfigure(Text_sprite::Builder(view.sans)
                                  << setprecision(0) << fixed
                                  << get_load_percent() << '%');

    auto fps_posn  = Position{margin};
    sprites.add_sprite(view.fps, fps_posn);

    auto load_posn = Position{scene_dimensions.width, 0}
            .down_left_by(margin)
            .left_by(view.load.dimensions().width);
    sprites.add_sprite(view.load, load_posn);
}

// CONSTRUCTING THE GAME OBJECT

Fireworks::Fireworks()
        : view(mixer())
{ }

// FUNCTION DEFINITIONS FOR CONTROLLER

void Fireworks::on_key(Key key)
{
    if (key == Key::code('q')) {
        quit();
    } else if (key == Key::code('f')) {
        get_window().set_fullscreen(!get_window().get_fullscreen());
    } else if (key == Key::code('p')) {
        is_paused = !is_paused;
    } else if (key == Key::code(' ') && !is_paused) {
        auto dims = get_window().get_dimensions();
        auto pos0 = Projectile::Position(dims.width / 2., dims.height);
        model.add_random(get_random(), pos0);
    }
}

void Fireworks::on_frame(double dt)
{
    if (is_paused) return;

    unsigned explosion_count = model.update(dt);

    if (view.pop)
        while (explosion_count--)
            mixer().play_effect(view.pop);
}

void Fireworks::on_mouse_up(Mouse_button, Position position)
{
    if (is_paused) return;

    model.add_random(get_random(), Projectile::Position(position));
}
