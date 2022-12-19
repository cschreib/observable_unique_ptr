#include "memory_tracker.hpp"
#include "testing.hpp"
#include "tests_common.hpp"

#include <algorithm>
#include <vector>

TEMPLATE_LIST_TEST_CASE("observer expiring scope", "[lifetime][owner][observer]", owner_types) {
    volatile memory_tracker mem_track;

    {
        observer_ptr<TestType> optr;
        {
            TestType ptr     = make_pointer_deleter_1<TestType>();
            auto*    ptr_raw = ptr.get();
            optr             = ptr;

            CHECK(optr.get() == ptr_raw);
            CHECK(!optr.expired());
        }

        CHECK(optr.get() == nullptr);
        CHECK(optr.expired());
        CHECK_INSTANCES(0, 0);
    }

    CHECK_NO_LEAKS;
}

TEMPLATE_LIST_TEST_CASE(
    "observer not expiring when owner moved", "[lifetime][owner][observer]", owner_types) {
    volatile memory_tracker mem_track;

    {
        TestType outer_ptr;
        {
            observer_ptr<TestType> optr;
            {
                TestType ptr     = make_pointer_deleter_1<TestType>();
                auto*    ptr_raw = ptr.get();
                optr             = ptr;

                CHECK(optr.get() == ptr_raw);
                CHECK(!optr.expired());

                outer_ptr = std::move(ptr);

                CHECK(optr.get() == ptr_raw);
                CHECK(!optr.expired());
                CHECK_INSTANCES(1, 2);
            }

            CHECK(optr.get() == outer_ptr.get());
            CHECK(!optr.expired());
            CHECK_INSTANCES(1, 1);
        }

        CHECK_INSTANCES(1, 1);
    }

    CHECK_NO_LEAKS;
}

TEMPLATE_LIST_TEST_CASE("observer expiring reset", "[lifetime][owner][observer]", owner_types) {
    volatile memory_tracker mem_track;

    {
        observer_ptr<TestType> optr;
        TestType               ptr     = make_pointer_deleter_1<TestType>();
        auto*                  ptr_raw = ptr.get();

        optr = ptr;

        CHECK(optr.get() == ptr_raw);
        CHECK(!optr.expired());

        ptr.reset();

        CHECK(optr.get() == nullptr);
        CHECK(optr.expired());
        CHECK_INSTANCES(0, 1);
    }

    CHECK_NO_LEAKS;
}

TEMPLATE_LIST_TEST_CASE(
    "release valid owner with observer", "[lifetime][release][owner][observer]", owner_types) {
    if constexpr (!is_sealed<TestType>) {
        volatile memory_tracker mem_track;

        {
            observer_ptr<TestType> optr;
            {
                TestType ptr     = make_pointer_deleter_1<TestType>();
                auto*    ptr_raw = ptr.get();

                optr = ptr;

                CHECK(optr.get() == ptr_raw);
                CHECK(!optr.expired());

                auto* ptr_released = ptr.release();

                CHECK(ptr_released == ptr_raw);
                CHECK(ptr.get() == nullptr);
                if constexpr (has_eoft<TestType>) {
                    CHECK(optr.get() != nullptr);
                    CHECK(!optr.expired());
                } else {
                    CHECK(optr.get() == nullptr);
                    CHECK(optr.expired());
                }
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(ptr.get_deleter().state_ == test_deleter::state::special_init_1);
                }
                CHECK_INSTANCES(1, 1);

                delete ptr_released;
            }

            CHECK(optr.get() == nullptr);
            CHECK(optr.expired());
            CHECK_INSTANCES(0, 0);
        }

        CHECK_NO_LEAKS;
    }
}

TEMPLATE_LIST_TEST_CASE(
    "release valid owner with observer subobject",
    "[lifetime][release][owner][observer]",
    owner_types) {
    if constexpr (!is_sealed<TestType>) {
        volatile memory_tracker mem_track;

        {
            state_observer_ptr<TestType> optr;
            {
                TestType ptr     = make_pointer_deleter_1<TestType>();
                auto*    ptr_raw = ptr.get();

                optr = state_observer_ptr<TestType>{ptr, &ptr->state_};
                CHECK(optr.get() == &ptr_raw->state_);
                CHECK(!optr.expired());

                auto* ptr_released = ptr.release();
                CHECK(ptr_released == ptr_raw);
                CHECK(ptr.get() == nullptr);
                if constexpr (has_eoft<TestType>) {
                    CHECK(optr.get() != nullptr);
                    CHECK(!optr.expired());
                } else {
                    CHECK(optr.get() == nullptr);
                    CHECK(optr.expired());
                }
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(ptr.get_deleter().state_ == test_deleter::state::special_init_1);
                }
                CHECK_INSTANCES(1, 1);

                delete ptr_released;
            }

            CHECK(optr.get() == nullptr);
            CHECK(optr.expired());
            CHECK_INSTANCES(0, 0);
        }

        CHECK_NO_LEAKS;
    }
}

