#include "memory_tracker.hpp"
#include "testing.hpp"

TEMPLATE_LIST_TEST_CASE(
    "observer move assignment operator valid to empty", "[assignment][observer]", owner_types) {
    volatile memory_tracker mem_track;

    {
        TestType               ptr_orig = make_pointer_deleter_1<TestType>();
        observer_ptr<TestType> optr_orig{ptr_orig};
        {
            observer_ptr<TestType> optr;
            optr = std::move(optr_orig);

            CHECK(optr.get() == ptr_orig.get());
            CHECK(optr_orig.get() == nullptr);
            CHECK(optr.expired() == false);
            CHECK(optr_orig.expired() == true);
            CHECK_INSTANCES(1, 1);
        }

        CHECK_INSTANCES(1, 1);
    }

    CHECK_NO_LEAKS;
}

TEMPLATE_LIST_TEST_CASE(
    "observer move assignment operator empty to valid", "[assignment][observer]", owner_types) {
    volatile memory_tracker mem_track;

    {
        observer_ptr<TestType> optr_orig;
        {
            TestType               ptr = make_pointer_deleter_1<TestType>();
            observer_ptr<TestType> optr{ptr};
            optr = std::move(optr_orig);

            CHECK(optr.get() == nullptr);
            CHECK(optr_orig.get() == nullptr);
            CHECK(optr.expired() == true);
            CHECK(optr_orig.expired() == true);
            CHECK_INSTANCES(1, 1);
        }

        CHECK_INSTANCES(0, 0);
    }

    CHECK_NO_LEAKS;
}

TEMPLATE_LIST_TEST_CASE(
    "observer move assignment operator empty to empty", "[assignment][observer]", owner_types) {
    volatile memory_tracker mem_track;

    {
        observer_ptr<TestType> optr_orig;
        {
            observer_ptr<TestType> optr;
            optr = std::move(optr_orig);

            CHECK(optr.get() == nullptr);
            CHECK(optr_orig.get() == nullptr);
            CHECK(optr.expired() == true);
            CHECK(optr_orig.expired() == true);
            CHECK_INSTANCES(0, 0);
        }

        CHECK_INSTANCES(0, 0);
    }

    CHECK_NO_LEAKS;
}

TEMPLATE_LIST_TEST_CASE(
    "observer move assignment operator valid to valid", "[assignment][observer]", owner_types) {
    volatile memory_tracker mem_track;

    {
        TestType               ptr_orig = make_pointer_deleter_1<TestType>();
        observer_ptr<TestType> optr_orig{ptr_orig};
        {
            TestType               ptr = make_pointer_deleter_1<TestType>();
            observer_ptr<TestType> optr{ptr};
            optr = std::move(optr_orig);

            CHECK(optr.get() == ptr_orig.get());
            CHECK(optr_orig.get() == nullptr);
            CHECK(optr.expired() == false);
            CHECK(optr_orig.expired() == true);
            CHECK_INSTANCES(2, 2);
        }

        CHECK_INSTANCES(1, 1);
    }

    CHECK_NO_LEAKS;
}

TEMPLATE_LIST_TEST_CASE(
    "observer move assignment converting operator valid to empty",
    "[assignment][observer]",
    owner_types) {
    volatile memory_tracker mem_track;

    if constexpr (has_base<TestType>) {
        {
            TestType               ptr_orig = make_pointer_deleter_1<TestType>();
            observer_ptr<TestType> optr_orig{ptr_orig};
            {
                base_observer_ptr<TestType> optr;
                optr = std::move(optr_orig);

                CHECK(optr.get() == ptr_orig.get());
                CHECK(optr_orig.get() == nullptr);
                CHECK(optr.expired() == false);
                CHECK(optr_orig.expired() == true);
                CHECK_INSTANCES(1, 1);
            }

            CHECK_INSTANCES(1, 1);
        }

        CHECK_NO_LEAKS;
    }
}

TEMPLATE_LIST_TEST_CASE(
    "observer move assignment converting operator empty to valid",
    "[assignment][observer]",
    owner_types) {
    volatile memory_tracker mem_track;

    if constexpr (has_base<TestType>) {
        {
            observer_ptr<TestType> optr_orig;
            {
                TestType                    ptr = make_pointer_deleter_1<TestType>();
                base_observer_ptr<TestType> optr{ptr};
                optr = std::move(optr_orig);

                CHECK(optr.get() == nullptr);
                CHECK(optr_orig.get() == nullptr);
                CHECK(optr.expired() == true);
                CHECK(optr_orig.expired() == true);
                CHECK_INSTANCES(1, 1);
            }

            CHECK_INSTANCES(0, 0);
        }

        CHECK_NO_LEAKS;
    }
}

TEMPLATE_LIST_TEST_CASE(
    "observer move assignment converting operator empty to empty",
    "[assignment][observer]",
    owner_types) {
    volatile memory_tracker mem_track;

    if constexpr (has_base<TestType>) {
        {
            observer_ptr<TestType> optr_orig;
            {
                base_observer_ptr<TestType> optr;
                optr = std::move(optr_orig);

                CHECK(optr.get() == nullptr);
                CHECK(optr_orig.get() == nullptr);
                CHECK(optr.expired() == true);
                CHECK(optr_orig.expired() == true);
                CHECK_INSTANCES(0, 0);
            }

            CHECK_INSTANCES(0, 0);
        }

        CHECK_NO_LEAKS;
    }
}

TEMPLATE_LIST_TEST_CASE(
    "observer move assignment converting operator valid to valid",
    "[assignment][observer]",
    owner_types) {
    volatile memory_tracker mem_track;

    if constexpr (has_base<TestType>) {
        {
            TestType               ptr_orig = make_pointer_deleter_1<TestType>();
            observer_ptr<TestType> optr_orig{ptr_orig};
            {
                TestType                    ptr = make_pointer_deleter_1<TestType>();
                base_observer_ptr<TestType> optr{ptr};
                optr = std::move(optr_orig);

                CHECK(optr.get() == ptr_orig.get());
                CHECK(optr_orig.get() == nullptr);
                CHECK(optr.expired() == false);
                CHECK(optr_orig.expired() == true);
                CHECK_INSTANCES(2, 2);
            }

            CHECK_INSTANCES(1, 1);
        }

        CHECK_NO_LEAKS;
    }
}

TEMPLATE_LIST_TEST_CASE(
    "observer move assignment operator self to self valid", "[assignment][observer]", owner_types) {
    volatile memory_tracker mem_track;

    {
        TestType               ptr = make_pointer_deleter_1<TestType>();
        observer_ptr<TestType> optr{ptr};
        optr = std::move(optr);
        CHECK(optr.get() == nullptr);
        CHECK(optr.expired() == true);
        CHECK_INSTANCES(1, 1);
    }

    CHECK_NO_LEAKS;
}

TEMPLATE_LIST_TEST_CASE(
    "observer move assignment operator self to self empty", "[assignment][observer]", owner_types) {
    volatile memory_tracker mem_track;

    {
        observer_ptr<TestType> optr;
        optr = std::move(optr);
        CHECK(optr.get() == nullptr);
        CHECK(optr.expired() == true);
        CHECK_INSTANCES(0, 0);
    }

    CHECK_NO_LEAKS;
}
