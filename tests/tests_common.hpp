#include "oup/observable_unique_ptr.hpp"

#include <exception>

int instances = 0;
int instances_derived = 0;
int instances_thrower = 0;
int instances_deleter = 0;

struct test_object {
    int state_ = 1337;

    test_object() noexcept { ++instances; }
    virtual ~test_object() noexcept { --instances; }

    test_object(const test_object&) = delete;
    test_object(test_object&&) = delete;

    test_object& operator=(const test_object&) = delete;
    test_object& operator=(test_object&&) = delete;
};

struct test_object_derived : test_object {
    test_object_derived() noexcept { ++instances_derived; }
    virtual ~test_object_derived() noexcept { --instances_derived; }
};

struct throw_constructor : std::exception {};

struct test_object_thrower {
    test_object_thrower() { throw throw_constructor{}; }
    ~test_object_thrower() { --instances_thrower; }

    test_object_thrower(const test_object_thrower&) = delete;
    test_object_thrower(test_object_thrower&&) = delete;

    test_object_thrower& operator=(const test_object_thrower&) = delete;
    test_object_thrower& operator=(test_object_thrower&&) = delete;
};

struct test_object_observer_from_this :
    public test_object,
    public oup::enable_observer_from_this<test_object_observer_from_this> {};

struct test_object_observer_from_this_derived :
    public test_object_observer_from_this {};

struct test_deleter {
    int state_ = 0;

    test_deleter() noexcept { ++instances_deleter; }
    explicit test_deleter(int state) noexcept : state_(state) { ++instances_deleter; }

    test_deleter(const test_deleter& source) noexcept : state_(source.state_) { ++instances_deleter; }
    test_deleter(test_deleter&& source) noexcept : state_(source.state_) {
        source.state_ = 0;
        ++instances_deleter;
    }

    ~test_deleter() noexcept { --instances_deleter; }

    test_deleter& operator=(const test_deleter&) = default;
    test_deleter& operator=(test_deleter&& source) noexcept {
        state_ = source.state_;
        source.state_ = 0;
        return *this;
    }

    void operator() (test_object* ptr) noexcept { delete ptr; }
    void operator() (test_object_thrower* ptr) noexcept { delete ptr; }
    void operator() (std::nullptr_t) noexcept {}
};

using test_ptr = oup::observable_unique_ptr<test_object>;
using test_sptr = oup::observable_sealed_ptr<test_object>;
using test_ptr_derived = oup::observable_unique_ptr<test_object_derived>;
using test_sptr_derived = oup::observable_sealed_ptr<test_object_derived>;
using test_ptr_with_deleter = oup::observable_unique_ptr<test_object,test_deleter>;
using test_ptr_derived_with_deleter = oup::observable_unique_ptr<test_object_derived,test_deleter>;
using test_ptr_thrower = oup::observable_unique_ptr<test_object_thrower>;
using test_sptr_thrower = oup::observable_sealed_ptr<test_object_thrower>;
using test_ptr_thrower_with_deleter = oup::observable_unique_ptr<test_object_thrower,test_deleter>;
using test_ptr_from_this = oup::observable_unique_ptr<test_object_observer_from_this>;
using test_sptr_from_this = oup::observable_sealed_ptr<test_object_observer_from_this>;
using test_cptr_from_this = oup::observable_unique_ptr<const test_object_observer_from_this>;
using test_csptr_from_this = oup::observable_sealed_ptr<const test_object_observer_from_this>;
using test_ptr_from_this_derived = oup::observable_unique_ptr<test_object_observer_from_this_derived>;
using test_sptr_from_this_derived = oup::observable_sealed_ptr<test_object_observer_from_this_derived>;

using test_optr = oup::observer_ptr<test_object>;
using test_optr_derived = oup::observer_ptr<test_object_derived>;
using test_optr_from_this = oup::observer_ptr<test_object_observer_from_this>;
using test_optr_from_this_const = oup::observer_ptr<const test_object_observer_from_this>;
using test_optr_from_this_derived = oup::observer_ptr<test_object_observer_from_this_derived>;
using test_optr_from_this_derived_const = oup::observer_ptr<const test_object_observer_from_this_derived>;
