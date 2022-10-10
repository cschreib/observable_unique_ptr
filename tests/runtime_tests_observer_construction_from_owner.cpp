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

            CHECK(ptr.get() == ptr_owner.get());
            CHECK(ptr.expired() == true);
            CHECK_INSTANCES(0, 1);
        }

        CHECK_INSTANCES(0, 1);
    }

    CHECK_NO_LEAKS;
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

            CHECK(ptr.get() == ptr_owner.get());
            CHECK(ptr.expired() == false);
            CHECK_INSTANCES(1, 1);
        }

        CHECK_INSTANCES(1, 1);
    }

    CHECK_NO_LEAKS;
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

                CHECK(ptr.get() == ptr_owner.get());
                CHECK(ptr.expired() == true);
                CHECK_INSTANCES(0, 1);
            }

            CHECK_INSTANCES(0, 1);
        }

        CHECK_NO_LEAKS;
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

                CHECK(ptr.get() == ptr_owner.get());
                CHECK(ptr.expired() == false);
                CHECK_INSTANCES(1, 1);
            }

            CHECK_INSTANCES(1, 1);
        }

        CHECK_NO_LEAKS;
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

                CHECK(ptr.get() == ptr_owner.get());
                CHECK(ptr.expired() == true);
                CHECK_INSTANCES(0, 1);
            }

            CHECK_INSTANCES(0, 1);
        }

        CHECK_NO_LEAKS;
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

                CHECK(ptr.get() == ptr_owner.get());
                CHECK(ptr.expired() == false);
                CHECK_INSTANCES(1, 1);
            }

            CHECK_INSTANCES(1, 1);
        }

        CHECK_NO_LEAKS;
    }
};
