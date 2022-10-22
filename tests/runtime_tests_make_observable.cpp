#include "memory_tracker.hpp"
#include "testing.hpp"
#include "tests_common.hpp"

SUITE {
    TEMPLATE_LIST_TEST_CASE("make observable", "[make_observable][owner]", owner_types) {
        if constexpr (can_use_make_observable<TestType>) {
            memory_tracker mem_track;

            {
                TestType ptr = oup::make_observable<get_object<TestType>, get_policy<TestType>>();

                if constexpr (is_sealed<TestType>) {
                    CHECK(mem_track.allocated() == 1u);
                } else {
                    CHECK(mem_track.allocated() == 2u);
                }
                CHECK(ptr.get() != nullptr);
                CHECK(ptr->state_ == test_object::state::default_init);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(ptr.get_deleter().state_ == test_deleter::state::default_init);
                }
                CHECK_INSTANCES(1, 1);
            }

            CHECK_NO_LEAKS;
        }
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE(
        "make observable with arguments", "[make_observable][owner]", owner_types) {
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
                CHECK(ptr.get() != nullptr);
                CHECK(ptr->state_ == test_object::state::special_init);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(ptr.get_deleter().state_ == test_deleter::state::default_init);
                }
                CHECK_INSTANCES(1, 1);
            }

            CHECK_NO_LEAKS;
        }
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE(
        "make observable throw in constructor", "[make_observable][owner]", owner_types) {
        if constexpr (can_use_make_observable<TestType>) {
            memory_tracker mem_track;

            next_test_object_constructor_throws = true;
            REQUIRE_THROWS_AS(
                (oup::make_observable<get_object<TestType>, get_policy<TestType>>()),
                throw_constructor);

            CHECK_NO_LEAKS;
        }
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE("make observable bad alloc", "[make_observable][owner]", owner_types) {
        if constexpr (can_use_make_observable<TestType>) {
            memory_tracker mem_track;

            force_next_allocation_failure = true;
            REQUIRE_THROWS_AS(
                (oup::make_observable<get_object<TestType>, get_policy<TestType>>()),
                std::bad_alloc);

            CHECK_NO_LEAKS;
        }
    }
    | owner_types{};

    TEST_CASE("make observable unique", "[make_observable][owner]") {
        using TestType = oup::observable_unique_ptr<test_object>;
        memory_tracker mem_track;

        {
            TestType ptr = oup::make_observable_unique<test_object>();

            CHECK(mem_track.allocated() == 2u);
            CHECK(ptr.get() != nullptr);
            CHECK(ptr->state_ == test_object::state::default_init);
            CHECK_INSTANCES(1, 1);
        }

        CHECK_NO_LEAKS;
    };

    TEST_CASE("make observable sealed", "[make_observable][owner]") {
        using TestType = oup::observable_sealed_ptr<test_object>;
        memory_tracker mem_track;

        {
            TestType ptr = oup::make_observable_sealed<test_object>();

            CHECK(mem_track.allocated() == 1u);
            CHECK(ptr.get() != nullptr);
            CHECK(ptr->state_ == test_object::state::default_init);
            CHECK_INSTANCES(1, 1);
        }

        CHECK_NO_LEAKS;
    };
};
