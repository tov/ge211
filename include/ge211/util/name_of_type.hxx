#pragma once

#include <ostream>
#include <sstream>
#include <string>

namespace util {
namespace reflection {
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

template <class... A>
std::ostream&
operator<<(std::ostream& o, Comma_seq<A...> cs)
{
    cs.insert(o);
    return o;
}

/// Empty sequence.
template <>
struct Comma_seq<>
{
    void insert(std::ostream&) const { }
};

/// Singleton sequence.
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

/// Longer sequence.
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

/// Partially specialize this for printing the name of a template.
template <template <class...> class>
struct Name_of_tmpl;

}  // end namespace detail

/// Stream-inserting Name_of_type<T>() will print the name of type T.
template <class>
struct Name_of_type;

/// Returns the name of a type as a string. Takes a fallback argument for
/// printing unknown types.
template <class TYPE>
std::string
name_of_type(char const *fallback = nullptr);

template <class TYPE>
std::ostream&
operator<<(std::ostream&, Name_of_type<TYPE>);

template <template <class...> class TEMPLATE>
std::ostream&
operator<<(std::ostream&, detail::Name_of_tmpl<TEMPLATE>);

template <class>
struct Name_of_type : detail::Name_base
{
    using Name_base::Name_base;

    void insert(std::ostream& o) const
    {
        o << fallback;
    }
};

template <template <class...> class>
struct detail::Name_of_tmpl : detail::Name_base
{
    using Name_base::Name_base;

    void insert(std::ostream& o) const
    {
        o << fallback;
    }
};

template <class T>
struct Name_of_type<T *> : detail::Name_base
{
    using Name_base::Name_base;

    void insert(std::ostream& o) const
    {
        o << Name_of_type<T>(fallback)() << " *";
    }
};

template <class T>
struct Name_of_type<T const> : detail::Name_base
{
    using Name_base::Name_base;

    void insert(std::ostream& o) const
    {
        o << Name_of_type<T>(fallback) << " const";
    }
};

template <template <class...> class M, class... A>
struct Name_of_type<M<A...>> : detail::Name_base
{
    using Name_base::Name_base;

    void insert(std::ostream& o) const
    {
        o << detail::Name_of_tmpl<M>(fallback)
          << '<'
          << detail::comma_seq(Name_of_type<A>(fallback)...)
          << '>';
    }
};


#define GE211_DEFINE_TYPE_NAME_AS(T, N)             \
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
#define GE211_DEFINE_TMPL_NAME_AS(T, N)             \
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

#define GE211_DEFINE_TYPE_NAME(T)  GE211_DEFINE_TYPE_NAME_AS(T, #T)
#define GE211_DEFINE_TMPL_NAME(T)  GE211_DEFINE_TMPL_NAME_AS(T, #T)


GE211_DEFINE_TYPE_NAME(bool);
GE211_DEFINE_TYPE_NAME(char);
GE211_DEFINE_TYPE_NAME(signed char);
GE211_DEFINE_TYPE_NAME(unsigned char);
GE211_DEFINE_TYPE_NAME(short);
GE211_DEFINE_TYPE_NAME(unsigned short);
GE211_DEFINE_TYPE_NAME(int);
GE211_DEFINE_TYPE_NAME(unsigned int);
GE211_DEFINE_TYPE_NAME(long);
GE211_DEFINE_TYPE_NAME(unsigned long);
GE211_DEFINE_TYPE_NAME(float);
GE211_DEFINE_TYPE_NAME(double);
GE211_DEFINE_TYPE_NAME(long double);

#ifdef LLONG_MAX
GE211_DEFINE_TYPE_NAME(long long);
#endif

#ifdef ULLONG_MAX
GE211_DEFINE_TYPE_NAME(unsigned long long);
#endif

GE211_DEFINE_TYPE_NAME(std::string);

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

template <template <class...> class TEMPLATE>
std::ostream&
operator<<(std::ostream& o, detail::Name_of_tmpl<TEMPLATE> nt)
{
    nt.insert(o);
    return o;
}

}  // end namespace reflection
}  // end namespace util
