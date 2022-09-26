#include "catch2_and_overrides.hpp"
#include "memory_tracker.hpp"
#include "tests_common.hpp"

TEMPLATE_LIST_TEST_CASE(
    "release valid owner with observer", "[lifetime],[release],[owner],[observer]", owner_types) {
    if constexpr (!is_sealed<TestType>) {
        memory_tracker mem_track;

        {
            observer_ptr<TestType> optr;
            {
                TestType ptr     = make_pointer_deleter_1<TestType>();
                auto*    ptr_raw = ptr.get();

                optr = ptr;
                REQUIRE(optr.get() == ptr_raw);
                REQUIRE(!optr.expired());

                auto* ptr_released = ptr.release();
                REQUIRE(ptr_released == ptr_raw);
                REQUIRE(ptr.get() == nullptr);

                if constexpr (has_eoft<TestType>) {
                    REQUIRE(optr.get() != nullptr);
                    REQUIRE(!optr.expired());
                } else {
                    REQUIRE(optr.get() == nullptr);
                    REQUIRE(optr.expired());
                }

                REQUIRE(instances == 1);
                if constexpr (has_stateful_deleter<TestType>) {
                    REQUIRE(instances_deleter == 1);
                    REQUIRE(ptr.get_deleter().state_ == test_deleter::state::special_init_1);
                }

                delete ptr_released;
            }

            REQUIRE(optr.get() == nullptr);
            REQUIRE(optr.expired());
            REQUIRE(instances == 0);
            if constexpr (has_stateful_deleter<TestType>) {
                REQUIRE(instances_deleter == 0);
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
    "release valid owner with observer subobject",
    "[lifetime],[release],[owner],[observer]",
    owner_types) {
    if constexpr (!is_sealed<TestType>) {
        memory_tracker mem_track;

        {
            state_observer_ptr<TestType> optr;
            {
                TestType ptr     = make_pointer_deleter_1<TestType>();
                auto*    ptr_raw = ptr.get();

                optr = state_observer_ptr<TestType>{ptr, &ptr->state_};
                REQUIRE(optr.get() == &ptr_raw->state_);
                REQUIRE(!optr.expired());

                auto* ptr_released = ptr.release();
                REQUIRE(ptr_released == ptr_raw);
                REQUIRE(ptr.get() == nullptr);

                if constexpr (has_eoft<TestType>) {
                    REQUIRE(optr.get() != nullptr);
                    REQUIRE(!optr.expired());
                } else {
                    REQUIRE(optr.get() == nullptr);
                    REQUIRE(optr.expired());
                }

                REQUIRE(instances == 1);
                if constexpr (has_stateful_deleter<TestType>) {
                    REQUIRE(instances_deleter == 1);
                    REQUIRE(ptr.get_deleter().state_ == test_deleter::state::special_init_1);
                }

                delete ptr_released;
            }

            REQUIRE(optr.get() == nullptr);
            REQUIRE(optr.expired());
            REQUIRE(instances == 0);
            if constexpr (has_stateful_deleter<TestType>) {
                REQUIRE(instances_deleter == 0);
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
