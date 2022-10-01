#include "memory_tracker.hpp"
#include "testing.hpp"
#include "tests_common.hpp"

TEMPLATE_LIST_TEST_CASE("make observable", "[make_observable],[owner]", owner_types) {
    if constexpr (can_use_make_observable<TestType>) {
        memory_tracker mem_track;

        {
            TestType ptr = oup::make_observable<get_object<TestType>, get_policy<TestType>>();
            if constexpr (is_sealed<TestType>) {
                CHECK(mem_track.allocated() == 1u);
            } else {
                CHECK(mem_track.allocated() == 2u);
            }
            CHECK(instances == 1);
            CHECK(ptr->state_ == test_object::state::default_init);
            CHECK(ptr.get() != nullptr);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
                CHECK(ptr.get_deleter().state_ == test_deleter::state::default_init);
            }
        }

        CHECK(instances == 0);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 0);
        }

        CHECK(mem_track.allocated() == 0u);
        CHECK(mem_track.double_delete() == 0u);
    }
};

TEMPLATE_LIST_TEST_CASE(
    "make observable with arguments", "[make_observable],[owner]", owner_types) {
    if constexpr (can_use_make_observable<TestType>) {
        memory_tracker mem_track;

        {
            TestType ptr = oup::make_observable<get_object<TestType>, get_policy<TestType>>(
                test_object::state::special_init);
            if constexpr (is_sealed<TestType>) {
                CHECK(mem_track.allocated() == 1u);
            } else {
                CHECK(mem_track.allocated() == 2u);
            }
            CHECK(instances == 1);
            CHECK(ptr->state_ == test_object::state::special_init);
            CHECK(ptr.get() != nullptr);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
                CHECK(ptr.get_deleter().state_ == test_deleter::state::default_init);
            }
        }

        CHECK(instances == 0);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 0);
        }

        CHECK(mem_track.allocated() == 0u);
        CHECK(mem_track.double_delete() == 0u);
    }
};

TEMPLATE_LIST_TEST_CASE(
    "make observable throw in constructor", "[make_observable],[owner]", owner_types) {
    if constexpr (can_use_make_observable<TestType>) {
        memory_tracker mem_track;

        next_test_object_constructor_throws = true;
        REQUIRE_THROWS_AS(
            (oup::make_observable<get_object<TestType>, get_policy<TestType>>()),
            throw_constructor);

        CHECK(instances == 0);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 0);
        }

        CHECK(mem_track.allocated() == 0u);
        CHECK(mem_track.double_delete() == 0u);
    }
};

TEMPLATE_LIST_TEST_CASE("make observable bad alloc", "[make_observable],[owner]", owner_types) {
    if constexpr (can_use_make_observable<TestType>) {
        memory_tracker mem_track;

        force_next_allocation_failure = true;
        REQUIRE_THROWS_AS(
            (oup::make_observable<get_object<TestType>, get_policy<TestType>>()), std::bad_alloc);

        CHECK(instances == 0);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 0);
        }

        CHECK(mem_track.allocated() == 0u);
        CHECK(mem_track.double_delete() == 0u);
    }
};
