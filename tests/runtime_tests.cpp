#include "catch2_and_overrides.hpp"
#include "memory_tracker.hpp"
#include "tests_common.hpp"

// TEST_CASE("owner size", "[owner_size]") {
//     REQUIRE(sizeof(test_ptr) == 2 * sizeof(void*));
// }

// TEST_CASE("owner size sealed", "[owner_size]") {
//     REQUIRE(sizeof(test_sptr) == 2 * sizeof(void*));
// }

// TEST_CASE("owner default constructor", "[owner_construction]") {
//     memory_tracker mem_track;

//     {
//         test_ptr ptr;
//         REQUIRE(instances == 0);
//         REQUIRE(ptr.get() == nullptr);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner default constructor sealed", "[owner_construction]") {
//     memory_tracker mem_track;

//     {
//         test_sptr ptr;
//         REQUIRE(instances == 0);
//         REQUIRE(ptr.get() == nullptr);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner default constructor with deleter", "[owner_construction]") {
//     memory_tracker mem_track;

//     {
//         test_ptr_with_deleter ptr;
//         REQUIRE(instances == 0);
//         REQUIRE(instances_deleter == 1);
//         REQUIRE(ptr.get() == nullptr);
//         REQUIRE(ptr.get_deleter().state_ == 0);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(instances_deleter == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner nullptr constructor", "[owner_construction]") {
//     memory_tracker mem_track;

//     {
//         test_ptr ptr{nullptr};
//         REQUIRE(instances == 0);
//         REQUIRE(ptr.get() == nullptr);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner nullptr constructor sealed", "[owner_construction]") {
//     memory_tracker mem_track;

//     {
//         test_sptr ptr{nullptr};
//         REQUIRE(instances == 0);
//         REQUIRE(ptr.get() == nullptr);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner nullptr constructor with deleter", "[owner_construction]") {
//     memory_tracker mem_track;

//     {
//         test_ptr_with_deleter ptr{nullptr, test_deleter{42}};
//         REQUIRE(instances == 0);
//         REQUIRE(instances_deleter == 1);
//         REQUIRE(ptr.get() == nullptr);
//         REQUIRE(ptr.get_deleter().state_ == 42);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(instances_deleter == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner move constructor", "[owner_construction]") {
//     memory_tracker mem_track;

//     {
//         test_ptr ptr_orig(new test_object);
//         {
//             test_ptr ptr(std::move(ptr_orig));
//             REQUIRE(instances == 1);
//             REQUIRE(ptr.get() != nullptr);
//         }

//         REQUIRE(instances == 0);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner move constructor sealed", "[owner_construction]") {
//     memory_tracker mem_track;

//     {
//         test_sptr ptr_orig = oup::make_observable_sealed<test_object>();
//         {
//             test_sptr ptr(std::move(ptr_orig));
//             REQUIRE(instances == 1);
//             REQUIRE(ptr.get() != nullptr);
//         }

//         REQUIRE(instances == 0);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner move constructor with deleter", "[owner_construction]") {
//     memory_tracker mem_track;

//     {
//         test_ptr_with_deleter ptr_orig(new test_object, test_deleter{42});
//         {
//             test_ptr_with_deleter ptr(std::move(ptr_orig));
//             REQUIRE(instances == 1);
//             REQUIRE(instances_deleter == 2);
//             REQUIRE(ptr.get() != nullptr);
//             REQUIRE(ptr.get_deleter().state_ == 42);
//         }

//         REQUIRE(instances == 0);
//         REQUIRE(instances_deleter == 1);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(instances_deleter == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner acquiring constructor", "[owner_construction]") {
//     memory_tracker mem_track;

//     {
//         test_ptr ptr{new test_object};
//         REQUIRE(instances == 1);
//         REQUIRE(ptr.get() != nullptr);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner acquiring constructor bad alloc", "[owner_construction]") {
//     memory_tracker mem_track;

//     {
//         test_object* raw_ptr = new test_object;
//         try {
//             force_next_allocation_failure = true;
//             test_ptr{raw_ptr};
//         } catch (...) {
//         }
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner acquiring constructor with deleter", "[owner_construction]") {
//     memory_tracker mem_track;

//     {
//         test_ptr_with_deleter ptr{new test_object, test_deleter{42}};
//         REQUIRE(instances == 1);
//         REQUIRE(instances_deleter == 1);
//         REQUIRE(ptr.get() != nullptr);
//         REQUIRE(ptr.get_deleter().state_ == 42);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(instances_deleter == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner acquiring constructor with deleter bad alloc", "[owner_construction]") {
//     memory_tracker mem_track;

//     {
//         test_object* raw_ptr = new test_object;
//         try {
//             force_next_allocation_failure = true;
//             test_ptr_with_deleter{raw_ptr, test_deleter{42}};
//         } catch (...) {
//         }
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner acquiring constructor null", "[owner_construction]") {
//     memory_tracker mem_track;

//     {
//         test_ptr ptr{static_cast<test_object*>(nullptr)};
//         REQUIRE(instances == 0);
//         REQUIRE(ptr.get() == nullptr);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner acquiring constructor null with deleter", "[owner_construction]") {
//     memory_tracker mem_track;

//     {
//         test_ptr_with_deleter ptr{static_cast<test_object*>(nullptr), test_deleter{42}};
//         REQUIRE(instances == 0);
//         REQUIRE(instances_deleter == 1);
//         REQUIRE(ptr.get() == nullptr);
//         REQUIRE(ptr.get_deleter().state_ == 42);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(instances_deleter == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner implicit conversion constructor", "[owner_construction]") {
//     memory_tracker mem_track;

//     {
//         test_ptr_derived ptr_orig{new test_object_derived};
//         {
//             test_ptr ptr(std::move(ptr_orig));
//             REQUIRE(instances == 1);
//             REQUIRE(instances_derived == 1);
//             REQUIRE(ptr.get() != nullptr);
//         }

//         REQUIRE(instances == 0);
//         REQUIRE(instances_derived == 0);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(instances_derived == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner implicit conversion constructor sealed", "[owner_construction]") {
//     memory_tracker mem_track;

//     {
//         test_sptr_derived ptr_orig = oup::make_observable_sealed<test_object_derived>();
//         {
//             test_sptr ptr(std::move(ptr_orig));
//             REQUIRE(instances == 1);
//             REQUIRE(instances_derived == 1);
//             REQUIRE(ptr.get() != nullptr);
//         }

//         REQUIRE(instances == 0);
//         REQUIRE(instances_derived == 0);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(instances_derived == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner implicit conversion constructor with deleter", "[owner_construction]") {
//     memory_tracker mem_track;

//     {
//         test_ptr_derived_with_deleter ptr_orig{new test_object_derived, test_deleter{42}};
//         {
//             test_ptr_with_deleter ptr(std::move(ptr_orig));
//             REQUIRE(instances == 1);
//             REQUIRE(instances_derived == 1);
//             REQUIRE(instances_deleter == 2);
//             REQUIRE(ptr.get() != nullptr);
//             REQUIRE(ptr.get_deleter().state_ == 42);
//         }

//         REQUIRE(instances == 0);
//         REQUIRE(instances_derived == 0);
//         REQUIRE(instances_deleter == 1);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(instances_derived == 0);
//     REQUIRE(instances_deleter == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner explicit conversion constructor", "[owner_construction]") {
//     memory_tracker mem_track;

//     {
//         test_ptr ptr_orig{new test_object_derived};
//         {
//             test_ptr_derived ptr(
//                 std::move(ptr_orig), dynamic_cast<test_object_derived*>(ptr_orig.get()));
//             REQUIRE(instances == 1);
//             REQUIRE(instances_derived == 1);
//             REQUIRE(ptr.get() != nullptr);
//         }

//         REQUIRE(instances == 0);
//         REQUIRE(instances_derived == 0);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(instances_derived == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner explicit conversion constructor sealed", "[owner_construction]") {
//     memory_tracker mem_track;

//     {
//         test_sptr ptr_orig = oup::make_observable_sealed<test_object_derived>();
//         {
//             test_sptr_derived ptr(
//                 std::move(ptr_orig), dynamic_cast<test_object_derived*>(ptr_orig.get()));
//             REQUIRE(instances == 1);
//             REQUIRE(instances_derived == 1);
//             REQUIRE(ptr.get() != nullptr);
//         }

//         REQUIRE(instances == 0);
//         REQUIRE(instances_derived == 0);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(instances_derived == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner explicit conversion constructor with default deleter", "[owner_construction]") {
//     memory_tracker mem_track;

//     {
//         test_ptr_with_deleter ptr_orig{new test_object_derived, test_deleter{42}};
//         {
//             test_ptr_derived_with_deleter ptr(
//                 std::move(ptr_orig), dynamic_cast<test_object_derived*>(ptr_orig.get()));
//             REQUIRE(instances == 1);
//             REQUIRE(instances_derived == 1);
//             REQUIRE(instances_deleter == 2);
//             REQUIRE(ptr.get() != nullptr);
//             REQUIRE(ptr.get_deleter().state_ == 42);
//         }

//         REQUIRE(instances == 0);
//         REQUIRE(instances_derived == 0);
//         REQUIRE(instances_deleter == 1);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(instances_derived == 0);
//     REQUIRE(instances_deleter == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner explicit conversion constructor with custom deleter", "[owner_construction]") {
//     memory_tracker mem_track;

//     {
//         test_ptr_with_deleter ptr_orig{new test_object_derived, test_deleter{42}};
//         {
//             test_ptr_derived_with_deleter ptr(
//                 std::move(ptr_orig), dynamic_cast<test_object_derived*>(ptr_orig.get()),
//                 test_deleter{43});
//             REQUIRE(instances == 1);
//             REQUIRE(instances_derived == 1);
//             REQUIRE(instances_deleter == 2);
//             REQUIRE(ptr.get() != nullptr);
//             REQUIRE(ptr.get_deleter().state_ == 43);
//         }

//         REQUIRE(instances == 0);
//         REQUIRE(instances_derived == 0);
//         REQUIRE(instances_deleter == 1);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(instances_derived == 0);
//     REQUIRE(instances_deleter == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner explicit conversion constructor with nullptr", "[owner_construction]") {
//     memory_tracker mem_track;

//     {
//         test_ptr ptr_orig{new test_object_derived};
//         {
//             test_ptr_derived ptr(std::move(ptr_orig),
//             static_cast<test_object_derived*>(nullptr)); REQUIRE(instances == 0);
//             REQUIRE(instances_derived == 0);
//             REQUIRE(ptr.get() == nullptr);
//         }

//         REQUIRE(instances == 0);
//         REQUIRE(instances_derived == 0);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(instances_derived == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner explicit conversion constructor with nullptr sealed", "[owner_construction]") {
//     memory_tracker mem_track;

//     {
//         test_sptr ptr_orig = oup::make_observable_sealed<test_object_derived>();
//         {
//             test_sptr_derived ptr(std::move(ptr_orig),
//             static_cast<test_object_derived*>(nullptr)); REQUIRE(instances == 0);
//             REQUIRE(instances_derived == 0);
//             REQUIRE(ptr.get() == nullptr);
//         }

//         REQUIRE(instances == 0);
//         REQUIRE(instances_derived == 0);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(instances_derived == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE(
//     "owner explicit conversion constructor with nullptr with custom deleter",
//     "[owner_construction]") {
//     memory_tracker mem_track;

//     {
//         test_ptr_with_deleter ptr_orig{new test_object_derived, test_deleter{42}};
//         {
//             test_ptr_derived_with_deleter ptr(
//                 std::move(ptr_orig), static_cast<test_object_derived*>(nullptr),
//                 test_deleter{43});
//             REQUIRE(instances == 0);
//             REQUIRE(instances_derived == 0);
//             REQUIRE(instances_deleter == 2);
//             REQUIRE(ptr.get() == nullptr);
//             REQUIRE(ptr.get_deleter().state_ == 43);
//         }

//         REQUIRE(instances == 0);
//         REQUIRE(instances_derived == 0);
//         REQUIRE(instances_deleter == 1);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(instances_derived == 0);
//     REQUIRE(instances_deleter == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner move assignment operator valid to empty", "[owner_assignment]") {
//     memory_tracker mem_track;

//     {
//         test_ptr ptr_orig(new test_object);
//         {
//             test_ptr ptr;
//             ptr = std::move(ptr_orig);
//             REQUIRE(instances == 1);
//             REQUIRE(ptr.get() != nullptr);
//         }

//         REQUIRE(instances == 0);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner move assignment operator valid to empty sealed", "[owner_assignment]") {
//     memory_tracker mem_track;

//     {
//         test_sptr ptr_orig = oup::make_observable_sealed<test_object>();
//         {
//             test_sptr ptr;
//             ptr = std::move(ptr_orig);
//             REQUIRE(instances == 1);
//             REQUIRE(ptr.get() != nullptr);
//         }

//         REQUIRE(instances == 0);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner move assignment operator valid to empty with deleter", "[owner_assignment]") {
//     memory_tracker mem_track;

