#include "memory_tracker.hpp"
#include "testing.hpp"
#include "tests_common.hpp"

TEMPLATE_LIST_TEST_CASE("observer size", "[size][observer]", owner_types) {
    CHECK(sizeof(observer_ptr<TestType>) == 2 * sizeof(void*));
};

TEMPLATE_LIST_TEST_CASE("observer reset to null", "[reset][observer]", owner_types) {
    memory_tracker mem_track;

    {
        TestType               ptr = make_pointer_deleter_1<TestType>();
        observer_ptr<TestType> optr{ptr};
        optr.reset();
        CHECK(optr.get() == nullptr);
        CHECK(optr.expired() == true);
        CHECK_INSTANCES(1, 1);
    }

    CHECK_NO_LEAKS;
};

TEMPLATE_LIST_TEST_CASE("observer swap empty vs empty", "[swap][observer]", owner_types) {
    memory_tracker mem_track;

    {
        observer_ptr<TestType> optr1;
        observer_ptr<TestType> optr2;
        optr2.swap(optr1);
        CHECK_INSTANCES(0, 0);
        CHECK(optr1.get() == nullptr);
        CHECK(optr2.get() == nullptr);
        CHECK(optr1.expired() == true);
        CHECK(optr2.expired() == true);
    }

    CHECK_NO_LEAKS;
};

TEMPLATE_LIST_TEST_CASE("observer swap valid vs empty", "[swap][observer]", owner_types) {
    memory_tracker mem_track;

    {
        TestType               ptr1 = make_pointer_deleter_1<TestType>();
        observer_ptr<TestType> optr1{ptr1};
        observer_ptr<TestType> optr2;
        optr2.swap(optr1);
        CHECK_INSTANCES(1, 1);
        CHECK(optr1.get() == nullptr);
        CHECK(optr2.get() != nullptr);
        CHECK(optr2.get() == ptr1.get());
        CHECK(optr1.expired() == true);
        CHECK(optr2.expired() == false);
    }

    CHECK_NO_LEAKS;
};

TEMPLATE_LIST_TEST_CASE("observer swap empty vs valid", "[swap][observer]", owner_types) {
    memory_tracker mem_track;

    {
        TestType               ptr2 = make_pointer_deleter_2<TestType>();
        observer_ptr<TestType> optr1;
        observer_ptr<TestType> optr2{ptr2};
        optr2.swap(optr1);
        CHECK_INSTANCES(1, 1);
        CHECK(optr1.get() != nullptr);
        CHECK(optr1.get() == ptr2.get());
        CHECK(optr2.get() == nullptr);
        CHECK(optr1.expired() == false);
        CHECK(optr2.expired() == true);
    }

    CHECK_NO_LEAKS;
};

TEMPLATE_LIST_TEST_CASE("observer swap valid vs valid", "[swap][observer]", owner_types) {
    memory_tracker mem_track;

    {
        TestType               ptr1 = make_pointer_deleter_1<TestType>();
        TestType               ptr2 = make_pointer_deleter_2<TestType>();
        observer_ptr<TestType> optr1{ptr1};
        observer_ptr<TestType> optr2{ptr2};
        optr2.swap(optr1);
        CHECK_INSTANCES(2, 2);
        CHECK(optr1.get() != ptr1.get());
        CHECK(optr1.get() == ptr2.get());
        CHECK(optr2.get() != ptr2.get());
        CHECK(optr2.get() == ptr1.get());
        CHECK(optr1.expired() == false);
        CHECK(optr2.expired() == false);
    }

    CHECK_NO_LEAKS;
};

