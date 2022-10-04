#include "memory_tracker.hpp"
#include "testing.hpp"
#include "tests_common.hpp"

TEMPLATE_LIST_TEST_CASE("observer from this", "[observer_from_this],[observer]", owner_types) {
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

                CHECK(instances == 1);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(instances_deleter == 1);
                }
                CHECK(optr.expired() == false);
                CHECK(optr_const.expired() == false);
                CHECK(optr.get() == raw_ptr);
                CHECK(optr_const.get() == craw_ptr);
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

        CHECK(instances == 0);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 0);
        }
        CHECK(mem_track.allocated() == 0u);
        CHECK(mem_track.double_delete() == 0u);
    }
};

TEMPLATE_LIST_TEST_CASE(
    "observer from this after owner reset to empty",
    "[observer_from_this],[observer]",
    owner_types) {
    if constexpr (has_eoft<TestType>) {
        memory_tracker mem_track;

        {
            TestType ptr  = make_pointer_deleter_1<TestType>();
            auto     optr = make_observer_from_this<TestType>(ptr.get());

            CHECK(instances == 1);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
            CHECK(optr.expired() == false);
            CHECK(optr.get() == ptr.get());

            ptr.reset();

            CHECK(instances == 0);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
            CHECK(optr.expired() == true);
            CHECK(optr.get() == nullptr);
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
    "observer from this after owner reset to valid",
    "[observer_from_this],[observer]",
    owner_types) {
    if constexpr (has_eoft<TestType> && can_reset_to_new<TestType>) {
        memory_tracker mem_track;

        {
            TestType ptr  = make_pointer_deleter_1<TestType>();
            auto     optr = make_observer_from_this<TestType>(ptr.get());

            CHECK(instances == 1);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
            CHECK(optr.expired() == false);
            CHECK(optr.get() == ptr.get());

            ptr.reset(make_instance<TestType>());

            CHECK(instances == 1);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
            CHECK(optr.expired() == true);
            CHECK(optr.get() == nullptr);
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
    "observer from this after owner release", "[observer_from_this],[observer]", owner_types) {
    if constexpr (has_eoft<TestType> && can_release<TestType>) {
        memory_tracker mem_track;

        {
            TestType ptr  = make_pointer_deleter_1<TestType>();
            auto     optr = make_observer_from_this<TestType>(ptr.get());

            CHECK(instances == 1);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
            CHECK(optr.expired() == false);
            CHECK(optr.get() == ptr.get());

            auto* raw_ptr = ptr.release();

            CHECK(instances == 1);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
            CHECK(optr.expired() == false);
            CHECK(optr.get() == raw_ptr);

            delete raw_ptr;

            CHECK(instances == 0);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
            CHECK(optr.expired() == true);
            CHECK(optr.get() == nullptr);
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
    "observer from this after owner release then reset to same",
    "[observer_from_this],[observer]",
    owner_types) {
    if constexpr (has_eoft<TestType> && can_release<TestType> && can_reset_to_new<TestType>) {
        memory_tracker mem_track;

        {
            TestType ptr  = make_pointer_deleter_1<TestType>();
            auto     optr = make_observer_from_this<TestType>(ptr.get());

            CHECK(instances == 1);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
            CHECK(optr.expired() == false);
            CHECK(optr.get() == ptr.get());

            auto* raw_ptr = ptr.release();

            CHECK(instances == 1);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
            CHECK(optr.expired() == false);
            CHECK(optr.get() == raw_ptr);

            ptr.reset(raw_ptr);

            CHECK(instances == 1);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
            CHECK(optr.expired() == false);
            CHECK(optr.get() == raw_ptr);

            ptr.reset();

            CHECK(instances == 0);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
            CHECK(optr.expired() == true);
            CHECK(optr.get() == nullptr);
        }

        CHECK(instances == 0);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 0);
        }
        CHECK(mem_track.allocated() == 0u);
        CHECK(mem_track.double_delete() == 0u);
    }
};
