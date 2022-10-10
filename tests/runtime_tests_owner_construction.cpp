#include "memory_tracker.hpp"
#include "testing.hpp"
#include "tests_common.hpp"

TEMPLATE_LIST_TEST_CASE("owner default constructor", "[construction],[owner]", owner_types) {
    memory_tracker mem_track;

    {
        TestType ptr;

        CHECK(ptr.get() == nullptr);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(ptr.get_deleter().state_ == test_deleter::state::default_init);
        }
        CHECK_INSTANCES(0, 1);
    }

    CHECK_NO_LEAKS;
};

TEMPLATE_LIST_TEST_CASE("owner nullptr constructor", "[construction],[owner]", owner_types) {
    memory_tracker mem_track;

    {
        TestType ptr(nullptr);

        CHECK(ptr.get() == nullptr);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(ptr.get_deleter().state_ == test_deleter::state::default_init);
        }
        CHECK_INSTANCES(0, 1);
    }

    CHECK_NO_LEAKS;
};

TEMPLATE_LIST_TEST_CASE("owner move constructor", "[construction],[owner]", owner_types) {
    memory_tracker mem_track;

    {
        TestType ptr_orig = make_pointer_deleter_1<TestType>();
        {
            TestType ptr(std::move(ptr_orig));

            CHECK(ptr.get() != nullptr);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(ptr.get_deleter().state_ == test_deleter::state::special_init_1);
            }
            CHECK_INSTANCES(1, 2);
        }

        CHECK_INSTANCES(0, 1);
    }

    CHECK_NO_LEAKS;
};

TEMPLATE_LIST_TEST_CASE("owner acquiring constructor", "[construction],[owner]", owner_types) {
    if constexpr (!must_use_make_observable<TestType>) {
        memory_tracker mem_track;

        {
            TestType ptr(make_instance<TestType>());

            CHECK(ptr.get() != nullptr);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(ptr.get_deleter().state_ == test_deleter::state::default_init);
            }
            CHECK_INSTANCES(1, 1);
        }

        CHECK_NO_LEAKS;
    }
};

TEMPLATE_LIST_TEST_CASE(
    "owner acquiring constructor with deleter", "[construction],[owner]", owner_types) {
    if constexpr (!must_use_make_observable<TestType> && has_stateful_deleter<TestType>) {
        memory_tracker mem_track;

        {
            TestType ptr(make_instance<TestType>(), make_deleter_instance_1<TestType>());

            CHECK(ptr.get() != nullptr);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(ptr.get_deleter().state_ == test_deleter::state::special_init_1);
            }
            CHECK_INSTANCES(1, 1);
        }

        CHECK_NO_LEAKS;
    }
};

TEMPLATE_LIST_TEST_CASE(
    "owner acquiring constructor bad alloc", "[construction],[owner]", owner_types) {
    if constexpr (!must_use_make_observable<TestType>) {
        memory_tracker mem_track;

        {
            auto* raw_ptr = make_instance<TestType>();
            if constexpr (eoft_allocates<TestType>) {
                fail_next_allocation{}, TestType{raw_ptr};
            } else {
                REQUIRE_THROWS_AS((fail_next_allocation{}, TestType{raw_ptr}), std::bad_alloc);
            }
        }

        CHECK_NO_LEAKS;
    }
};

TEMPLATE_LIST_TEST_CASE(
    "owner acquiring constructor bad alloc with deleter", "[construction],[owner]", owner_types) {
    if constexpr (!must_use_make_observable<TestType> && has_stateful_deleter<TestType>) {
        memory_tracker mem_track;

        {
            auto* raw_ptr = make_instance<TestType>();
            auto  deleter = make_deleter_instance_1<TestType>();
            if constexpr (eoft_allocates<TestType>) {
                fail_next_allocation{}, TestType{raw_ptr, deleter};
            } else {
                REQUIRE_THROWS_AS(
                    (fail_next_allocation{}, TestType{raw_ptr, deleter}), std::bad_alloc);
            }
        }

        CHECK_NO_LEAKS;
    }
};

TEMPLATE_LIST_TEST_CASE("owner acquiring constructor null", "[construction],[owner]", owner_types) {
    if constexpr (!must_use_make_observable<TestType>) {
        memory_tracker mem_track;

        {
            TestType ptr(static_cast<get_object<TestType>*>(nullptr));

            CHECK(ptr.get() == nullptr);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(ptr.get_deleter().state_ == test_deleter::state::default_init);
            }
            CHECK_INSTANCES(0, 1);
        }

        CHECK_NO_LEAKS;
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

                CHECK(ptr.get() != nullptr);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(ptr.get_deleter().state_ == test_deleter::state::special_init_1);
                }
                CHECK_INSTANCES_DERIVED(1, 1, 2);
            }

            CHECK(ptr_orig.get() == nullptr);
            CHECK_INSTANCES_DERIVED(0, 0, 1);
        }

        CHECK_NO_LEAKS;
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

                CHECK(ptr.get() != nullptr);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(ptr.get_deleter().state_ == test_deleter::state::special_init_1);
                }
                CHECK_INSTANCES_DERIVED(1, 1, 2);
            }

            CHECK(ptr_orig.get() == nullptr);
            CHECK_INSTANCES_DERIVED(0, 0, 1);
        }

        CHECK_NO_LEAKS;
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

                CHECK(ptr.get() != nullptr);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(ptr.get_deleter().state_ == test_deleter::state::special_init_2);
                }
                CHECK_INSTANCES_DERIVED(1, 1, 2);
            }

            CHECK(ptr_orig.get() == nullptr);
            CHECK_INSTANCES_DERIVED(0, 0, 1);
        }

        CHECK_NO_LEAKS;
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

                CHECK(ptr.get() == nullptr);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(ptr.get_deleter().state_ == test_deleter::state::special_init_1);
                }
                CHECK_INSTANCES_DERIVED(0, 0, 2);
            }

            CHECK(ptr_orig.get() == nullptr);
            CHECK_INSTANCES_DERIVED(0, 0, 1);
        }

        CHECK_NO_LEAKS;
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

                CHECK(ptr.get() == nullptr);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(ptr.get_deleter().state_ == test_deleter::state::special_init_2);
                }
                CHECK_INSTANCES_DERIVED(0, 0, 2);
            }

            CHECK(ptr_orig.get() == nullptr);
            CHECK_INSTANCES_DERIVED(0, 0, 1);
        }

        CHECK_NO_LEAKS;
    }
};
