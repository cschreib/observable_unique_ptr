#include "tests_common.hpp"

int main() {
    oup::observable_unique_ptr<test_object> ptr_orig(new test_object);
    oup::observable_unique_ptr<test_object> ptr(ptr_orig);
    return 0;
}
