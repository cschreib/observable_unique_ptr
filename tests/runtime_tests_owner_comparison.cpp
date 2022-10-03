#include "memory_tracker.hpp"
#include "testing.hpp"
#include "tests_common.hpp"

TEMPLATE_LIST_TEST_CASE("owner comparison valid vs nullptr", "[comparison],[owner]", owner_types) {
    memory_tracker mem_track;

    {
        TestType ptr = make_pointer_deleter_1<TestType>();
        CHECK(ptr != nullptr);
        CHECK(!(ptr == nullptr));
        CHECK(nullptr != ptr);
        CHECK(!(nullptr == ptr));
    }

    CHECK(instances == 0);
    if constexpr (has_stateful_deleter<TestType>) {
        CHECK(instances_deleter == 0);
    }

    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE("owner comparison empty vs nullptr", "[comparison],[owner]", owner_types) {
    memory_tracker mem_track;

    {
        TestType ptr = make_empty_pointer_deleter_1<TestType>();
        CHECK(ptr == nullptr);
        CHECK(!(ptr != nullptr));
        CHECK(nullptr == ptr);
        CHECK(!(nullptr != ptr));
    }

    CHECK(instances == 0);
    if constexpr (has_stateful_deleter<TestType>) {
        CHECK(instances_deleter == 0);
    }

    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE("owner comparison empty vs empty", "[comparison],[owner]", owner_types) {
    memory_tracker mem_track;

    {
        TestType ptr1 = make_empty_pointer_deleter_1<TestType>();
        TestType ptr2 = make_empty_pointer_deleter_2<TestType>();
        CHECK(ptr1 == ptr2);
        CHECK(ptr2 == ptr1);
        CHECK(!(ptr1 != ptr2));
        CHECK(!(ptr2 != ptr1));
    }

    CHECK(instances == 0);
    if constexpr (has_stateful_deleter<TestType>) {
        CHECK(instances_deleter == 0);
    }

    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE("owner comparison empty vs valid", "[comparison],[owner]", owner_types) {
    memory_tracker mem_track;

    {
        TestType ptr1 = make_empty_pointer_deleter_1<TestType>();
        TestType ptr2 = make_pointer_deleter_2<TestType>();
        CHECK(ptr1 != ptr2);
        CHECK(ptr2 != ptr1);
        CHECK(!(ptr1 == ptr2));
        CHECK(!(ptr2 == ptr1));
    }

    CHECK(instances == 0);
    if constexpr (has_stateful_deleter<TestType>) {
        CHECK(instances_deleter == 0);
    }

    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE("owner comparison valid vs valid", "[comparison],[owner]", owner_types) {
    memory_tracker mem_track;

    {
        TestType ptr1 = make_pointer_deleter_1<TestType>();
        TestType ptr2 = make_pointer_deleter_2<TestType>();
        CHECK(ptr1 != ptr2);
        CHECK(ptr2 != ptr1);
        CHECK(!(ptr1 == ptr2));
        CHECK(!(ptr2 == ptr1));
    }

    CHECK(instances == 0);
    if constexpr (has_stateful_deleter<TestType>) {
        CHECK(instances_deleter == 0);
    }

    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};
