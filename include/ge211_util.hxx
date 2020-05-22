#pragma once

#include <cstdlib>
#include <memory>
#include <string>
#include <sstream>
#include <type_traits>

namespace ge211 {

/// Type alias to indicate that the given pointer owns
/// its object.
template<class T>
using Owned = T*;

/// Type alias to indicate that the given pointer does
/// not own its object.
template<class T>
using Borrowed = T*;

/// Converts any printable type to a `std::string`.
template<class T>
std::string to_string(const T& value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

// `detail` is for implementation details
namespace detail {

template<class T>
using deleter_t = void (*)(Owned<T>);

template<class T>
void no_op_deleter(Owned<T>) {}

template<class T>
void c_heap_deleter(Owned<T> o)
{
    std::free(o);
}

template<
        class        T,
        deleter_t<T> Deleter     = &c_heap_deleter,
        bool         Delete_null = false
>
class delete_ptr
{
public:
    using object_type           = T;
    using owned_pointer         = Owned<object_type>;
    using borrowed_pointer      = Borrowed<object_type>;
    using deleter_function_type = deleter_t<object_type>;

    static constexpr deleter_function_type deleter_function = Deleter;
    static constexpr bool                  delete_null_v = Delete_null;

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

    explicit operator std::unique_ptr<object_type, deleter_function_type>()
    && noexcept
    {
        return {release(), deleter_function};
    };

    friend void swap(delete_ptr& a, delete_ptr& b) noexcept
    {
        std::swap(a.ptr_, b.ptr_);
    }

private:
    void delete_it_() noexcept
    {
        if (delete_null_v || ptr_)
            deleter_function(ptr_);
    }

    owned_pointer ptr_;
};

template<class T, deleter_t<T> deleter>
bool operator==(delete_ptr<T, deleter> const& a,
                delete_ptr<T, deleter> const& b)
{
    return a.get() == b.get();
}

template<class T, deleter_t<T> deleter>
bool operator!=(delete_ptr<T, deleter> const& a,
                delete_ptr<T, deleter> const& b)
{
    return !(a == b);
}

template<class T>
class lazy_ptr
{
public:
    using value     = T;
    using reference = T&;
    using pointer   = T*;

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

/// Can type `T` be converted to type `U` without risk of an exception?
template <class T, class U = T>
constexpr bool is_nothrow_convertible()
{
    T t{};
    return noexcept(U(t));
}

/// Can type `T` be compared to itself without risk of an exception?
template<class T>
constexpr bool is_nothrow_comparable()
{
    T t{};
    return noexcept(t == t) && noexcept(t != t);
}

/// Can types `T` and `U` be used for basic arithmetic (addition,
/// subtraction, multiplication) without risk of an exception?
template<class T, class U = T>
constexpr bool has_nothrow_arithmetic()
{
    T t{};
    U u{};
    return noexcept(t + u) && noexcept(t - u) && noexcept(t * u);
}

/// Can types `T` and `U` be used for division without risk of an exception?
template<class T, class U = T>
constexpr bool has_nothrow_division()
{
    T t{};
    U u{};
    return noexcept(t / u);
}

} // end namespace detail

} // end namespace ge211

