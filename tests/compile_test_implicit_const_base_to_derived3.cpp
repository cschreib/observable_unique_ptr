#include "tests_common.hpp"

int main() {
    oup::observable_unique_ptr<test_object, test_deleter> ptr_orig(
        new test_object, test_deleter{42});
    oup::observable_unique_ptr<test_object_derived, test_deleter> ptr(std::move(ptr_orig));
    return 0;
}
