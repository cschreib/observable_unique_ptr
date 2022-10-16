#include "tests_common.hpp"

int main() {
    oup::observable_unique_ptr<test_object>         ptr_orig(new test_object);
    oup::observable_unique_ptr<test_object_derived> ptr(std::move(ptr_orig));
    return 0;
}
