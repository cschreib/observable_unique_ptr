#include "catch2_and_overrides.hpp"
#include "memory_tracker.hpp"
#include "tests_common2.hpp"

TEMPLATE_LIST_TEST_CASE("make observable", "[make_observable],[owner]", owner_types) {
    if constexpr (can_use_make_observable<TestType>) {
        memory_tracker mem_track;

        {
            TestType ptr = oup::make_observable<get_object<TestType>, get_policy<TestType>>();
            if constexpr (is_sealed<TestType>) {
                REQUIRE(mem_track.allocated() == 1u);
            } else {
                REQUIRE(mem_track.allocated() == 2u);
            }
            REQUIRE(instances == 1);
            REQUIRE(ptr->state_ == test_object::state::default_init);
            REQUIRE(ptr.get() != nullptr);
            if constexpr (has_stateful_deleter<TestType>) {
                REQUIRE(instances_deleter == 1);
                REQUIRE(ptr.get_deleter().state_ == test_deleter::state::default_init);
            }
        }

        REQUIRE(instances == 0);
        if constexpr (has_stateful_deleter<TestType>) {
            REQUIRE(instances_deleter == 0);
        }

        REQUIRE(mem_track.allocated() == 0u);
        REQUIRE(mem_track.double_delete() == 0u);
    }
}

TEMPLATE_LIST_TEST_CASE(
    "make observable with arguments", "[make_observable],[owner]", owner_types) {
    if constexpr (can_use_make_observable<TestType>) {
        memory_tracker mem_track;

        {
            TestType ptr = oup::make_observable<get_object<TestType>, get_policy<TestType>>(
                test_object::state::special_init);
            if constexpr (is_sealed<TestType>) {
                REQUIRE(mem_track.allocated() == 1u);
            } else {
                REQUIRE(mem_track.allocated() == 2u);
            }
            REQUIRE(instances == 1);
            REQUIRE(ptr->state_ == test_object::state::special_init);
            REQUIRE(ptr.get() != nullptr);
            if constexpr (has_stateful_deleter<TestType>) {
                REQUIRE(instances_deleter == 1);
                REQUIRE(ptr.get_deleter().state_ == test_deleter::state::default_init);
            }
        }

        REQUIRE(instances == 0);
        if constexpr (has_stateful_deleter<TestType>) {
            REQUIRE(instances_deleter == 0);
        }

        REQUIRE(mem_track.allocated() == 0u);
        REQUIRE(mem_track.double_delete() == 0u);
    }
}

TEMPLATE_LIST_TEST_CASE(
    "make observable throw in constructor", "[make_observable],[owner]", owner_types) {
    if constexpr (can_use_make_observable<TestType>) {
        memory_tracker mem_track;

        next_test_object_constructor_throws = true;
        REQUIRE_THROWS_AS(
            (oup::make_observable<get_object<TestType>, get_policy<TestType>>()),
            throw_constructor);

        REQUIRE(instances == 0);
        if constexpr (has_stateful_deleter<TestType>) {
            REQUIRE(instances_deleter == 0);
        }

        REQUIRE(mem_track.allocated() == 0u);
        REQUIRE(mem_track.double_delete() == 0u);
    }
}

TEMPLATE_LIST_TEST_CASE("make observable bad alloc", "[make_observable],[owner]", owner_types) {
    if constexpr (can_use_make_observable<TestType>) {
        memory_tracker mem_track;

        force_next_allocation_failure = true;
        REQUIRE_THROWS_AS(
            (oup::make_observable<get_object<TestType>, get_policy<TestType>>()), std::bad_alloc);

        REQUIRE(instances == 0);
        if constexpr (has_stateful_deleter<TestType>) {
            REQUIRE(instances_deleter == 0);
        }

        REQUIRE(mem_track.allocated() == 0u);
        REQUIRE(mem_track.double_delete() == 0u);
    }
}
