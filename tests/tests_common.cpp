#include "tests_common.hpp"

int instances         = 0;
int instances_derived = 0;
int instances_deleter = 0;

bool next_test_object_constructor_throws = false;
