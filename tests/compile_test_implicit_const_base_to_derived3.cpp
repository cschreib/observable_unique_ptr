#include "tests_common.hpp"

int main() {
    test_ptr_with_deleter ptr_orig(new test_object, test_deleter{42});
    test_ptr_derived_with_deleter ptr(std::move(ptr_orig));
    return 0;
}
