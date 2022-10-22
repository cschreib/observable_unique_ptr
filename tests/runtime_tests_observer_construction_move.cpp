#include "memory_tracker.hpp"
#include "testing.hpp"
#include "tests_common.hpp"

SUITE {
    TEMPLATE_LIST_TEST_CASE(
        "observer move from valid observer constructor", "[construction][observer][from_observer]",
        owner_types) {
        memory_tracker mem_track;

        {
            TestType               ptr_owner = make_pointer_deleter_1<TestType>();
            observer_ptr<TestType> ptr_orig{ptr_owner};
            {
                observer_ptr<TestType> ptr(std::move(ptr_orig));
                CHECK_INSTANCES(1, 1);
                CHECK(ptr.get() != nullptr);
                CHECK(ptr.expired() == false);
                CHECK(ptr_orig.get() == nullptr);
                CHECK(ptr_orig.expired() == true);
            }

            CHECK_INSTANCES(1, 1);
        }

        CHECK_NO_LEAKS;
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE(
        "observer move from empty observer constructor", "[construction][observer][from_observer]",
        owner_types) {
        memory_tracker mem_track;

        {
            observer_ptr<TestType> ptr_orig;
            {
                observer_ptr<TestType> ptr(std::move(ptr_orig));
                CHECK_INSTANCES(0, 0);
                CHECK(ptr.get() == nullptr);
                CHECK(ptr.expired() == true);
                CHECK(ptr_orig.get() == nullptr);
                CHECK(ptr_orig.expired() == true);
            }

            CHECK_INSTANCES(0, 0);
        }

        CHECK_NO_LEAKS;
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE(
        "observer move from valid observer implicit conversion constructor",
        "[construction][observer][from_observer]", owner_types) {
        if constexpr (has_base<TestType>) {
            memory_tracker mem_track;

            {
                TestType               ptr_owner = make_pointer_deleter_1<TestType>();
                observer_ptr<TestType> ptr_orig{ptr_owner};
                {
                    base_observer_ptr<TestType> ptr{std::move(ptr_orig)};
                    CHECK_INSTANCES(1, 1);
                    CHECK(ptr.get() == ptr_owner.get());
                    CHECK(ptr.expired() == false);
                    CHECK(ptr_orig.get() == nullptr);
                    CHECK(ptr_orig.expired() == true);
                }

                CHECK_INSTANCES(1, 1);
            }

            CHECK_NO_LEAKS;
        }
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE(
        "observer move from empty observer implicit conversion constructor",
        "[construction][observer][from_observer]", owner_types) {
        memory_tracker mem_track;

        {
            observer_ptr<TestType> ptr_orig;
            {
                base_observer_ptr<TestType> ptr{std::move(ptr_orig)};

                CHECK(ptr.get() == nullptr);
                CHECK(ptr.expired() == true);
                CHECK(ptr_orig.get() == nullptr);
                CHECK(ptr_orig.expired() == true);
                CHECK_INSTANCES(0, 0);
            }

            CHECK_INSTANCES(0, 0);
        }

        CHECK_NO_LEAKS;
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE(
        "observer move from valid observer explicit conversion constructor",
        "[construction][observer][from_observer]", owner_types) {

        if constexpr (has_base<TestType>) {
            memory_tracker mem_track;

            {
                base_ptr<TestType>          ptr_owner = make_pointer_deleter_1<TestType>();
                base_observer_ptr<TestType> ptr_orig{ptr_owner};
                {
                    observer_ptr<TestType> ptr{
                        std::move(ptr_orig), dynamic_cast<get_object<TestType>*>(ptr_orig.get())};

                    CHECK(ptr.get() == dynamic_cast<get_object<TestType>*>(ptr_owner.get()));
                    CHECK(ptr.expired() == false);
                    CHECK(ptr_orig.get() == nullptr);
                    CHECK(ptr_orig.expired() == true);
                    CHECK_INSTANCES(1, 1);
                }

                CHECK_INSTANCES(1, 1);
            }

            CHECK_NO_LEAKS;
        }
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE(
        "observer move from empty observer explicit conversion constructor",
        "[construction][observer][from_observer]", owner_types) {
        if constexpr (has_base<TestType>) {
            memory_tracker mem_track;

            {
                base_observer_ptr<TestType> ptr_orig;
                {
                    observer_ptr<TestType> ptr{
                        std::move(ptr_orig), static_cast<get_object<TestType>*>(nullptr)};

                    CHECK(ptr.get() == nullptr);
                    CHECK(ptr.expired() == true);
                    CHECK(ptr_orig.get() == nullptr);
                    CHECK(ptr_orig.expired() == true);
                    CHECK_INSTANCES(0, 0);
                }

                CHECK_INSTANCES(0, 0);
            }

            CHECK_NO_LEAKS;
        }
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE(
        "observer move from valid observer explicit conversion constructor with null",
        "[construction][observer][from_observer]", owner_types) {
        if constexpr (has_base<TestType>) {
            memory_tracker mem_track;

            {
                base_ptr<TestType>          ptr_owner = make_pointer_deleter_1<TestType>();
                base_observer_ptr<TestType> ptr_orig{ptr_owner};
                {
                    observer_ptr<TestType> ptr{
                        std::move(ptr_orig), static_cast<get_object<TestType>*>(nullptr)};

                    CHECK(ptr.get() == nullptr);
                    CHECK(ptr.expired() == true);
                    CHECK(ptr_orig.get() == nullptr);
                    CHECK(ptr_orig.expired() == true);
                    CHECK_INSTANCES(1, 1);
                }

                CHECK_INSTANCES(1, 1);
            }

            CHECK_NO_LEAKS;
        }
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE(
        "observer move from valid observer explicit conversion constructor subobject",
        "[construction][observer][from_observer]", owner_types) {
        memory_tracker mem_track;

        {
            TestType               ptr_owner = make_pointer_deleter_1<TestType>();
            observer_ptr<TestType> ptr_orig{ptr_owner};
            {
                state_observer_ptr<TestType> ptr{std::move(ptr_orig), &ptr_owner->state_};

                CHECK(ptr.get() == &ptr_owner->state_);
                CHECK(ptr.expired() == false);
                CHECK(ptr_orig.get() == nullptr);
                CHECK(ptr_orig.expired() == true);
                CHECK_INSTANCES(1, 1);
            }

            CHECK_INSTANCES(1, 1);
        }

        CHECK_NO_LEAKS;
    }
    | owner_types{};
};
