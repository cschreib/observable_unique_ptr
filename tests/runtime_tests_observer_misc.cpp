#include "catch2_and_overrides.hpp"
#include "memory_tracker.hpp"
#include "tests_common2.hpp"

TEMPLATE_LIST_TEST_CASE("observer size", "[size],[observer]", owner_types) {
    REQUIRE(sizeof(observer_ptr<TestType>) == 2 * sizeof(void*));
}
