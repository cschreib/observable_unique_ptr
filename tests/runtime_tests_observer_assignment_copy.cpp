#include "memory_tracker.hpp"
#include "testing.hpp"
#include "tests_common.hpp"

TEMPLATE_LIST_TEST_CASE(
    "observer copy assignment operator valid to empty", "[assignment],[observer]", owner_types) {
    memory_tracker mem_track;

    {
        TestType               ptr_orig = make_pointer_deleter_1<TestType>();
        observer_ptr<TestType> optr_orig{ptr_orig};
        {
            observer_ptr<TestType> optr;
            optr = optr_orig;
            CHECK(instances == 1);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
            CHECK(optr.get() == ptr_orig.get());
            CHECK(optr_orig.get() == ptr_orig.get());
            CHECK(optr.expired() == false);
            CHECK(optr_orig.expired() == false);
        }

        CHECK(instances == 1);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 1);
        }
    }

    CHECK(instances == 0);
    if constexpr (has_stateful_deleter<TestType>) {
        CHECK(instances_deleter == 0);
    }

    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE(
    "observer copy assignment operator empty to valid", "[assignment],[observer]", owner_types) {
    memory_tracker mem_track;

    {
        observer_ptr<TestType> optr_orig;
        {
            TestType               ptr = make_pointer_deleter_1<TestType>();
            observer_ptr<TestType> optr{ptr};
            optr = optr_orig;
            CHECK(instances == 1);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
            CHECK(optr.get() == nullptr);
            CHECK(optr_orig.get() == nullptr);
            CHECK(optr.expired() == true);
            CHECK(optr_orig.expired() == true);
        }

        CHECK(instances == 0);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 0);
        }
    }

    CHECK(instances == 0);
    if constexpr (has_stateful_deleter<TestType>) {
        CHECK(instances_deleter == 0);
    }

    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE(
    "observer copy assignment operator empty to empty", "[assignment],[observer]", owner_types) {
    memory_tracker mem_track;

    {
        observer_ptr<TestType> optr_orig;
        {
            observer_ptr<TestType> optr;
            optr = optr_orig;
            CHECK(instances == 0);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 0);
            }
            CHECK(optr.get() == nullptr);
            CHECK(optr_orig.get() == nullptr);
            CHECK(optr.expired() == true);
            CHECK(optr_orig.expired() == true);
        }

        CHECK(instances == 0);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 0);
        }
    }

    CHECK(instances == 0);
    if constexpr (has_stateful_deleter<TestType>) {
        CHECK(instances_deleter == 0);
    }

    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE(
    "observer copy assignment operator valid to valid", "[assignment],[observer]", owner_types) {
    memory_tracker mem_track;

    {
        TestType               ptr_orig = make_pointer_deleter_1<TestType>();
        observer_ptr<TestType> optr_orig{ptr_orig};
        {
            TestType               ptr = make_pointer_deleter_1<TestType>();
            observer_ptr<TestType> optr{ptr};
            optr = optr_orig;
            CHECK(instances == 2);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 2);
            }
            CHECK(optr.get() == ptr_orig.get());
            CHECK(optr_orig.get() == ptr_orig.get());
            CHECK(optr.expired() == false);
            CHECK(optr_orig.expired() == false);
        }

        CHECK(instances == 1);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 1);
        }
    }

    CHECK(instances == 0);
    if constexpr (has_stateful_deleter<TestType>) {
        CHECK(instances_deleter == 0);
    }

    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE(
    "observer copy assignment converting operator valid to empty",
    "[assignment],[observer]",
    owner_types) {
    memory_tracker mem_track;

    if constexpr (has_base<TestType>) {
        {
            TestType               ptr_orig = make_pointer_deleter_1<TestType>();
            observer_ptr<TestType> optr_orig{ptr_orig};
            {
                base_observer_ptr<TestType> optr;
                optr = optr_orig;
                CHECK(instances == 1);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(instances_deleter == 1);
                }
                CHECK(optr.get() == ptr_orig.get());
                CHECK(optr_orig.get() == ptr_orig.get());
                CHECK(optr.expired() == false);
                CHECK(optr_orig.expired() == false);
            }

            CHECK(instances == 1);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
        }

        CHECK(instances == 0);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 0);
        }

        CHECK(mem_track.allocated() == 0u);
        CHECK(mem_track.double_delete() == 0u);
    }
};

