#pragma once

namespace util {
namespace pointers {

/// Like @ref std::unique_ptr, but the pointed-to object is constructed lazily,
/// the first time it is accessed.
template <typename OBJECT_TYPE>
class Lazy_ptr
{
public:
    /// Type of the lazily-initialized value.
    using value_type = OBJECT_TYPE;

    /// Type of a reference to the lazily-initialized value.
    using reference = OBJECT_TYPE&;

    /// Type of a pointer to the lazily-initialized value.
    using pointer = OBJECT_TYPE *;

    /// Constructs a lazy pointer with the given constructor arguments.
    Lazy_ptr()
    { }

    /// Checks whether the value has been constructed yet.
    bool is_forced() const
    {
        return ptr_ != nullptr;
    }

    /// Forces construction and returns a reference to the value.
    reference operator*() const
    {
        force_();
        return *ptr_;
    }

    /// Forces construction and returns a pointer to the value.
    pointer operator->() const
    {
        return std::addressof(operator*());
    }

private:
    mutable std::unique_ptr<value_type> ptr_;

    void force_() const
    {
        if (!ptr_)
            ptr_.reset(new value_type);
    }
};

}  // end namespace smart_ptrs
}  // end namespace util
