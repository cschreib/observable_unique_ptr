#include "memory_tracker.hpp"
#include "testing.hpp"
#include "tests_common.hpp"

// For std::bad_cast
#include <typeinfo>

SUITE {
    TEMPLATE_LIST_TEST_CASE("owner static_cast move from valid", "[cast][owner]", owner_types) {
        memory_tracker mem_track;

        {
            auto run_test = [&]<typename cast_type, typename expected_return_type>() {
                TestType              ptr1    = make_pointer_deleter_1<TestType>();
                get_object<TestType>* raw_ptr = ptr1.get();
                auto                  ptr2 = oup::static_pointer_cast<cast_type>(std::move(ptr1));

                using return_type = std::remove_cv_t<decltype(ptr2)>;

                CHECK(ptr1.get() == nullptr);
                CHECK(ptr2.get() == raw_ptr);
                CHECK(snatch::type_name<return_type> == snatch::type_name<expected_return_type>);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(ptr2.get_deleter().state_ == test_deleter::state::special_init_1);
                }
                CHECK_INSTANCES(1, 2);
            };

            run_test.template operator()<get_object<TestType>, TestType>();
            run_test.template operator()<const get_object<TestType>, const_ptr<TestType>>();
            if constexpr (has_base<TestType>) {
                run_test.template operator()<get_base_object<TestType>, base_ptr<TestType>>();
            }
        }

        CHECK_NO_LEAKS;
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE("owner static_cast move from empty", "[cast][owner]", owner_types) {
        memory_tracker mem_track;

        {
            auto run_test = [&]<typename cast_type, typename expected_return_type>() {
                TestType ptr1 = make_empty_pointer_deleter_1<TestType>();
                auto     ptr2 = oup::static_pointer_cast<cast_type>(std::move(ptr1));

                using return_type = std::remove_cv_t<decltype(ptr2)>;

                CHECK(ptr1.get() == nullptr);
                CHECK(ptr2.get() == nullptr);
                CHECK(snatch::type_name<return_type> == snatch::type_name<expected_return_type>);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(ptr2.get_deleter().state_ == test_deleter::state::special_init_1);
                }
                CHECK_INSTANCES(0, 2);
            };

            run_test.template operator()<get_object<TestType>, TestType>();
            run_test.template operator()<const get_object<TestType>, const_ptr<TestType>>();
            if constexpr (has_base<TestType>) {
                run_test.template operator()<get_base_object<TestType>, base_ptr<TestType>>();
            }
        }

        CHECK_NO_LEAKS;
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE("owner const_cast move from valid", "[cast][owner]", owner_types) {
        memory_tracker mem_track;

        {
            auto run_test = [&]<typename cast_type, typename expected_return_type>() {
                TestType              ptr1    = make_pointer_deleter_1<TestType>();
                get_object<TestType>* raw_ptr = ptr1.get();
                auto                  ptr2    = oup::const_pointer_cast<cast_type>(std::move(ptr1));

                using return_type = std::remove_cv_t<decltype(ptr2)>;

                CHECK(ptr1.get() == nullptr);
                CHECK(ptr2.get() == raw_ptr);
                CHECK(snatch::type_name<return_type> == snatch::type_name<expected_return_type>);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(ptr2.get_deleter().state_ == test_deleter::state::special_init_1);
                }
                CHECK_INSTANCES(1, 2);
            };

            run_test.template operator()<const get_object<TestType>, const_ptr<TestType>>();
            run_test.template
            operator()<std::remove_cv_t<get_object<TestType>>, mutable_ptr<TestType>>();
        }

        CHECK_NO_LEAKS;
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE("owner const_cast move from empty", "[cast][owner]", owner_types) {
        memory_tracker mem_track;

        {
            auto run_test = [&]<typename cast_type, typename expected_return_type>() {
                TestType ptr1 = make_empty_pointer_deleter_1<TestType>();
                auto     ptr2 = oup::const_pointer_cast<cast_type>(std::move(ptr1));

                using return_type = std::remove_cv_t<decltype(ptr2)>;

                CHECK(ptr1.get() == nullptr);
                CHECK(ptr2.get() == nullptr);
                CHECK(snatch::type_name<return_type> == snatch::type_name<expected_return_type>);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(ptr2.get_deleter().state_ == test_deleter::state::special_init_1);
                }
                CHECK_INSTANCES(0, 2);
            };

            run_test.template operator()<const get_object<TestType>, const_ptr<TestType>>();
            run_test.template
            operator()<std::remove_cv_t<get_object<TestType>>, mutable_ptr<TestType>>();
        }

        CHECK_NO_LEAKS;
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE("owner dynamic_cast move from valid", "[cast][owner]", owner_types) {
        memory_tracker mem_track;

        {
            auto run_test =
                [&]<typename start_type, typename cast_type, typename expected_return_type>() {
                    TestType              ptr0    = make_pointer_deleter_1<TestType>();
                    get_object<TestType>* raw_ptr = ptr0.get();
                    start_type            ptr1    = std::move(ptr0);
                    auto ptr2 = oup::dynamic_pointer_cast<cast_type>(std::move(ptr1));

                    using return_type = std::remove_cv_t<decltype(ptr2)>;

                    CHECK(ptr1.get() == nullptr);
                    CHECK(ptr2.get() == raw_ptr);
                    CHECK(
                        snatch::type_name<return_type> == snatch::type_name<expected_return_type>);
                    if constexpr (has_stateful_deleter<TestType>) {
                        CHECK(ptr2.get_deleter().state_ == test_deleter::state::special_init_1);
                    }
                    CHECK_INSTANCES(1, 3);
                };

            run_test.template operator()<TestType, get_object<TestType>, TestType>();
            run_test
                .template operator()<TestType, const get_object<TestType>, const_ptr<TestType>>();
            if constexpr (has_base<TestType>) {
                run_test
                    .template operator()<TestType, get_base_object<TestType>, base_ptr<TestType>>();
                run_test.template operator()<base_ptr<TestType>, get_object<TestType>, TestType>();
            }
        }

        CHECK_NO_LEAKS;
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE("owner dynamic_cast move from empty", "[cast][owner]", owner_types) {
        memory_tracker mem_track;

        {
            auto run_test =
                [&]<typename start_type, typename cast_type, typename expected_return_type>() {
                    TestType   ptr0 = make_empty_pointer_deleter_1<TestType>();
                    start_type ptr1 = std::move(ptr0);
                    auto       ptr2 = oup::dynamic_pointer_cast<cast_type>(std::move(ptr1));

                    using return_type = std::remove_cv_t<decltype(ptr2)>;

                    CHECK(ptr1.get() == nullptr);
                    CHECK(ptr2.get() == nullptr);
                    CHECK(
                        snatch::type_name<return_type> == snatch::type_name<expected_return_type>);
                    if constexpr (has_stateful_deleter<TestType>) {
                        CHECK(ptr2.get_deleter().state_ == test_deleter::state::special_init_1);
                    }
                    CHECK_INSTANCES(0, 3);
                };

            run_test.template operator()<TestType, get_object<TestType>, TestType>();
            run_test
                .template operator()<TestType, const get_object<TestType>, const_ptr<TestType>>();
            if constexpr (has_base<TestType>) {
                run_test
                    .template operator()<TestType, get_base_object<TestType>, base_ptr<TestType>>();
                run_test.template operator()<base_ptr<TestType>, get_object<TestType>, TestType>();
            }
        }

        CHECK_NO_LEAKS;
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE("owner dynamic_cast move from invalid", "[cast][owner]", owner_types) {
        if constexpr (has_base<TestType>) {
            memory_tracker mem_track;

            {
                TestType              ptr0    = make_pointer_deleter_1<TestType>();
                get_object<TestType>* raw_ptr = ptr0.get();
                base_ptr<TestType>    ptr1    = std::move(ptr0);

                CHECK_THROWS_AS(
                    (oup::dynamic_pointer_cast<test_object_dead_end>(std::move(ptr1))),
                    std::bad_cast);

                CHECK(ptr1.get() == raw_ptr);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(ptr1.get_deleter().state_ == test_deleter::state::special_init_1);
                }
                CHECK_INSTANCES(1, 2);
            }

            CHECK_NO_LEAKS;
        }
    }
    | owner_types{};
};
