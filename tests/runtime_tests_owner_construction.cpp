#include "memory_tracker.hpp"
#include "testing.hpp"
#include "tests_common.hpp"

TEMPLATE_LIST_TEST_CASE("owner default constructor", "[construction],[owner]", owner_types) {
    memory_tracker mem_track;

    {
        TestType ptr;
        CHECK(instances == 0);
        CHECK(ptr.get() == nullptr);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 1);
            CHECK(ptr.get_deleter().state_ == test_deleter::state::default_init);
        }
    }

    CHECK(instances == 0);
    if constexpr (has_stateful_deleter<TestType>) {
        CHECK(instances_deleter == 0);
    }

    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE("owner nullptr constructor", "[construction],[owner]", owner_types) {
    memory_tracker mem_track;

    {
        TestType ptr(nullptr);
        CHECK(instances == 0);
        CHECK(ptr.get() == nullptr);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 1);
            CHECK(ptr.get_deleter().state_ == test_deleter::state::default_init);
        }
    }

    CHECK(instances == 0);
    if constexpr (has_stateful_deleter<TestType>) {
        CHECK(instances_deleter == 0);
    }

    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE("owner move constructor", "[construction],[owner]", owner_types) {
    memory_tracker mem_track;

    {
        TestType ptr_orig = make_pointer_deleter_1<TestType>();
        {
            TestType ptr(std::move(ptr_orig));
            CHECK(instances == 1);
            CHECK(ptr.get() != nullptr);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 2);
                CHECK(ptr.get_deleter().state_ == test_deleter::state::special_init_1);
            }
        }

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

TEMPLATE_LIST_TEST_CASE("owner acquiring constructor", "[construction],[owner]", owner_types) {
    if constexpr (!must_use_make_observable<TestType>) {
        memory_tracker mem_track;

        {
            TestType ptr(make_instance<TestType>());
            CHECK(instances == 1);
            CHECK(ptr.get() != nullptr);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
                CHECK(ptr.get_deleter().state_ == test_deleter::state::default_init);
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
    "owner acquiring constructor with deleter", "[construction],[owner]", owner_types) {
    if constexpr (!must_use_make_observable<TestType> && has_stateful_deleter<TestType>) {
        memory_tracker mem_track;

        {
            TestType ptr(make_instance<TestType>(), make_deleter_instance_1<TestType>());
            CHECK(instances == 1);
            CHECK(ptr.get() != nullptr);
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

TEMPLATE_LIST_TEST_CASE(
    "owner acquiring constructor bad alloc", "[construction],[owner]", owner_types) {
    if constexpr (!must_use_make_observable<TestType>) {
        memory_tracker mem_track;

        {
            auto* raw_ptr    = make_instance<TestType>();
            bool  has_thrown = false;
            try {
                force_next_allocation_failure = true;
                TestType{raw_ptr};
                force_next_allocation_failure = false;
            } catch (const std::bad_alloc&) {
                has_thrown = true;
            }

            if constexpr (eoft_allocates<TestType>) {
                CHECK(!has_thrown);
            } else {
                CHECK(has_thrown);
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
    "owner acquiring constructor bad alloc with deleter", "[construction],[owner]", owner_types) {
    if constexpr (
        !must_use_make_observable<TestType> && !eoft_allocates<TestType> &&
        has_stateful_deleter<TestType>) {
        memory_tracker mem_track;

        {
            auto* raw_ptr    = make_instance<TestType>();
            auto  deleter    = make_deleter_instance_1<TestType>();
            bool  has_thrown = false;
            try {
                force_next_allocation_failure = true;
                TestType{raw_ptr, deleter};
                force_next_allocation_failure = false;
            } catch (const std::bad_alloc&) {
                has_thrown = true;
            }

            CHECK(has_thrown);
        }

        CHECK(instances == 0);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 0);
        }

        CHECK(mem_track.allocated() == 0u);
        CHECK(mem_track.double_delete() == 0u);
    }
};

TEMPLATE_LIST_TEST_CASE("owner acquiring constructor null", "[construction],[owner]", owner_types) {
    if constexpr (!must_use_make_observable<TestType>) {
        memory_tracker mem_track;

        {
            TestType ptr(static_cast<get_object<TestType>*>(nullptr));
            CHECK(instances == 0);
            CHECK(ptr.get() == nullptr);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
                CHECK(ptr.get_deleter().state_ == test_deleter::state::default_init);
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
    "owner implicit conversion constructor", "[construction],[owner]", owner_types) {
    if constexpr (has_base<TestType>) {
        memory_tracker mem_track;

        {
            TestType ptr_orig = make_pointer_deleter_1<TestType>();
            {
                base_ptr<TestType> ptr(std::move(ptr_orig));
                CHECK(instances == 1);
                CHECK(instances_derived == 1);
                CHECK(ptr.get() != nullptr);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(instances_deleter == 2);
                    CHECK(ptr.get_deleter().state_ == test_deleter::state::special_init_1);
                }
            }

            CHECK(instances == 0);
            CHECK(instances_derived == 0);
            CHECK(ptr_orig.get() == nullptr);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
        }

        CHECK(instances == 0);
        CHECK(instances_derived == 0);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 0);
        }

        CHECK(mem_track.allocated() == 0u);
        CHECK(mem_track.double_delete() == 0u);
    }
};

TEMPLATE_LIST_TEST_CASE(
    "owner explicit conversion constructor", "[construction],[owner]", owner_types) {
    if constexpr (has_base<TestType>) {
        memory_tracker mem_track;

        {
            base_ptr<TestType> ptr_orig = make_pointer_deleter_1<TestType>();
            {
                TestType ptr(
                    std::move(ptr_orig), dynamic_cast<get_object<TestType>*>(ptr_orig.get()));
                CHECK(instances == 1);
                CHECK(instances_derived == 1);
                CHECK(ptr.get() != nullptr);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(instances_deleter == 2);
                    CHECK(ptr.get_deleter().state_ == test_deleter::state::special_init_1);
                }
            }

            CHECK(instances == 0);
            CHECK(instances_derived == 0);
            CHECK(ptr_orig.get() == nullptr);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
        }

        CHECK(instances == 0);
        CHECK(instances_derived == 0);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 0);
        }

        CHECK(mem_track.allocated() == 0u);
        CHECK(mem_track.double_delete() == 0u);
    }
};

TEMPLATE_LIST_TEST_CASE(
    "owner explicit conversion constructor with custom deleter",
    "[construction],[owner]",
    owner_types) {
    if constexpr (has_base<TestType> && has_stateful_deleter<TestType>) {
        memory_tracker mem_track;

        {
            base_ptr<TestType> ptr_orig = make_pointer_deleter_1<TestType>();
            {
                TestType ptr(
                    std::move(ptr_orig), dynamic_cast<get_object<TestType>*>(ptr_orig.get()),
                    make_deleter_instance_2<TestType>());
                CHECK(instances == 1);
                CHECK(instances_derived == 1);
                CHECK(ptr.get() != nullptr);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(instances_deleter == 2);
                    CHECK(ptr.get_deleter().state_ == test_deleter::state::special_init_2);
                }
            }

            CHECK(instances == 0);
            CHECK(instances_derived == 0);
            CHECK(ptr_orig.get() == nullptr);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
        }

        CHECK(instances == 0);
        CHECK(instances_derived == 0);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 0);
        }

        CHECK(mem_track.allocated() == 0u);
        CHECK(mem_track.double_delete() == 0u);
    }
};

