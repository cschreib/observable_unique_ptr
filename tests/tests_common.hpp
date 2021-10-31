#include "oup/observable_unique_ptr.hpp"

int instances = 0;
int instances_derived = 0;
int instances_thrower = 0;
int instances_deleter = 0;

struct test_object {
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

struct test_object_thrower {
    test_object_thrower() { throw std::exception{}; }
    ~test_object_thrower() noexcept { --instances_thrower; }

    test_object_thrower(const test_object_thrower&) = delete;
    test_object_thrower(test_object_thrower&&) = delete;

    test_object_thrower& operator=(const test_object_thrower&) = delete;
    test_object_thrower& operator=(test_object_thrower&&) = delete;
};

struct test_deleter {
    int state_ = 0;

    test_deleter() { ++instances_deleter; }
    explicit test_deleter(int state) : state_(state) { ++instances_deleter; }

    test_deleter(const test_deleter& source) : state_(source.state_) { ++instances_deleter; }
    test_deleter(test_deleter&& source) : state_(source.state_) {
        source.state_ = 0;
        ++instances_deleter;
    }

    ~test_deleter() { --instances_deleter; }

    test_deleter& operator=(const test_deleter&) = default;
    test_deleter& operator=(test_deleter&& source) {
        state_ = source.state_;
        source.state_ = 0;
        return *this;
    }

    void operator() (test_object* ptr) { delete ptr; }
};

using test_ptr = oup::observable_unique_ptr<test_object>;
using test_ptr_derived = oup::observable_unique_ptr<test_object_derived>;
using test_ptr_with_deleter = oup::observable_unique_ptr<test_object,test_deleter>;
using test_ptr_derived_with_deleter = oup::observable_unique_ptr<test_object_derived,test_deleter>;
using test_ptr_thrower = oup::observable_unique_ptr<test_object_thrower>;
using test_ptr_thrower_with_deleter = oup::observable_unique_ptr<test_object_thrower,test_deleter>;

using test_wptr = oup::weak_ptr<test_object>;
using test_wptr_derived = oup::weak_ptr<test_object_derived>;
