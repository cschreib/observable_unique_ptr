#include "tests_common.hpp"

#include <cstdio>

int instances         = 0;
int instances_derived = 0;
int instances_deleter = 0;

bool next_test_object_constructor_throws                   = false;
bool next_test_object_constructor_calls_observer_from_this = false;

constexpr bool debug_instances = false;

test_object::test_object() {
    if (next_test_object_constructor_throws) {
        next_test_object_constructor_throws = false;

        if constexpr (debug_instances) {
            std::printf("%p creation throws\n", static_cast<void*>(this));
        }

        throw throw_constructor{};
    }

    if constexpr (debug_instances) {
        std::printf("%p created\n", static_cast<void*>(this));
    }

    ++instances;
}

test_object::test_object(state s) : state_(s) {
    if (next_test_object_constructor_throws) {
        next_test_object_constructor_throws = false;

        if constexpr (debug_instances) {
            std::printf("%p creation throws\n", static_cast<void*>(this));
        }

        throw throw_constructor{};
    }

    if constexpr (debug_instances) {
        std::printf("%p created\n", static_cast<void*>(this));
    }

    ++instances;
}

test_object::~test_object() noexcept {
    if constexpr (debug_instances) {
        std::printf("%p deleted\n", static_cast<void*>(this));
    }

    --instances;
}

test_object_derived::test_object_derived() {
    ++instances_derived;
}

test_object_derived::test_object_derived(state s) : test_object(s) {
    ++instances_derived;
}

test_object_derived::~test_object_derived() noexcept {
    --instances_derived;
}

test_deleter::test_deleter() noexcept {
    ++instances_deleter;
}

test_deleter::test_deleter(state s) noexcept : state_(s) {
    ++instances_deleter;
}

test_deleter::test_deleter(const test_deleter& source) noexcept : state_(source.state_) {
    ++instances_deleter;
}

test_deleter::test_deleter(test_deleter&& source) noexcept : state_(source.state_) {
    source.state_ = state::empty;
    ++instances_deleter;
}

test_deleter::~test_deleter() noexcept {
    --instances_deleter;
}

test_deleter& test_deleter::operator=(test_deleter&& source) noexcept {
    state_        = source.state_;
    source.state_ = state::empty;
    return *this;
}

void test_deleter::operator()(test_object* ptr) noexcept {
    delete ptr;
}

void test_deleter::operator()(const test_object* ptr) noexcept {
    delete ptr;
}

void test_deleter::operator()(std::nullptr_t) noexcept {}
