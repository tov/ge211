#pragma once

namespace ge211 {

namespace detail {

class Session
{
public:
    Session();

    ~Session();

    Session(Session&&) = default;
    Session& operator=(Session&&) = default;

    Session(const Session&) = delete;
    Session& operator=(const Session&) = delete;
};

} // end namespace detail

}
