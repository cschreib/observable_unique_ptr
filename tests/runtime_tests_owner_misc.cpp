#include "memory_tracker.hpp"
#include "testing.hpp"
#include "tests_common.hpp"

TEMPLATE_LIST_TEST_CASE("owner size", "[size],[owner]", owner_types) {
    using deleter_type = get_deleter<TestType>;

    constexpr auto round_up = [](std::size_t i, std::size_t m) {
        return i % m == 0 ? i : i + m - i % m;
    };

    // The deleter should have no overhead when stateless.
    // Otherwise, the overhead should be exactly the size of the deleter, modulo alignment.
    constexpr std::size_t deleter_overhead =
        std::is_empty_v<deleter_type>
            ? 0
            : round_up(sizeof(deleter_type), std::max(alignof(deleter_type), alignof(void*)));

    CHECK(sizeof(TestType) == 2 * sizeof(void*) + deleter_overhead);
};

TEMPLATE_LIST_TEST_CASE("owner reset to null", "[reset],[owner]", owner_types) {
    memory_tracker mem_track;

    {
        TestType ptr = make_pointer_deleter_1<TestType>();
        ptr.reset();
        CHECK(instances == 0);
        CHECK(ptr.get() == nullptr);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 1);
            CHECK(ptr.get_deleter().state_ == test_deleter::state::special_init_1);
        }
    }

    CHECK(instances == 0);
    if constexpr (has_stateful_deleter<TestType>) {
        CHECK(instances_deleter == 0);
    }

    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE("owner reset to new", "[reset],[owner]", owner_types) {
    if constexpr (!must_use_make_observable<TestType>) {
        memory_tracker mem_track;

        {
            TestType ptr          = make_pointer_deleter_1<TestType>();
            auto*    raw_ptr_orig = ptr.get();
            ptr.reset(make_instance<TestType>());
            CHECK(instances == 1);
            CHECK(ptr.get() != nullptr);
            CHECK(ptr.get() != raw_ptr_orig);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
                CHECK(ptr.get_deleter().state_ == test_deleter::state::special_init_1);
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

TEMPLATE_LIST_TEST_CASE("owner reset to new bad alloc", "[reset],[owner]", owner_types) {
    if constexpr (!must_use_make_observable<TestType>) {
        memory_tracker mem_track;

        {
            auto*    raw_ptr1 = make_instance<TestType>();
            auto*    raw_ptr2 = make_instance<TestType>();
            TestType ptr(raw_ptr1);
            bool     has_thrown = false;
            try {
                force_next_allocation_failure = true;
                ptr.reset(raw_ptr2);
                force_next_allocation_failure = false;
            } catch (const std::bad_alloc&) {
                has_thrown = true;
            }

            if constexpr (eoft_allocates<TestType>) {
                CHECK(!has_thrown);
            } else {
                CHECK(has_thrown);
            }

            CHECK(instances == 1);
            if (has_thrown) {
                CHECK(ptr.get() != raw_ptr2);
                CHECK(ptr.get() == raw_ptr1);
            } else {
                CHECK(ptr.get() != raw_ptr1);
                CHECK(ptr.get() == raw_ptr2);
            }
        }

        CHECK(instances == 0);
        CHECK(mem_track.allocated() == 0u);
        CHECK(mem_track.double_delete() == 0u);
    }
};

TEMPLATE_LIST_TEST_CASE("owner swap empty vs empty", "[swap],[owner]", owner_types) {
    memory_tracker mem_track;

    {
        TestType ptr1 = make_empty_pointer_deleter_1<TestType>();
        TestType ptr2 = make_empty_pointer_deleter_2<TestType>();
        ptr2.swap(ptr1);
        CHECK(instances == 0);
        CHECK(ptr1.get() == nullptr);
        CHECK(ptr2.get() == nullptr);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 2);
            CHECK(ptr1.get_deleter().state_ == test_deleter::state::special_init_2);
            CHECK(ptr2.get_deleter().state_ == test_deleter::state::special_init_1);
        }
    }

    CHECK(instances == 0);
    if constexpr (has_stateful_deleter<TestType>) {
        CHECK(instances_deleter == 0);
    }

    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE("owner swap valid vs empty", "[swap],[owner]", owner_types) {
    memory_tracker mem_track;

    {
        TestType ptr1 = make_pointer_deleter_1<TestType>();
        TestType ptr2 = make_empty_pointer_deleter_2<TestType>();
        ptr2.swap(ptr1);
        CHECK(instances == 1);
        CHECK(ptr1.get() == nullptr);
        CHECK(ptr2.get() != nullptr);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 2);
            CHECK(ptr1.get_deleter().state_ == test_deleter::state::special_init_2);
            CHECK(ptr2.get_deleter().state_ == test_deleter::state::special_init_1);
        }
    }

    CHECK(instances == 0);
    if constexpr (has_stateful_deleter<TestType>) {
        CHECK(instances_deleter == 0);
    }

    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE("owner swap empty vs valid", "[swap],[owner]", owner_types) {
    memory_tracker mem_track;

    {
        TestType ptr1 = make_empty_pointer_deleter_1<TestType>();
        TestType ptr2 = make_pointer_deleter_2<TestType>();
        ptr2.swap(ptr1);
        CHECK(instances == 1);
        CHECK(ptr1.get() != nullptr);
        CHECK(ptr2.get() == nullptr);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 2);
            CHECK(ptr1.get_deleter().state_ == test_deleter::state::special_init_2);
            CHECK(ptr2.get_deleter().state_ == test_deleter::state::special_init_1);
        }
    }

    CHECK(instances == 0);
    if constexpr (has_stateful_deleter<TestType>) {
        CHECK(instances_deleter == 0);
    }

    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE("owner swap valid vs valid", "[swap],[owner]", owner_types) {
    memory_tracker mem_track;

    {
        TestType ptr1     = make_pointer_deleter_1<TestType>();
        TestType ptr2     = make_pointer_deleter_2<TestType>();
        auto*    raw_ptr1 = ptr1.get();
        auto*    raw_ptr2 = ptr2.get();
        ptr2.swap(ptr1);
        CHECK(instances == 2);
        CHECK(ptr1.get() != raw_ptr1);
        CHECK(ptr1.get() == raw_ptr2);
        CHECK(ptr2.get() != raw_ptr2);
        CHECK(ptr2.get() == raw_ptr1);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 2);
            CHECK(ptr1.get_deleter().state_ == test_deleter::state::special_init_2);
            CHECK(ptr2.get_deleter().state_ == test_deleter::state::special_init_1);
        }
    }

    CHECK(instances == 0);
    if constexpr (has_stateful_deleter<TestType>) {
        CHECK(instances_deleter == 0);
    }

    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE("owner swap self vs self empty", "[swap],[owner]", owner_types) {
    memory_tracker mem_track;

    {
        TestType ptr = make_empty_pointer_deleter_1<TestType>();
        ptr.swap(ptr);
        CHECK(instances == 0);
        CHECK(ptr.get() == nullptr);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(ptr.get_deleter().state_ == test_deleter::state::special_init_1);
        }
    }

    CHECK(instances == 0);
    if constexpr (has_stateful_deleter<TestType>) {
        CHECK(instances_deleter == 0);
    }

    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE("owner swap self vs self valid", "[swap],[owner]", owner_types) {
    memory_tracker mem_track;

    {
        TestType ptr = make_pointer_deleter_1<TestType>();
        ptr.swap(ptr);
        CHECK(instances == 1);
        CHECK(ptr.get() != nullptr);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(ptr.get_deleter().state_ == test_deleter::state::special_init_1);
        }
    }

    CHECK(instances == 0);
    if constexpr (has_stateful_deleter<TestType>) {
        CHECK(instances_deleter == 0);
    }

    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE("owner dereference", "[dereference],[owner]", owner_types) {
    memory_tracker mem_track;

    {
        TestType ptr = make_pointer_deleter_1<TestType>();
        CHECK(ptr->state_ == test_object::state::default_init);
        CHECK((*ptr).state_ == test_object::state::default_init);
    }

    CHECK(instances == 0);
    if constexpr (has_stateful_deleter<TestType>) {
        CHECK(instances_deleter == 0);
    }

    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE("owner get valid", "[get],[owner]", owner_types) {
    memory_tracker mem_track;

    {
        TestType ptr = make_pointer_deleter_1<TestType>();
        CHECK(ptr.get() != nullptr);
        CHECK(ptr.get()->state_ == test_object::state::default_init);
    }

    CHECK(instances == 0);
    if constexpr (has_stateful_deleter<TestType>) {
        CHECK(instances_deleter == 0);
    }

    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE("owner get empty", "[get],[owner]", owner_types) {
    memory_tracker mem_track;

    {
        TestType ptr = make_empty_pointer_deleter_1<TestType>();
        CHECK(ptr.get() == nullptr);
    }

    CHECK(instances == 0);
    if constexpr (has_stateful_deleter<TestType>) {
        CHECK(instances_deleter == 0);
    }

    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE("owner operator bool valid", "[bool],[owner]", owner_types) {
    memory_tracker mem_track;

    {
        TestType ptr = make_pointer_deleter_1<TestType>();
        if (ptr) {
        } else {
            FAIL("if (ptr) should have been true");
        }
    }

    CHECK(instances == 0);
    if constexpr (has_stateful_deleter<TestType>) {
        CHECK(instances_deleter == 0);
    }

    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE("owner operator bool empty", "[bool],[owner]", owner_types) {
    memory_tracker mem_track;

    {
        TestType ptr = make_empty_pointer_deleter_1<TestType>();
        if (ptr) {
            FAIL("if (ptr) should have been true");
        }
    }

    CHECK(instances == 0);
    if constexpr (has_stateful_deleter<TestType>) {
        CHECK(instances_deleter == 0);
    }

    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE("owner release valid", "[release],[owner]", owner_types) {
    if constexpr (!is_sealed<TestType>) {
        memory_tracker mem_track;

        {
            TestType ptr          = make_pointer_deleter_1<TestType>();
            auto*    ptr_raw      = ptr.get();
            auto*    ptr_released = ptr.release();
            CHECK(ptr_released == ptr_raw);
            CHECK(ptr.get() == nullptr);
            CHECK(instances == 1);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
                CHECK(ptr.get_deleter().state_ == test_deleter::state::special_init_1);
            }
            delete ptr_released;
        }

        CHECK(instances == 0);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 0);
        }

        CHECK(mem_track.allocated() == 0u);
        CHECK(mem_track.double_delete() == 0u);
    }
};

TEMPLATE_LIST_TEST_CASE("owner release empty", "[release],[owner]", owner_types) {
    if constexpr (!is_sealed<TestType>) {
        memory_tracker mem_track;

        {
            TestType ptr          = make_empty_pointer_deleter_1<TestType>();
            auto*    ptr_released = ptr.release();
            CHECK(ptr_released == nullptr);
            CHECK(ptr.get() == nullptr);
            CHECK(instances == 0);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
                CHECK(ptr.get_deleter().state_ == test_deleter::state::special_init_1);
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