TEMPLATE_LIST_TEST_CASE(
    "observer get and raw get", "[lifetime][get][raw_get][owner][observer]", owner_types) {
    volatile memory_tracker mem_track;

    {
        observer_ptr<TestType> optr;

        CHECK(optr.raw_get() == nullptr);
        CHECK(optr.get() == nullptr);

        TestType ptr = make_pointer_deleter_1<TestType>();
        optr         = ptr;

        CHECK(optr.raw_get() == ptr.get());
        CHECK(optr.get() == ptr.get());

        get_object<TestType>* raw_ptr = ptr.get();
        ptr.reset();

        CHECK(optr.raw_get() == raw_ptr);
        CHECK(optr.get() == nullptr);
    }

    CHECK_NO_LEAKS;
}

TEMPLATE_LIST_TEST_CASE(
    "object owning observer pointer to itself",
    "[lifetime][cycles][owner][observer]",
    owner_types) {
    if constexpr (is_cyclic<TestType>) {
        volatile memory_tracker mem_track;

        {
            TestType ptr = make_pointer_deleter_1<TestType>();
            ptr->obs     = ptr;

            CHECK_INSTANCES(1, 1);
        }

        CHECK_NO_LEAKS;
    }
}

TEMPLATE_LIST_TEST_CASE(
    "object owning observer pointer to other", "[lifetime][cycles][owner][observer]", owner_types) {
    if constexpr (is_cyclic<TestType>) {
        volatile memory_tracker mem_track;

        {
            TestType ptr1 = make_pointer_deleter_1<TestType>();
            TestType ptr2 = make_pointer_deleter_2<TestType>();
            ptr1->obs     = ptr2;
            ptr2->obs     = ptr1;

            CHECK_INSTANCES(2, 2);
        }

        CHECK_NO_LEAKS;
    }
}

TEMPLATE_LIST_TEST_CASE(
    "object owning observer pointer open chain",
    "[lifetime][cycles][owner][observer]",
    owner_types) {
    if constexpr (is_cyclic<TestType>) {
        volatile memory_tracker mem_track;

        {
            TestType ptr1 = make_pointer_deleter_1<TestType>();
            TestType ptr2 = make_pointer_deleter_2<TestType>();
            TestType ptr3 = make_pointer_deleter_1<TestType>();
            ptr1->obs     = ptr2;
            ptr2->obs     = ptr3;

            CHECK_INSTANCES(3, 3);
        }

        CHECK_NO_LEAKS;
    }
}

TEMPLATE_LIST_TEST_CASE(
    "object owning observer pointer open chain reversed",
    "[lifetime][cycles][owner][observer]",
    owner_types) {
    if constexpr (is_cyclic<TestType>) {
        volatile memory_tracker mem_track;

        {
            TestType ptr1 = make_pointer_deleter_1<TestType>();
            TestType ptr2 = make_pointer_deleter_2<TestType>();
            TestType ptr3 = make_pointer_deleter_1<TestType>();
            ptr3->obs     = ptr2;
            ptr2->obs     = ptr1;

            CHECK_INSTANCES(3, 3);
        }

        CHECK_NO_LEAKS;
    }
}

TEMPLATE_LIST_TEST_CASE(
    "object owning observer pointer closed chain interleaved",
    "[lifetime][cycles][owner][observer]",
    owner_types) {
    if constexpr (is_cyclic<TestType>) {
        volatile memory_tracker mem_track;

        {
            TestType ptr1 = make_pointer_deleter_1<TestType>();
            TestType ptr2 = make_pointer_deleter_2<TestType>();
            TestType ptr3 = make_pointer_deleter_1<TestType>();
            TestType ptr4 = make_pointer_deleter_2<TestType>();
            ptr1->obs     = ptr2;
            ptr2->obs     = ptr4;
            ptr3->obs     = ptr1;
            ptr4->obs     = ptr3;

            CHECK_INSTANCES(4, 4);
        }

        CHECK_NO_LEAKS;
    }
}

TEMPLATE_LIST_TEST_CASE("pointers in vector", "[lifetime][array][owner][observer]", owner_types) {
    volatile memory_tracker mem_track;

    {
        std::vector<TestType>               vec_own;
        std::vector<observer_ptr<TestType>> vec_obs;

        vec_own.resize(100);

        CHECK(std::all_of(vec_own.begin(), vec_own.end(), [](const auto& p) {
                  return p == nullptr;
              }) == true);

        std::generate(
            vec_own.begin(), vec_own.end(), []() { return make_pointer_deleter_1<TestType>(); });

        CHECK(std::none_of(vec_own.begin(), vec_own.end(), [](const auto& p) {
                  return p == nullptr;
              }) == true);

        vec_obs.resize(100);

        CHECK(std::all_of(vec_obs.begin(), vec_obs.end(), [](const auto& p) {
                  return p == nullptr;
              }) == true);

        std::copy(vec_own.begin(), vec_own.end(), vec_obs.begin());

        CHECK(std::none_of(vec_own.begin(), vec_own.end(), [](const auto& p) {
                  return p == nullptr;
              }) == true);

        std::vector<TestType> vec_own_new = std::move(vec_own);

        CHECK(std::none_of(vec_own.begin(), vec_own.end(), [](const auto& p) {
                  return p == nullptr;
              }) == true);

        vec_own_new.clear();

        CHECK(std::all_of(vec_obs.begin(), vec_obs.end(), [](const auto& p) {
                  return p == nullptr;
              }) == true);
    }

    CHECK_NO_LEAKS;
}
