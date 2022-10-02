#include "memory_tracker.hpp"
#include "testing.hpp"
#include "tests_common.hpp"

TEMPLATE_LIST_TEST_CASE("observer default constructor", "[construction],[observer]", owner_types) {
    memory_tracker mem_track;

    {
        observer_ptr<TestType> ptr;
        CHECK(instances == 0);
        CHECK(ptr.get() == nullptr);
        CHECK(ptr.expired() == true);
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

TEMPLATE_LIST_TEST_CASE("observer nullptr constructor", "[construction],[observer]", owner_types) {
    memory_tracker mem_track;

    {
        observer_ptr<TestType> ptr(nullptr);
        CHECK(instances == 0);
        CHECK(ptr.get() == nullptr);
        CHECK(ptr.expired() == true);
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
    "observer from empty owner constructor",
    "[construction],[observer],[from_owner]",
    owner_types) {
    memory_tracker mem_track;

    {
        TestType ptr_owner;
        {
            observer_ptr<TestType> ptr{ptr_owner};
            CHECK(instances == 0);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
            CHECK(ptr.get() == ptr_owner.get());
            CHECK(ptr.expired() == true);
        }

        CHECK(instances == 0);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 1);
        }
    }

    CHECK(instances == 0);
    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE(
    "observer from valid owner constructor",
    "[construction],[observer],[from_owner]",
    owner_types) {
    memory_tracker mem_track;

    {
        TestType ptr_owner = make_pointer_deleter_1<TestType>();
        {
            observer_ptr<TestType> ptr{ptr_owner};
            CHECK(instances == 1);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
            CHECK(ptr.get() == ptr_owner.get());
            CHECK(ptr.expired() == false);
        }

        CHECK(instances == 1);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 1);
        }
    }

    CHECK(instances == 0);
    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE(
    "observer from empty owner conversion constructor",
    "[construction],[observer],[from_owner]",
    owner_types) {
    if constexpr (has_base<TestType>) {
        memory_tracker mem_track;

        {
            TestType ptr_owner;
            {
                base_observer_ptr<TestType> ptr{ptr_owner};
                CHECK(instances == 0);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(instances_deleter == 1);
                }
                CHECK(ptr.get() == ptr_owner.get());
                CHECK(ptr.expired() == true);
            }

            CHECK(instances == 0);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
        }

        CHECK(instances == 0);
        CHECK(mem_track.allocated() == 0u);
        CHECK(mem_track.double_delete() == 0u);
    }
};

TEMPLATE_LIST_TEST_CASE(
    "observer from valid owner conversion constructor",
    "[construction],[observer],[from_owner]",
    owner_types) {
    if constexpr (has_base<TestType>) {
        memory_tracker mem_track;

        {
            TestType ptr_owner = make_pointer_deleter_1<TestType>();
            {
                base_observer_ptr<TestType> ptr{ptr_owner};
                CHECK(instances == 1);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(instances_deleter == 1);
                }
                CHECK(ptr.get() == ptr_owner.get());
                CHECK(ptr.expired() == false);
            }

            CHECK(instances == 1);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
        }

        CHECK(instances == 0);
        CHECK(mem_track.allocated() == 0u);
        CHECK(mem_track.double_delete() == 0u);
    }
};

TEMPLATE_LIST_TEST_CASE(
    "observer from empty owner explicit conversion constructor",
    "[construction],[observer],[from_owner]",
    owner_types) {
    if constexpr (has_base<TestType>) {
        memory_tracker mem_track;

        {
            base_ptr<TestType> ptr_owner;
            {
                observer_ptr<TestType> ptr{ptr_owner, static_cast<get_object<TestType>*>(nullptr)};
                CHECK(instances == 0);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(instances_deleter == 1);
                }
                CHECK(ptr.get() == ptr_owner.get());
                CHECK(ptr.expired() == true);
            }

            CHECK(instances == 0);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
        }

        CHECK(instances == 0);
        CHECK(mem_track.allocated() == 0u);
        CHECK(mem_track.double_delete() == 0u);
    }
};

