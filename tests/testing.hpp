#include "snitch/snitch_macros_check.hpp"
#include "snitch/snitch_macros_exceptions.hpp"
#include "snitch/snitch_macros_test_case.hpp"
#include "tests_common.hpp"

// clang-format off
using owner_types = snitch::type_list<
    oup::observable_unique_ptr<test_object>,
    oup::observable_sealed_ptr<test_object>,
    oup::observable_unique_ptr<const test_object>,
    oup::observable_sealed_ptr<const test_object>,
    oup::observable_unique_ptr<test_object_derived>,
    oup::observable_sealed_ptr<test_object_derived>,
    oup::observable_unique_ptr<test_object, test_deleter>,
    oup::observable_unique_ptr<test_object_derived, test_deleter>,
    oup::observable_unique_ptr<test_object_observer_from_this_unique>,
    oup::observable_sealed_ptr<test_object_observer_from_this_sealed>,
    oup::basic_observable_ptr<test_object_observer_from_this_non_virtual_unique, oup::default_delete, unique_non_virtual_policy>,
    oup::basic_observable_ptr<test_object_observer_from_this_maybe_no_block_unique, oup::default_delete, unique_maybe_no_block_policy>,
    oup::basic_observable_ptr<test_object_observer_from_this_virtual_sealed, oup::placement_delete, sealed_virtual_policy>,
    oup::observable_unique_ptr<const test_object_observer_from_this_unique>,
    oup::observable_sealed_ptr<const test_object_observer_from_this_sealed>,
    oup::observable_unique_ptr<test_object_observer_from_this_derived_unique>,
    oup::observable_sealed_ptr<test_object_observer_from_this_derived_sealed>,
    oup::observable_unique_ptr<test_object_observer_from_this_multi_unique>,
    oup::observable_sealed_ptr<test_object_observer_from_this_multi_sealed>,
    oup::observable_unique_ptr<test_object_observer_from_this_constructor_unique>,
    oup::observable_sealed_ptr<test_object_observer_from_this_constructor_sealed>,
    oup::observable_unique_ptr<test_object_observer_from_this_constructor_multi_unique>,
    oup::observable_sealed_ptr<test_object_observer_from_this_constructor_multi_sealed>,
    oup::observable_unique_ptr<test_object_observer_owner>,
    oup::observable_sealed_ptr<test_object_observer_owner>
    >;
// clang-format on

#define CHECK_INSTANCES(TEST_OBJECTS, TEST_DELETER)                                                \
    do {                                                                                           \
        CHECK(instances == (TEST_OBJECTS));                                                        \
        if constexpr (has_stateful_deleter<TestType>) {                                            \
            CHECK(instances_deleter == (TEST_DELETER));                                            \
        }                                                                                          \
    } while (0)

#define CHECK_INSTANCES_DERIVED(TEST_OBJECTS, TEST_DERIVED, TEST_DELETER)                          \
    do {                                                                                           \
        CHECK(instances == (TEST_OBJECTS));                                                        \
        CHECK(instances_derived == (TEST_DERIVED));                                                \
        if constexpr (has_stateful_deleter<TestType>) {                                            \
            CHECK(instances_deleter == (TEST_DELETER));                                            \
        }                                                                                          \
    } while (0)

#define CHECK_NO_LEAKS                                                                             \
    do {                                                                                           \
        CHECK_INSTANCES_DERIVED(0, 0, 0);                                                          \
        CHECK(mem_track.allocated() == 0u);                                                        \
        CHECK(mem_track.double_delete() == 0u);                                                    \
    } while (0)

#if defined(NDEBUG)
// When not in debug (hence, assuming optimisations are turned on),
// some compilers manage to optimise-out some heap allocations, so use a looser
// check.
#    define CHECK_MAX_ALLOC(MAX_ALLOC) CHECK(mem_track.allocated() <= MAX_ALLOC)
#else
// In debug, allocations must be exactly as expected.
#    define CHECK_MAX_ALLOC(MAX_ALLOC) CHECK(mem_track.allocated() == MAX_ALLOC)
#endif

// clang-format off
#if defined(__clang__)
#    define SNITCH_WARNING_DISABLE_SELF_ASSIGN _Pragma("clang diagnostic ignored \"-Wself-assign-overloaded\"")
#elif defined(__GNUC__)
#    define SNITCH_WARNING_DISABLE_SELF_ASSIGN do {} while (0)
#elif defined(_MSC_VER)
#    define SNITCH_WARNING_DISABLE_SELF_ASSIGN do {} while (0)
#else
#    define SNITCH_WARNING_DISABLE_SELF_ASSIGN do {} while (0)
#endif
// clang-format on
