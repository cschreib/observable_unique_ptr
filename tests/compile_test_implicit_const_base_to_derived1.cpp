#include "tests_common.hpp"

int main() {
    oup::observable_unique_ptr<test_object_derived> ptr(new test_object);
    return 0;
}
