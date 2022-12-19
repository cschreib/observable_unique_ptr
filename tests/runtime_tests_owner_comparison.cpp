#include "memory_tracker.hpp"
#include "testing.hpp"
#include "tests_common.hpp"

TEMPLATE_LIST_TEST_CASE("owner comparison valid vs nullptr", "[comparison][owner]", owner_types) {
    volatile memory_tracker mem_track;

    {
        TestType ptr = make_pointer_deleter_1<TestType>();

        CHECK(ptr != nullptr);
        CHECK(!(ptr == nullptr));
        CHECK(nullptr != ptr);
        CHECK(!(nullptr == ptr));
    }

    CHECK_NO_LEAKS;
}

TEMPLATE_LIST_TEST_CASE("owner comparison empty vs nullptr", "[comparison][owner]", owner_types) {
    volatile memory_tracker mem_track;

    {
        TestType ptr = make_empty_pointer_deleter_1<TestType>();

        CHECK(ptr == nullptr);
        CHECK(!(ptr != nullptr));
        CHECK(nullptr == ptr);
        CHECK(!(nullptr != ptr));
    }

    CHECK_NO_LEAKS;
}

TEMPLATE_LIST_TEST_CASE("owner comparison empty vs empty", "[comparison][owner]", owner_types) {
    volatile memory_tracker mem_track;

    {
        TestType ptr1 = make_empty_pointer_deleter_1<TestType>();
        TestType ptr2 = make_empty_pointer_deleter_2<TestType>();

        CHECK(ptr1 == ptr2);
        CHECK(ptr2 == ptr1);
        CHECK(!(ptr1 != ptr2));
        CHECK(!(ptr2 != ptr1));
    }

    CHECK_NO_LEAKS;
}

TEMPLATE_LIST_TEST_CASE("owner comparison empty vs valid", "[comparison][owner]", owner_types) {
    volatile memory_tracker mem_track;

    {
        TestType ptr1 = make_empty_pointer_deleter_1<TestType>();
        TestType ptr2 = make_pointer_deleter_2<TestType>();

        CHECK(ptr1 != ptr2);
        CHECK(ptr2 != ptr1);
        CHECK(!(ptr1 == ptr2));
        CHECK(!(ptr2 == ptr1));
    }

    CHECK_NO_LEAKS;
}

TEMPLATE_LIST_TEST_CASE("owner comparison valid vs valid", "[comparison][owner]", owner_types) {
    volatile memory_tracker mem_track;

    {
        TestType ptr1 = make_pointer_deleter_1<TestType>();
        TestType ptr2 = make_pointer_deleter_2<TestType>();

        CHECK(ptr1 != ptr2);
        CHECK(ptr2 != ptr1);
        CHECK(!(ptr1 == ptr2));
        CHECK(!(ptr2 == ptr1));
    }

    CHECK_NO_LEAKS;
}
