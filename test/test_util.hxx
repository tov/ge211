#pragma once

#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include <ge211/util.hxx>

namespace util_testers {

struct Delete_ptr_tester
{
    static bool deleted;

    static void go(Delete_ptr_tester *)
    {
        deleted = true;
    }
};

bool Delete_ptr_tester::deleted;


struct Lazy_ptr_tester
{
    static bool forced;

    Lazy_ptr_tester()
    {
        forced = true;
    }
};

bool Lazy_ptr_tester::forced;


struct Name_of_type_tester
{
    static constexpr char unknown[] = "<unknown>";

    template <typename T, const char *... O>
    static std::string
    go()
    {
        return util::reflection::name_of_type<T, O...>;
    }

};

constexpr char Name_of_type_tester::unknown[];

}  // end namespace util_testers