TEMPLATE_LIST_TEST_CASE(
    "observer swap valid vs valid same instance", "[swap][observer]", owner_types) {
    memory_tracker mem_track;

    {
        TestType               ptr = make_pointer_deleter_1<TestType>();
        observer_ptr<TestType> optr1{ptr};
        observer_ptr<TestType> optr2{ptr};
        optr2.swap(optr1);
        CHECK_INSTANCES(1, 1);
        CHECK(optr1.get() == ptr.get());
        CHECK(optr2.get() == ptr.get());
        CHECK(optr1.expired() == false);
        CHECK(optr2.expired() == false);
    }

    CHECK_NO_LEAKS;
};

TEMPLATE_LIST_TEST_CASE("observer swap self vs self empty", "[swap][observer]", owner_types) {
    memory_tracker mem_track;

    {
        observer_ptr<TestType> optr;
        optr.swap(optr);
        CHECK_INSTANCES(0, 0);
        CHECK(optr.get() == nullptr);
        CHECK(optr.expired() == true);
    }

    CHECK_NO_LEAKS;
};

TEMPLATE_LIST_TEST_CASE("observer swap self vs self valid", "[swap][observer]", owner_types) {
    memory_tracker mem_track;

    {
        TestType               ptr = make_pointer_deleter_1<TestType>();
        observer_ptr<TestType> optr{ptr};
        optr.swap(optr);
        CHECK_INSTANCES(1, 1);
        CHECK(optr.get() == ptr.get());
        CHECK(optr.expired() == false);
    }

    CHECK_NO_LEAKS;
};

TEMPLATE_LIST_TEST_CASE("observer dereference valid", "[dereference][observer]", owner_types) {
    memory_tracker mem_track;

    {
        TestType               ptr = make_pointer_deleter_1<TestType>();
        observer_ptr<TestType> optr{ptr};
        CHECK(optr->state_ == test_object::state::default_init);
        CHECK((*optr).state_ == test_object::state::default_init);
    }

    CHECK_NO_LEAKS;
};

TEMPLATE_LIST_TEST_CASE("observer get valid", "[get][observer]", owner_types) {
    memory_tracker mem_track;

    {
        TestType               ptr = make_pointer_deleter_1<TestType>();
        observer_ptr<TestType> optr{ptr};
        CHECK(optr.get() != nullptr);
        CHECK(optr.get()->state_ == test_object::state::default_init);
    }

    CHECK_NO_LEAKS;
};

TEMPLATE_LIST_TEST_CASE("observer get empty", "[get][observer]", owner_types) {
    memory_tracker mem_track;

    {
        observer_ptr<TestType> optr;
        CHECK(optr.get() == nullptr);
    }

    CHECK_NO_LEAKS;
};

TEMPLATE_LIST_TEST_CASE("observer raw_get valid", "[raw_get][observer]", owner_types) {
    memory_tracker mem_track;

    {
        TestType               ptr = make_pointer_deleter_1<TestType>();
        observer_ptr<TestType> optr{ptr};
        CHECK(optr.raw_get() != nullptr);
        CHECK(optr.raw_get()->state_ == test_object::state::default_init);
    }

    CHECK_NO_LEAKS;
};

TEMPLATE_LIST_TEST_CASE("observer raw_get empty", "[raw_get][observer]", owner_types) {
    memory_tracker mem_track;

    {
        observer_ptr<TestType> optr;
        CHECK(optr.raw_get() == nullptr);
    }

    CHECK_NO_LEAKS;
};

TEMPLATE_LIST_TEST_CASE("observer operator bool valid", "[bool][observer]", owner_types) {
    memory_tracker mem_track;

    {
        TestType               ptr = make_pointer_deleter_1<TestType>();
        observer_ptr<TestType> optr{ptr};
        if (optr) {
        } else {
            FAIL("if (optr) should have been true");
        }
    }

    CHECK_NO_LEAKS;
};

TEMPLATE_LIST_TEST_CASE("observer operator bool empty", "[bool][observer]", owner_types) {
    memory_tracker mem_track;

    {
        observer_ptr<TestType> optr;
        if (optr) {
            FAIL("if (optr) should have been false");
        }
    }

    CHECK_NO_LEAKS;
};
