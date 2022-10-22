#include "memory_tracker.hpp"
#include "testing.hpp"
#include "tests_common.hpp"

SUITE {
    TEMPLATE_LIST_TEST_CASE(
        "observer static_cast copy from valid", "[cast][observer]", owner_types) {
        memory_tracker mem_track;

        {
            auto run_test = [&]<typename cast_type, typename expected_return_type>() {
                TestType               ptr1    = make_pointer_deleter_1<TestType>();
                observer_ptr<TestType> optr1   = ptr1;
                get_object<TestType>*  raw_ptr = ptr1.get();
                auto                   optr2   = oup::static_pointer_cast<cast_type>(optr1);

                using return_type = std::remove_cv_t<decltype(optr2)>;

                CHECK(optr1.get() == raw_ptr);
                CHECK(optr2.get() == raw_ptr);
                CHECK(snatch::type_name<return_type> == snatch::type_name<expected_return_type>);
                CHECK_INSTANCES(1, 1);
            };

            run_test.template operator()<get_object<TestType>, observer_ptr<TestType>>();
            run_test
                .template operator()<const get_object<TestType>, const_observer_ptr<TestType>>();
            if constexpr (has_base<TestType>) {
                run_test
                    .template operator()<get_base_object<TestType>, base_observer_ptr<TestType>>();
            }
        }

        CHECK_NO_LEAKS;
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE(
        "observer static_cast copy from empty", "[cast][observer]", owner_types) {
        memory_tracker mem_track;

        {
            auto run_test = [&]<typename cast_type, typename expected_return_type>() {
                TestType               ptr1  = make_empty_pointer_deleter_1<TestType>();
                observer_ptr<TestType> optr1 = ptr1;
                auto                   optr2 = oup::static_pointer_cast<cast_type>(optr1);

                using return_type = std::remove_cv_t<decltype(optr2)>;

                CHECK(optr1.get() == nullptr);
                CHECK(optr2.get() == nullptr);
                CHECK(snatch::type_name<return_type> == snatch::type_name<expected_return_type>);
                CHECK_INSTANCES(0, 1);
            };

            run_test.template operator()<get_object<TestType>, observer_ptr<TestType>>();
            run_test
                .template operator()<const get_object<TestType>, const_observer_ptr<TestType>>();
            if constexpr (has_base<TestType>) {
                run_test
                    .template operator()<get_base_object<TestType>, base_observer_ptr<TestType>>();
            }
        }

        CHECK_NO_LEAKS;
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE(
        "observer const_cast copy from valid", "[cast][observer]", owner_types) {
        memory_tracker mem_track;

        {
            auto run_test = [&]<typename cast_type, typename expected_return_type>() {
                TestType               ptr1    = make_pointer_deleter_1<TestType>();
                observer_ptr<TestType> optr1   = ptr1;
                get_object<TestType>*  raw_ptr = ptr1.get();
                auto                   optr2   = oup::const_pointer_cast<cast_type>(optr1);

                using return_type = std::remove_cv_t<decltype(optr2)>;

                CHECK(optr1.get() == raw_ptr);
                CHECK(optr2.get() == raw_ptr);
                CHECK(snatch::type_name<return_type> == snatch::type_name<expected_return_type>);
                CHECK_INSTANCES(1, 1);
            };

            run_test
                .template operator()<const get_object<TestType>, const_observer_ptr<TestType>>();
            run_test.template
            operator()<std::remove_cv_t<get_object<TestType>>, mutable_observer_ptr<TestType>>();
        }

        CHECK_NO_LEAKS;
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE(
        "observer const_cast copy from empty", "[cast][observer]", owner_types) {
        memory_tracker mem_track;

        {
            auto run_test = [&]<typename cast_type, typename expected_return_type>() {
                TestType               ptr1  = make_empty_pointer_deleter_1<TestType>();
                observer_ptr<TestType> optr1 = ptr1;
                auto                   optr2 = oup::const_pointer_cast<cast_type>(optr1);

                using return_type = std::remove_cv_t<decltype(optr2)>;

                CHECK(optr1.get() == nullptr);
                CHECK(optr2.get() == nullptr);
                CHECK(snatch::type_name<return_type> == snatch::type_name<expected_return_type>);
                CHECK_INSTANCES(0, 1);
            };

            run_test
                .template operator()<const get_object<TestType>, const_observer_ptr<TestType>>();
            run_test.template
            operator()<std::remove_cv_t<get_object<TestType>>, mutable_observer_ptr<TestType>>();
        }

        CHECK_NO_LEAKS;
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE(
        "observer dynamic_cast copy from valid", "[cast][observer]", owner_types) {
        memory_tracker mem_track;

        {
            auto run_test =
                [&]<typename start_type, typename cast_type, typename expected_return_type>() {
                    TestType                 ptr0    = make_pointer_deleter_1<TestType>();
                    get_object<TestType>*    raw_ptr = ptr0.get();
                    observer_ptr<start_type> optr1   = ptr0;
                    auto                     optr2   = oup::dynamic_pointer_cast<cast_type>(optr1);

                    using return_type = std::remove_cv_t<decltype(optr2)>;

                    CHECK(optr1.get() == raw_ptr);
                    CHECK(optr2.get() == raw_ptr);
                    CHECK(
                        snatch::type_name<return_type> == snatch::type_name<expected_return_type>);
                    CHECK_INSTANCES(1, 1);
                };

            run_test.template operator()<TestType, get_object<TestType>, observer_ptr<TestType>>();
            run_test.template
            operator()<TestType, const get_object<TestType>, const_observer_ptr<TestType>>();
            if constexpr (has_base<TestType>) {
                run_test.template
                operator()<TestType, get_base_object<TestType>, base_observer_ptr<TestType>>();
                run_test.template
                operator()<base_ptr<TestType>, get_object<TestType>, observer_ptr<TestType>>();
            }
        }

        CHECK_NO_LEAKS;
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE(
        "observer dynamic_cast copy from empty", "[cast][observer]", owner_types) {
        memory_tracker mem_track;

        {
            auto run_test =
                [&]<typename start_type, typename cast_type, typename expected_return_type>() {
                    TestType                 ptr0  = make_empty_pointer_deleter_1<TestType>();
                    observer_ptr<start_type> optr1 = ptr0;
                    auto                     optr2 = oup::dynamic_pointer_cast<cast_type>(optr1);

                    using return_type = std::remove_cv_t<decltype(optr2)>;

                    CHECK(optr1.get() == nullptr);
                    CHECK(optr2.get() == nullptr);
                    CHECK(
                        snatch::type_name<return_type> == snatch::type_name<expected_return_type>);
                    CHECK_INSTANCES(0, 1);
                };

            run_test.template operator()<TestType, get_object<TestType>, observer_ptr<TestType>>();
            run_test.template
            operator()<TestType, const get_object<TestType>, const_observer_ptr<TestType>>();
            if constexpr (has_base<TestType>) {
                run_test.template
                operator()<TestType, get_base_object<TestType>, base_observer_ptr<TestType>>();
                run_test.template
                operator()<base_ptr<TestType>, get_object<TestType>, observer_ptr<TestType>>();
            }
        }

        CHECK_NO_LEAKS;
    }
    | owner_types{};

    TEMPLATE_LIST_TEST_CASE(
        "observer dynamic_cast copy from invalid", "[cast][observer]", owner_types) {
        if constexpr (has_base<TestType>) {
            memory_tracker mem_track;

            {
                TestType                    ptr0    = make_pointer_deleter_1<TestType>();
                get_object<TestType>*       raw_ptr = ptr0.get();
                base_observer_ptr<TestType> optr1   = ptr0;
                auto optr2 = oup::dynamic_pointer_cast<test_object_dead_end>(optr1);

                using return_type = std::remove_cv_t<decltype(optr2)>;
                using expected_return_type =
                    oup::basic_observer_ptr<test_object_dead_end, get_observer_policy<TestType>>;

                CHECK(optr1.get() == raw_ptr);
                CHECK(optr2.get() == nullptr);
                CHECK(snatch::type_name<return_type> == snatch::type_name<expected_return_type>);
                CHECK_INSTANCES(1, 1);
            }

            CHECK_NO_LEAKS;
        }
    }
    | owner_types{};
};
