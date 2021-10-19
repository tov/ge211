#include "doctest.hxx"

#include <ge211.hxx>

using util::reflection::name_of_type;

#define DO_CHECK(...) CHECK(name_of_type<__VA_ARGS__>() == #__VA_ARGS__)

TEST_SUITE_BEGIN("util::reflection");

TEST_CASE("names of built-in types")
{
    DO_CHECK(bool);
    DO_CHECK(char);
    DO_CHECK(unsigned char);
    DO_CHECK(signed char);
    DO_CHECK(char16_t);
    DO_CHECK(char32_t);
    DO_CHECK(wchar_t);
    DO_CHECK(short);
    DO_CHECK(unsigned short);
    DO_CHECK(int);
    DO_CHECK(unsigned int);
    DO_CHECK(long);
    DO_CHECK(unsigned long);
    DO_CHECK(float);
    DO_CHECK(double);
    DO_CHECK(long double);
#ifdef LLONG_MAX
    DO_CHECK(long long);
#endif
#ifdef ULLONG_MAX
    DO_CHECK(unsigned long long);
#endif
}

TEST_CASE("names of raw pointer-to-builtin types")
{
    DO_CHECK(bool *);
    DO_CHECK(bool const *);
    DO_CHECK(char *);
    DO_CHECK(char const *);
}

namespace some_ns { struct Some_unknown_type { }; }

TEST_CASE("name of unknown type")
{
    CHECK(name_of_type<some_ns::Some_unknown_type>() == "?");
    CHECK(name_of_type<some_ns::Some_unknown_type>("<_>") == "<_>");
    CHECK(name_of_type<int>("<unknown>") == "int");
}

namespace dummy_ns {
template <class T> struct allocator { typedef T value_type; };
}

GE211_REGISTER_TMPL_NAME(dummy_ns::allocator);

TEST_CASE("names of STL types")
{
    DO_CHECK(std::istringstream);
    DO_CHECK(std::ostream);
    DO_CHECK(std::string);
    DO_CHECK(std::basic_string<char16_t,
                     std::char_traits<char16_t>, std::allocator<char16_t>>);
    DO_CHECK(std::vector<int>);
    DO_CHECK(std::vector<long>);
    DO_CHECK(std::vector<int, dummy_ns::allocator<int>>);
    DO_CHECK(std::vector<double, dummy_ns::allocator<double>>);
}

TEST_CASE("names of ge211 types")
{
    DO_CHECK(ge211::Abstract_game);
    DO_CHECK(ge211::Color);
    DO_CHECK(ge211::Font);
    DO_CHECK(ge211::Sprite_set);
    DO_CHECK(ge211::Channel_state);
    DO_CHECK(ge211::Mixer);
    DO_CHECK(ge211::Audio_clip);
    DO_CHECK(ge211::Music_track);
    DO_CHECK(ge211::Sound_effect);
    DO_CHECK(ge211::Sound_effect_handle);
    DO_CHECK(ge211::Key);
    DO_CHECK(ge211::Mouse_button);
    DO_CHECK(ge211::Exception_base);
    DO_CHECK(ge211::Client_logic_error);
    DO_CHECK(ge211::Session_needed_error);
    DO_CHECK(ge211::Random_source_error);
    DO_CHECK(ge211::Random_source_bounds_error);
    DO_CHECK(ge211::Random_source_empty_stub_error);
    DO_CHECK(ge211::Random_source_unsupported_error);
    DO_CHECK(ge211::Environment_error);
    DO_CHECK(ge211::Ge211_logic_error);
    DO_CHECK(ge211::Late_paint_error);
    DO_CHECK(ge211::Host_error);
    DO_CHECK(ge211::File_open_error);
    DO_CHECK(ge211::Font_load_error);
    DO_CHECK(ge211::Image_load_error);
    DO_CHECK(ge211::Mixer_error);
    DO_CHECK(ge211::Audio_load_error);
    DO_CHECK(ge211::Out_of_channels_error);
    DO_CHECK(ge211::Mixer_not_enabled_error);
    DO_CHECK(ge211::Origin_type);
    DO_CHECK(ge211::Transform);
    DO_CHECK(ge211::Sprite);
    DO_CHECK(ge211::Circle_sprite);
    DO_CHECK(ge211::Image_sprite);
    DO_CHECK(ge211::Multiplexed_sprite);
    DO_CHECK(ge211::Rectangle_sprite);
    DO_CHECK(ge211::Text_sprite);
    DO_CHECK(ge211::Duration);
    DO_CHECK(ge211::Time_point);
    DO_CHECK(ge211::internal::Render_sprite);
}

TEST_CASE("names of ge211 templates")
{
    DO_CHECK(ge211::Random_source<int>);
    DO_CHECK(ge211::Random_source<double>);
    DO_CHECK(ge211::Dims<int>);
    DO_CHECK(ge211::Dims<double>);
    DO_CHECK(ge211::Posn<int>);
    DO_CHECK(ge211::Posn<double>);
    DO_CHECK(ge211::Rect<int>);
    DO_CHECK(ge211::Rect<double>);
}

TEST_SUITE_END();
