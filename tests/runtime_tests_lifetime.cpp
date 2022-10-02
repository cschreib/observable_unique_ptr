#include "memory_tracker.hpp"
#include "testing.hpp"
#include "tests_common.hpp"

TEMPLATE_LIST_TEST_CASE("observer expiring scope", "[lifetime],[owner],[observer]", owner_types) {
    memory_tracker mem_track;

    {
        observer_ptr<TestType> optr;
        {
            TestType ptr     = make_pointer_deleter_1<TestType>();
            auto*    ptr_raw = ptr.get();

            optr = ptr;
            CHECK(optr.get() == ptr_raw);
            CHECK(!optr.expired());
        }

        CHECK(optr.get() == nullptr);
        CHECK(optr.expired());
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
    "observer not expiring when owner moved", "[lifetime],[owner],[observer]", owner_types) {
    memory_tracker mem_track;

    {
        TestType outer_ptr;
        {
            observer_ptr<TestType> optr;
            {
                TestType ptr     = make_pointer_deleter_1<TestType>();
                auto*    ptr_raw = ptr.get();

                optr = ptr;
                CHECK(optr.get() == ptr_raw);
                CHECK(!optr.expired());

                outer_ptr = std::move(ptr);
                CHECK(optr.get() == ptr_raw);
                CHECK(!optr.expired());

                CHECK(instances == 1);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(instances_deleter == 2);
                }
            }

            CHECK(optr.get() == outer_ptr.get());
            CHECK(!optr.expired());
            CHECK(instances == 1);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
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

TEMPLATE_LIST_TEST_CASE("observer expiring reset", "[lifetime],[owner],[observer]", owner_types) {
    memory_tracker mem_track;

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
        CHECK(instances == 0);
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
    "release valid owner with observer", "[lifetime],[release],[owner],[observer]", owner_types) {
    if constexpr (!is_sealed<TestType>) {
        memory_tracker mem_track;

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

                CHECK(instances == 1);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(instances_deleter == 1);
                    CHECK(ptr.get_deleter().state_ == test_deleter::state::special_init_1);
                }

                delete ptr_released;
            }

            CHECK(optr.get() == nullptr);
            CHECK(optr.expired());
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
    "release valid owner with observer subobject",
    "[lifetime],[release],[owner],[observer]",
    owner_types) {
    if constexpr (!is_sealed<TestType>) {
        memory_tracker mem_track;

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

                CHECK(instances == 1);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(instances_deleter == 1);
                    CHECK(ptr.get_deleter().state_ == test_deleter::state::special_init_1);
                }

                delete ptr_released;
            }

            CHECK(optr.get() == nullptr);
            CHECK(optr.expired());
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
    "observer get and raw get", "[lifetime],[get],[raw_get],[owner],[observer]", owner_types) {
    memory_tracker mem_track;

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

    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};
