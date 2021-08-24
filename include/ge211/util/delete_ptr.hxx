#pragma once

#include <cstdlib>
#include <memory>

namespace util {
namespace pointers {

/// A @ref std::unique_ptr%-like smart pointer that uses a statically-determined
/// deleter, which does not have to be included in the pointer's representation.
template <
        typename OBJECT_TYPE,
        void (*Deleter)(OBJECT_TYPE *),
        bool Delete_Null = false
>
class Delete_ptr
{
public:
    /// Type of the pointed-to object.
    using object_type = OBJECT_TYPE;

    /// Type of the deleter function or functor.
    using deleter_type = decltype(Deleter);

    /// Type of a reference to the pointed-to object.
    using reference = object_type&;

    /// Type of an owned pointer to the pointed-to object.
    using owned_ptr = object_type *;

    /// Type of a borrowed pointer to the pointed-to object.
    using borrowed_ptr = object_type *;

    /// Type of a `std::unique_ptr`, with deleter, to the pointed-to object.
    using unique_ptr = std::unique_ptr<object_type, deleter_type>;

    /// The deleter function.
    static constexpr deleter_type deleter = Deleter;

    /// Whether to delete the null pointer or not.
    static constexpr bool delete_null = Delete_Null;


    /// Constructs the null pointer.
    Delete_ptr() noexcept
            : ptr_(nullptr)
    { }

    /// Constructs a Delete_ptr from a raw, owning pointer.
    explicit Delete_ptr(owned_ptr ptr) noexcept
            : ptr_(ptr)
    { }

    /// Coerces the null pointer into a Delete_ptr.
    Delete_ptr(std::nullptr_t) noexcept
            : ptr_(nullptr)
    { }

    /// Copying is disabled.
    Delete_ptr(Delete_ptr const&) = delete;

    /// Copying is disabled.
    Delete_ptr& operator=(Delete_ptr const&) = delete;

    /// Moving is allowed.
    Delete_ptr(Delete_ptr&& that) noexcept
            : ptr_(that.release())
    { }

    /// Moving is allowed.
    Delete_ptr& operator=(Delete_ptr&& that) noexcept
    {
        delete_it_();
        ptr_ = that.release();
        return *this;
    }

    /// Takes ownership of a raw, owning pointer.
    Delete_ptr& operator=(owned_ptr that) noexcept
    {
        return *this = Delete_ptr(that);
    }

    /// Borrows the managed raw pointer.
    borrowed_ptr get() const noexcept
    {
        return ptr_;
    }

    /// Releases the managed raw pointer, returning ownership to the caller
    /// and setting the managed pointer to null.
    owned_ptr release() noexcept
    {
        return std::exchange(ptr_, nullptr);
    }

    /// Returns a reference to the pointed-to object.
    reference operator*() const
    {
        return *ptr_;
    }

    /// Returns a borrowed pointer to the pointed-to object.
    borrowed_ptr operator->() const noexcept
    {
        return ptr_;
    }

    /// Contextually converts null pointers to `false` and non-null pointers
    /// to `true`.
    explicit operator bool() const noexcept
    {
        return ptr_ != nullptr;
    }

    /// Releases the managed pointer and returns it wrapped as a
    /// @ref std::unique_ptr with the correct deleter.
    explicit operator unique_ptr()&& noexcept
    {
        return {release(), deleter};
    };

    /// Swaps two Delete_ptr%s.
    void
    swap(Delete_ptr& other) noexcept
    {
        std::swap(ptr_, other.ptr_);
    }

    /// Deletes the managed pointer.
    ~Delete_ptr()
    {
        delete_it_();
    }

private:
    void delete_it_() noexcept
    {
        if (delete_null || ptr_) {
            deleter(ptr_);
        }
    }

    owned_ptr ptr_;
};

/// Pointer equality.
template <typename OBJ, void (*DEL)(OBJ *), bool NUL>
bool
operator==(
        Delete_ptr<OBJ, DEL, NUL> const& a,
        Delete_ptr<OBJ, DEL, NUL> const& b)
{
    return a.get() == b.get();
}

/// Pointer disequality.
template <typename OBJ, void (*DEL)(OBJ *), bool NUL>
bool
operator!=(
        Delete_ptr<OBJ, DEL, NUL> const& a,
        Delete_ptr<OBJ, DEL, NUL> const& b)
{
    return a.get() != b.get();
}

}  // end namespace smart_ptrs
}  // end namespace util
