#include "catch2_and_overrides.hpp"
#include "memory_tracker.hpp"
#include "tests_common.hpp"

// TEMPLATE_LIST_TEST_CASE(
//     "owner comparison valid ptr vs nullptr", "[comparison],[owner]", owner_types) {
//     memory_tracker mem_track;

//     {
//         TestType ptr = make_pointer_deleter_1<TestType>();
//         REQUIRE(ptr != nullptr);
//         REQUIRE(!(ptr == nullptr));
//         REQUIRE(nullptr != ptr);
//         REQUIRE(!(nullptr == ptr));
//     }

//     REQUIRE(instances == 0);
//     if constexpr (has_stateful_deleter<TestType>) {
//         REQUIRE(instances_deleter == 0);
//     }

//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEMPLATE_LIST_TEST_CASE(
//     "owner comparison empty ptr vs nullptr", "[comparison],[owner]", owner_types) {
//     memory_tracker mem_track;

//     {
//         TestType ptr = make_empty_pointer_deleter_1<TestType>();
//         REQUIRE(ptr == nullptr);
//         REQUIRE(!(ptr != nullptr));
//         REQUIRE(nullptr == ptr);
//         REQUIRE(!(nullptr != ptr));
//     }

//     REQUIRE(instances == 0);
//     if constexpr (has_stateful_deleter<TestType>) {
//         REQUIRE(instances_deleter == 0);
//     }

//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEMPLATE_LIST_TEST_CASE(
//     "owner comparison empty ptr vs empty ptr", "[comparison],[owner]", owner_types) {
//     memory_tracker mem_track;

//     {
//         TestType ptr1 = make_empty_pointer_deleter_1<TestType>();
//         TestType ptr2 = make_empty_pointer_deleter_2<TestType>();
//         REQUIRE(ptr1 == ptr2);
//         REQUIRE(ptr2 == ptr1);
//         REQUIRE(!(ptr1 != ptr2));
//         REQUIRE(!(ptr2 != ptr1));
//     }

//     REQUIRE(instances == 0);
//     if constexpr (has_stateful_deleter<TestType>) {
//         REQUIRE(instances_deleter == 0);
//     }

//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEMPLATE_LIST_TEST_CASE(
//     "owner comparison empty ptr vs valid ptr", "[comparison],[owner]", owner_types) {
//     memory_tracker mem_track;

//     {
//         TestType ptr1 = make_empty_pointer_deleter_1<TestType>();
//         TestType ptr2 = make_pointer_deleter_2<TestType>();
//         REQUIRE(ptr1 != ptr2);
//         REQUIRE(ptr2 != ptr1);
//         REQUIRE(!(ptr1 == ptr2));
//         REQUIRE(!(ptr2 == ptr1));
//     }

//     REQUIRE(instances == 0);
//     if constexpr (has_stateful_deleter<TestType>) {
//         REQUIRE(instances_deleter == 0);
//     }

//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEMPLATE_LIST_TEST_CASE(
//     "owner comparison valid ptr vs valid ptr", "[comparison],[owner]", owner_types) {
//     memory_tracker mem_track;

//     {
//         TestType ptr1 = make_pointer_deleter_1<TestType>();
//         TestType ptr2 = make_pointer_deleter_2<TestType>();
//         REQUIRE(ptr1 != ptr2);
//         REQUIRE(ptr2 != ptr1);
//         REQUIRE(!(ptr1 == ptr2));
//         REQUIRE(!(ptr2 == ptr1));
//     }

//     REQUIRE(instances == 0);
//     if constexpr (has_stateful_deleter<TestType>) {
//         REQUIRE(instances_deleter == 0);
//     }

//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }
