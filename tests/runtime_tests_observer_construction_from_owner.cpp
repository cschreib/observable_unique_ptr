#include "memory_tracker.hpp"
#include "testing.hpp"
#include "tests_common.hpp"

TEMPLATE_LIST_TEST_CASE(
    "observer from empty owner constructor",
    "[construction],[observer],[from_owner]",
    owner_types) {
    memory_tracker mem_track;

    {
        TestType ptr_owner;
        {
            observer_ptr<TestType> ptr{ptr_owner};
            CHECK(instances == 0);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
            CHECK(ptr.get() == ptr_owner.get());
            CHECK(ptr.expired() == true);
        }

        CHECK(instances == 0);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 1);
        }
    }

    CHECK(instances == 0);
    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE(
    "observer from valid owner constructor",
    "[construction],[observer],[from_owner]",
    owner_types) {
    memory_tracker mem_track;

    {
        TestType ptr_owner = make_pointer_deleter_1<TestType>();
        {
            observer_ptr<TestType> ptr{ptr_owner};
            CHECK(instances == 1);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
            CHECK(ptr.get() == ptr_owner.get());
            CHECK(ptr.expired() == false);
        }

        CHECK(instances == 1);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 1);
        }
    }

    CHECK(instances == 0);
    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE(
    "observer from empty owner conversion constructor",
    "[construction],[observer],[from_owner]",
    owner_types) {
    if constexpr (has_base<TestType>) {
        memory_tracker mem_track;

        {
            TestType ptr_owner;
            {
                base_observer_ptr<TestType> ptr{ptr_owner};
                CHECK(instances == 0);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(instances_deleter == 1);
                }
                CHECK(ptr.get() == ptr_owner.get());
                CHECK(ptr.expired() == true);
            }

            CHECK(instances == 0);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
        }

        CHECK(instances == 0);
        CHECK(mem_track.allocated() == 0u);
        CHECK(mem_track.double_delete() == 0u);
    }
};

TEMPLATE_LIST_TEST_CASE(
    "observer from valid owner conversion constructor",
    "[construction],[observer],[from_owner]",
    owner_types) {
    if constexpr (has_base<TestType>) {
        memory_tracker mem_track;

        {
            TestType ptr_owner = make_pointer_deleter_1<TestType>();
            {
                base_observer_ptr<TestType> ptr{ptr_owner};
                CHECK(instances == 1);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(instances_deleter == 1);
                }
                CHECK(ptr.get() == ptr_owner.get());
                CHECK(ptr.expired() == false);
            }

            CHECK(instances == 1);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
        }

        CHECK(instances == 0);
        CHECK(mem_track.allocated() == 0u);
        CHECK(mem_track.double_delete() == 0u);
    }
};

TEMPLATE_LIST_TEST_CASE(
    "observer from empty owner explicit conversion constructor",
    "[construction],[observer],[from_owner]",
    owner_types) {
    if constexpr (has_base<TestType>) {
        memory_tracker mem_track;

        {
            base_ptr<TestType> ptr_owner;
            {
                observer_ptr<TestType> ptr{ptr_owner, static_cast<get_object<TestType>*>(nullptr)};
                CHECK(instances == 0);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(instances_deleter == 1);
                }
                CHECK(ptr.get() == ptr_owner.get());
                CHECK(ptr.expired() == true);
            }

            CHECK(instances == 0);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
        }

        CHECK(instances == 0);
        CHECK(mem_track.allocated() == 0u);
        CHECK(mem_track.double_delete() == 0u);
    }
};

TEMPLATE_LIST_TEST_CASE(
    "observer from valid owner explicit conversion constructor",
    "[construction],[observer],[from_owner]",
    owner_types) {
    if constexpr (has_base<TestType>) {
        memory_tracker mem_track;

        {
            base_ptr<TestType> ptr_owner = make_pointer_deleter_1<TestType>();
            {
                observer_ptr<TestType> ptr{
                    ptr_owner, dynamic_cast<get_object<TestType>*>(ptr_owner.get())};
                CHECK(instances == 1);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(instances_deleter == 1);
                }
                CHECK(ptr.get() == ptr_owner.get());
                CHECK(ptr.expired() == false);
            }

            CHECK(instances == 1);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
        }

        CHECK(instances == 0);
        CHECK(mem_track.allocated() == 0u);
        CHECK(mem_track.double_delete() == 0u);
    }
};