TEMPLATE_LIST_TEST_CASE(
    "observer copy assignment converting operator empty to valid",
    "[assignment],[observer]",
    owner_types) {
    memory_tracker mem_track;

    if constexpr (has_base<TestType>) {
        {
            observer_ptr<TestType> optr_orig;
            {
                TestType                    ptr = make_pointer_deleter_1<TestType>();
                base_observer_ptr<TestType> optr{ptr};
                optr = optr_orig;
                CHECK(instances == 1);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(instances_deleter == 1);
                }
                CHECK(optr.get() == nullptr);
                CHECK(optr_orig.get() == nullptr);
                CHECK(optr.expired() == true);
                CHECK(optr_orig.expired() == true);
            }

            CHECK(instances == 0);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 0);
            }
        }

        CHECK(instances == 0);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 0);
        }

        CHECK(mem_track.allocated() == 0u);
        CHECK(mem_track.double_delete() == 0u);
    }
};

TEMPLATE_LIST_TEST_CASE(
    "observer copy assignment converting operator empty to empty",
    "[assignment],[observer]",
    owner_types) {
    memory_tracker mem_track;

    if constexpr (has_base<TestType>) {
        {
            observer_ptr<TestType> optr_orig;
            {
                base_observer_ptr<TestType> optr;
                optr = optr_orig;
                CHECK(instances == 0);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(instances_deleter == 0);
                }
                CHECK(optr.get() == nullptr);
                CHECK(optr_orig.get() == nullptr);
                CHECK(optr.expired() == true);
                CHECK(optr_orig.expired() == true);
            }

            CHECK(instances == 0);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 0);
            }
        }

        CHECK(instances == 0);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 0);
        }

        CHECK(mem_track.allocated() == 0u);
        CHECK(mem_track.double_delete() == 0u);
    }
};

TEMPLATE_LIST_TEST_CASE(
    "observer copy assignment converting operator valid to valid",
    "[assignment],[observer]",
    owner_types) {
    memory_tracker mem_track;

    if constexpr (has_base<TestType>) {
        {
            TestType               ptr_orig = make_pointer_deleter_1<TestType>();
            observer_ptr<TestType> optr_orig{ptr_orig};
            {
                TestType                    ptr = make_pointer_deleter_1<TestType>();
                base_observer_ptr<TestType> optr{ptr};
                optr = optr_orig;
                CHECK(instances == 2);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(instances_deleter == 2);
                }
                CHECK(optr.get() == ptr_orig.get());
                CHECK(optr_orig.get() == ptr_orig.get());
                CHECK(optr.expired() == false);
                CHECK(optr_orig.expired() == false);
            }

            CHECK(instances == 1);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
        }

        CHECK(instances == 0);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 0);
        }

        CHECK(mem_track.allocated() == 0u);
        CHECK(mem_track.double_delete() == 0u);
    }
};

TEMPLATE_LIST_TEST_CASE(
    "observer copy assignment operator self to self valid",
    "[assignment],[observer]",
    owner_types) {
    memory_tracker mem_track;

    {
        TestType               ptr = make_pointer_deleter_1<TestType>();
        observer_ptr<TestType> optr{ptr};
        optr = optr;
        CHECK(instances == 1);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 1);
        }
        CHECK(optr.get() == ptr.get());
        CHECK(optr.expired() == false);
    }

    CHECK(instances == 0);
    if constexpr (has_stateful_deleter<TestType>) {
        CHECK(instances_deleter == 0);
    }

    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE(
    "observer copy assignment operator self to self empty",
    "[assignment],[observer]",
    owner_types) {
    memory_tracker mem_track;

    {
        observer_ptr<TestType> optr;
        optr = optr;
        CHECK(instances == 0);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 0);
        }
        CHECK(optr.get() == nullptr);
        CHECK(optr.expired() == true);
    }

    CHECK(instances == 0);
    if constexpr (has_stateful_deleter<TestType>) {
        CHECK(instances_deleter == 0);
    }

    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};
