#include "tests_common.hpp"

int main() {
    oup::observable_sealed_ptr<test_object> ptr;
    ptr.release();
    return 0;
}