//     {
//         test_ptr_with_deleter ptr_orig(new test_object, test_deleter{42});
//         {
//             test_ptr_with_deleter ptr;
//             ptr = std::move(ptr_orig);
//             REQUIRE(instances == 1);
//             REQUIRE(instances_deleter == 2);
//             REQUIRE(ptr.get() != nullptr);
//             REQUIRE(ptr.get_deleter().state_ == 42);
//         }

//         REQUIRE(instances == 0);
//         REQUIRE(instances_deleter == 1);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(instances_deleter == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner move assignment operator empty to valid", "[owner_assignment]") {
//     memory_tracker mem_track;

//     {
//         test_ptr ptr_orig;
//         {
//             test_ptr ptr(new test_object);
//             ptr = std::move(ptr_orig);
//             REQUIRE(instances == 0);
//             REQUIRE(ptr.get() == nullptr);
//         }

//         REQUIRE(instances == 0);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner move assignment operator empty to valid sealed", "[owner_assignment]") {
//     memory_tracker mem_track;

//     {
//         test_sptr ptr_orig;
//         {
//             test_sptr ptr = oup::make_observable_sealed<test_object>();
//             ptr           = std::move(ptr_orig);
//             REQUIRE(instances == 0);
//             REQUIRE(ptr.get() == nullptr);
//         }

//         REQUIRE(instances == 0);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner move assignment operator empty to valid with deleter", "[owner_assignment]") {
//     memory_tracker mem_track;

//     {
//         test_ptr_with_deleter ptr_orig;
//         {
//             test_ptr_with_deleter ptr(new test_object, test_deleter{42});
//             ptr = std::move(ptr_orig);
//             REQUIRE(instances == 0);
//             REQUIRE(instances_deleter == 2);
//             REQUIRE(ptr.get() == nullptr);
//             REQUIRE(ptr.get_deleter().state_ == 0);
//         }

//         REQUIRE(instances == 0);
//         REQUIRE(instances_deleter == 1);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(instances_deleter == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner move assignment operator empty to empty", "[owner_assignment]") {
//     memory_tracker mem_track;

//     {
//         test_ptr ptr_orig;
//         {
//             test_ptr ptr;
//             ptr = std::move(ptr_orig);
//             REQUIRE(instances == 0);
//             REQUIRE(ptr.get() == nullptr);
//         }

//         REQUIRE(instances == 0);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner move assignment operator empty to empty sealed", "[owner_assignment]") {
//     memory_tracker mem_track;

//     {
//         test_sptr ptr_orig;
//         {
//             test_sptr ptr;
//             ptr = std::move(ptr_orig);
//             REQUIRE(instances == 0);
//             REQUIRE(ptr.get() == nullptr);
//         }

//         REQUIRE(instances == 0);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner move assignment operator empty to empty with deleter", "[owner_assignment]") {
//     memory_tracker mem_track;

//     {
//         test_ptr_with_deleter ptr_orig;
//         {
//             test_ptr_with_deleter ptr;
//             ptr = std::move(ptr_orig);
//             REQUIRE(instances == 0);
//             REQUIRE(instances_deleter == 2);
//             REQUIRE(ptr.get() == nullptr);
//             REQUIRE(ptr.get_deleter().state_ == 0);
//         }

//         REQUIRE(instances == 0);
//         REQUIRE(instances_deleter == 1);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(instances_deleter == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner move assignment operator valid to valid", "[owner_assignment]") {
//     memory_tracker mem_track;

//     {
//         test_ptr ptr_orig(new test_object);
//         {
//             test_ptr ptr(new test_object);
//             ptr = std::move(ptr_orig);
//             REQUIRE(instances == 1);
//             REQUIRE(ptr.get() != nullptr);
//         }

//         REQUIRE(instances == 0);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner move assignment operator valid to valid sealed", "[owner_assignment]") {
//     memory_tracker mem_track;

//     {
//         test_sptr ptr_orig = oup::make_observable_sealed<test_object>();
//         {
//             test_sptr ptr = oup::make_observable_sealed<test_object>();
//             ptr           = std::move(ptr_orig);
//             REQUIRE(instances == 1);
//             REQUIRE(ptr.get() != nullptr);
//         }

//         REQUIRE(instances == 0);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner move assignment operator valid to valid with deleter", "[owner_assignment]") {
//     memory_tracker mem_track;

//     {
//         test_ptr_with_deleter ptr_orig(new test_object, test_deleter{42});
//         {
//             test_ptr_with_deleter ptr(new test_object, test_deleter{43});
//             ptr = std::move(ptr_orig);
//             REQUIRE(instances == 1);
//             REQUIRE(instances_deleter == 2);
//             REQUIRE(ptr.get() != nullptr);
//             REQUIRE(ptr.get_deleter().state_ == 42);
//         }

//         REQUIRE(instances == 0);
//         REQUIRE(instances_deleter == 1);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(instances_deleter == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner move assignment operator self to self", "[owner_assignment]") {
//     memory_tracker mem_track;

//     {
//         test_ptr ptr(new test_object);
//         ptr = std::move(ptr);
//         REQUIRE(instances == 0);
//         REQUIRE(ptr.get() == nullptr);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner move assignment operator self to self sealed", "[owner_assignment]") {
//     memory_tracker mem_track;

//     {
//         test_sptr ptr = oup::make_observable_sealed<test_object>();
//         ptr           = std::move(ptr);
//         REQUIRE(instances == 0);
//         REQUIRE(ptr.get() == nullptr);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner move assignment operator self to self with deleter", "[owner_assignment]") {
//     memory_tracker mem_track;

//     {
//         test_ptr_with_deleter ptr(new test_object, test_deleter{42});
//         ptr = std::move(ptr);
//         REQUIRE(instances == 0);
//         REQUIRE(ptr.get() == nullptr);
//         REQUIRE(instances_deleter == 1);
//         REQUIRE(ptr.get_deleter().state_ == 0);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(instances_deleter == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner move assignment operator self to self empty", "[owner_assignment]") {
//     memory_tracker mem_track;

//     {
//         test_ptr ptr;
//         ptr = std::move(ptr);
//         REQUIRE(instances == 0);
//         REQUIRE(ptr.get() == nullptr);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner move assignment operator self to self empty sealed", "[owner_assignment]") {
//     memory_tracker mem_track;

//     {
//         test_sptr ptr;
//         ptr = std::move(ptr);
//         REQUIRE(instances == 0);
//         REQUIRE(ptr.get() == nullptr);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner move assignment operator self to self empty with deleter", "[owner_assignment]")
// {
//     memory_tracker mem_track;

//     {
//         test_ptr_with_deleter ptr;
//         ptr = std::move(ptr);
//         REQUIRE(instances == 0);
//         REQUIRE(ptr.get() == nullptr);
//         REQUIRE(instances_deleter == 1);
//         REQUIRE(ptr.get_deleter().state_ == 0);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(instances_deleter == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner comparison valid ptr vs nullptr", "[owner_comparison]") {
//     memory_tracker mem_track;

//     {
//         test_ptr ptr(new test_object);
//         REQUIRE(ptr != nullptr);
//         REQUIRE(!(ptr == nullptr));
//         REQUIRE(nullptr != ptr);
//         REQUIRE(!(nullptr == ptr));
//     }

//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner comparison valid ptr vs nullptr sealed", "[owner_comparison]") {
//     memory_tracker mem_track;

//     {
//         test_sptr ptr = oup::make_observable_sealed<test_object>();
//         REQUIRE(ptr != nullptr);
//         REQUIRE(!(ptr == nullptr));
//         REQUIRE(nullptr != ptr);
//         REQUIRE(!(nullptr == ptr));
//     }

//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner comparison valid ptr vs nullptr with deleter", "[owner_comparison]") {
//     memory_tracker mem_track;

//     {
//         test_ptr_with_deleter ptr(new test_object, test_deleter{42});
//         REQUIRE(ptr != nullptr);
//         REQUIRE(!(ptr == nullptr));
//         REQUIRE(nullptr != ptr);
//         REQUIRE(!(nullptr == ptr));
//     }

//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner comparison invalid ptr vs nullptr", "[owner_comparison]") {
//     memory_tracker mem_track;

//     {
//         test_ptr ptr;
//         REQUIRE(ptr == nullptr);
//         REQUIRE(!(ptr != nullptr));
//         REQUIRE(nullptr == ptr);
//         REQUIRE(!(nullptr != ptr));
//     }

//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner comparison invalid ptr vs nullptr sealed", "[owner_comparison]") {
//     memory_tracker mem_track;

//     {
//         test_sptr ptr;
//         REQUIRE(ptr == nullptr);
//         REQUIRE(!(ptr != nullptr));
//         REQUIRE(nullptr == ptr);
//         REQUIRE(!(nullptr != ptr));
//     }

//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner comparison invalid ptr vs nullptr with deleter", "[owner_comparison]") {
//     memory_tracker mem_track;

//     {
//         test_ptr_with_deleter ptr;
//         REQUIRE(ptr == nullptr);
//         REQUIRE(!(ptr != nullptr));
//         REQUIRE(nullptr == ptr);
//         REQUIRE(!(nullptr != ptr));
//     }

//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner comparison invalid ptr vs nullptr with deleter explicit", "[owner_comparison]")
// {
//     memory_tracker mem_track;

//     {
//         test_ptr_with_deleter ptr(nullptr, test_deleter{42});
//         REQUIRE(ptr == nullptr);
//         REQUIRE(!(ptr != nullptr));
//         REQUIRE(nullptr == ptr);
//         REQUIRE(!(nullptr != ptr));
//     }

//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner comparison invalid ptr vs invalid ptr", "[owner_comparison]") {
//     memory_tracker mem_track;

//     {
//         test_ptr ptr1;
//         test_ptr ptr2;
//         REQUIRE(ptr1 == ptr2);
//         REQUIRE(!(ptr1 != ptr2));
//     }

//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner comparison invalid ptr vs invalid ptr sealed", "[owner_comparison]") {
//     memory_tracker mem_track;

//     {
//         test_sptr ptr1;
//         test_sptr ptr2;
//         REQUIRE(ptr1 == ptr2);
//         REQUIRE(!(ptr1 != ptr2));
//     }

//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner comparison invalid ptr vs invalid ptr with deleter", "[owner_comparison]") {
//     memory_tracker mem_track;

//     {
//         test_ptr_with_deleter ptr1;
//         test_ptr_with_deleter ptr2;
//         REQUIRE(ptr1 == ptr2);
//         REQUIRE(!(ptr1 != ptr2));
//     }

//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE(
//     "owner comparison invalid ptr vs invalid ptr with deleter explicit", "[owner_comparison]") {
//     memory_tracker mem_track;

//     {
//         test_ptr_with_deleter ptr1;
//         test_ptr_with_deleter ptr2(nullptr, test_deleter{42});
//         REQUIRE(ptr1 == ptr2);
//         REQUIRE(ptr2 == ptr1);
//         REQUIRE(!(ptr2 != ptr1));
//         REQUIRE(!(ptr2 != ptr1));
//     }

//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE(
//     "owner comparison invalid ptr vs invalid ptr with both deleter explicit",
//     "[owner_comparison]") {
//     memory_tracker mem_track;

//     {
//         test_ptr_with_deleter ptr1(nullptr, test_deleter{43});
//         test_ptr_with_deleter ptr2(nullptr, test_deleter{42});
//         REQUIRE(ptr1 == ptr2);
//         REQUIRE(ptr2 == ptr1);
//         REQUIRE(!(ptr2 != ptr1));
//         REQUIRE(!(ptr2 != ptr1));
//     }

//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner comparison invalid ptr vs valid ptr", "[owner_comparison]") {
//     memory_tracker mem_track;

//     {
//         test_ptr ptr1;
//         test_ptr ptr2(new test_object);
//         REQUIRE(ptr1 != ptr2);
//         REQUIRE(!(ptr1 == ptr2));
//         REQUIRE(ptr2 != ptr1);
//         REQUIRE(!(ptr2 == ptr1));
//     }

//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner comparison invalid ptr vs valid ptr sealed", "[owner_comparison]") {
//     memory_tracker mem_track;

//     {
//         test_sptr ptr1;
//         test_sptr ptr2 = oup::make_observable_sealed<test_object>();
//         REQUIRE(ptr1 != ptr2);
//         REQUIRE(!(ptr1 == ptr2));
//         REQUIRE(ptr2 != ptr1);
//         REQUIRE(!(ptr2 == ptr1));
//     }

//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner comparison invalid ptr vs valid ptr with deleter", "[owner_comparison]") {
//     memory_tracker mem_track;

//     {
//         test_ptr_with_deleter ptr1;
//         test_ptr_with_deleter ptr2(new test_object, test_deleter{42});
//         REQUIRE(ptr1 != ptr2);
//         REQUIRE(!(ptr1 == ptr2));
//         REQUIRE(ptr2 != ptr1);
//         REQUIRE(!(ptr2 == ptr1));
//     }

//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner comparison invalid ptr vs valid ptr with deleter explicit",
// "[owner_comparison]") {
//     memory_tracker mem_track;

