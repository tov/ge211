#include <ge211.hxx>

#include <cmath>
#include <iomanip>
#include <vector>
#include <utility>

using namespace ge211;
using namespace std;

// MODEL CONSTANTS

Dims<int> const    scene_dimensions{1024, 768};
Dims<double> const gravity_acceleration{0, 120}; // px/s^2
int const          min_launch_speed{350}; // px/s
int const          max_launch_speed{500}; // px/s
int const          max_launch_angle{30}; // degrees from vertical
double const       fuse_seconds{2};
int const          min_stars{40};
int const          max_stars{400};
int const          min_star_speed{10}; // px/s
int const          max_star_speed{100}; // px/s
double const       burn_seconds{2};
int const          number_of_colors{12};

// VIEW CONSTANTS

int const   mortar_radius = 5;
Color const mortar_color{255, 255, 127, 80};
int const   star_radius   = 2;

// MODEL DATA DEFINITIONS

struct Random_sources
{
    Random_source<int> launch_speed{min_launch_speed, max_launch_speed};
    Random_source<int> launch_angle{-90 - max_launch_angle,
                                    -90 + max_launch_angle};
    Random_source<int> star_count{min_stars, max_stars};
    Random_source<int> star_speed{min_star_speed, max_star_speed};
    Random_source<int> star_angle{360};
    Random_source<int> color{number_of_colors};
};

struct Projectile
{
    using Position = Posn<double>;
    using Velocity = Dims<double>;

    Position position;
    Velocity velocity;

    void update(double dt);

    /// Creates a Projectile with the given Position and a random velocity
    /// within the given speed range and angle range.
    static Projectile random(
            Position,
            Random_source<int>& random_speed,
            Random_source<int>& random_angle);
};

struct Firework
{
    enum class Stage
    {
        mortar, stars, done
    };

    Stage              stage;
    Projectile         mortar;
    vector<Projectile> stars;
    int                star_color;
    double             stage_time;

    unsigned update(double dt);
    static Firework random(Random_sources&, Projectile::Position);
};

struct Model
{
    vector<Firework> fireworks;
    Random_sources randomness;

    unsigned update(double dt);
    void add_random(Projectile::Position);
};

// VIEW DATA DEFINITIONS

struct View
{
    View(Mixer&);

    Font                  sans{"sans.ttf", 30};
    Text_sprite           fps;
    Text_sprite           load;
    Circle_sprite         mortar{mortar_radius, mortar_color};
    vector<Circle_sprite> stars;
    Sound_effect          pop;
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
    Dims<int> initial_window_dimensions() const override;
    void draw(Sprite_set& sprites) override;
    void draw_fireworks(Sprite_set& sprites) const;
    void draw_stats(Sprite_set& sprites);

    // Controller
    bool is_paused = false;
    void on_key(Key key) override;
    void on_mouse_up(Mouse_button button, Posn<int> position) override;
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
Projectile::random(Position position,
                   Random_source<int>& random_speed,
                   Random_source<int>& random_angle)
{
    double speed   = random_speed.next();
    double radians = M_PI / 180 * random_angle.next();
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
            stage      = Stage::stars;
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

Firework Firework::random(Random_sources& random, Projectile::Position p0)
{
    Projectile mortar = Projectile::random(p0,
                                           random.launch_speed,
                                           random.launch_angle);

    vector<Projectile> stars;

    int      star_count = random.star_count();
    for (int i          = 0; i < star_count; ++i) {
        Projectile star = Projectile::random(the_origin,
                                             random.star_speed,
                                             random.star_angle);
        stars.push_back(star);
    }

    int star_color = random.color();

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

void Model::add_random(Projectile::Position position0)
{
    fireworks.push_back(Firework::random(randomness, position0));
}

// FUNCTION DEFINITIONS FOR VIEW

View::View(Mixer& mixer)
{
    double hue  = 1.0;
    double dhue = 360.0 / number_of_colors;

    for (int i = 0; i < number_of_colors; ++i) {
        stars.emplace_back(star_radius, Color::from_hsla(hue, .75, .75, .75));
        hue += dhue;
    }

    pop.try_load("pop.ogg", mixer);
}

Dims<int> Fireworks::initial_window_dimensions() const
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
                               firework.mortar.position.into<int>());
            break;

        case Firework::Stage::stars:
            for (Projectile const& star : firework.stars) {
                sprites.add_sprite(view.stars[firework.star_color],
                                   star.position.into<int>());
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
    Dims<int> const margin{20, 10};

    view.fps.reconfigure(Text_sprite::Builder(view.sans)
                                 << setprecision(3)
                                 << get_frame_rate());
    view.load.reconfigure(Text_sprite::Builder(view.sans)
                                  << setprecision(0) << fixed
                                  << get_load_percent() << '%');

    auto fps_posn = Posn<int>{margin};
    sprites.add_sprite(view.fps, fps_posn);

    auto load_posn = Posn<int>{scene_dimensions.width, 0}
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
        auto dims             = get_window().get_dimensions();
        auto initial_position = Posn<double>(dims.width / 2, dims.height);
        model.add_random(initial_position);
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

void Fireworks::on_mouse_up(Mouse_button, Posn<int> posn)
{
    if (is_paused) return;

    model.add_random(Projectile::Position(posn));
}
