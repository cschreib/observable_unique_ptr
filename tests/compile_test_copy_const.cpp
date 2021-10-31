#include "tests_common.hpp"

int main() {
    test_ptr ptr_orig(new test_object);
    test_ptr ptr(ptr_orig);
    return 0;
}
