#pragma once

#include "to_string.hxx"

namespace util {
namespace format {

/// Type-erased interface for stream-insertable values. See the constructor,
/// @ref Stringable::Stringable, for an example.
class Stringable
{
    using stringer_type_ = std::function<void(std::ostream&)>;

    stringer_type_ stringer_;

    template <typename... PRINTABLE>
    static stringer_type_
    make_stringer_(PRINTABLE const *... pvalue)
    {
        return [=](std::ostream& o) {
            detail::concat_to(o, *pvalue...);
        };
    }

public:
    /// Constructs an object that, when stream-inserted, prints the given
    /// values.
    ///
    /// \example
    ///
    /// ```
    /// Stringable to_print("default");
    ///
    /// if (want_to_print_an_int) {
    ///     to_print = an_int;
    /// } else if (want_to_print_a_double) {
    ///     to_print = a_double;
    /// }
    ///
    /// std::cout << to_print;
    /// ```
    template <typename... PRINTABLE>
    Stringable(PRINTABLE const& ... value)
            : stringer_(make_stringer_(std::addressof(value)...)) { }

    /// Default-constructs a Stringable object that prints nothing.
    Stringable()
            : stringer_([](auto&) { }) { }

    /// Converts a @ref Stringable to the @ref std::string that it would print.
    explicit operator std::string() const
    {
        return to_string(*this);
    }

    /// Stream insertion for @ref Stringable.
    friend std::ostream&
    operator<<(std::ostream& o, Stringable const& s)
    {
        s.stringer_(o);
        return o;
    }
};

} // end namespace format
} // end namespace util