//     {
//         test_ptr_with_deleter ptr1(nullptr, test_deleter{43});
//         test_ptr_with_deleter ptr2(new test_object, test_deleter{42});
//         REQUIRE(ptr1 != ptr2);
//         REQUIRE(!(ptr1 == ptr2));
//         REQUIRE(ptr2 != ptr1);
//         REQUIRE(!(ptr2 == ptr1));
//     }

//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner comparison valid ptr vs valid ptr", "[owner_comparison]") {
//     memory_tracker mem_track;

//     {
//         test_ptr ptr1(new test_object);
//         test_ptr ptr2(new test_object);
//         REQUIRE(ptr1 != ptr2);
//         REQUIRE(!(ptr1 == ptr2));
//         REQUIRE(ptr2 != ptr1);
//         REQUIRE(!(ptr2 == ptr1));
//     }

//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner comparison valid ptr vs valid ptr sealed", "[owner_comparison]") {
//     memory_tracker mem_track;

//     {
//         test_sptr ptr1 = oup::make_observable_sealed<test_object>();
//         test_sptr ptr2 = oup::make_observable_sealed<test_object>();
//         REQUIRE(ptr1 != ptr2);
//         REQUIRE(!(ptr1 == ptr2));
//         REQUIRE(ptr2 != ptr1);
//         REQUIRE(!(ptr2 == ptr1));
//     }

//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner comparison valid ptr vs valid ptr with deleter", "[owner_comparison]") {
//     memory_tracker mem_track;

//     {
//         test_ptr_with_deleter ptr1(new test_object, test_deleter{43});
//         test_ptr_with_deleter ptr2(new test_object, test_deleter{42});
//         REQUIRE(ptr1 != ptr2);
//         REQUIRE(!(ptr1 == ptr2));
//         REQUIRE(ptr2 != ptr1);
//         REQUIRE(!(ptr2 == ptr1));
//     }

//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner reset to null", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_ptr ptr(new test_object);
//         ptr.reset();
//         REQUIRE(instances == 0);
//         REQUIRE(ptr.get() == nullptr);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner reset to null sealed", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_sptr ptr = oup::make_observable_sealed<test_object>();
//         ptr.reset();
//         REQUIRE(instances == 0);
//         REQUIRE(ptr.get() == nullptr);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner reset to null with deleter", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_ptr_with_deleter ptr(new test_object, test_deleter{42});
//         ptr.reset();
//         REQUIRE(instances == 0);
//         REQUIRE(instances_deleter == 1);
//         REQUIRE(ptr.get() == nullptr);
//         REQUIRE(ptr.get_deleter().state_ == 42);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(instances_deleter == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner reset to new", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_ptr ptr(new test_object);
//         ptr.reset(new test_object);
//         REQUIRE(instances == 1);
//         REQUIRE(ptr.get() != nullptr);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner reset to new bad alloc", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_object* raw_ptr1 = new test_object;
//         test_object* raw_ptr2 = new test_object;
//         test_ptr     ptr(raw_ptr1);
//         try {
//             force_next_allocation_failure = true;
//             ptr.reset(raw_ptr2);
//         } catch (...) {
//         }
//         REQUIRE(instances == 1);
//         REQUIRE(ptr.get() == raw_ptr1);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner reset to new with deleter", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_ptr_with_deleter ptr(new test_object, test_deleter{42});
//         ptr.reset(new test_object);
//         REQUIRE(instances == 1);
//         REQUIRE(instances_deleter == 1);
//         REQUIRE(ptr.get() != nullptr);
//         REQUIRE(ptr.get_deleter().state_ == 42);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(instances_deleter == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner swap no instance", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_ptr ptr_orig;
//         test_ptr ptr;
//         ptr.swap(ptr_orig);
//         REQUIRE(instances == 0);
//         REQUIRE(ptr_orig.get() == nullptr);
//         REQUIRE(ptr.get() == nullptr);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner swap no instance sealed", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_sptr ptr_orig;
//         test_sptr ptr;
//         ptr.swap(ptr_orig);
//         REQUIRE(instances == 0);
//         REQUIRE(ptr_orig.get() == nullptr);
//         REQUIRE(ptr.get() == nullptr);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner swap no instance with deleter", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_ptr_with_deleter ptr_orig(nullptr, test_deleter{42});
//         test_ptr_with_deleter ptr(nullptr, test_deleter{43});
//         ptr.swap(ptr_orig);
//         REQUIRE(instances == 0);
//         REQUIRE(instances_deleter == 2);
//         REQUIRE(ptr_orig.get() == nullptr);
//         REQUIRE(ptr.get() == nullptr);
//         REQUIRE(ptr.get_deleter().state_ == 42);
//         REQUIRE(ptr_orig.get_deleter().state_ == 43);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(instances_deleter == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner swap one instance", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_ptr ptr_orig(new test_object);
//         test_ptr ptr;
//         ptr.swap(ptr_orig);
//         REQUIRE(instances == 1);
//         REQUIRE(ptr_orig.get() == nullptr);
//         REQUIRE(ptr.get() != nullptr);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner swap one instance sealed", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_sptr ptr_orig = oup::make_observable_sealed<test_object>();
//         test_sptr ptr;
//         ptr.swap(ptr_orig);
//         REQUIRE(instances == 1);
//         REQUIRE(ptr_orig.get() == nullptr);
//         REQUIRE(ptr.get() != nullptr);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner swap one instance with deleter", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_ptr_with_deleter ptr_orig(new test_object, test_deleter{42});
//         test_ptr_with_deleter ptr(nullptr, test_deleter{43});
//         ptr.swap(ptr_orig);
//         REQUIRE(instances == 1);
//         REQUIRE(instances_deleter == 2);
//         REQUIRE(ptr_orig.get() == nullptr);
//         REQUIRE(ptr.get() != nullptr);
//         REQUIRE(ptr.get_deleter().state_ == 42);
//         REQUIRE(ptr_orig.get_deleter().state_ == 43);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(instances_deleter == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner swap two instances", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_ptr     ptr_orig(new test_object);
//         test_object* ptr_orig_raw = ptr_orig.get();
//         test_ptr     ptr(new test_object);
//         test_object* ptr_raw = ptr.get();
//         ptr.swap(ptr_orig);
//         REQUIRE(instances == 2);
//         REQUIRE(ptr_orig.get() == ptr_raw);
//         REQUIRE(ptr.get() == ptr_orig_raw);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner swap two instances sealed", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_sptr    ptr_orig     = oup::make_observable_sealed<test_object>();
//         test_object* ptr_orig_raw = ptr_orig.get();
//         test_sptr    ptr          = oup::make_observable_sealed<test_object>();
//         test_object* ptr_raw      = ptr.get();
//         ptr.swap(ptr_orig);
//         REQUIRE(instances == 2);
//         REQUIRE(ptr_orig.get() == ptr_raw);
//         REQUIRE(ptr.get() == ptr_orig_raw);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner swap two instances with deleter", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_ptr_with_deleter ptr_orig(new test_object, test_deleter{42});
//         test_object*          ptr_orig_raw = ptr_orig.get();
//         test_ptr_with_deleter ptr(new test_object, test_deleter{43});
//         test_object*          ptr_raw = ptr.get();
//         ptr.swap(ptr_orig);
//         REQUIRE(instances == 2);
//         REQUIRE(instances_deleter == 2);
//         REQUIRE(ptr_orig.get() == ptr_raw);
//         REQUIRE(ptr.get() == ptr_orig_raw);
//         REQUIRE(ptr.get_deleter().state_ == 42);
//         REQUIRE(ptr_orig.get_deleter().state_ == 43);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(instances_deleter == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner swap self", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_ptr ptr(new test_object);
//         ptr.swap(ptr);
//         REQUIRE(instances == 1);
//         REQUIRE(ptr.get() != nullptr);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner swap self sealed", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_sptr ptr = oup::make_observable_sealed<test_object>();
//         ptr.swap(ptr);
//         REQUIRE(instances == 1);
//         REQUIRE(ptr.get() != nullptr);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner swap self with deleter", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_ptr_with_deleter ptr(new test_object, test_deleter{43});
//         ptr.swap(ptr);
//         REQUIRE(instances == 1);
//         REQUIRE(ptr.get() != nullptr);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(instances_deleter == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner dereference", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_ptr ptr(new test_object);
//         REQUIRE(ptr->state_ == 1337);
//         REQUIRE((*ptr).state_ == 1337);
//     }

//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner dereference sealed", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_sptr ptr = oup::make_observable_sealed<test_object>();
//         REQUIRE(ptr->state_ == 1337);
//         REQUIRE((*ptr).state_ == 1337);
//     }

//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner operator bool valid", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_ptr ptr(new test_object);
//         if (ptr) {
//         } else
//             FAIL("if (ptr) should have been true");
//     }

//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner operator bool valid sealed", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_sptr ptr = oup::make_observable_sealed<test_object>();
//         if (ptr) {
//         } else
//             FAIL("if (ptr) should have been true");
//     }

//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner operator bool invalid", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_ptr ptr;
//         if (ptr)
//             FAIL("if (ptr) should not have been true");
//     }

//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner operator bool invalid sealed", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_sptr ptr;
//         if (ptr)
//             FAIL("if (ptr) should not have been true");
//     }

//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner release valid", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_ptr     ptr(new test_object);
//         test_object* ptr_raw = ptr.release();
//         REQUIRE(ptr_raw != nullptr);
//         REQUIRE(ptr.get() == nullptr);
//         REQUIRE(instances == 1);
//         delete ptr_raw;
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner release valid with observer", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_optr optr;
//         {
//             test_ptr ptr(new test_object);
//             optr                 = ptr;
//             test_object* ptr_raw = ptr.release();
//             REQUIRE(ptr_raw != nullptr);
//             REQUIRE(ptr.get() == nullptr);
//             REQUIRE(instances == 1);
//             delete ptr_raw;
//         }

//         REQUIRE(optr.expired());
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner release valid from make_observable_unique", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_ptr     ptr     = oup::make_observable_unique<test_object>();
//         test_object* ptr_raw = ptr.release();
//         REQUIRE(ptr_raw != nullptr);
//         REQUIRE(ptr.get() == nullptr);
//         REQUIRE(instances == 1);
//         delete ptr_raw;
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner release valid from make_observable_unique with observer", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_optr optr;
//         {
//             test_ptr ptr         = oup::make_observable_unique<test_object>();
//             optr                 = ptr;
//             test_object* ptr_raw = ptr.release();
//             REQUIRE(ptr_raw != nullptr);
//             REQUIRE(ptr.get() == nullptr);
//             REQUIRE(instances == 1);
//             delete ptr_raw;
//         }

