#include "memory_tracker.hpp"
#include "testing.hpp"
#include "tests_common.hpp"

TEMPLATE_LIST_TEST_CASE("observer from this", "[observer_from_this]", owner_types) {
    if constexpr (has_eoft<TestType>) {
        memory_tracker mem_track;

        {
            TestType                    ptr      = make_pointer_deleter_1<TestType>();
            get_object<TestType>*       raw_ptr  = ptr.get();
            const get_object<TestType>* craw_ptr = ptr.get();

            auto run_checks = [&](auto&& optr, auto&& optr_const) {
                using obs_type       = std::remove_reference_t<decltype(*optr.get())>;
                using const_obs_type = std::remove_reference_t<decltype(*optr_const.get())>;
                CHECK(std::is_const_v<obs_type> == std::is_const_v<get_object<TestType>>);
                CHECK(std::is_const_v<const_obs_type> == true);

                if constexpr (has_eoft_direct_base<TestType>) {
                    // For types that inherit directly from eoft<this>.
                    CHECK((std::is_same_v<obs_type, get_object<TestType>>) == true);
                    CHECK((std::is_same_v<const_obs_type, const get_object<TestType>>) == true);
                } else {
                    // For types that inherit from a base class that inherits from eoft<base>.
                    CHECK((std::is_base_of_v<obs_type, get_object<TestType>>) == true);
                    CHECK(
                        (std::is_base_of_v<
                            std::remove_cv_t<const_obs_type>, get_object<TestType>>) == true);
                }

                CHECK(optr.expired() == false);
                CHECK(optr_const.expired() == false);
                CHECK(optr.get() == raw_ptr);
                CHECK(optr_const.get() == craw_ptr);
                CHECK_INSTANCES(1, 1);
            };

            if constexpr (has_eoft_multi_base<TestType>) {
                // Need an explicit choice of which base to call.
                auto optr_from_this       = raw_ptr->get_eoft<TestType>::observer_from_this();
                auto optr_from_this_const = craw_ptr->get_eoft<TestType>::observer_from_this();

                run_checks(optr_from_this, optr_from_this_const);
            } else {
                // No ambiguity, just call normally.
                auto optr_from_this       = raw_ptr->observer_from_this();
                auto optr_from_this_const = craw_ptr->observer_from_this();

                run_checks(optr_from_this, optr_from_this_const);
            }
        }

        CHECK_NO_LEAKS;
    }
};

TEMPLATE_LIST_TEST_CASE(
    "observer from this with no owner heap", "[observer_from_this]", owner_types) {
    if constexpr (has_eoft<TestType> && !must_use_make_observable<TestType>) {
        memory_tracker mem_track;

        {
            get_object<TestType>* orig_ptr = make_instance<TestType>();

            if constexpr (eoft_always_has_block<TestType>) {
                auto optr  = make_observer_from_this<TestType>(orig_ptr);
                auto coptr = make_const_observer_from_this<TestType>(orig_ptr);

                CHECK(optr.expired() == false);
                CHECK(optr.get() == orig_ptr);
                CHECK(coptr.expired() == false);
                CHECK(coptr.get() == orig_ptr);
            } else {
                REQUIRE_THROWS_MATCHES(
                    (make_observer_from_this<TestType>(orig_ptr)), oup::bad_observer_from_this,
                    snatch::matchers::with_what_contains{
                        "observer_from_this() called with uninitialized control block"});
                REQUIRE_THROWS_MATCHES(
                    (make_const_observer_from_this<TestType>(orig_ptr)),
                    oup::bad_observer_from_this,
                    snatch::matchers::with_what_contains{
                        "observer_from_this() called with uninitialized control block"});
            }

            CHECK_INSTANCES(1, 0);

            delete orig_ptr;
        }

        CHECK_NO_LEAKS;
    }
};

TEMPLATE_LIST_TEST_CASE("observer from this no owner stack", "[observer_from_this]", owner_types) {
    if constexpr (has_eoft<TestType> && !eoft_constructor_takes_control_block<TestType>) {
        memory_tracker mem_track;

        {
            get_object<TestType> obj;

            if constexpr (eoft_always_has_block<TestType>) {
                auto optr  = make_observer_from_this<TestType>(&obj);
                auto coptr = make_const_observer_from_this<TestType>(&obj);

                CHECK(optr.expired() == false);
                CHECK(optr.get() == &obj);
                CHECK(coptr.expired() == false);
                CHECK(coptr.get() == &obj);
            } else {
                REQUIRE_THROWS_MATCHES(
                    (make_observer_from_this<TestType>(&obj)), oup::bad_observer_from_this,
                    snatch::matchers::with_what_contains{
                        "observer_from_this() called with uninitialized control block"});
                REQUIRE_THROWS_MATCHES(
                    (make_const_observer_from_this<TestType>(&obj)), oup::bad_observer_from_this,
                    snatch::matchers::with_what_contains{
                        "observer_from_this() called with uninitialized control block"});
            }

            CHECK_INSTANCES(1, 0);
        }

        CHECK_NO_LEAKS;
    }
};

TEMPLATE_LIST_TEST_CASE(
    "observer from this acquired into base owner as base", "[observer_from_this]", owner_types) {
    if constexpr (has_eoft<TestType> && !must_use_make_observable<TestType>) {
        memory_tracker mem_track;

        {
            get_object<TestType>*      orig_ptr      = make_instance<TestType>();
            get_base_object<TestType>* orig_base_ptr = orig_ptr;
            base_ptr<TestType>         ptr{orig_base_ptr};

            if constexpr (eoft_always_has_block<TestType>) {
                auto optr  = make_observer_from_this<TestType>(orig_ptr);
                auto coptr = make_const_observer_from_this<TestType>(orig_ptr);

                CHECK(optr.expired() == false);
                CHECK(optr.get() == ptr.get());
                CHECK(coptr.expired() == false);
                CHECK(coptr.get() == orig_ptr);
                CHECK_INSTANCES(1, 1);
            } else {
                REQUIRE_THROWS_MATCHES(
                    (make_observer_from_this<TestType>(orig_ptr)), oup::bad_observer_from_this,
                    snatch::matchers::with_what_contains{
                        "observer_from_this() called with uninitialized control block"});
                REQUIRE_THROWS_MATCHES(
                    (make_const_observer_from_this<TestType>(orig_ptr)),
                    oup::bad_observer_from_this,
                    snatch::matchers::with_what_contains{
                        "observer_from_this() called with uninitialized control block"});
            }
        }

        CHECK_NO_LEAKS;
    }
};

TEMPLATE_LIST_TEST_CASE(
    "observer from this acquired into base owner as derived", "[observer_from_this]", owner_types) {
    if constexpr (has_eoft<TestType> && has_base<TestType> && !must_use_make_observable<TestType>) {
        memory_tracker mem_track;

        {
            get_object<TestType>* orig_ptr = make_instance<TestType>();
            base_ptr<TestType>    ptr{orig_ptr};

            base_observer_ptr<TestType> optr = make_observer_from_this<TestType>(orig_ptr);

            CHECK(optr.expired() == false);
            CHECK(optr.get() == ptr.get());
            CHECK_INSTANCES(1, 1);
        }

        CHECK_NO_LEAKS;
    }
};

TEMPLATE_LIST_TEST_CASE(
    "observer from this after owner reset to empty", "[observer_from_this]", owner_types) {
    if constexpr (has_eoft<TestType>) {
        memory_tracker mem_track;

        {
            TestType ptr  = make_pointer_deleter_1<TestType>();
            auto     optr = make_observer_from_this<TestType>(ptr.get());

            CHECK(optr.expired() == false);
            CHECK(optr.get() == ptr.get());
            CHECK_INSTANCES(1, 1);

            ptr.reset();

            CHECK(optr.expired() == true);
            CHECK(optr.get() == nullptr);
            CHECK_INSTANCES(0, 1);
        }

        CHECK_NO_LEAKS;
    }
};

TEMPLATE_LIST_TEST_CASE(
    "observer from this after owner reset to valid", "[observer_from_this]", owner_types) {
    if constexpr (has_eoft<TestType> && can_reset_to_new<TestType>) {
        memory_tracker mem_track;

        {
            TestType ptr  = make_pointer_deleter_1<TestType>();
            auto     optr = make_observer_from_this<TestType>(ptr.get());

            CHECK(optr.expired() == false);
            CHECK(optr.get() == ptr.get());
            CHECK_INSTANCES(1, 1);

            ptr.reset(make_instance<TestType>());

            CHECK(optr.expired() == true);
            CHECK(optr.get() == nullptr);
            CHECK_INSTANCES(1, 1);
        }

        CHECK_NO_LEAKS;
    }
};

TEMPLATE_LIST_TEST_CASE(
    "observer from this after owner release", "[observer_from_this]", owner_types) {
    if constexpr (has_eoft<TestType> && can_release<TestType>) {
        memory_tracker mem_track;

        {
            TestType ptr  = make_pointer_deleter_1<TestType>();
            auto     optr = make_observer_from_this<TestType>(ptr.get());

            CHECK(optr.expired() == false);
            CHECK(optr.get() == ptr.get());
            CHECK_INSTANCES(1, 1);

            auto* raw_ptr = ptr.release();

            CHECK(optr.expired() == false);
            CHECK(optr.get() == raw_ptr);
            CHECK_INSTANCES(1, 1);

            delete raw_ptr;

            CHECK(optr.expired() == true);
            CHECK(optr.get() == nullptr);
            CHECK_INSTANCES(0, 1);
        }

        CHECK_NO_LEAKS;
    }
};

