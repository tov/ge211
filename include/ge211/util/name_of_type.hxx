#pragma once

#include <iosfwd>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace util {
namespace reflection {

//
// PUBLIC INTERFACE
//

/// Stream-inserting <tt>%Name_of_type<T>()</tt> will print the name of type
/// `T`. If type `T` has not been registered using
/// `GE211_REGISTER_TYPE_NAME(T);` then this mechanism will
/// fall back to printing a question mark (`"?"`).
///
/// While the actual implementation is more complex, you can think of
/// <tt>%Name_of_type</tt> as having this interface:
///
/// ```
/// template <typename TYPE>
/// struct Name_of_type
/// {
///     // Constructs a value that, when stream-inserted, prints the name of
///     // type TYPE. If TYPE or some parts of TYPE are unrecognized, they are
///     // printed using the fallback parameter.
///     explicit Name_of_type(char const* fallback = "?");
///
///     // Inserts the name of type TYPE into output stream o.
///     void insert(std::ostream& o);
/// };
/// ```
///
/// You can replace the question
/// mark by passing a different value to
/// @ref Name_of_type::Name_of_type(char const*).
template <class>
struct Name_of_type;

/// Stream insertion for the name of type TYPE.
template <class TYPE>
std::ostream&
operator<<(std::ostream&, Name_of_type<TYPE>);

/// Returns the name of type TYPE as a @ref std::string. Takes a fallback
/// argument for printing unknown types, which defaults to `"?"`.
template <class TYPE>
std::string
name_of_type(char const *fallback = nullptr);


//
// DETAILS...
//

namespace detail {

/// Helper struct for printing a comma-separated sequence.
template <class...>
struct Comma_seq;

/// Factory function for template instantiation inference.
template <class... A>
Comma_seq<A...>
comma_seq(A const& ... a)
{
    return Comma_seq<A...>(a...);
}

/// Stream insertion for a comma-separated sequence.
template <class... PRINTABLE>
std::ostream&
operator<<(std::ostream& o, Comma_seq<PRINTABLE...> cs)
{
    cs.insert(o);
    return o;
}

/// The empty comma sequence.
template <>
struct Comma_seq<>
{
    void insert(std::ostream&) const { }
};

/// Singleton comma sequence.
template <class A>
struct Comma_seq<A>
{
    explicit Comma_seq(A const& ra)
            : pa(std::addressof(ra)) { }

    void insert(std::ostream& o) const
    {
        o << *pa;
    }

    A const *pa;
};

/// Longer-than-singleton comma sequence.
template <class A, class... B>
struct Comma_seq<A, B...>
{
    explicit Comma_seq(A const& ra, B const& ... rbs)
            : pa(std::addressof(ra)),
              bs(rbs ...) { }

    void insert(std::ostream& o) const
    {
        o << *pa << ", " << bs;
    }

    A const *pa;
    Comma_seq<B...> bs;
};


// Provides fallback storage and a constructor for specializations of
// Name_of_type and Name_of_tmpl.
class Name_base
{
public:
    /// Constructs an object for printing a type or template name. Takes a
    /// fallback argument for printing unknown types and templates.
    explicit Name_base(char const *fallback = nullptr)
            : fallback(fallback ? fallback : "?") { }

protected:
    char const *fallback;
};

// Partially specialize this for printing the name of a template.
template <template <class...> class>
struct Name_of_tmpl : detail::Name_base
{
    using Name_base::Name_base;

    void insert(std::ostream& o) const
    {
        o << fallback;
    }
};

template <template <class...> class TEMPLATE>
std::ostream&
operator<<(std::ostream& o, detail::Name_of_tmpl<TEMPLATE> nt)
{
    nt.insert(o);
    return o;
}

}  // end namespace detail


//
// Name_of_tmpl AND Name_of_type SPECIALIZATIONS
//

#define GE211_SPECIALIZE_TYPE_NAME(P, ...)                          \
    struct Name_of_type P : detail::Name_base                       \
    {                                                               \
        using Name_base::Name_base;                                 \
        void insert(std::ostream& o) const { o << __VA_ARGS__; }    \
    }

template <class>
GE211_SPECIALIZE_TYPE_NAME(, fallback);

template <class T>
GE211_SPECIALIZE_TYPE_NAME(<T *>, Name_of_type<T>(fallback) << " *");

template <class T>
GE211_SPECIALIZE_TYPE_NAME(<T const>, Name_of_type<T>(fallback) << " const");

template <template <class...> class M, class... A>
GE211_SPECIALIZE_TYPE_NAME(
        <M<A...>>,
        detail::Name_of_tmpl<M>(fallback)
                << "<"
                << detail::comma_seq(Name_of_type<A>(fallback)...)
                << ">");

#undef GE211_SPECIALIZE_TYPE_NAME


//
// TEMPLATE IMPLEMENTATIONS
//

template <class TYPE>
std::string
name_of_type(char const *fallback)
{
    std::ostringstream o;
    o << Name_of_type<TYPE>(fallback);
    return o.str();
}

template <class TYPE>
std::ostream&
operator<<(std::ostream& o, Name_of_type<TYPE> nt)
{
    nt.insert(o);
    return o;
}

}  // end namespace reflection
}  // end namespace util


