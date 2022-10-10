#include "memory_tracker.hpp"
#include "testing.hpp"
#include "tests_common.hpp"

TEMPLATE_LIST_TEST_CASE(
    "observer from owner assignment operator valid to empty",
    "[assignment],[observer]",
    owner_types) {
    memory_tracker mem_track;

    {
        TestType ptr_orig = make_pointer_deleter_1<TestType>();
        {
            observer_ptr<TestType> optr;
            optr = ptr_orig;

            CHECK(optr.get() == ptr_orig.get());
            CHECK(optr.expired() == false);
            CHECK_INSTANCES(1, 1);
        }

        CHECK_INSTANCES(1, 1);
    }

    CHECK_NO_LEAKS;
};

TEMPLATE_LIST_TEST_CASE(
    "observer from owner assignment operator empty to valid",
    "[assignment],[observer]",
    owner_types) {
    memory_tracker mem_track;

    {
        TestType ptr_orig;
        {
            TestType               ptr = make_pointer_deleter_1<TestType>();
            observer_ptr<TestType> optr{ptr};
            optr = ptr_orig;

            CHECK(optr.get() == nullptr);
            CHECK(optr.expired() == true);
            CHECK(ptr_orig.get() == nullptr);
            CHECK_INSTANCES(1, 2);
        }

        CHECK_INSTANCES(0, 1);
    }

    CHECK_NO_LEAKS;
};

TEMPLATE_LIST_TEST_CASE(
    "observer from owner assignment operator empty to empty",
    "[assignment],[observer]",
    owner_types) {
    memory_tracker mem_track;

    {
        TestType ptr_orig;
        {
            observer_ptr<TestType> optr;
            optr = ptr_orig;

            CHECK(optr.get() == nullptr);
            CHECK(optr.expired() == true);
            CHECK(ptr_orig.get() == nullptr);
            CHECK_INSTANCES(0, 1);
        }

        CHECK_INSTANCES(0, 1);
    }

    CHECK_NO_LEAKS;
};

TEMPLATE_LIST_TEST_CASE(
    "observer from owner assignment operator valid to valid",
    "[assignment],[observer]",
    owner_types) {
    memory_tracker mem_track;

    {
        TestType              ptr_orig     = make_pointer_deleter_1<TestType>();
        get_object<TestType>* raw_ptr_orig = ptr_orig.get();
        {
            TestType               ptr     = make_pointer_deleter_1<TestType>();
            get_object<TestType>*  raw_ptr = ptr.get();
            observer_ptr<TestType> optr{ptr};
            optr = ptr_orig;

            CHECK(optr.get() == ptr_orig.get());
            CHECK(optr.expired() == false);
            CHECK(ptr_orig.get() == raw_ptr_orig);
            CHECK(ptr.get() == raw_ptr);
            CHECK_INSTANCES(2, 2);
        }

        CHECK_INSTANCES(1, 1);
    }

    CHECK_NO_LEAKS;
};

TEMPLATE_LIST_TEST_CASE(
    "observer from owner assignment converting operator valid to empty",
    "[assignment],[observer]",
    owner_types) {
    memory_tracker mem_track;

    if constexpr (has_base<TestType>) {
        {
            TestType ptr_orig = make_pointer_deleter_1<TestType>();
            {
                base_observer_ptr<TestType> optr;
                optr = ptr_orig;

                CHECK(optr.get() == ptr_orig.get());
                CHECK(optr.expired() == false);
                CHECK_INSTANCES(1, 1);
            }

            CHECK_INSTANCES(1, 1);
        }

        CHECK_NO_LEAKS;
    }
};

TEMPLATE_LIST_TEST_CASE(
    "observer from owner assignment converting operator empty to valid",
    "[assignment],[observer]",
    owner_types) {
    memory_tracker mem_track;

    if constexpr (has_base<TestType>) {
        {
            TestType ptr_orig;
            {
                TestType                    ptr = make_pointer_deleter_1<TestType>();
                base_observer_ptr<TestType> optr{ptr};
                optr = ptr_orig;

                CHECK(optr.get() == nullptr);
                CHECK(optr.expired() == true);
                CHECK(ptr_orig.get() == nullptr);
                CHECK_INSTANCES(1, 2);
            }

            CHECK_INSTANCES(0, 1);
        }

        CHECK_NO_LEAKS;
    }
};

TEMPLATE_LIST_TEST_CASE(
    "observer from owner assignment converting operator empty to empty",
    "[assignment],[observer]",
    owner_types) {
    memory_tracker mem_track;

    if constexpr (has_base<TestType>) {
        {
            TestType ptr_orig;
            {
                base_observer_ptr<TestType> optr;
                optr = ptr_orig;

                CHECK(optr.get() == nullptr);
                CHECK(optr.expired() == true);
                CHECK(ptr_orig.get() == nullptr);
                CHECK_INSTANCES(0, 1);
            }

            CHECK_INSTANCES(0, 1);
        }

        CHECK_NO_LEAKS;
    }
};

TEMPLATE_LIST_TEST_CASE(
    "observer from owner assignment converting operator valid to valid",
    "[assignment],[observer]",
    owner_types) {
    memory_tracker mem_track;

    if constexpr (has_base<TestType>) {
        {
            TestType              ptr_orig     = make_pointer_deleter_1<TestType>();
            get_object<TestType>* raw_ptr_orig = ptr_orig.get();
            {
                TestType                    ptr     = make_pointer_deleter_1<TestType>();
                get_object<TestType>*       raw_ptr = ptr.get();
                base_observer_ptr<TestType> optr{ptr};
                optr = ptr_orig;

                CHECK(optr.get() == ptr_orig.get());
                CHECK(optr.expired() == false);
                CHECK(ptr_orig.get() == raw_ptr_orig);
                CHECK(ptr.get() == raw_ptr);
                CHECK_INSTANCES(2, 2);
            }

            CHECK_INSTANCES(1, 1);
        }

        CHECK_NO_LEAKS;
    }
};
