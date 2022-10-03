#include "memory_tracker.hpp"
#include "testing.hpp"
#include "tests_common.hpp"

TEMPLATE_LIST_TEST_CASE("observer default constructor", "[construction],[observer]", owner_types) {
    memory_tracker mem_track;

    {
        observer_ptr<TestType> ptr;
        CHECK(instances == 0);
        CHECK(ptr.get() == nullptr);
        CHECK(ptr.expired() == true);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 0);
        }
    }

    CHECK(instances == 0);
    if constexpr (has_stateful_deleter<TestType>) {
        CHECK(instances_deleter == 0);
    }

    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE("observer nullptr constructor", "[construction],[observer]", owner_types) {
    memory_tracker mem_track;

    {
        observer_ptr<TestType> ptr(nullptr);
        CHECK(instances == 0);
        CHECK(ptr.get() == nullptr);
        CHECK(ptr.expired() == true);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 0);
        }
    }

    CHECK(instances == 0);
    if constexpr (has_stateful_deleter<TestType>) {
        CHECK(instances_deleter == 0);
    }

    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};
