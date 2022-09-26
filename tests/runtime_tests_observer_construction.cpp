#include "catch2_and_overrides.hpp"
#include "memory_tracker.hpp"
#include "tests_common2.hpp"

TEMPLATE_LIST_TEST_CASE("observer default constructor", "[construction],[observer]", owner_types) {
    memory_tracker mem_track;

    {
        observer_ptr<TestType> ptr;
        REQUIRE(instances == 0);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.expired() == true);
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

TEMPLATE_LIST_TEST_CASE("observer nullptr constructor", "[construction],[observer]", owner_types) {
    memory_tracker mem_track;

    {
        observer_ptr<TestType> ptr(nullptr);
        REQUIRE(instances == 0);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.expired() == true);
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

TEMPLATE_LIST_TEST_CASE(
    "observer copy constructor valid", "[construction],[observer]", owner_types) {
    memory_tracker mem_track;

    {
        TestType               ptr_owner = make_pointer_deleter_1<TestType>();
        observer_ptr<TestType> ptr_orig{ptr_owner};
        {
            observer_ptr<TestType> ptr(ptr_orig);
            REQUIRE(ptr.get() != nullptr);
            REQUIRE(ptr.expired() == false);
            REQUIRE(ptr_orig.get() != nullptr);
            REQUIRE(ptr_orig.expired() == false);
            REQUIRE(instances == 1);
            if constexpr (has_stateful_deleter<TestType>) {
                REQUIRE(instances_deleter == 1);
            }
        }

        REQUIRE(instances == 1);
        if constexpr (has_stateful_deleter<TestType>) {
            REQUIRE(instances_deleter == 1);
        }
    }

    REQUIRE(instances == 0);
    if constexpr (has_stateful_deleter<TestType>) {
        REQUIRE(instances_deleter == 0);
    }

    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEMPLATE_LIST_TEST_CASE(
    "observer copy constructor empty", "[construction],[observer]", owner_types) {
    memory_tracker mem_track;

    {
        observer_ptr<TestType> ptr_orig;
        {
            observer_ptr<TestType> ptr(ptr_orig);
            REQUIRE(ptr.get() == nullptr);
            REQUIRE(ptr.expired() == true);
            REQUIRE(instances == 0);
            if constexpr (has_stateful_deleter<TestType>) {
                REQUIRE(instances_deleter == 0);
            }
        }

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

TEMPLATE_LIST_TEST_CASE(
    "observer explicit conversion copy constructor", "[construction],[observer]", owner_types) {
    if constexpr (has_base<TestType>) {
        memory_tracker mem_track;

        {
            base_ptr<TestType>          ptr_owner = make_pointer_deleter_1<TestType>();
            base_observer_ptr<TestType> ptr_orig{ptr_owner};
            {
                observer_ptr<TestType> ptr{
                    ptr_orig, static_cast<get_object<TestType>*>(ptr_orig.get())};
                REQUIRE(ptr.get() == static_cast<get_object<TestType>*>(ptr_owner.get()));
                REQUIRE(ptr.expired() == false);
                REQUIRE(ptr_orig.get() == ptr_owner.get());
                REQUIRE(ptr_orig.expired() == false);
                REQUIRE(instances == 1);
                if constexpr (has_stateful_deleter<TestType>) {
                    REQUIRE(instances_deleter == 1);
                }
            }

            REQUIRE(instances == 1);
            if constexpr (has_stateful_deleter<TestType>) {
                REQUIRE(instances_deleter == 1);
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
    "observer explicit conversion copy constructor null pointer",
    "[construction],[observer]",
    owner_types) {
    memory_tracker mem_track;

    {
        TestType               ptr_owner = make_pointer_deleter_1<TestType>();
        observer_ptr<TestType> ptr_orig{ptr_owner};
        {
            observer_ptr<TestType> ptr{ptr_orig, static_cast<get_object<TestType>*>(nullptr)};
            REQUIRE(ptr.get() == nullptr);
            REQUIRE(ptr.expired() == true);
            REQUIRE(ptr_orig.get() == ptr_owner.get());
            REQUIRE(ptr_orig.expired() == false);
            REQUIRE(instances == 1);
            if constexpr (has_stateful_deleter<TestType>) {
                REQUIRE(instances_deleter == 1);
            }
        }

        REQUIRE(instances == 1);
        if constexpr (has_stateful_deleter<TestType>) {
            REQUIRE(instances_deleter == 1);
        }
    }

    REQUIRE(instances == 0);
    if constexpr (has_stateful_deleter<TestType>) {
        REQUIRE(instances_deleter == 0);
    }

    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEMPLATE_LIST_TEST_CASE(
    "observer explicit conversion copy constructor subobject",
    "[construction],[observer]",
    owner_types) {
    memory_tracker mem_track;

    {
        TestType               ptr_owner = make_pointer_deleter_1<TestType>();
        observer_ptr<TestType> ptr_orig{ptr_owner};
        {
            state_observer_ptr<TestType> ptr{ptr_orig, &ptr_owner->state_};
            REQUIRE(ptr.get() == &ptr_owner->state_);
            REQUIRE(ptr.expired() == false);
            REQUIRE(ptr_orig.get() == ptr_owner.get());
            REQUIRE(ptr_orig.expired() == false);
            REQUIRE(instances == 1);
            if constexpr (has_stateful_deleter<TestType>) {
                REQUIRE(instances_deleter == 1);
            }
        }

        REQUIRE(instances == 1);
        if constexpr (has_stateful_deleter<TestType>) {
            REQUIRE(instances_deleter == 1);
        }
    }

    REQUIRE(instances == 0);
    if constexpr (has_stateful_deleter<TestType>) {
        REQUIRE(instances_deleter == 0);
    }

    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}