TEMPLATE_LIST_TEST_CASE(
    "owner explicit conversion constructor with null", "[construction],[owner]", owner_types) {
    if constexpr (has_base<TestType>) {
        memory_tracker mem_track;

        {
            base_ptr<TestType> ptr_orig = make_pointer_deleter_1<TestType>();
            {
                TestType ptr(std::move(ptr_orig), static_cast<get_object<TestType>*>(nullptr));
                CHECK(instances == 0);
                CHECK(instances_derived == 0);
                CHECK(ptr.get() == nullptr);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(instances_deleter == 2);
                    CHECK(ptr.get_deleter().state_ == test_deleter::state::special_init_1);
                }
            }

            CHECK(instances == 0);
            CHECK(instances_derived == 0);
            CHECK(ptr_orig.get() == nullptr);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
        }

        CHECK(instances == 0);
        CHECK(instances_derived == 0);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 0);
        }

        CHECK(mem_track.allocated() == 0u);
        CHECK(mem_track.double_delete() == 0u);
    }
};

TEMPLATE_LIST_TEST_CASE(
    "owner explicit conversion constructor with custom deleter with null",
    "[construction],[owner]",
    owner_types) {
    if constexpr (has_base<TestType> && has_stateful_deleter<TestType>) {
        memory_tracker mem_track;

        {
            base_ptr<TestType> ptr_orig = make_pointer_deleter_1<TestType>();
            {
                TestType ptr(
                    std::move(ptr_orig), static_cast<get_object<TestType>*>(nullptr),
                    make_deleter_instance_2<TestType>());
                CHECK(instances == 0);
                CHECK(instances_derived == 0);
                CHECK(ptr.get() == nullptr);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(instances_deleter == 2);
                    CHECK(ptr.get_deleter().state_ == test_deleter::state::special_init_2);
                }
            }

            CHECK(instances == 0);
            CHECK(instances_derived == 0);
            CHECK(ptr_orig.get() == nullptr);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
        }

        CHECK(instances == 0);
        CHECK(instances_derived == 0);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 0);
        }

        CHECK(mem_track.allocated() == 0u);
        CHECK(mem_track.double_delete() == 0u);
    }
};
