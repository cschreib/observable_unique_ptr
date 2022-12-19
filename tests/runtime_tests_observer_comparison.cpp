#include "memory_tracker.hpp"
#include "testing.hpp"
#include "tests_common.hpp"

TEMPLATE_LIST_TEST_CASE(
    "observer comparison valid vs nullptr", "[comparison][observer]", owner_types) {
    volatile memory_tracker mem_track;

    {
        TestType               ptr = make_pointer_deleter_1<TestType>();
        observer_ptr<TestType> optr{ptr};

        CHECK(optr != nullptr);
        CHECK(!(optr == nullptr));
        CHECK(nullptr != optr);
        CHECK(!(nullptr == optr));
    }

    CHECK_NO_LEAKS;
}

TEMPLATE_LIST_TEST_CASE(
    "observer comparison empty vs nullptr", "[comparison][observer]", owner_types) {
    volatile memory_tracker mem_track;

    {
        observer_ptr<TestType> optr;

        CHECK(optr == nullptr);
        CHECK(!(optr != nullptr));
        CHECK(nullptr == optr);
        CHECK(!(nullptr != optr));
    }

    CHECK_NO_LEAKS;
}

TEMPLATE_LIST_TEST_CASE(
    "observer comparison empty vs empty", "[comparison][observer]", owner_types) {
    volatile memory_tracker mem_track;

    {
        observer_ptr<TestType> optr1;
        observer_ptr<TestType> optr2;

        CHECK(optr1 == optr2);
        CHECK(optr2 == optr1);
        CHECK(!(optr1 != optr2));
        CHECK(!(optr2 != optr1));
    }

    CHECK_NO_LEAKS;
}

TEMPLATE_LIST_TEST_CASE(
    "observer comparison empty vs valid", "[comparison][observer]", owner_types) {
    volatile memory_tracker mem_track;

    {
        observer_ptr<TestType> optr1;
        TestType               ptr2 = make_pointer_deleter_2<TestType>();
        observer_ptr<TestType> optr2{ptr2};

        CHECK(optr1 != optr2);
        CHECK(optr2 != optr1);
        CHECK(!(optr1 == optr2));
        CHECK(!(optr2 == optr1));
    }

    CHECK_NO_LEAKS;
}

TEMPLATE_LIST_TEST_CASE(
    "observer comparison valid vs valid different instance",
    "[comparison][observer]",
    owner_types) {
    volatile memory_tracker mem_track;

    {
        TestType               ptr1 = make_pointer_deleter_1<TestType>();
        observer_ptr<TestType> optr1{ptr1};
        TestType               ptr2 = make_pointer_deleter_2<TestType>();
        observer_ptr<TestType> optr2{ptr2};

        CHECK(optr1 != optr2);
        CHECK(optr2 != optr1);
        CHECK(!(optr1 == optr2));
        CHECK(!(optr2 == optr1));
    }

    CHECK_NO_LEAKS;
}

TEMPLATE_LIST_TEST_CASE(
    "observer comparison valid vs valid same instance", "[comparison][observer]", owner_types) {
    volatile memory_tracker mem_track;

    {
        TestType               ptr = make_pointer_deleter_1<TestType>();
        observer_ptr<TestType> optr1{ptr};
        observer_ptr<TestType> optr2{ptr};

        CHECK(optr1 == optr2);
        CHECK(optr2 == optr1);
        CHECK(!(optr1 != optr2));
        CHECK(!(optr2 != optr1));
    }

    CHECK_NO_LEAKS;
}

TEMPLATE_LIST_TEST_CASE(
    "observer comparison valid vs valid different instance derived",
    "[comparison][observer]",
    owner_types) {
    if constexpr (has_base<TestType>) {
        volatile memory_tracker mem_track;

        {
            TestType                    ptr1 = make_pointer_deleter_1<TestType>();
            observer_ptr<TestType>      optr1{ptr1};
            TestType                    ptr2 = make_pointer_deleter_2<TestType>();
            base_observer_ptr<TestType> optr2{ptr2};

            CHECK(optr1 != optr2);
            CHECK(optr2 != optr1);
            CHECK(!(optr1 == optr2));
            CHECK(!(optr2 == optr1));
        }

        CHECK_NO_LEAKS;
    }
}

TEMPLATE_LIST_TEST_CASE(
    "observer comparison valid vs valid same instance derived",
    "[comparison][observer]",
    owner_types) {
    if constexpr (has_base<TestType>) {
        volatile memory_tracker mem_track;

        {
            TestType                    ptr = make_pointer_deleter_1<TestType>();
            observer_ptr<TestType>      optr1{ptr};
            base_observer_ptr<TestType> optr2{ptr};

            CHECK(optr1 == optr2);
            CHECK(optr2 == optr1);
            CHECK(!(optr1 != optr2));
            CHECK(!(optr2 != optr1));
        }

        CHECK_NO_LEAKS;
    }
}