TEMPLATE_LIST_TEST_CASE(
    "observer from valid owner explicit conversion constructor",
    "[construction],[observer],[from_owner]",
    owner_types) {
    if constexpr (has_base<TestType>) {
        memory_tracker mem_track;

        {
            base_ptr<TestType> ptr_owner = make_pointer_deleter_1<TestType>();
            {
                observer_ptr<TestType> ptr{
                    ptr_owner, dynamic_cast<get_object<TestType>*>(ptr_owner.get())};
                CHECK(instances == 1);
                if constexpr (has_stateful_deleter<TestType>) {
                    CHECK(instances_deleter == 1);
                }
                CHECK(ptr.get() == ptr_owner.get());
                CHECK(ptr.expired() == false);
            }

            CHECK(instances == 1);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
        }

        CHECK(instances == 0);
        CHECK(mem_track.allocated() == 0u);
        CHECK(mem_track.double_delete() == 0u);
    }
};

TEMPLATE_LIST_TEST_CASE(
    "observer copy constructor valid", "[construction],[observer],[from_observer]", owner_types) {
    memory_tracker mem_track;

    {
        TestType               ptr_owner = make_pointer_deleter_1<TestType>();
        observer_ptr<TestType> ptr_orig{ptr_owner};
        {
            observer_ptr<TestType> ptr(ptr_orig);
            CHECK(ptr.get() != nullptr);
            CHECK(ptr.expired() == false);
            CHECK(ptr_orig.get() != nullptr);
            CHECK(ptr_orig.expired() == false);
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

TEMPLATE_LIST_TEST_CASE(
    "observer copy constructor empty", "[construction],[observer],[from_observer]", owner_types) {
    memory_tracker mem_track;

    {
        observer_ptr<TestType> ptr_orig;
        {
            observer_ptr<TestType> ptr(ptr_orig);
            CHECK(ptr.get() == nullptr);
            CHECK(ptr.expired() == true);
            CHECK(instances == 0);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 0);
            }
        }

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
    "observer copy from valid observer implicit conversion constructor",
    "[construction],[observer],[from_observer]",
    owner_types) {
    if constexpr (has_base<TestType>) {
        memory_tracker mem_track;

        {
            TestType               ptr_owner = make_pointer_deleter_1<TestType>();
            observer_ptr<TestType> ptr_orig{ptr_owner};
            {
                base_observer_ptr<TestType> ptr{ptr_orig};
                CHECK(ptr.get() == ptr_owner.get());
                CHECK(ptr.expired() == false);
                CHECK(ptr_orig.get() == ptr_owner.get());
                CHECK(ptr_orig.expired() == false);
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
    }
};

TEMPLATE_LIST_TEST_CASE(
    "observer copy from empty observer implicit conversion constructor",
    "[construction],[observer],[from_observer]",
    owner_types) {
    memory_tracker mem_track;

    {
        observer_ptr<TestType> ptr_orig;
        {
            base_observer_ptr<TestType> ptr{ptr_orig};
            CHECK(ptr.get() == nullptr);
            CHECK(ptr.expired() == true);
            CHECK(ptr_orig.get() == nullptr);
            CHECK(ptr_orig.expired() == true);
            CHECK(instances == 0);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 0);
            }
        }

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
    "observer copy from valid observer explicit conversion constructor",
    "[construction],[observer],[from_observer]",
    owner_types) {
    if constexpr (has_base<TestType>) {
        memory_tracker mem_track;

        {
            base_ptr<TestType>          ptr_owner = make_pointer_deleter_1<TestType>();
            base_observer_ptr<TestType> ptr_orig{ptr_owner};
            {
                observer_ptr<TestType> ptr{
                    ptr_orig, static_cast<get_object<TestType>*>(ptr_orig.get())};
                CHECK(ptr.get() == static_cast<get_object<TestType>*>(ptr_owner.get()));
                CHECK(ptr.expired() == false);
                CHECK(ptr_orig.get() == ptr_owner.get());
                CHECK(ptr_orig.expired() == false);
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
    }
};

TEMPLATE_LIST_TEST_CASE(
    "observer copy from empty observer explicit conversion constructor",
    "[construction],[observer],[from_observer]",
    owner_types) {
    memory_tracker mem_track;

    {
        base_observer_ptr<TestType> ptr_orig;
        {
            observer_ptr<TestType> ptr{ptr_orig, static_cast<get_object<TestType>*>(nullptr)};
            CHECK(ptr.get() == nullptr);
            CHECK(ptr.expired() == true);
            CHECK(ptr_orig.get() == nullptr);
            CHECK(ptr_orig.expired() == true);
            CHECK(instances == 0);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 0);
            }
        }

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
    "observer copy from valid observer explicit conversion constructor with null",
    "[construction],[observer],[from_observer]",
    owner_types) {
    memory_tracker mem_track;

    {
        base_ptr<TestType>          ptr_owner = make_pointer_deleter_1<TestType>();
        base_observer_ptr<TestType> ptr_orig{ptr_owner};
        {
            observer_ptr<TestType> ptr{ptr_orig, static_cast<get_object<TestType>*>(nullptr)};
            CHECK(ptr.get() == nullptr);
            CHECK(ptr.expired() == true);
            CHECK(ptr_orig.get() == ptr_owner.get());
            CHECK(ptr_orig.expired() == false);
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

TEMPLATE_LIST_TEST_CASE(
    "observer copy from valid observer explicit conversion constructor subobject",
    "[construction],[observer],[from_observer]",
    owner_types) {
    memory_tracker mem_track;

    {
        TestType               ptr_owner = make_pointer_deleter_1<TestType>();
        observer_ptr<TestType> ptr_orig{ptr_owner};
        {
            state_observer_ptr<TestType> ptr{ptr_orig, &ptr_owner->state_};
            CHECK(ptr.get() == &ptr_owner->state_);
            CHECK(ptr.expired() == false);
            CHECK(ptr_orig.get() == ptr_owner.get());
            CHECK(ptr_orig.expired() == false);
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

TEMPLATE_LIST_TEST_CASE(
    "observer move from valid observer constructor",
    "[construction],[observer],[from_observer]",
    owner_types) {
    memory_tracker mem_track;

    {
        TestType               ptr_owner = make_pointer_deleter_1<TestType>();
        observer_ptr<TestType> ptr_orig{ptr_owner};
        {
            observer_ptr<TestType> ptr(std::move(ptr_orig));
            CHECK(instances == 1);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
            CHECK(ptr.get() != nullptr);
            CHECK(ptr.expired() == false);
            CHECK(ptr_orig.get() == nullptr);
            CHECK(ptr_orig.expired() == true);
        }

        CHECK(instances == 1);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 1);
        }
    }

    CHECK(instances == 0);
    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE(
    "observer move from empty observer constructor",
    "[construction],[observer],[from_observer]",
    owner_types) {
    memory_tracker mem_track;

    {
        observer_ptr<TestType> ptr_orig;
        {
            observer_ptr<TestType> ptr(std::move(ptr_orig));
            CHECK(instances == 0);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 0);
            }
            CHECK(ptr.get() == nullptr);
            CHECK(ptr.expired() == true);
            CHECK(ptr_orig.get() == nullptr);
            CHECK(ptr_orig.expired() == true);
        }

        CHECK(instances == 0);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 0);
        }
    }

    CHECK(instances == 0);
    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE(
    "observer move from valid observer implicit conversion constructor",
    "[construction],[observer],[from_observer]",
    owner_types) {
    memory_tracker mem_track;

    {
        TestType               ptr_owner = make_pointer_deleter_1<TestType>();
        observer_ptr<TestType> ptr_orig{ptr_owner};
        {
            base_observer_ptr<TestType> ptr{std::move(ptr_orig)};
            CHECK(instances == 1);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
            CHECK(ptr.get() == ptr_owner.get());
            CHECK(ptr.expired() == false);
            CHECK(ptr_orig.get() == nullptr);
            CHECK(ptr_orig.expired() == true);
        }

        CHECK(instances == 1);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 1);
        }
    }

    CHECK(instances == 0);
    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE(
    "observer move from empty observer implicit conversion constructor",
    "[construction],[observer],[from_observer]",
    owner_types) {
    memory_tracker mem_track;

    {
        observer_ptr<TestType> ptr_orig;
        {
            base_observer_ptr<TestType> ptr{std::move(ptr_orig)};
            CHECK(instances == 0);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 0);
            }
            CHECK(ptr.get() == nullptr);
            CHECK(ptr.expired() == true);
            CHECK(ptr_orig.get() == nullptr);
            CHECK(ptr_orig.expired() == true);
        }

        CHECK(instances == 0);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 0);
        }
    }

    CHECK(instances == 0);
    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE(
    "observer move from valid observer explicit conversion constructor",
    "[construction],[observer],[from_observer]",
    owner_types) {
    memory_tracker mem_track;

    {
        base_ptr<TestType>          ptr_owner = make_pointer_deleter_1<TestType>();
        base_observer_ptr<TestType> ptr_orig{ptr_owner};
        {
            observer_ptr<TestType> ptr{
                std::move(ptr_orig), dynamic_cast<get_object<TestType>*>(ptr_orig.get())};
            CHECK(instances == 1);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
            CHECK(ptr.get() == dynamic_cast<get_object<TestType>*>(ptr_owner.get()));
            CHECK(ptr.expired() == false);
            CHECK(ptr_orig.get() == nullptr);
            CHECK(ptr_orig.expired() == true);
        }

        CHECK(instances == 1);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 1);
        }
    }

    CHECK(instances == 0);
    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE(
    "observer move from empty observer explicit conversion constructor",
    "[construction],[observer],[from_observer]",
    owner_types) {
    memory_tracker mem_track;

    {
        base_observer_ptr<TestType> ptr_orig;
        {
            observer_ptr<TestType> ptr{
                std::move(ptr_orig), static_cast<get_object<TestType>*>(nullptr)};
            CHECK(instances == 0);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 0);
            }
            CHECK(ptr.get() == nullptr);
            CHECK(ptr.expired() == true);
            CHECK(ptr_orig.get() == nullptr);
            CHECK(ptr_orig.expired() == true);
        }

        CHECK(instances == 0);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 0);
        }
    }

    CHECK(instances == 0);
    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE(
    "observer move from valid observer explicit conversion constructor with null",
    "[construction],[observer],[from_observer]",
    owner_types) {
    memory_tracker mem_track;

    {
        base_ptr<TestType>          ptr_owner = make_pointer_deleter_1<TestType>();
        base_observer_ptr<TestType> ptr_orig{ptr_owner};
        {
            observer_ptr<TestType> ptr{
                std::move(ptr_orig), static_cast<get_object<TestType>*>(nullptr)};
            CHECK(instances == 1);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
            CHECK(ptr.get() == nullptr);
            CHECK(ptr.expired() == true);
            CHECK(ptr_orig.get() == nullptr);
            CHECK(ptr_orig.expired() == true);
        }

        CHECK(instances == 1);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 1);
        }
    }

    CHECK(instances == 0);
    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};

TEMPLATE_LIST_TEST_CASE(
    "observer move from valid observer explicit conversion constructor subobject",
    "[construction],[observer],[from_observer]",
    owner_types) {
    memory_tracker mem_track;

    {
        TestType               ptr_owner = make_pointer_deleter_1<TestType>();
        observer_ptr<TestType> ptr_orig{ptr_owner};
        {
            state_observer_ptr<TestType> ptr{std::move(ptr_orig), &ptr_owner->state_};
            CHECK(instances == 1);
            if constexpr (has_stateful_deleter<TestType>) {
                CHECK(instances_deleter == 1);
            }
            CHECK(ptr.get() == &ptr_owner->state_);
            CHECK(ptr.expired() == false);
            CHECK(ptr_orig.get() == nullptr);
            CHECK(ptr_orig.expired() == true);
        }

        CHECK(instances == 1);
        if constexpr (has_stateful_deleter<TestType>) {
            CHECK(instances_deleter == 1);
        }
    }

    CHECK(instances == 0);
    CHECK(mem_track.allocated() == 0u);
    CHECK(mem_track.double_delete() == 0u);
};
