#include "memory_tracker.hpp"
#include "testing.hpp"
#include "tests_common.hpp"

SUITE {
    TEMPLATE_LIST_TEST_CASE(
        "observer default constructor", "[construction][observer]", owner_types) {
        memory_tracker mem_track;

        {
            observer_ptr<TestType> ptr;

            CHECK(ptr.get() == nullptr);
            CHECK(ptr.expired() == true);
            CHECK_INSTANCES(0, 0);
        }

        CHECK_NO_LEAKS;
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE(
        "observer nullptr constructor", "[construction][observer]", owner_types) {
        memory_tracker mem_track;

        {
            observer_ptr<TestType> ptr(nullptr);

            CHECK(ptr.get() == nullptr);
            CHECK(ptr.expired() == true);
            CHECK_INSTANCES(0, 0);
        }

        CHECK_NO_LEAKS;
    }
    | owner_types{};
};