//         REQUIRE(optr.expired());
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner release valid with deleter", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_ptr_with_deleter ptr(new test_object, test_deleter{42});
//         test_object*          ptr_raw = ptr.release();
//         REQUIRE(ptr_raw != nullptr);
//         REQUIRE(ptr.get() == nullptr);
//         REQUIRE(instances == 1);
//         REQUIRE(instances_deleter == 1);
//         REQUIRE(ptr.get_deleter().state_ == 42);
//         delete ptr_raw;
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(instances_deleter == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner release valid with deleter with observer", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_optr optr;
//         {
//             test_ptr_with_deleter ptr(new test_object, test_deleter{42});
//             optr                 = ptr;
//             test_object* ptr_raw = ptr.release();
//             REQUIRE(ptr_raw != nullptr);
//             REQUIRE(ptr.get() == nullptr);
//             REQUIRE(instances == 1);
//             REQUIRE(instances_deleter == 1);
//             REQUIRE(ptr.get_deleter().state_ == 42);
//             delete ptr_raw;
//         }

//         REQUIRE(optr.expired());
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(instances_deleter == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner release invalid", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_ptr ptr;
//         REQUIRE(ptr.release() == nullptr);
//         REQUIRE(instances == 0);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("owner release invalid with deleter", "[owner_utility]") {
//     memory_tracker mem_track;

//     {
//         test_ptr_with_deleter ptr;
//         REQUIRE(ptr.release() == nullptr);
//         REQUIRE(instances == 0);
//         REQUIRE(instances_deleter == 1);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(instances_deleter == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("make observable unique", "[make_observable_unique]") {
//     memory_tracker mem_track;

//     {
//         test_ptr ptr = oup::make_observable_unique<test_object>();
//         REQUIRE(instances == 1);
//         REQUIRE(ptr.get() != nullptr);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("make observable unique throw in constructor", "[make_observable_unique]") {
//     memory_tracker mem_track;

//     REQUIRE_THROWS_AS(oup::make_observable_unique<test_object_thrower>(), throw_constructor);

//     REQUIRE(instances_thrower == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("make observable non virtual unique throw in constructor", "[make_observable_unique]")
// {
//     memory_tracker mem_track;

//     REQUIRE_THROWS_AS(
//         (oup::make_observable<
//             test_object_thrower_observer_from_this_non_virtual_unique,
//             unique_non_virtual_policy>()),
//         throw_constructor);

//     REQUIRE(instances_thrower == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("make observable sealed", "[make_observable_sealed]") {
//     memory_tracker mem_track;

//     {
//         test_sptr ptr = oup::make_observable_sealed<test_object>();
//         REQUIRE(instances == 1);
//         REQUIRE(ptr.get() != nullptr);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("make observable sealed throw in constructor", "[make_observable_sealed]") {
//     memory_tracker mem_track;

//     REQUIRE_THROWS_AS(oup::make_observable_sealed<test_object_thrower>(), throw_constructor);

//     REQUIRE(instances_thrower == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("observer size", "[observer_size]") {
//     REQUIRE(sizeof(test_optr) == 2 * sizeof(void*));
// }

// TEST_CASE("observer default constructor", "[observer_construction]") {
//     memory_tracker mem_track;

//     {
//         test_optr ptr;
//         REQUIRE(instances == 0);
//         REQUIRE(ptr.get() == nullptr);
//         REQUIRE(ptr.expired() == true);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("observer nullptr constructor", "[observer_construction]") {
//     memory_tracker mem_track;

//     {
//         test_optr ptr{nullptr};
//         REQUIRE(instances == 0);
//         REQUIRE(ptr.get() == nullptr);
//         REQUIRE(ptr.expired() == true);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("observer copy constructor", "[observer_construction]") {
//     memory_tracker mem_track;

//     {
//         test_ptr  ptr_owner{new test_object};
//         test_optr ptr_orig{ptr_owner};
//         {
//             test_optr ptr(ptr_orig);
//             REQUIRE(instances == 1);
//             REQUIRE(ptr.get() != nullptr);
//             REQUIRE(ptr.expired() == false);
//             REQUIRE(ptr_orig.get() != nullptr);
//             REQUIRE(ptr_orig.expired() == false);

//             ptr_owner.reset();
//             REQUIRE(ptr.get() == nullptr);
//             REQUIRE(ptr.expired() == true);
//         }

//         REQUIRE(instances == 0);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("observer explicit conversion copy constructor ", "[observer_construction]") {
//     memory_tracker mem_track;

//     {
//         test_ptr  ptr_owner{new test_object_derived};
//         test_optr ptr_orig{ptr_owner};
//         {
//             test_optr ptr{ptr_orig, static_cast<test_object_derived*>(ptr_orig.get())};
//             REQUIRE(instances == 1);
//             REQUIRE(ptr.get() == static_cast<test_object_derived*>(ptr_owner.get()));
//             REQUIRE(ptr.expired() == false);
//             REQUIRE(ptr_orig.get() == ptr_owner.get());
//             REQUIRE(ptr_orig.expired() == false);

//             ptr_owner.reset();
//             REQUIRE(ptr.get() == nullptr);
//             REQUIRE(ptr.expired() == true);
//         }

//         REQUIRE(instances == 0);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("observer explicit conversion copy constructor null pointer",
// "[observer_construction]") {
//     memory_tracker mem_track;

//     {
//         test_ptr  ptr_owner{new test_object_derived};
//         test_optr ptr_orig{ptr_owner};
//         {
//             test_optr ptr{ptr_orig, static_cast<test_object_derived*>(nullptr)};
//             REQUIRE(instances == 1);
//             REQUIRE(ptr.get() == nullptr);
//             REQUIRE(ptr.expired() == true);
//             REQUIRE(ptr_orig.get() == ptr_owner.get());
//             REQUIRE(ptr_orig.expired() == false);
//         }

//         REQUIRE(instances == 1);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

// TEST_CASE("observer explicit conversion copy constructor subobject", "[observer_construction]") {
//     memory_tracker mem_track;

//     {
//         test_ptr  ptr_owner{new test_object_derived};
//         test_optr ptr_orig{ptr_owner};
//         {
//             int_optr ptr{ptr_orig, &ptr_owner->state_};
//             REQUIRE(instances == 1);
//             REQUIRE(ptr.get() == &ptr_owner->state_);
//             REQUIRE(ptr.expired() == false);
//             REQUIRE(ptr_orig.get() == ptr_owner.get());
//             REQUIRE(ptr_orig.expired() == false);

//             ptr_owner.reset();
//             REQUIRE(ptr.get() == nullptr);
//             REQUIRE(ptr.expired() == true);
//         }

//         REQUIRE(instances == 0);
//     }

//     REQUIRE(instances == 0);
//     REQUIRE(mem_track.allocated() == 0u);
//     REQUIRE(mem_track.double_delete() == 0u);
// }

TEST_CASE("observer from null owner constructor", "[observer_construction]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_owner;
        {
            test_optr ptr{ptr_owner};
            REQUIRE(instances == 0);
            REQUIRE(ptr.get() == ptr_owner.get());
            REQUIRE(ptr.expired() == true);
        }

        REQUIRE(instances == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from owner constructor", "[observer_construction]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_owner{new test_object};
        {
            test_optr ptr{ptr_owner};
            REQUIRE(instances == 1);
            REQUIRE(ptr.get() == ptr_owner.get());
            REQUIRE(ptr.expired() == false);

            ptr_owner.reset();
            REQUIRE(ptr.get() == nullptr);
            REQUIRE(ptr.expired() == true);
        }

        REQUIRE(instances == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from null owner casting constructor", "[observer_construction]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_owner;
        {
            test_optr ptr{ptr_owner, static_cast<test_object*>(nullptr)};
            REQUIRE(instances == 0);
            REQUIRE(ptr.get() == ptr_owner.get());
            REQUIRE(ptr.expired() == true);
        }

        REQUIRE(instances == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from owner casting constructor", "[observer_construction]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_owner{new test_object_derived};
        {
            test_optr ptr{ptr_owner, dynamic_cast<test_object_derived*>(ptr_owner.get())};
            REQUIRE(instances == 1);
            REQUIRE(ptr.get() == ptr_owner.get());
            REQUIRE(ptr.expired() == false);

            ptr_owner.reset();
            REQUIRE(ptr.get() == nullptr);
            REQUIRE(ptr.expired() == true);
        }

        REQUIRE(instances == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer move constructor", "[observer_construction]") {
    memory_tracker mem_track;

    {
        test_ptr  ptr_owner{new test_object};
        test_optr ptr_orig{ptr_owner};
        {
            test_optr ptr(std::move(ptr_orig));
            REQUIRE(instances == 1);
            REQUIRE(ptr.get() != nullptr);
            REQUIRE(ptr.expired() == false);
            REQUIRE(ptr_orig.get() == nullptr);
            REQUIRE(ptr_orig.expired() == true);

            ptr_owner.reset();
            REQUIRE(ptr.get() == nullptr);
            REQUIRE(ptr.expired() == true);
        }

        REQUIRE(instances == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer explicit conversion move constructor ", "[observer_construction]") {
    memory_tracker mem_track;

    {
        test_ptr  ptr_owner{new test_object_derived};
        test_optr ptr_orig{ptr_owner};
        {
            test_optr ptr{std::move(ptr_orig), static_cast<test_object_derived*>(ptr_orig.get())};
            REQUIRE(instances == 1);
            REQUIRE(ptr.get() == static_cast<test_object_derived*>(ptr_owner.get()));
            REQUIRE(ptr.expired() == false);
            REQUIRE(ptr_orig.get() == nullptr);
            REQUIRE(ptr_orig.expired() == true);

            ptr_owner.reset();
            REQUIRE(ptr.get() == nullptr);
            REQUIRE(ptr.expired() == true);
        }

        REQUIRE(instances == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer explicit conversion move constructor null pointer", "[observer_construction]") {
    memory_tracker mem_track;

    {
        test_ptr  ptr_owner{new test_object_derived};
        test_optr ptr_orig{ptr_owner};
        {
            test_optr ptr{std::move(ptr_orig), static_cast<test_object_derived*>(nullptr)};
            REQUIRE(instances == 1);
            REQUIRE(ptr.get() == nullptr);
            REQUIRE(ptr.expired() == true);
            REQUIRE(ptr_orig.get() == nullptr);
            REQUIRE(ptr_orig.expired() == true);
        }

        REQUIRE(instances == 1);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer explicit conversion move constructor subobject", "[observer_construction]") {
    memory_tracker mem_track;

    {
        test_ptr  ptr_owner{new test_object_derived};
        test_optr ptr_orig{ptr_owner};
        {
            int_optr ptr{std::move(ptr_orig), &ptr_owner->state_};
            REQUIRE(instances == 1);
            REQUIRE(ptr.get() == &ptr_owner->state_);
            REQUIRE(ptr.expired() == false);
            REQUIRE(ptr_orig.get() == nullptr);
            REQUIRE(ptr_orig.expired() == true);

            ptr_owner.reset();
            REQUIRE(ptr.get() == nullptr);
            REQUIRE(ptr.expired() == true);
        }

        REQUIRE(instances == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer acquiring constructor", "[observer_construction]") {
    memory_tracker mem_track;

    {
        test_ptr  ptr_owner{new test_object};
        test_optr ptr{ptr_owner};
        REQUIRE(instances == 1);
        REQUIRE(ptr.get() != nullptr);
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer acquiring constructor sealed", "[observer_construction]") {
    memory_tracker mem_track;

    {
        test_sptr ptr_owner = oup::make_observable_sealed<test_object>();
        test_optr ptr{ptr_owner};
        REQUIRE(instances == 1);
        REQUIRE(ptr.get() != nullptr);
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer acquiring constructor derived", "[observer_construction]") {
    memory_tracker mem_track;

    {
        test_ptr_derived ptr_owner{new test_object_derived};
        test_optr        ptr{ptr_owner};
        REQUIRE(instances == 1);
        REQUIRE(ptr.get() != nullptr);
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer acquiring constructor derived sealed", "[observer_construction]") {
    memory_tracker mem_track;

    {
        test_sptr_derived ptr_owner = oup::make_observable_sealed<test_object_derived>();
        test_optr         ptr{ptr_owner};
        REQUIRE(instances == 1);
        REQUIRE(ptr.get() != nullptr);
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer acquiring constructor with deleter", "[observer_construction]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr_owner{new test_object, test_deleter{42}};
        test_optr             ptr{ptr_owner};
        REQUIRE(instances == 1);
        REQUIRE(ptr.get() != nullptr);
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer implicit copy conversion constructor", "[observer_construction]") {
    memory_tracker mem_track;

    {
        test_ptr_derived  ptr_owner{new test_object_derived};
        test_optr_derived ptr_orig{ptr_owner};
        {
            test_optr ptr(ptr_orig);
            REQUIRE(instances == 1);
            REQUIRE(instances_derived == 1);
            REQUIRE(ptr.get() != nullptr);
            REQUIRE(ptr.expired() == false);
        }

        REQUIRE(instances == 1);
        REQUIRE(instances_derived == 1);
        REQUIRE(ptr_orig.get() != nullptr);
        REQUIRE(ptr_orig.expired() == false);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_derived == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer implicit move conversion constructor", "[observer_construction]") {
    memory_tracker mem_track;

    {
        test_ptr_derived  ptr_owner{new test_object_derived};
        test_optr_derived ptr_orig{ptr_owner};
        {
            test_optr ptr(std::move(ptr_orig));
            REQUIRE(instances == 1);
            REQUIRE(instances_derived == 1);
            REQUIRE(ptr.get() != nullptr);
            REQUIRE(ptr.expired() == false);
        }

        REQUIRE(instances == 1);
        REQUIRE(instances_derived == 1);
        REQUIRE(ptr_orig.get() == nullptr);
        REQUIRE(ptr_orig.expired() == true);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_derived == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer expiring", "[observer_utility]") {
    memory_tracker mem_track;

    {
        test_optr ptr;

        {
            test_ptr ptr_owner{new test_object};
            ptr = ptr_owner;
            REQUIRE(instances == 1);
            REQUIRE(ptr.get() != nullptr);
            REQUIRE(ptr.expired() == false);
        }

        REQUIRE(instances == 0);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.expired() == true);
    }

    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer expiring sealed", "[observer_utility]") {
    memory_tracker mem_track;

    {
        test_optr ptr;

        {
            test_sptr ptr_owner = oup::make_observable_sealed<test_object>();
            ptr                 = ptr_owner;
            REQUIRE(instances == 1);
            REQUIRE(ptr.get() != nullptr);
            REQUIRE(ptr.expired() == false);
        }

        REQUIRE(instances == 0);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.expired() == true);
    }

    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer expiring reset", "[observer_utility]") {
    memory_tracker mem_track;

    {
        test_ptr  ptr_owner{new test_object};
        test_optr ptr = ptr_owner;
        REQUIRE(!ptr.expired());
        ptr_owner.reset();
        REQUIRE(ptr.expired());
    }

    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer expiring reset sealed", "[observer_utility]") {
    memory_tracker mem_track;

    {
        test_sptr ptr_owner = oup::make_observable_sealed<test_object>();
        test_optr ptr       = ptr_owner;
        REQUIRE(!ptr.expired());
        ptr_owner.reset();
        REQUIRE(ptr.expired());
    }

    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer not expiring when owner moved", "[observer_utility]") {
    memory_tracker mem_track;

    {
        test_ptr  ptr_owner{new test_object};
        test_optr ptr = ptr_owner;
        REQUIRE(!ptr.expired());
        test_ptr ptr_owner_new = std::move(ptr_owner);
        REQUIRE(!ptr.expired());
    }

    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer not expiring when owner moved sealed", "[observer_utility]") {
    memory_tracker mem_track;

    {
        test_sptr ptr_owner = oup::make_observable_sealed<test_object>();
        test_optr ptr       = ptr_owner;
        REQUIRE(!ptr.expired());
        test_sptr ptr_owner_new = std::move(ptr_owner);
        REQUIRE(!ptr.expired());
    }

    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer reset to null", "[observer_utility]") {
    memory_tracker mem_track;

    {
        test_ptr  ptr_owner(new test_object);
        test_optr ptr(ptr_owner);
        ptr.reset();
        REQUIRE(instances == 1);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.expired() == true);
    }

    REQUIRE(instances == 0);
}

TEST_CASE("observer swap no instance", "[observer_utility]") {
    memory_tracker mem_track;

    {
        test_optr ptr_orig;
        test_optr ptr;
        ptr.swap(ptr_orig);
        REQUIRE(instances == 0);
        REQUIRE(ptr_orig.get() == nullptr);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr_orig.expired() == true);
        REQUIRE(ptr.expired() == true);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer swap one instance", "[observer_utility]") {
    memory_tracker mem_track;

    {
        test_ptr  ptr_owner(new test_object);
        test_optr ptr_orig(ptr_owner);
        test_optr ptr;
        ptr.swap(ptr_orig);
        REQUIRE(instances == 1);
        REQUIRE(ptr_orig.get() == nullptr);
        REQUIRE(ptr.get() != nullptr);
        REQUIRE(ptr_orig.expired() == true);
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer swap two same instance", "[observer_utility]") {
    memory_tracker mem_track;

    {
        test_ptr  ptr_owner(new test_object);
        test_optr ptr_orig(ptr_owner);
        test_optr ptr(ptr_owner);
        ptr.swap(ptr_orig);
        REQUIRE(instances == 1);
        REQUIRE(ptr_orig.get() == ptr_owner.get());
        REQUIRE(ptr.get() == ptr_owner.get());
        REQUIRE(ptr_orig.expired() == false);
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer swap two different instances", "[observer_utility]") {
    memory_tracker mem_track;

    {
        test_ptr  ptr_owner1(new test_object);
        test_ptr  ptr_owner2(new test_object);
        test_optr ptr_orig(ptr_owner1);
        test_optr ptr(ptr_owner2);
        ptr.swap(ptr_orig);
        REQUIRE(instances == 2);
        REQUIRE(ptr_orig.get() == ptr_owner2.get());
        REQUIRE(ptr.get() == ptr_owner1.get());
        REQUIRE(ptr_orig.expired() == false);
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer swap self", "[observer_utility]") {
    memory_tracker mem_track;

    {
        test_ptr  ptr_owner(new test_object);
        test_optr ptr(ptr_owner);
        ptr.swap(ptr);
        REQUIRE(instances == 1);
        REQUIRE(ptr.get() == ptr_owner.get());
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer dereference", "[observer_utility]") {
    memory_tracker mem_track;

    {
        test_ptr  ptr_owner(new test_object);
        test_optr ptr(ptr_owner);
        REQUIRE(ptr->state_ == 1337);
        REQUIRE((*ptr).state_ == 1337);
    }

    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer operator bool valid", "[observer_utility]") {
    memory_tracker mem_track;

    {
        test_ptr  ptr_owner(new test_object);
        test_optr ptr(ptr_owner);
        if (ptr) {
        } else
            FAIL("if (ptr) should have been true");
    }

    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer operator bool invalid", "[observer_utility]") {
    memory_tracker mem_track;

    {
        test_optr ptr;
        if (ptr)
            FAIL("if (ptr) should not have been true");
    }

    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer get and raw get", "[observer_utility]") {
    memory_tracker mem_track;

    {
        test_optr ptr;
        REQUIRE(ptr.raw_get() == nullptr);
        REQUIRE(ptr.get() == nullptr);
        test_ptr owner_ptr{new test_object};
        ptr = owner_ptr;
        REQUIRE(ptr.raw_get() == owner_ptr.get());
        REQUIRE(ptr.get() == owner_ptr.get());
        test_object* raw_ptr = owner_ptr.get();
        owner_ptr.reset();
        REQUIRE(ptr.raw_get() == raw_ptr);
        REQUIRE(ptr.get() == nullptr);
    }

    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer copy assignment operator valid to empty", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_ptr  ptr_owner{new test_object};
        test_optr ptr_orig{ptr_owner};
        {
            test_optr ptr;
            ptr = ptr_orig;
            REQUIRE(instances == 1);
            REQUIRE(ptr.get() == ptr_owner.get());
            REQUIRE(ptr.expired() == false);
            REQUIRE(ptr_orig.get() == ptr_owner.get());
            REQUIRE(ptr_orig.expired() == false);
        }

        REQUIRE(instances == 1);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer copy assignment operator valid to valid", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_ptr  ptr_owner1{new test_object};
        test_ptr  ptr_owner2{new test_object};
        test_optr ptr_orig{ptr_owner1};
        {
            test_optr ptr(ptr_owner2);
            ptr = ptr_orig;
            REQUIRE(instances == 2);
            REQUIRE(ptr.get() == ptr_owner1.get());
            REQUIRE(ptr.expired() == false);
            REQUIRE(ptr_orig.get() == ptr_owner1.get());
            REQUIRE(ptr_orig.expired() == false);
        }

        REQUIRE(instances == 2);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer copy assignment operator empty to empty", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_optr ptr_orig;
        {
            test_optr ptr;
            ptr = ptr_orig;
            REQUIRE(instances == 0);
            REQUIRE(ptr.get() == nullptr);
            REQUIRE(ptr.expired() == true);
            REQUIRE(ptr_orig.get() == nullptr);
            REQUIRE(ptr_orig.expired() == true);
        }

        REQUIRE(instances == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer copy assignment operator self to self", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_ptr  ptr_owner{new test_object};
        test_optr ptr{ptr_owner};
        ptr = ptr;
        REQUIRE(instances == 1);
        REQUIRE(ptr.get() != nullptr);
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer copy assignment operator self to self expired", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_optr ptr;
        {
            test_ptr ptr_owner{new test_object};
            ptr = ptr_owner;
        }
        ptr = ptr;
        REQUIRE(instances == 0);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.expired() == true);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer copy assignment operator self to self empty", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_optr ptr;
        ptr = ptr;
        REQUIRE(instances == 0);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.expired() == true);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer move assignment operator valid to empty", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_ptr  ptr_owner{new test_object};
        test_optr ptr_orig{ptr_owner};
        {
            test_optr ptr;
            ptr = std::move(ptr_orig);
            REQUIRE(instances == 1);
            REQUIRE(ptr.get() == ptr_owner.get());
            REQUIRE(ptr.expired() == false);
            REQUIRE(ptr_orig.get() == nullptr);
            REQUIRE(ptr_orig.expired() == true);
        }

        REQUIRE(instances == 1);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer move assignment operator valid to valid", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_ptr  ptr_owner1{new test_object};
        test_ptr  ptr_owner2{new test_object};
        test_optr ptr_orig{ptr_owner1};
        {
            test_optr ptr{ptr_owner2};
            ptr = std::move(ptr_orig);
            REQUIRE(instances == 2);
            REQUIRE(ptr.get() == ptr_owner1.get());
            REQUIRE(ptr.expired() == false);
            REQUIRE(ptr_orig.get() == nullptr);
            REQUIRE(ptr_orig.expired() == true);
        }

        REQUIRE(instances == 2);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer move assignment operator empty to empty", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_optr ptr_orig;
        {
            test_optr ptr;
            ptr = std::move(ptr_orig);
            REQUIRE(instances == 0);
            REQUIRE(ptr.get() == nullptr);
            REQUIRE(ptr.expired() == true);
            REQUIRE(ptr_orig.get() == nullptr);
            REQUIRE(ptr_orig.expired() == true);
        }

        REQUIRE(instances == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer move assignment operator self to self", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_ptr  ptr_owner{new test_object};
        test_optr ptr{ptr_owner};
        ptr = std::move(ptr);
        REQUIRE(instances == 1);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.expired() == true);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer move assignment operator self to self expired", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_optr ptr;
        {
            test_ptr ptr_owner{new test_object};
            ptr = ptr_owner;
        }
        ptr = std::move(ptr);
        REQUIRE(instances == 0);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.expired() == true);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer move assignment operator self to self empty", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_optr ptr;
        ptr = std::move(ptr);
        REQUIRE(instances == 0);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.expired() == true);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer acquiring assignment operator valid to empty", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_ptr  ptr_owner{new test_object};
        test_optr ptr;
        ptr = ptr_owner;

        REQUIRE(instances == 1);
        REQUIRE(ptr.get() == ptr_owner.get());
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer acquiring assignment operator valid to valid", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_ptr  ptr_owner1{new test_object};
        test_ptr  ptr_owner2{new test_object};
        test_optr ptr{ptr_owner1};
        ptr = ptr_owner2;

        REQUIRE(instances == 2);
        REQUIRE(ptr.get() == ptr_owner2.get());
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer acquiring assignment operator empty to valid", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_ptr  ptr_owner1{new test_object};
        test_ptr  ptr_owner2;
        test_optr ptr{ptr_owner1};
        ptr = ptr_owner2;

        REQUIRE(instances == 1);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.expired() == true);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer acquiring assignment operator empty to empty", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_ptr  ptr_owner;
        test_optr ptr;
        ptr = ptr_owner;

        REQUIRE(instances == 0);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.expired() == true);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer acquiring assignment operator valid to empty sealed", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_sptr ptr_owner = oup::make_observable_sealed<test_object>();
        test_optr ptr;
        ptr = ptr_owner;

        REQUIRE(instances == 1);
        REQUIRE(ptr.get() == ptr_owner.get());
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer acquiring assignment operator valid to valid sealed", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_sptr ptr_owner1 = oup::make_observable_sealed<test_object>();
        test_sptr ptr_owner2 = oup::make_observable_sealed<test_object>();
        test_optr ptr{ptr_owner1};
        ptr = ptr_owner2;

        REQUIRE(instances == 2);
        REQUIRE(ptr.get() == ptr_owner2.get());
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer acquiring assignment operator empty to valid sealed", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_sptr ptr_owner1 = oup::make_observable_sealed<test_object>();
        test_sptr ptr_owner2;
        test_optr ptr{ptr_owner1};
        ptr = ptr_owner2;

        REQUIRE(instances == 1);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.expired() == true);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer acquiring assignment operator empty to empty sealed", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_sptr ptr_owner;
        test_optr ptr;
        ptr = ptr_owner;

        REQUIRE(instances == 0);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.expired() == true);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE(
    "observer acquiring assignment operator valid to empty with deleter", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr_owner{new test_object, test_deleter{42}};
        test_optr             ptr;
        ptr = ptr_owner;

        REQUIRE(instances == 1);
        REQUIRE(ptr.get() == ptr_owner.get());
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE(
    "observer acquiring assignment operator valid to valid with deleter", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr_owner1{new test_object, test_deleter{42}};
        test_ptr_with_deleter ptr_owner2{new test_object, test_deleter{42}};
        test_optr             ptr{ptr_owner1};
        ptr = ptr_owner2;

        REQUIRE(instances == 2);
        REQUIRE(ptr.get() == ptr_owner2.get());
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE(
    "observer acquiring assignment operator empty to valid with deleter", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr_owner1{new test_object, test_deleter{42}};
        test_ptr_with_deleter ptr_owner2{nullptr, test_deleter{43}};
        test_optr             ptr{ptr_owner1};
        ptr = ptr_owner2;

        REQUIRE(instances == 1);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.expired() == true);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE(
    "observer acquiring assignment operator empty to empty with deleter", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr_owner{nullptr, test_deleter{42}};
        test_optr             ptr;
        ptr = ptr_owner;

        REQUIRE(instances == 0);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.expired() == true);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer comparison valid ptr vs nullptr", "[observer_comparison]") {
    memory_tracker mem_track;

    {
        test_ptr  ptr_owner(new test_object);
        test_optr ptr(ptr_owner);
        REQUIRE(ptr != nullptr);
        REQUIRE(!(ptr == nullptr));
        REQUIRE(nullptr != ptr);
        REQUIRE(!(nullptr == ptr));
    }

    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer comparison invalid ptr vs nullptr", "[observer_comparison]") {
    memory_tracker mem_track;

    {
        test_optr ptr;
        REQUIRE(ptr == nullptr);
        REQUIRE(!(ptr != nullptr));
        REQUIRE(nullptr == ptr);
        REQUIRE(!(nullptr != ptr));
    }

    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer comparison invalid ptr vs invalid ptr", "[observer_comparison]") {
    memory_tracker mem_track;

    {
        test_optr ptr1;
        test_optr ptr2;
        REQUIRE(ptr1 == ptr2);
        REQUIRE(!(ptr1 != ptr2));
    }

    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer comparison invalid ptr vs valid ptr", "[observer_comparison]") {
    memory_tracker mem_track;

    {
        test_ptr  ptr_owner(new test_object);
        test_optr ptr1;
        test_optr ptr2(ptr_owner);
        REQUIRE(ptr1 != ptr2);
        REQUIRE(!(ptr1 == ptr2));
        REQUIRE(ptr2 != ptr1);
        REQUIRE(!(ptr2 == ptr1));
    }

    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer comparison valid ptr vs valid ptr same owner", "[observer_comparison]") {
    memory_tracker mem_track;

    {
        test_ptr  ptr_owner(new test_object);
        test_optr ptr1(ptr_owner);
        test_optr ptr2(ptr_owner);
        REQUIRE(ptr1 == ptr2);
        REQUIRE(!(ptr1 != ptr2));
        REQUIRE(ptr2 == ptr1);
        REQUIRE(!(ptr2 != ptr1));
    }

    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer comparison valid ptr vs valid ptr different owner", "[observer_comparison]") {
    memory_tracker mem_track;

    {
        test_ptr  ptr_owner1(new test_object);
        test_ptr  ptr_owner2(new test_object);
        test_optr ptr1(ptr_owner1);
        test_optr ptr2(ptr_owner2);
        REQUIRE(ptr1 != ptr2);
        REQUIRE(!(ptr1 == ptr2));
        REQUIRE(ptr2 != ptr1);
        REQUIRE(!(ptr2 == ptr1));
    }

    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE(
    "observer comparison valid ptr vs valid ptr same owner derived", "[observer_comparison]") {
    memory_tracker mem_track;

    {
        test_ptr_derived  ptr_owner(new test_object_derived);
        test_optr         ptr1(ptr_owner);
        test_optr_derived ptr2(ptr_owner);
        REQUIRE(ptr1 == ptr2);
        REQUIRE(!(ptr1 != ptr2));
        REQUIRE(ptr2 == ptr1);
        REQUIRE(!(ptr2 != ptr1));
    }

    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE(
    "observer comparison valid ptr vs valid ptr different owner derived", "[observer_comparison]") {
    memory_tracker mem_track;

    {
        test_ptr          ptr_owner1(new test_object);
        test_ptr_derived  ptr_owner2(new test_object_derived);
        test_optr         ptr1(ptr_owner1);
        test_optr_derived ptr2(ptr_owner2);
        REQUIRE(ptr1 != ptr2);
        REQUIRE(!(ptr1 == ptr2));
        REQUIRE(ptr2 != ptr1);
        REQUIRE(!(ptr2 == ptr1));
    }

    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

struct observer_owner {
    oup::observer_ptr<observer_owner> obs;
};

TEST_CASE("object owning observer pointer to itself", "[system_tests]") {
    memory_tracker mem_track;

    {
        auto ptr = oup::make_observable_sealed<observer_owner>();
        ptr->obs = ptr;
    }

    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("object owning observer pointer to other", "[system_tests]") {
    memory_tracker mem_track;

    {
        auto ptr1 = oup::make_observable_sealed<observer_owner>();
        auto ptr2 = oup::make_observable_sealed<observer_owner>();
        ptr1->obs = ptr2;
        ptr2->obs = ptr1;
    }

    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("object owning observer pointer open chain", "[system_tests]") {
    memory_tracker mem_track;

    {
        auto ptr1 = oup::make_observable_sealed<observer_owner>();
        auto ptr2 = oup::make_observable_sealed<observer_owner>();
        auto ptr3 = oup::make_observable_sealed<observer_owner>();
        ptr1->obs = ptr2;
        ptr2->obs = ptr3;
    }

    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("object owning observer pointer open chain reversed", "[system_tests]") {
    memory_tracker mem_track;

    {
        auto ptr1 = oup::make_observable_sealed<observer_owner>();
        auto ptr2 = oup::make_observable_sealed<observer_owner>();
        auto ptr3 = oup::make_observable_sealed<observer_owner>();
        ptr3->obs = ptr2;
        ptr2->obs = ptr1;
    }

    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("object owning observer pointer closed chain interleaved", "[system_tests]") {
    memory_tracker mem_track;

    {
        auto ptr1 = oup::make_observable_sealed<observer_owner>();
        auto ptr2 = oup::make_observable_sealed<observer_owner>();
        auto ptr3 = oup::make_observable_sealed<observer_owner>();
        auto ptr4 = oup::make_observable_sealed<observer_owner>();
        ptr1->obs = ptr2;
        ptr2->obs = ptr4;
        ptr3->obs = ptr1;
        ptr4->obs = ptr3;
    }

    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("pointers in vector", "[system_tests]") {
    memory_tracker mem_track;

    {
        std::vector<test_sptr> vec_own;
        std::vector<test_optr> vec_obs;

        vec_own.resize(100);
        REQUIRE(std::all_of(vec_own.begin(), vec_own.end(), [](const auto& p) {
                    return p == nullptr;
                }) == true);

        std::generate(vec_own.begin(), vec_own.end(), []() {
            return oup::make_observable_sealed<test_object>();
        });
        REQUIRE(std::none_of(vec_own.begin(), vec_own.end(), [](const auto& p) {
                    return p == nullptr;
                }) == true);

        vec_obs.resize(100);
        REQUIRE(std::all_of(vec_obs.begin(), vec_obs.end(), [](const auto& p) {
                    return p == nullptr;
                }) == true);

        std::copy(vec_own.begin(), vec_own.end(), vec_obs.begin());
        REQUIRE(std::none_of(vec_own.begin(), vec_own.end(), [](const auto& p) {
                    return p == nullptr;
                }) == true);

        std::vector<test_sptr> vec_own_new = std::move(vec_own);
        REQUIRE(std::none_of(vec_own.begin(), vec_own.end(), [](const auto& p) {
                    return p == nullptr;
                }) == true);

        vec_own_new.clear();
        REQUIRE(std::all_of(vec_obs.begin(), vec_obs.end(), [](const auto& p) {
                    return p == nullptr;
                }) == true);
    }

    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from this unique", "[observer_from_this]") {
    memory_tracker mem_track;

    {
        test_ptr_from_this        ptr{new test_object_observer_from_this_unique};
        const test_ptr_from_this& cptr = ptr;

        test_optr_from_this       optr_from_this       = ptr->observer_from_this();
        test_optr_from_this_const optr_from_this_const = cptr->observer_from_this();

        REQUIRE(instances == 1);
        REQUIRE(optr_from_this.expired() == false);
        REQUIRE(optr_from_this_const.expired() == false);
        REQUIRE(optr_from_this.get() == ptr.get());
        REQUIRE(optr_from_this_const.get() == ptr.get());
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from this sealed", "[observer_from_this]") {
    memory_tracker mem_track;

    {
        test_sptr_from_this ptr =
            oup::make_observable_sealed<test_object_observer_from_this_sealed>();
        const test_sptr_from_this& cptr = ptr;

        test_optr_from_this_sealed       optr_from_this       = ptr->observer_from_this();
        test_optr_from_this_const_sealed optr_from_this_const = cptr->observer_from_this();

        REQUIRE(instances == 1);
        REQUIRE(optr_from_this.expired() == false);
        REQUIRE(optr_from_this_const.expired() == false);
        REQUIRE(optr_from_this.get() == ptr.get());
        REQUIRE(optr_from_this_const.get() == ptr.get());
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from this non virtual unique", "[observer_from_this]") {
    memory_tracker mem_track;

    {
        test_ptr_from_this_non_virtual ptr = oup::make_observable<
            test_object_observer_from_this_non_virtual_unique, unique_non_virtual_policy>();
        const test_ptr_from_this_non_virtual& cptr = ptr;

        test_optr_from_this_non_virtual_unique       optr_from_this = ptr->observer_from_this();
        test_optr_from_this_const_non_virtual_unique optr_from_this_const =
            cptr->observer_from_this();

        REQUIRE(instances == 1);
        REQUIRE(optr_from_this.expired() == false);
        REQUIRE(optr_from_this_const.expired() == false);
        REQUIRE(optr_from_this.get() == ptr.get());
        REQUIRE(optr_from_this_const.get() == ptr.get());
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from this maybe no block unique", "[observer_from_this]") {
    memory_tracker mem_track;

    {
        test_ptr_from_this_maybe_no_block ptr = oup::make_observable<
            test_object_observer_from_this_maybe_no_block_unique, unique_maybe_no_block_policy>();
        const test_ptr_from_this_maybe_no_block& cptr = ptr;

        test_optr_from_this_maybe_no_block_unique       optr_from_this = ptr->observer_from_this();
        test_optr_from_this_const_maybe_no_block_unique optr_from_this_const =
            cptr->observer_from_this();

        REQUIRE(instances == 1);
        REQUIRE(optr_from_this.expired() == false);
        REQUIRE(optr_from_this_const.expired() == false);
        REQUIRE(optr_from_this.get() == ptr.get());
        REQUIRE(optr_from_this_const.get() == ptr.get());
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from this maybe no block acquire", "[observer_from_this]") {
    memory_tracker mem_track;

    {
        test_ptr_from_this_maybe_no_block ptr{
            new test_object_observer_from_this_maybe_no_block_unique};
        const test_ptr_from_this_maybe_no_block& cptr = ptr;

        test_optr_from_this_maybe_no_block_unique       optr_from_this = ptr->observer_from_this();
        test_optr_from_this_const_maybe_no_block_unique optr_from_this_const =
            cptr->observer_from_this();

        REQUIRE(instances == 1);
        REQUIRE(optr_from_this.expired() == false);
        REQUIRE(optr_from_this_const.expired() == false);
        REQUIRE(optr_from_this.get() == ptr.get());
        REQUIRE(optr_from_this_const.get() == ptr.get());
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from this maybe no block reset to new", "[observer_from_this]") {
    memory_tracker mem_track;

    {
        auto* raw_ptr1 = new test_object_observer_from_this_maybe_no_block_unique;
        auto* raw_ptr2 = new test_object_observer_from_this_maybe_no_block_unique;

        test_ptr_from_this_maybe_no_block        ptr{raw_ptr1};
        const test_ptr_from_this_maybe_no_block& cptr = ptr;

        ptr.reset(raw_ptr2);

        test_optr_from_this_maybe_no_block_unique       optr_from_this = ptr->observer_from_this();
        test_optr_from_this_const_maybe_no_block_unique optr_from_this_const =
            cptr->observer_from_this();

        REQUIRE(instances == 1);
        REQUIRE(optr_from_this.expired() == false);
        REQUIRE(optr_from_this_const.expired() == false);
        REQUIRE(optr_from_this.get() == ptr.get());
        REQUIRE(optr_from_this.get() == raw_ptr2);
        REQUIRE(optr_from_this_const.get() == ptr.get());
        REQUIRE(optr_from_this_const.get() == raw_ptr2);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from this maybe no block reset to new after release", "[observer_from_this]") {
    memory_tracker mem_track;

    {
        auto* raw_ptr = new test_object_observer_from_this_maybe_no_block_unique;

        test_ptr_from_this_maybe_no_block        ptr{raw_ptr};
        const test_ptr_from_this_maybe_no_block& cptr = ptr;

        test_optr_from_this_maybe_no_block_unique       optr_from_this = ptr->observer_from_this();
        test_optr_from_this_const_maybe_no_block_unique optr_from_this_const =
            cptr->observer_from_this();

        ptr.release();

        REQUIRE(instances == 1);
        REQUIRE(optr_from_this.expired() == false);
        REQUIRE(optr_from_this_const.expired() == false);
        REQUIRE(optr_from_this.get() == raw_ptr);
        REQUIRE(optr_from_this_const.get() == raw_ptr);

        ptr.reset(raw_ptr);

        REQUIRE(instances == 1);
        REQUIRE(optr_from_this.expired() == false);
        REQUIRE(optr_from_this_const.expired() == false);
        REQUIRE(optr_from_this.get() == ptr.get());
        REQUIRE(optr_from_this_const.get() == ptr.get());

        ptr.reset();

        REQUIRE(instances == 0);
        REQUIRE(optr_from_this.expired() == true);
        REQUIRE(optr_from_this_const.expired() == true);
        REQUIRE(optr_from_this.get() == nullptr);
        REQUIRE(optr_from_this_const.get() == nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from this maybe no block reset to new bad alloc", "[observer_from_this]") {
    memory_tracker mem_track;

    {
        auto* raw_ptr1 = new test_object_observer_from_this_maybe_no_block_unique;
        auto* raw_ptr2 = new test_object_observer_from_this_maybe_no_block_unique;

        test_ptr_from_this_maybe_no_block ptr{raw_ptr1};

        try {
            force_next_allocation_failure = true;
            ptr.reset(raw_ptr2);
        } catch (...) {
        }

        REQUIRE(instances == 1);
        REQUIRE(ptr.get() == raw_ptr1);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from this virtual sealed", "[observer_from_this]") {
    memory_tracker mem_track;

    {
        test_sptr_from_this_virtual ptr = oup::make_observable<
            test_object_observer_from_this_virtual_sealed, sealed_virtual_policy>();
        const test_sptr_from_this_virtual& cptr = ptr;

        test_optr_from_this_virtual_sealed       optr_from_this       = ptr->observer_from_this();
        test_optr_from_this_const_virtual_sealed optr_from_this_const = cptr->observer_from_this();

        REQUIRE(instances == 1);
        REQUIRE(optr_from_this.expired() == false);
        REQUIRE(optr_from_this_const.expired() == false);
        REQUIRE(optr_from_this.get() == ptr.get());
        REQUIRE(optr_from_this_const.get() == ptr.get());
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from this derived", "[observer_from_this]") {
    memory_tracker mem_track;

    {
        test_ptr_from_this_derived        ptr{new test_object_observer_from_this_derived_unique};
        const test_ptr_from_this_derived& cptr = ptr;

        test_optr_from_this       optr_from_this       = ptr->observer_from_this();
        test_optr_from_this_const optr_from_this_const = cptr->observer_from_this();

        REQUIRE(instances == 1);
        REQUIRE(optr_from_this.expired() == false);
        REQUIRE(optr_from_this_const.expired() == false);
        REQUIRE(optr_from_this.get() == ptr.get());
        REQUIRE(optr_from_this_const.get() == ptr.get());
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from this derived into base", "[observer_from_this]") {
    memory_tracker mem_track;

    {
        test_object_observer_from_this_unique* orig_ptr = new test_object_observer_from_this_unique;
        test_ptr                               ptr{orig_ptr};

        test_optr_from_this optr_from_this = orig_ptr->observer_from_this();

        REQUIRE(instances == 1);
        REQUIRE(optr_from_this.expired() == false);
        REQUIRE(optr_from_this.get() == ptr.get());
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from this derived into base after cast", "[observer_from_this]") {
    memory_tracker mem_track;

    {
        test_object_observer_from_this_unique* orig_ptr = new test_object_observer_from_this_unique;
        test_ptr                               ptr{static_cast<test_object*>(orig_ptr)};

        const test_object_observer_from_this_unique* orig_cptr = orig_ptr;

        test_optr_from_this       optr_from_this       = orig_ptr->observer_from_this();
        test_optr_from_this_const optr_from_this_const = orig_cptr->observer_from_this();

        REQUIRE(instances == 1);
        REQUIRE(optr_from_this.expired() == false);
        REQUIRE(optr_from_this.get() == ptr.get());
        REQUIRE(optr_from_this_const.expired() == false);
        REQUIRE(optr_from_this_const.get() == ptr.get());
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from this const", "[observer_from_this]") {
    memory_tracker mem_track;

    {
        test_cptr_from_this       ptr{new test_object_observer_from_this_unique};
        test_optr_from_this_const optr_from_this = ptr->observer_from_this();

        REQUIRE(instances == 1);
        REQUIRE(optr_from_this.expired() == false);
        REQUIRE(optr_from_this.get() == ptr.get());
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from this const sealed", "[observer_from_this]") {
    memory_tracker mem_track;

    {
        test_csptr_from_this ptr =
            oup::make_observable_sealed<const test_object_observer_from_this_sealed>();
        test_optr_from_this_const_sealed optr_from_this = ptr->observer_from_this();

        REQUIRE(instances == 1);
        REQUIRE(optr_from_this.expired() == false);
        REQUIRE(optr_from_this.get() == ptr.get());
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from this after move", "[observer_from_this]") {
    memory_tracker mem_track;

    {
        test_ptr_from_this        ptr1{new test_object_observer_from_this_unique};
        test_ptr_from_this        ptr2{std::move(ptr1)};
        const test_ptr_from_this& cptr2 = ptr2;

        test_optr_from_this       optr_from_this       = ptr2->observer_from_this();
        test_optr_from_this_const optr_from_this_const = cptr2->observer_from_this();

        REQUIRE(instances == 1);
        REQUIRE(optr_from_this.expired() == false);
        REQUIRE(optr_from_this_const.expired() == false);
        REQUIRE(optr_from_this.get() == ptr2.get());
        REQUIRE(optr_from_this_const.get() == ptr2.get());
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from this after move sealed", "[observer_from_this]") {
    memory_tracker mem_track;

    {
        test_sptr_from_this ptr1 =
            oup::make_observable_sealed<test_object_observer_from_this_sealed>();
        test_sptr_from_this        ptr2{std::move(ptr1)};
        const test_sptr_from_this& cptr2 = ptr2;

        test_optr_from_this_sealed       optr_from_this       = ptr2->observer_from_this();
        test_optr_from_this_const_sealed optr_from_this_const = cptr2->observer_from_this();

        REQUIRE(instances == 1);
        REQUIRE(optr_from_this.expired() == false);
        REQUIRE(optr_from_this_const.expired() == false);
        REQUIRE(optr_from_this.get() == ptr2.get());
        REQUIRE(optr_from_this_const.get() == ptr2.get());
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from this after move assignment", "[observer_from_this]") {
    memory_tracker mem_track;

    {
        test_ptr_from_this ptr1{new test_object_observer_from_this_unique};
        test_ptr_from_this ptr2;
        ptr2 = std::move(ptr1);

        const test_ptr_from_this& cptr2                = ptr2;
        test_optr_from_this       optr_from_this       = ptr2->observer_from_this();
        test_optr_from_this_const optr_from_this_const = cptr2->observer_from_this();

        REQUIRE(instances == 1);
        REQUIRE(optr_from_this.expired() == false);
        REQUIRE(optr_from_this_const.expired() == false);
        REQUIRE(optr_from_this.get() == ptr2.get());
        REQUIRE(optr_from_this_const.get() == ptr2.get());
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from this after move assignment sealed", "[observer_from_this]") {
    memory_tracker mem_track;

    {
        test_sptr_from_this ptr1 =
            oup::make_observable_sealed<test_object_observer_from_this_sealed>();
        test_sptr_from_this ptr2;
        ptr2                             = std::move(ptr1);
        const test_sptr_from_this& cptr2 = ptr2;

        test_optr_from_this_sealed       optr_from_this       = ptr2->observer_from_this();
        test_optr_from_this_const_sealed optr_from_this_const = cptr2->observer_from_this();

        REQUIRE(instances == 1);
        REQUIRE(optr_from_this.expired() == false);
        REQUIRE(optr_from_this_const.expired() == false);
        REQUIRE(optr_from_this.get() == ptr2.get());
        REQUIRE(optr_from_this_const.get() == ptr2.get());
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from this after release", "[observer_from_this]") {
    memory_tracker mem_track;

    {
        test_ptr_from_this                     ptr1{new test_object_observer_from_this_unique};
        test_object_observer_from_this_unique* ptr2        = ptr1.release();
        const test_object_observer_from_this_unique* cptr2 = ptr2;

        {
            test_optr_from_this       optr_from_this       = ptr2->observer_from_this();
            test_optr_from_this_const optr_from_this_const = cptr2->observer_from_this();

            REQUIRE(instances == 1);
            REQUIRE(optr_from_this.expired() == false);
            REQUIRE(optr_from_this.get() == ptr2);
            REQUIRE(optr_from_this_const.expired() == false);
            REQUIRE(optr_from_this_const.get() == cptr2);
        }

        // The object holds the last reference to the control block
        delete ptr2;
        REQUIRE(instances == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from this after release and reset", "[observer_from_this]") {
    memory_tracker mem_track;

    {
        test_ptr_from_this                     ptr1{new test_object_observer_from_this_unique};
        test_object_observer_from_this_unique* ptr2        = ptr1.release();
        const test_object_observer_from_this_unique* cptr2 = ptr2;

        test_ptr_from_this ptr3;
        ptr3.reset(ptr2);

        test_optr_from_this       optr_from_this       = ptr2->observer_from_this();
        test_optr_from_this_const optr_from_this_const = cptr2->observer_from_this();

        REQUIRE(instances == 1);
        REQUIRE(optr_from_this.expired() == false);
        REQUIRE(optr_from_this_const.expired() == false);
        REQUIRE(optr_from_this.get() == ptr2);
        REQUIRE(optr_from_this_const.get() == cptr2);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from this stack", "[observer_from_this]") {
    memory_tracker mem_track;

    {
        test_object_observer_from_this_unique        obj;
        const test_object_observer_from_this_unique& cobj = obj;

        test_optr_from_this       optr_from_this       = obj.observer_from_this();
        test_optr_from_this_const optr_from_this_const = cobj.observer_from_this();

        REQUIRE(instances == 1);
        REQUIRE(optr_from_this.expired() == false);
        REQUIRE(optr_from_this_const.expired() == false);
        REQUIRE(optr_from_this.get() == &obj);
        REQUIRE(optr_from_this_const.get() == &cobj);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from this heap", "[observer_from_this]") {
    memory_tracker mem_track;

    {
        test_object_observer_from_this_unique* obj = new test_object_observer_from_this_unique;
        const test_object_observer_from_this_unique* cobj = obj;

        test_optr_from_this       optr_from_this       = obj->observer_from_this();
        test_optr_from_this_const optr_from_this_const = cobj->observer_from_this();

        REQUIRE(instances == 1);
        REQUIRE(optr_from_this.expired() == false);
        REQUIRE(optr_from_this_const.expired() == false);
        REQUIRE(optr_from_this.get() == obj);
        REQUIRE(optr_from_this_const.get() == cobj);

        delete obj;
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from this stack virtual sealed", "[observer_from_this]") {
    memory_tracker mem_track;

    {
        test_object_observer_from_this_virtual_sealed        obj;
        const test_object_observer_from_this_virtual_sealed& cobj = obj;

        REQUIRE_THROWS_AS(obj.observer_from_this(), oup::bad_observer_from_this);

        REQUIRE_THROWS_AS(cobj.observer_from_this(), oup::bad_observer_from_this);

        REQUIRE(instances == 1);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from this heap virtual sealed", "[observer_from_this]") {
    memory_tracker mem_track;

    {
        test_object_observer_from_this_virtual_sealed* obj =
            new test_object_observer_from_this_virtual_sealed;
        const test_object_observer_from_this_virtual_sealed* cobj = obj;

        REQUIRE_THROWS_AS(obj->observer_from_this(), oup::bad_observer_from_this);

        REQUIRE_THROWS_AS(cobj->observer_from_this(), oup::bad_observer_from_this);

        REQUIRE(instances == 1);

        delete obj;
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from this multiple inheritance", "[observer_from_this]") {
    memory_tracker mem_track;

    {
        using this_base =
            oup::enable_observer_from_this_unique<test_object_observer_from_this_unique>;
        using this_deriv =
            oup::enable_observer_from_this_unique<test_object_observer_from_this_multi_unique>;

        test_object_observer_from_this_multi_unique* raw_ptr_deriv =
            new test_object_observer_from_this_multi_unique;
        test_object_observer_from_this_unique* raw_ptr_base = raw_ptr_deriv;
        test_ptr_from_this_multi               ptr(raw_ptr_deriv);

        test_optr_from_this       optr_from_this_base  = ptr->this_base::observer_from_this();
        test_optr_from_this_multi optr_from_this_deriv = ptr->this_deriv::observer_from_this();

        REQUIRE(instances == 1);
        REQUIRE(optr_from_this_base.expired() == false);
        REQUIRE(optr_from_this_deriv.expired() == false);
        REQUIRE(optr_from_this_base.get() == raw_ptr_base);
        REQUIRE(optr_from_this_deriv.get() == raw_ptr_deriv);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from this in constructor", "[observer_from_this]") {
    memory_tracker mem_track;

    {
        test_ptr_from_this_constructor ptr =
            oup::make_observable_unique<test_object_observer_from_this_constructor_unique>();

        REQUIRE(instances == 1);
        REQUIRE(ptr->ptr.expired() == false);
        REQUIRE(ptr->ptr.get() == ptr.get());
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from this in constructor sealed", "[observer_from_this]") {
    memory_tracker mem_track;

    {
        test_sptr_from_this_constructor ptr =
            oup::make_observable_sealed<test_object_observer_from_this_constructor_sealed>();

        REQUIRE(instances == 1);
        REQUIRE(ptr->ptr.expired() == false);
        REQUIRE(ptr->ptr.get() == ptr.get());
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from this in constructor multiple inheritance", "[observer_from_this]") {
    memory_tracker mem_track;

    {
        test_ptr_from_this_constructor_multi ptr =
            oup::make_observable_unique<test_object_observer_from_this_constructor_multi_unique>();

        REQUIRE(instances == 1);
        REQUIRE(ptr->ptr.expired() == false);
        REQUIRE(ptr->ptr.get() == ptr.get());
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from this in constructor multiple inheritance sealed", "[observer_from_this]") {
    memory_tracker mem_track;

    {
        test_sptr_from_this_constructor_multi ptr =
            oup::make_observable_sealed<test_object_observer_from_this_constructor_multi_sealed>();

        REQUIRE(instances == 1);
        REQUIRE(ptr->ptr.expired() == false);
        REQUIRE(ptr->ptr.get() == ptr.get());
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("observer from this in constructor sealed virtual throws", "[observer_from_this]") {
    memory_tracker mem_track;

    {
        REQUIRE_THROWS_AS(
            (oup::make_observable<
                test_object_observer_from_this_constructor_bad, sealed_virtual_policy>()),
            oup::bad_observer_from_this);

        REQUIRE(instances == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("bad_observer_from_this", "[observer_from_this]") {
    memory_tracker mem_track;

    oup::bad_observer_from_this e;
    REQUIRE(e.what() != nullptr);

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("static pointer cast unique from valid", "[pointer_cast]") {
    memory_tracker mem_track;

    {
        test_object_derived* raw_ptr = new test_object_derived;
        test_ptr             ptr_orig{raw_ptr};
        test_ptr_derived ptr = oup::static_pointer_cast<test_object_derived>(std::move(ptr_orig));

        REQUIRE(instances == 1);
        REQUIRE(ptr_orig == nullptr);
        REQUIRE(ptr.get() == raw_ptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("static pointer cast unique from null", "[pointer_cast]") {
    memory_tracker mem_track;

    {
        test_ptr         ptr_orig;
        test_ptr_derived ptr = oup::static_pointer_cast<test_object_derived>(std::move(ptr_orig));

        REQUIRE(instances == 0);
        REQUIRE(ptr_orig == nullptr);
        REQUIRE(ptr == nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("static pointer cast sealed from valid", "[pointer_cast]") {
    memory_tracker mem_track;

    {
        test_sptr_derived    ptr_init = oup::make_observable_sealed<test_object_derived>();
        test_object_derived* raw_ptr  = ptr_init.get();
        test_sptr            ptr_orig{std::move(ptr_init)};
        test_sptr_derived ptr = oup::static_pointer_cast<test_object_derived>(std::move(ptr_orig));

        REQUIRE(instances == 1);
        REQUIRE(ptr_orig == nullptr);
        REQUIRE(ptr.get() == raw_ptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("static pointer cast sealed from null", "[pointer_cast]") {
    memory_tracker mem_track;

    {
        test_sptr         ptr_orig;
        test_sptr_derived ptr = oup::static_pointer_cast<test_object_derived>(std::move(ptr_orig));

        REQUIRE(instances == 0);
        REQUIRE(ptr_orig == nullptr);
        REQUIRE(ptr == nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("static pointer cast observer copy from valid", "[pointer_cast]") {
    memory_tracker mem_track;

    {
        test_sptr_derived    ptr_owner = oup::make_observable_sealed<test_object_derived>();
        test_object_derived* raw_ptr   = ptr_owner.get();
        test_optr            ptr_orig{ptr_owner};
        test_optr_derived    ptr = oup::static_pointer_cast<test_object_derived>(ptr_orig);

        REQUIRE(instances == 1);
        REQUIRE(ptr_orig.get() == raw_ptr);
        REQUIRE(ptr.get() == raw_ptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("static pointer cast observer copy from null", "[pointer_cast]") {
    memory_tracker mem_track;

    {
        test_optr         ptr_orig;
        test_optr_derived ptr = oup::static_pointer_cast<test_object_derived>(ptr_orig);

        REQUIRE(instances == 0);
        REQUIRE(ptr_orig == nullptr);
        REQUIRE(ptr == nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("static pointer cast observer move from valid", "[pointer_cast]") {
    memory_tracker mem_track;

    {
        test_sptr_derived    ptr_owner = oup::make_observable_sealed<test_object_derived>();
        test_object_derived* raw_ptr   = ptr_owner.get();
        test_optr            ptr_orig{ptr_owner};
        test_optr_derived ptr = oup::static_pointer_cast<test_object_derived>(std::move(ptr_orig));

        REQUIRE(instances == 1);
        REQUIRE(ptr_orig == nullptr);
        REQUIRE(ptr.get() == raw_ptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("static pointer cast observer move from null", "[pointer_cast]") {
    memory_tracker mem_track;

    {
        test_optr         ptr_orig;
        test_optr_derived ptr = oup::static_pointer_cast<test_object_derived>(std::move(ptr_orig));

        REQUIRE(instances == 0);
        REQUIRE(ptr_orig == nullptr);
        REQUIRE(ptr == nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("const pointer cast unique from valid", "[pointer_cast]") {
    memory_tracker mem_track;

    {
        test_object*   raw_ptr = new test_object;
        test_ptr_const ptr_orig{raw_ptr};
        test_ptr       ptr = oup::const_pointer_cast<test_object>(std::move(ptr_orig));

        REQUIRE(instances == 1);
        REQUIRE(ptr_orig == nullptr);
        REQUIRE(ptr.get() == raw_ptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("const pointer cast unique from null", "[pointer_cast]") {
    memory_tracker mem_track;

    {
        test_ptr_const ptr_orig;
        test_ptr       ptr = oup::const_pointer_cast<test_object>(std::move(ptr_orig));

        REQUIRE(instances == 0);
        REQUIRE(ptr_orig == nullptr);
        REQUIRE(ptr == nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("const pointer cast sealed from valid", "[pointer_cast]") {
    memory_tracker mem_track;

    {
        test_sptr       ptr_init = oup::make_observable_sealed<test_object>();
        test_object*    raw_ptr  = ptr_init.get();
        test_sptr_const ptr_orig{std::move(ptr_init)};
        test_sptr       ptr = oup::const_pointer_cast<test_object>(std::move(ptr_orig));

        REQUIRE(instances == 1);
        REQUIRE(ptr_orig == nullptr);
        REQUIRE(ptr.get() == raw_ptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("const pointer cast sealed from null", "[pointer_cast]") {
    memory_tracker mem_track;

    {
        test_sptr_const ptr_orig;
        test_sptr       ptr = oup::const_pointer_cast<test_object>(std::move(ptr_orig));

        REQUIRE(instances == 0);
        REQUIRE(ptr_orig == nullptr);
        REQUIRE(ptr == nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("const pointer cast observer copy from valid", "[pointer_cast]") {
    memory_tracker mem_track;

    {
        test_sptr       ptr_owner = oup::make_observable_sealed<test_object>();
        test_object*    raw_ptr   = ptr_owner.get();
        test_optr_const ptr_orig{ptr_owner};
        test_optr       ptr = oup::const_pointer_cast<test_object>(ptr_orig);

        REQUIRE(instances == 1);
        REQUIRE(ptr_orig.get() == raw_ptr);
        REQUIRE(ptr.get() == raw_ptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("const pointer cast observer copy from null", "[pointer_cast]") {
    memory_tracker mem_track;

    {
        test_optr_const ptr_orig;
        test_optr       ptr = oup::const_pointer_cast<test_object>(ptr_orig);

        REQUIRE(instances == 0);
        REQUIRE(ptr_orig == nullptr);
        REQUIRE(ptr == nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("const pointer cast observer move from valid", "[pointer_cast]") {
    memory_tracker mem_track;

    {
        test_sptr       ptr_owner = oup::make_observable_sealed<test_object_derived>();
        test_object*    raw_ptr   = ptr_owner.get();
        test_optr_const ptr_orig{ptr_owner};
        test_optr       ptr = oup::const_pointer_cast<test_object>(std::move(ptr_orig));

        REQUIRE(instances == 1);
        REQUIRE(ptr_orig == nullptr);
        REQUIRE(ptr.get() == raw_ptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("const pointer cast observer move from null", "[pointer_cast]") {
    memory_tracker mem_track;

    {
        test_optr_const ptr_orig;
        test_optr       ptr = oup::const_pointer_cast<test_object>(std::move(ptr_orig));

        REQUIRE(instances == 0);
        REQUIRE(ptr_orig == nullptr);
        REQUIRE(ptr == nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("dynamic pointer cast unique from valid", "[pointer_cast]") {
    memory_tracker mem_track;

    {
        test_object_derived* raw_ptr = new test_object_derived;
        test_ptr             ptr_orig{raw_ptr};
        test_ptr_derived ptr = oup::dynamic_pointer_cast<test_object_derived>(std::move(ptr_orig));

        REQUIRE(instances == 1);
        REQUIRE(ptr_orig == nullptr);
        REQUIRE(ptr.get() == raw_ptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("dynamic pointer cast unique from invalid", "[pointer_cast]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_orig{new test_object_observer_from_this_unique};

        REQUIRE_THROWS_AS(
            oup::dynamic_pointer_cast<test_object_derived>(std::move(ptr_orig)), std::bad_cast);

        REQUIRE(instances == 1);
        REQUIRE(ptr_orig != nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("dynamic pointer cast unique from null", "[pointer_cast]") {
    memory_tracker mem_track;

    {
        test_ptr         ptr_orig;
        test_ptr_derived ptr = oup::dynamic_pointer_cast<test_object_derived>(std::move(ptr_orig));

        REQUIRE(instances == 0);
        REQUIRE(ptr_orig == nullptr);
        REQUIRE(ptr == nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("dynamic pointer cast sealed from valid", "[pointer_cast]") {
    memory_tracker mem_track;

    {
        test_sptr_derived    ptr_init = oup::make_observable_sealed<test_object_derived>();
        test_object_derived* raw_ptr  = ptr_init.get();
        test_sptr            ptr_orig{std::move(ptr_init)};
        test_sptr_derived ptr = oup::dynamic_pointer_cast<test_object_derived>(std::move(ptr_orig));

        REQUIRE(instances == 1);
        REQUIRE(ptr_orig == nullptr);
        REQUIRE(ptr.get() == raw_ptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("dynamic pointer cast sealed from invalid", "[pointer_cast]") {
    memory_tracker mem_track;

    {
        test_sptr ptr_orig{oup::make_observable_sealed<test_object_observer_from_this_sealed>()};

        REQUIRE_THROWS_AS(
            oup::dynamic_pointer_cast<test_object_derived>(std::move(ptr_orig)), std::bad_cast);

        REQUIRE(instances == 1);
        REQUIRE(ptr_orig != nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("dynamic pointer cast sealed from null", "[pointer_cast]") {
    memory_tracker mem_track;

    {
        test_sptr         ptr_orig;
        test_sptr_derived ptr = oup::dynamic_pointer_cast<test_object_derived>(std::move(ptr_orig));

        REQUIRE(instances == 0);
        REQUIRE(ptr_orig == nullptr);
        REQUIRE(ptr == nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("dynamic pointer cast observer copy from valid", "[pointer_cast]") {
    memory_tracker mem_track;

    {
        test_sptr_derived    ptr_owner = oup::make_observable_sealed<test_object_derived>();
        test_object_derived* raw_ptr   = ptr_owner.get();
        test_optr            ptr_orig{ptr_owner};
        test_optr_derived    ptr = oup::dynamic_pointer_cast<test_object_derived>(ptr_orig);

        REQUIRE(instances == 1);
        REQUIRE(ptr_orig.get() == raw_ptr);
        REQUIRE(ptr.get() == raw_ptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("dynamic pointer cast observer copy from invalid", "[pointer_cast]") {
    memory_tracker mem_track;

    {
        test_sptr_from_this ptr_owner =
            oup::make_observable_sealed<test_object_observer_from_this_sealed>();
        test_optr         ptr_orig{ptr_owner};
        test_optr_derived ptr = oup::dynamic_pointer_cast<test_object_derived>(ptr_orig);

        REQUIRE(instances == 1);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr_orig.get() != nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("dynamic pointer cast observer copy from null", "[pointer_cast]") {
    memory_tracker mem_track;

    {
        test_optr         ptr_orig;
        test_optr_derived ptr = oup::dynamic_pointer_cast<test_object_derived>(ptr_orig);

        REQUIRE(instances == 0);
        REQUIRE(ptr_orig == nullptr);
        REQUIRE(ptr == nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("dynamic pointer cast observer move from valid", "[pointer_cast]") {
    memory_tracker mem_track;

    {
        test_sptr_derived    ptr_owner = oup::make_observable_sealed<test_object_derived>();
        test_object_derived* raw_ptr   = ptr_owner.get();
        test_optr            ptr_orig{ptr_owner};
        test_optr_derived ptr = oup::dynamic_pointer_cast<test_object_derived>(std::move(ptr_orig));

        REQUIRE(instances == 1);
        REQUIRE(ptr_orig == nullptr);
        REQUIRE(ptr.get() == raw_ptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("dynamic pointer cast observer move from invalid", "[pointer_cast]") {
    memory_tracker mem_track;

    {
        test_sptr_from_this ptr_owner =
            oup::make_observable_sealed<test_object_observer_from_this_sealed>();
        test_optr         ptr_orig{ptr_owner};
        test_optr_derived ptr = oup::dynamic_pointer_cast<test_object_derived>(std::move(ptr_orig));

        REQUIRE(instances == 1);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr_orig.get() == nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}

TEST_CASE("dynamic pointer cast observer move from null", "[pointer_cast]") {
    memory_tracker mem_track;

    {
        test_optr         ptr_orig;
        test_optr_derived ptr = oup::dynamic_pointer_cast<test_object_derived>(std::move(ptr_orig));

        REQUIRE(instances == 0);
        REQUIRE(ptr_orig == nullptr);
        REQUIRE(ptr == nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.allocated() == 0u);
    REQUIRE(mem_track.double_delete() == 0u);
}
