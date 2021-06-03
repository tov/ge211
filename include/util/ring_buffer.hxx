#pragma once

#include <array>
#include <cstddef>
#include <type_traits>
#include <utility>

namespace util {
namespace containers {

/// A fixed-sized ring buffer.
template <typename Element, std::size_t Capacity>
class Ring_buffer
{
    using Buffer_ = std::array<Element, Capacity>;

public:
    /// The type of the elements.
    using value_type = Element;

    /// The capacity of the buffer.
    static constexpr std::size_t capacity = Capacity;

    /// Constructs an empty, zero-filled ring buffer.
    constexpr Ring_buffer()
            : Ring_buffer(Element())
    { }

    /// Constructs a ring buffer filled with `fill`.
    explicit constexpr Ring_buffer(Element fill)
    {
        buf_.fill(fill);
    }

    /// Returns the number of elements in the ring buffer.
    constexpr std::size_t
    size() const
    {
        return size_;
    }

    /// Returns whether the ring buffer is empty.
    constexpr bool
    empty() const
    {
        return size() == 0;
    }

    /// Returns whether the ring buffer is full, meaning that enqueueing an
    /// element will dequeue the oldest element to make room.
    constexpr bool
    full() const
    {
        return size() == capacity;
    }

    /// Enqueues an element, returning the ejected oldest element if full, or
    /// `value_type()` if empty.
    constexpr value_type
    rotate(value_type in)
    {
        if (full()) {
            auto result = std::exchange(buf_[start_++], in);
            start_ %= capacity;
            return result;
        } else {
            buf_[size_++] = in;
            return value_type();
        }
    }

private:
    Buffer_ buf_;
    std::size_t start_{0}, size_{0};
};

}  // end namespace ring_buffer
}  // end namespace util
