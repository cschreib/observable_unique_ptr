#include "tests_common.hpp"

int main() {
    oup::observer_ptr<int> p(oup::make_observable_sealed<int>(1));
    return 0;
}
