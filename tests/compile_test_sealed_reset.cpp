#include "tests_common.hpp"

int main() {
    test_sptr ptr;
    ptr.reset(new test_object);
    return 0;
}
