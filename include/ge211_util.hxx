#pragma once

#include <cstdlib>
#include <memory>
#include <string>
#include <sstream>
#include <type_traits>

namespace ge211 {

/// Type alias to indicate that the given pointer owns
/// its object.
template <typename OBJECT_TYPE>
using Owned = OBJECT_TYPE*;

/// Type alias to indicate that the given pointer does
/// not own its object.
template <typename OBJECT_TYPE>
using Borrowed = OBJECT_TYPE*;

/// Converts any printable type to a `std::string`.
template <typename PRINTABLE>
std::string to_string(const PRINTABLE& value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

// `detail` is for implementation details
namespace detail {

template <typename OBJECT_TYPE>
using deleter_t = void (*)(Owned<OBJECT_TYPE>);

template <typename OBJECT_TYPE>
void no_op_deleter(Owned<OBJECT_TYPE>) {}

template <typename OBJECT_TYPE>
void c_heap_deleter(Owned<OBJECT_TYPE> o)
{
    std::free(o);
}

template <
        typename               OBJECT_TYPE,
        deleter_t<OBJECT_TYPE> Deleter_Fn = &c_heap_deleter,
        bool                   Delete_Null      = false
>
class delete_ptr
{
public:
    using object_type       = OBJECT_TYPE;
    using owned_pointer     = Owned<object_type>;
    using borrowed_pointer  = Borrowed<object_type>;
    using deleter_fn_type   = deleter_t<object_type>;

    static constexpr deleter_fn_type deleter_fn  = Deleter_Fn;
    static constexpr bool            delete_null = Delete_Null;

    delete_ptr() noexcept
            : ptr_(nullptr) { }

    explicit delete_ptr(owned_pointer ptr) noexcept
            : ptr_(ptr) { }

    delete_ptr(std::nullptr_t) noexcept
            : ptr_(nullptr) { }

    delete_ptr(delete_ptr const&) = delete;
    delete_ptr& operator=(delete_ptr const&) = delete;

    delete_ptr(delete_ptr&& that) noexcept
            : ptr_(that.release()) { }

    delete_ptr& operator=(delete_ptr&& that) noexcept
    {
        delete_it_();
        ptr_ = that.release();
        return *this;
    }

    delete_ptr& operator=(owned_pointer that) noexcept
    {
        return *this = delete_ptr(that);
    }

    ~delete_ptr()
    {
        delete_it_();
    }

    owned_pointer release() noexcept
    {
        return std::exchange(ptr_, nullptr);
    }

    borrowed_pointer get() const noexcept
    {
        return ptr_;
    }

    object_type& operator*() const
    {
        return *ptr_;
    }

    borrowed_pointer operator->() const noexcept
    {
        return ptr_;
    }

    explicit operator bool() const noexcept
    {
        return ptr_ != nullptr;
    }

    explicit operator std::unique_ptr<object_type, deleter_fn_type>()
    && noexcept
    {
        return {release(), deleter_fn};
    };

    friend void swap(delete_ptr& a, delete_ptr& b) noexcept
    {
        std::swap(a.ptr_, b.ptr_);
    }

private:
    void delete_it_() noexcept
    {
        if (delete_null || ptr_)
            deleter_fn(ptr_);
    }

    owned_pointer ptr_;
};

template <
        typename                OBJECT_TYPE,
        deleter_t<OBJECT_TYPE>  Deleter_Fn
>
bool operator==(delete_ptr<OBJECT_TYPE, Deleter_Fn> const& a,
                delete_ptr<OBJECT_TYPE, Deleter_Fn> const& b)
{
    return a.get() == b.get();
}

template <
        typename                OBJECT_TYPE,
        deleter_t<OBJECT_TYPE>  Deleter_Fn
>
bool operator!=(delete_ptr<OBJECT_TYPE, Deleter_Fn> const& a,
                delete_ptr<OBJECT_TYPE, Deleter_Fn> const& b)
{
    return !(a == b);
}

template <typename OBJECT_TYPE>
class lazy_ptr
{
public:
    using value     = OBJECT_TYPE;
    using reference = OBJECT_TYPE&;
    using pointer   = OBJECT_TYPE*;

    lazy_ptr()
    { }

    bool is_forced() const
    {
        return ptr_ != nullptr;
    }

    reference operator*() const
    {
        force_();
        return *ptr_;
    }

    pointer operator->() const
    {
        return std::addressof(operator*());
    }

private:
    mutable std::unique_ptr<value> ptr_;

    void force_() const
    {
        if (!ptr_)
            ptr_.reset(new value);
    }
};

/// Can type `FROM_TYPE` be converted to type `TO_TYPE` without risk of
/// an exception?
template <typename FROM_TYPE, typename TO_TYPE = FROM_TYPE>
constexpr bool is_nothrow_convertible()
{
    FROM_TYPE t{};
    return noexcept(TO_TYPE(t));
}

/// Can type `EQ_TYPE` be compared to itself without risk of an exception?
template <typename EQ_TYPE>
constexpr bool is_nothrow_comparable()
{
    EQ_TYPE t{};
    return noexcept(t == t) && noexcept(t != t);
}

/// Can types `ARITH_LEFT` and `ARITH_RIGHT` be used for basic arithmetic
/// (addition, subtraction, multiplication) without risk of an exception?
template <typename ARITH_LEFT, typename ARITH_RIGHT = ARITH_LEFT>
constexpr bool has_nothrow_arithmetic()
{
    ARITH_LEFT t{};
    ARITH_RIGHT u{};
    return noexcept(t + u) && noexcept(t - u) && noexcept(t * u);
}

/// Can types `DIVIDEND` and `DIVISOR` be used for division without
/// risk of an exception?
template <typename DIVIDEND, typename DIVISOR = DIVIDEND>
constexpr bool has_nothrow_division()
{
    DIVIDEND t{};
    DIVISOR u{};
    return noexcept(t / u);
}

/// Template constant for turning some built-in arithmetic types into
/// their names via template specialization.
template <typename>
struct Name_of_type
{
    static constexpr char const* value = "?";
};

template <typename TYPE>
char const* name_of_type()
{
    return Name_of_type<TYPE>::value;
}

#define Specialize_name_of_type(Type) \
    template <> \
    struct Name_of_type<Type> \
    { static constexpr char const* value = #Type; };

Specialize_name_of_type(char)
Specialize_name_of_type(signed char)
Specialize_name_of_type(unsigned char)
Specialize_name_of_type(short)
Specialize_name_of_type(unsigned short)
Specialize_name_of_type(int)
Specialize_name_of_type(unsigned)
Specialize_name_of_type(long)
Specialize_name_of_type(unsigned long)
Specialize_name_of_type(float)
Specialize_name_of_type(double)

#undef Specialize_name_of_type

} // end namespace detail

} // end namespace ge211

