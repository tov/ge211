#pragma once

#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include <ge211/util.hxx>

namespace util_testers {

struct Delete_ptr_tester
{
    static bool deleted;

    static void go(Delete_ptr_tester *)
    { deleted = true; }
};


struct Lazy_ptr_tester
{
    static bool forced;

    Lazy_ptr_tester()
    {
        forced = true;
    }
};

struct Name_of_type_tester
{
    static char const unknown[10];

    template <typename T, const char *... O>
    static std::string
    go()
    {
        return util::reflection::name_of_type<T, O...>;
    }

};

bool Delete_ptr_tester::deleted;
bool Lazy_ptr_tester::forced;
char const Name_of_type_tester::unknown[10] = "<unknown>";

}  // end namespace util_testers
