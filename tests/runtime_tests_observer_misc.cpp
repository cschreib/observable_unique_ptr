#include "memory_tracker.hpp"
#include "testing.hpp"
#include "tests_common.hpp"

TEMPLATE_LIST_TEST_CASE("observer size", "[size],[observer]", owner_types) {
    CHECK(sizeof(observer_ptr<TestType>) == 2 * sizeof(void*));
};
