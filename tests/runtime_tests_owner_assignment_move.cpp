#include "memory_tracker.hpp"
#include "testing.hpp"
#include "tests_common.hpp"

SUITE {
    TEMPLATE_LIST_TEST_CASE(
        "owner move assignment operator valid to empty", "[assignment][owner]", owner_types) {
        memory_tracker mem_track;

        {
            TestType ptr_orig = make_pointer_deleter_1<TestType>();
            {
                TestType ptr = make_empty_pointer_deleter_2<TestType>();
                ptr          = std::move(ptr_orig);

                CHECK(ptr.get() != nullptr);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(ptr.get_deleter().state_ == test_deleter::state::special_init_1);
                }
                CHECK_INSTANCES(1, 2);
            }

            CHECK_INSTANCES(0, 1);
        }

        CHECK_NO_LEAKS;
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE(
        "owner move assignment operator empty to valid", "[assignment][owner]", owner_types) {
        memory_tracker mem_track;

        {
            TestType ptr_orig = make_empty_pointer_deleter_1<TestType>();
            {
                TestType ptr = make_pointer_deleter_2<TestType>();
                ptr          = std::move(ptr_orig);

                CHECK(ptr.get() == nullptr);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(ptr.get_deleter().state_ == test_deleter::state::special_init_1);
                }
                CHECK_INSTANCES(0, 2);
            }

            CHECK_INSTANCES(0, 1);
        }

        CHECK_NO_LEAKS;
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE(
        "owner move assignment operator empty to empty", "[assignment][owner]", owner_types) {
        memory_tracker mem_track;

        {
            TestType ptr_orig = make_empty_pointer_deleter_1<TestType>();
            {
                TestType ptr = make_empty_pointer_deleter_2<TestType>();
                ptr          = std::move(ptr_orig);

                CHECK(ptr.get() == nullptr);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(ptr.get_deleter().state_ == test_deleter::state::special_init_1);
                }
                CHECK_INSTANCES(0, 2);
            }

            CHECK_INSTANCES(0, 1);
        }

        CHECK_NO_LEAKS;
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE(
        "owner move assignment operator valid to valid", "[assignment][owner]", owner_types) {
        memory_tracker mem_track;

        {
            TestType ptr_orig     = make_pointer_deleter_1<TestType>();
            auto*    raw_ptr_orig = ptr_orig.get();
            {
                TestType ptr = make_pointer_deleter_1<TestType>();
                ptr          = std::move(ptr_orig);

                CHECK(ptr.get() == raw_ptr_orig);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(ptr.get_deleter().state_ == test_deleter::state::special_init_1);
                }
                CHECK_INSTANCES(1, 2);
            }

            CHECK_INSTANCES(0, 1);
        }

        CHECK_NO_LEAKS;
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE(
        "owner move assignment converting operator valid to empty", "[assignment][owner]",
        owner_types) {
        memory_tracker mem_track;

        {
            TestType ptr_orig = make_pointer_deleter_1<TestType>();
            {
                base_ptr<TestType> ptr = make_empty_pointer_deleter_2<base_ptr<TestType>>();
                ptr                    = std::move(ptr_orig);

                CHECK(ptr.get() != nullptr);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(ptr.get_deleter().state_ == test_deleter::state::special_init_1);
                }
                CHECK_INSTANCES(1, 2);
            }

            CHECK_INSTANCES(0, 1);
        }

        CHECK_NO_LEAKS;
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE(
        "owner move assignment converting operator empty to valid", "[assignment][owner]",
        owner_types) {
        memory_tracker mem_track;

        {
            TestType ptr_orig = make_empty_pointer_deleter_1<TestType>();
            {
                base_ptr<TestType> ptr = make_pointer_deleter_2<base_ptr<TestType>>();
                ptr                    = std::move(ptr_orig);

                CHECK(ptr.get() == nullptr);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(ptr.get_deleter().state_ == test_deleter::state::special_init_1);
                }
                CHECK_INSTANCES(0, 2);
            }

            CHECK_INSTANCES(0, 1);
        }

        CHECK_NO_LEAKS;
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE(
        "owner move assignment converting operator empty to empty", "[assignment][owner]",
        owner_types) {
        memory_tracker mem_track;

        {
            TestType ptr_orig = make_empty_pointer_deleter_1<TestType>();
            {
                base_ptr<TestType> ptr = make_empty_pointer_deleter_2<base_ptr<TestType>>();
                ptr                    = std::move(ptr_orig);

                CHECK(ptr.get() == nullptr);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(ptr.get_deleter().state_ == test_deleter::state::special_init_1);
                }
                CHECK_INSTANCES(0, 2);
            }

            CHECK_INSTANCES(0, 1);
        }

        CHECK_NO_LEAKS;
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE(
        "owner move assignment converting operator valid to valid", "[assignment][owner]",
        owner_types) {
        memory_tracker mem_track;

        {
            TestType ptr_orig     = make_pointer_deleter_1<TestType>();
            auto*    raw_ptr_orig = ptr_orig.get();
            {
                base_ptr<TestType> ptr = make_pointer_deleter_1<base_ptr<TestType>>();
                ptr                    = std::move(ptr_orig);

                CHECK(ptr.get() == raw_ptr_orig);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(ptr.get_deleter().state_ == test_deleter::state::special_init_1);
                }
                CHECK_INSTANCES(1, 2);
            }

            CHECK_INSTANCES(0, 1);
        }

        CHECK_NO_LEAKS;
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE(
        "owner move assignment operator self to self valid", "[assignment][owner]", owner_types) {
        memory_tracker mem_track;

        {
            TestType ptr = make_pointer_deleter_1<TestType>();
            ptr          = std::move(ptr);

            CHECK(ptr.get() == nullptr);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(ptr.get_deleter().state_ == test_deleter::state::empty);
            }
            CHECK_INSTANCES(0, 1);
        }

        CHECK_NO_LEAKS;
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE(
        "owner move assignment operator self to self empty", "[assignment][owner]", owner_types) {
        memory_tracker mem_track;

        {
            TestType ptr = make_empty_pointer_deleter_1<TestType>();
            ptr          = std::move(ptr);

            CHECK(ptr.get() == nullptr);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(ptr.get_deleter().state_ == test_deleter::state::empty);
            }
            CHECK_INSTANCES(0, 1);
        }

        CHECK_NO_LEAKS;
    }
    | owner_types{};
};