//
// REGISTRATION MACROS
//

#define GE211_REGISTER_TYPE_NAME_AS(T, N)           \
    template <>                                     \
    struct util::reflection::Name_of_type<T>        \
            : util::reflection::detail::Name_base   \
    {                                               \
        using Name_base::Name_base;                 \
        void insert(std::ostream& o)                \
        {                                           \
            o << N;                                 \
        }                                           \
    }
#define GE211_REGISTER_TMPL_NAME_AS(T, N)           \
    template <>                                     \
    struct util::reflection::detail::Name_of_tmpl<T>\
            : util::reflection::detail::Name_base   \
    {                                               \
        using Name_base::Name_base;                 \
        void insert(std::ostream& o)                \
        {                                           \
            o << N;                                 \
        }                                           \
    }

#define GE211_REGISTER_TYPE_NAME(T)  GE211_REGISTER_TYPE_NAME_AS(T, #T)
#define GE211_REGISTER_TMPL_NAME(T)  GE211_REGISTER_TMPL_NAME_AS(T, #T)


//
// PRIMITIVE TYPE REGISTRATION
//

GE211_REGISTER_TYPE_NAME(bool);

GE211_REGISTER_TYPE_NAME(char);
GE211_REGISTER_TYPE_NAME(signed char);
GE211_REGISTER_TYPE_NAME(unsigned char);
GE211_REGISTER_TYPE_NAME(char16_t);
GE211_REGISTER_TYPE_NAME(char32_t);
GE211_REGISTER_TYPE_NAME(wchar_t);

GE211_REGISTER_TYPE_NAME(short);
GE211_REGISTER_TYPE_NAME(unsigned short);
GE211_REGISTER_TYPE_NAME(int);
GE211_REGISTER_TYPE_NAME(unsigned int);
GE211_REGISTER_TYPE_NAME(long);
GE211_REGISTER_TYPE_NAME(unsigned long);
GE211_REGISTER_TYPE_NAME(float);
GE211_REGISTER_TYPE_NAME(double);
GE211_REGISTER_TYPE_NAME(long double);

#ifdef LLONG_MAX
GE211_REGISTER_TYPE_NAME(long long);
#endif

#ifdef ULLONG_MAX
GE211_REGISTER_TYPE_NAME(unsigned long long);
#endif


//
// STL TYPE AND TEMPLATE REGISTRATION
//

GE211_REGISTER_TYPE_NAME(std::nullptr_t);

GE211_REGISTER_TYPE_NAME(std::string);
GE211_REGISTER_TYPE_NAME(std::ios);
GE211_REGISTER_TYPE_NAME(std::fstream);
GE211_REGISTER_TYPE_NAME(std::ifstream);
GE211_REGISTER_TYPE_NAME(std::ofstream);
GE211_REGISTER_TYPE_NAME(std::istream);
GE211_REGISTER_TYPE_NAME(std::iostream);
GE211_REGISTER_TYPE_NAME(std::ostream);
GE211_REGISTER_TYPE_NAME(std::stringstream);
GE211_REGISTER_TYPE_NAME(std::istringstream);
GE211_REGISTER_TYPE_NAME(std::ostringstream);
GE211_REGISTER_TYPE_NAME(std::streambuf);
GE211_REGISTER_TYPE_NAME(std::streampos);

GE211_REGISTER_TMPL_NAME(std::basic_string);
GE211_REGISTER_TMPL_NAME(std::basic_ios);
GE211_REGISTER_TMPL_NAME(std::basic_fstream);
GE211_REGISTER_TMPL_NAME(std::basic_ifstream);
GE211_REGISTER_TMPL_NAME(std::basic_ofstream);
GE211_REGISTER_TMPL_NAME(std::basic_istream);
GE211_REGISTER_TMPL_NAME(std::basic_iostream);
GE211_REGISTER_TMPL_NAME(std::basic_ostream);
GE211_REGISTER_TMPL_NAME(std::basic_stringstream);
GE211_REGISTER_TMPL_NAME(std::basic_istringstream);
GE211_REGISTER_TMPL_NAME(std::basic_ostringstream);
GE211_REGISTER_TMPL_NAME(std::basic_streambuf);
GE211_REGISTER_TMPL_NAME(std::fpos);

GE211_REGISTER_TMPL_NAME(std::allocator);
GE211_REGISTER_TMPL_NAME(std::char_traits);
GE211_REGISTER_TMPL_NAME(std::pair);
GE211_REGISTER_TMPL_NAME(std::tuple);
GE211_REGISTER_TMPL_NAME(std::vector);

// Special case for std::vector with default allocator.
template <typename VALUE_TYPE>
struct util::reflection::Name_of_type<std::vector<VALUE_TYPE>>
        : util::reflection::detail::Name_base
{
    using Name_base::Name_base;

    void insert(std::ostream& o)
    {
        o << "std::vector<" << Name_of_type<VALUE_TYPE>() << ">";
    }
};