TEMPLATE_LIST_TEST_CASE(
    "observer from this after owner release then reset to same",
    "[observer_from_this]",
    owner_types) {
    if constexpr (has_eoft<TestType> && can_release<TestType> && can_reset_to_new<TestType>) {
        memory_tracker mem_track;

        {
            TestType ptr  = make_pointer_deleter_1<TestType>();
            auto     optr = make_observer_from_this<TestType>(ptr.get());

            CHECK(optr.expired() == false);
            CHECK(optr.get() == ptr.get());
            CHECK_INSTANCES(1, 1);

            auto* raw_ptr = ptr.release();

            CHECK(optr.expired() == false);
            CHECK(optr.get() == raw_ptr);
            CHECK_INSTANCES(1, 1);

            ptr.reset(raw_ptr);

            CHECK(optr.expired() == false);
            CHECK(optr.get() == raw_ptr);
            CHECK_INSTANCES(1, 1);

            ptr.reset();

            CHECK(optr.expired() == true);
            CHECK(optr.get() == nullptr);
            CHECK_INSTANCES(0, 1);
        }

        CHECK_NO_LEAKS;
    }
};

TEMPLATE_LIST_TEST_CASE(
    "observer from this after owner move", "[observer_from_this]", owner_types) {
    if constexpr (has_eoft<TestType>) {
        memory_tracker mem_track;

        {
            TestType ptr1 = make_pointer_deleter_1<TestType>();
            TestType ptr2 = std::move(ptr1);

            auto optr  = make_observer_from_this<TestType>(ptr2.get());
            auto coptr = make_const_observer_from_this<TestType>(ptr2.get());

            CHECK(optr.expired() == false);
            CHECK(optr.get() == ptr2.get());
            CHECK(coptr.expired() == false);
            CHECK(coptr.get() == ptr2.get());
            CHECK_INSTANCES(1, 1);
        }

        CHECK_NO_LEAKS;
    }
};

TEMPLATE_LIST_TEST_CASE(
    "observer from this after owner move assignment", "[observer_from_this]", owner_types) {
    if constexpr (has_eoft<TestType>) {
        memory_tracker mem_track;

        {
            TestType ptr1 = make_pointer_deleter_1<TestType>();
            TestType ptr2;
            ptr2 = std::move(ptr1);

            auto optr  = make_observer_from_this<TestType>(ptr2.get());
            auto coptr = make_const_observer_from_this<TestType>(ptr2.get());

            CHECK(optr.expired() == false);
            CHECK(optr.get() == ptr2.get());
            CHECK(coptr.expired() == false);
            CHECK(coptr.get() == ptr2.get());
            CHECK_INSTANCES(1, 1);
        }

        CHECK_NO_LEAKS;
    }
};

TEST_CASE("observer from this multiple inheritance", "[observer_from_this]") {
    using base       = test_object_observer_from_this_unique;
    using deriv      = test_object_observer_from_this_multi_unique;
    using ptr_base   = oup::observable_unique_ptr<base>;
    using ptr_deriv  = oup::observable_unique_ptr<deriv>;
    using eoft_base  = oup::enable_observer_from_this_unique<base>;
    using eoft_deriv = oup::enable_observer_from_this_unique<deriv>;
    using TestType   = ptr_deriv;

    memory_tracker mem_track;

    {
        deriv*    raw_ptr_deriv = new deriv;
        base*     raw_ptr_base  = raw_ptr_deriv;
        ptr_deriv ptr(raw_ptr_deriv);

        observer_ptr<ptr_base>  optr_base  = ptr->eoft_base::observer_from_this();
        observer_ptr<ptr_deriv> optr_deriv = ptr->eoft_deriv::observer_from_this();

        CHECK(optr_base.expired() == false);
        CHECK(optr_deriv.expired() == false);
        CHECK(optr_base.get() == raw_ptr_base);
        CHECK(optr_deriv.get() == raw_ptr_deriv);
        CHECK_INSTANCES(1, 1);
    }

    CHECK_NO_LEAKS;
};

TEMPLATE_LIST_TEST_CASE("observer from this in constructor", "[observer_from_this]", owner_types) {
    if constexpr (has_eoft<TestType> && has_eoft_self_member<TestType>) {
        memory_tracker mem_track;

        if constexpr (eoft_always_has_block<TestType>) {
            next_test_object_constructor_calls_observer_from_this = true;
            TestType ptr = make_pointer_deleter_1<TestType>();
            next_test_object_constructor_calls_observer_from_this = false;
            CHECK(ptr->self == ptr.get());

            CHECK_INSTANCES(1, 1);
        } else {
            next_test_object_constructor_calls_observer_from_this = true;
            REQUIRE_THROWS_MATCHES(
                (make_pointer_deleter_1<TestType>()), oup::bad_observer_from_this,
                snatch::matchers::with_what_contains{
                    "observer_from_this() called with uninitialized control block"});
            next_test_object_constructor_calls_observer_from_this = false;
        }

        CHECK_NO_LEAKS;
    }
};
