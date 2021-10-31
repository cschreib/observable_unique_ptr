#include "tests_common.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("default constructor", "[construction]") {
    {
        test_ptr ptr{};
        REQUIRE(instances == 0);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.has_deleter() == false);
    }

    REQUIRE(instances == 0);
}

TEST_CASE("default constructor with deleter", "[construction]") {
    {
        test_ptr_with_deleter ptr{};
        REQUIRE(instances == 0);
        REQUIRE(instances_deleter == 1);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.has_deleter() == true);
        REQUIRE(ptr.get_deleter().state_ == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_deleter == 0);
}

TEST_CASE("nullptr constructor", "[construction]") {
    {
        test_ptr ptr{nullptr};
        REQUIRE(instances == 0);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.has_deleter() == false);
    }

    REQUIRE(instances == 0);
}

TEST_CASE("nullptr constructor with deleter", "[construction]") {
    {
        test_ptr_with_deleter ptr{nullptr, test_deleter{42}};
        REQUIRE(instances == 0);
        REQUIRE(instances_deleter == 1);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.has_deleter() == true);
        REQUIRE(ptr.get_deleter().state_ == 42);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_deleter == 0);
}

TEST_CASE("move constructor", "[construction]") {
    {
        test_ptr ptr_orig(new test_object);
        {
            test_ptr ptr(std::move(ptr_orig));
            REQUIRE(instances == 1);
            REQUIRE(ptr.get() != nullptr);
            REQUIRE(ptr.has_deleter() == false);
        }

        REQUIRE(instances == 0);
    }

    REQUIRE(instances == 0);
}

TEST_CASE("move constructor with deleter", "[construction]") {
    {
        test_ptr_with_deleter ptr_orig(new test_object, test_deleter{42});
        {
            test_ptr_with_deleter ptr(std::move(ptr_orig));
            REQUIRE(instances == 1);
            REQUIRE(instances_deleter == 1);
            REQUIRE(ptr.get() != nullptr);
            REQUIRE(ptr.has_deleter() == true);
            REQUIRE(ptr.get_deleter().state_ == 42);
        }

        REQUIRE(instances == 0);
        REQUIRE(instances_deleter == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_deleter == 0);
}

TEST_CASE("acquiring constructor", "[construction]") {
    {
        test_ptr ptr{new test_object};
        REQUIRE(instances == 1);
        REQUIRE(ptr.get() != nullptr);
        REQUIRE(ptr.has_deleter() == false);
    }

    REQUIRE(instances == 0);
}

TEST_CASE("acquiring constructor with deleter", "[construction]") {
    {
        test_ptr_with_deleter ptr{new test_object, test_deleter{42}};
        REQUIRE(instances == 1);
        REQUIRE(instances_deleter == 1);
        REQUIRE(ptr.get() != nullptr);
        REQUIRE(ptr.has_deleter() == true);
        REQUIRE(ptr.get_deleter().state_ == 42);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_deleter == 0);
}

TEST_CASE("implicit conversion constructor", "[construction]") {
    {
        test_ptr_derived ptr_orig{new test_object_derived};
        {
            test_ptr ptr(std::move(ptr_orig));
            REQUIRE(instances == 1);
            REQUIRE(instances_derived == 1);
            REQUIRE(ptr.get() != nullptr);
            REQUIRE(ptr.has_deleter() == false);
        }

        REQUIRE(instances == 0);
        REQUIRE(instances_derived == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_derived == 0);
}

TEST_CASE("implicit conversion constructor with deleter", "[construction]") {
    {
        test_ptr_derived_with_deleter ptr_orig{new test_object_derived, test_deleter{42}};
        {
            test_ptr_with_deleter ptr(std::move(ptr_orig));
            REQUIRE(instances == 1);
            REQUIRE(instances_derived == 1);
            REQUIRE(instances_deleter == 1);
            REQUIRE(ptr.get() != nullptr);
            REQUIRE(ptr.has_deleter() == true);
            REQUIRE(ptr.get_deleter().state_ == 42);
        }

        REQUIRE(instances == 0);
        REQUIRE(instances_derived == 0);
        REQUIRE(instances_deleter == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_derived == 0);
    REQUIRE(instances_deleter == 0);
}

TEST_CASE("explicit conversion constructor", "[construction]") {
    {
        test_ptr ptr_orig{new test_object_derived};
        {
            test_ptr_derived ptr(std::move(ptr_orig),
                dynamic_cast<test_object_derived*>(ptr_orig.get()));
            REQUIRE(instances == 1);
            REQUIRE(instances_derived == 1);
            REQUIRE(ptr.get() != nullptr);
            REQUIRE(ptr.has_deleter() == false);
        }

        REQUIRE(instances == 0);
        REQUIRE(instances_derived == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_derived == 0);
}

TEST_CASE("explicit conversion constructor with deleter", "[construction]") {
    {
        test_ptr_with_deleter ptr_orig{new test_object_derived, test_deleter{42}};
        {
            test_ptr_derived_with_deleter ptr(std::move(ptr_orig),
                dynamic_cast<test_object_derived*>(ptr_orig.get()));
            REQUIRE(instances == 1);
            REQUIRE(instances_derived == 1);
            REQUIRE(instances_deleter == 1);
            REQUIRE(ptr.get() != nullptr);
            REQUIRE(ptr.has_deleter() == true);
            REQUIRE(ptr.get_deleter().state_ == 42);
        }

        REQUIRE(instances == 0);
        REQUIRE(instances_derived == 0);
        REQUIRE(instances_deleter == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_derived == 0);
    REQUIRE(instances_deleter == 0);
}

TEST_CASE("move assignment operator", "[assignment]") {
    {
        test_ptr ptr_orig(new test_object);
        {
            test_ptr ptr;
            ptr = std::move(ptr_orig);
            REQUIRE(instances == 1);
            REQUIRE(ptr.get() != nullptr);
            REQUIRE(ptr.has_deleter() == false);
        }

        REQUIRE(instances == 0);
    }

    REQUIRE(instances == 0);
}

TEST_CASE("move assignment operator with deleter", "[assignment]") {
    {
        test_ptr_with_deleter ptr_orig(new test_object, test_deleter{42});
        {
            test_ptr_with_deleter ptr;
            ptr = std::move(ptr_orig);
            REQUIRE(instances == 1);
            REQUIRE(instances_deleter == 1);
            REQUIRE(ptr.get() != nullptr);
            REQUIRE(ptr.has_deleter() == true);
            REQUIRE(ptr.get_deleter().state_ == 42);
        }

        REQUIRE(instances == 0);
        REQUIRE(instances_deleter == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_deleter == 0);
}

TEST_CASE("reset to null", "[utility]") {
    {
        test_ptr ptr(new test_object);
        ptr.reset();
        REQUIRE(instances == 0);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.has_deleter() == false);
    }

    REQUIRE(instances == 0);
}

TEST_CASE("reset to null with deleter", "[utility]") {
    {
        test_ptr_with_deleter ptr(new test_object, test_deleter{42});
        ptr.reset();
        REQUIRE(instances == 0);
        REQUIRE(instances_deleter == 1);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.has_deleter() == true);
        REQUIRE(ptr.get_deleter().state_ == 42);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_deleter == 0);
}

TEST_CASE("reset to new", "[utility]") {
    {
        test_ptr ptr(new test_object);
        ptr.reset(new test_object);
        REQUIRE(instances == 1);
        REQUIRE(ptr.get() != nullptr);
        REQUIRE(ptr.has_deleter() == false);
    }

    REQUIRE(instances == 0);
}

TEST_CASE("reset to new with deleter", "[utility]") {
    {
        test_ptr_with_deleter ptr(new test_object, test_deleter{42});
        ptr.reset(new test_object);
        REQUIRE(instances == 1);
        REQUIRE(instances_deleter == 1);
        REQUIRE(ptr.get() != nullptr);
        REQUIRE(ptr.has_deleter() == true);
        REQUIRE(ptr.get_deleter().state_ == 42);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_deleter == 0);
}

TEST_CASE("reset to new with new deleter", "[utility]") {
    {
        test_ptr_with_deleter ptr(new test_object, test_deleter{42});
        ptr.reset(new test_object, test_deleter{43});
        REQUIRE(instances == 1);
        REQUIRE(instances_deleter == 1);
        REQUIRE(ptr.get() != nullptr);
        REQUIRE(ptr.has_deleter() == true);
        REQUIRE(ptr.get_deleter().state_ == 43);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_deleter == 0);
}

TEST_CASE("swap no instance", "[utility]") {
    {
        test_ptr ptr_orig;
        test_ptr ptr;
        ptr.swap(ptr_orig);
        REQUIRE(instances == 0);
        REQUIRE(ptr_orig.get() == nullptr);
        REQUIRE(ptr.get() == nullptr);
    }

    REQUIRE(instances == 0);
}

TEST_CASE("swap no instance with deleter", "[utility]") {
    {
        test_ptr_with_deleter ptr_orig(nullptr, test_deleter{42});
        test_ptr_with_deleter ptr(nullptr, test_deleter{43});
        ptr.swap(ptr_orig);
        REQUIRE(instances == 0);
        REQUIRE(instances_deleter == 2);
        REQUIRE(ptr_orig.get() == nullptr);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.has_deleter() == true);
        REQUIRE(ptr_orig.has_deleter() == true);
        REQUIRE(ptr.get_deleter().state_ == 42);
        REQUIRE(ptr_orig.get_deleter().state_ == 43);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_deleter == 0);
}

TEST_CASE("swap one instance", "[utility]") {
    {
        test_ptr ptr_orig(new test_object);
        test_ptr ptr;
        ptr.swap(ptr_orig);
        REQUIRE(instances == 1);
        REQUIRE(ptr_orig.get() == nullptr);
        REQUIRE(ptr.get() != nullptr);
    }

    REQUIRE(instances == 0);
}

TEST_CASE("swap one instance with deleter", "[utility]") {
    {
        test_ptr_with_deleter ptr_orig(new test_object, test_deleter{42});
        test_ptr_with_deleter ptr(nullptr, test_deleter{43});
        ptr.swap(ptr_orig);
        REQUIRE(instances == 1);
        REQUIRE(instances_deleter == 2);
        REQUIRE(ptr_orig.get() == nullptr);
        REQUIRE(ptr.get() != nullptr);
        REQUIRE(ptr.has_deleter() == true);
        REQUIRE(ptr_orig.has_deleter() == true);
        REQUIRE(ptr.get_deleter().state_ == 42);
        REQUIRE(ptr_orig.get_deleter().state_ == 43);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_deleter == 0);
}

TEST_CASE("swap two instances", "[utility]") {
    {
        test_ptr ptr_orig(new test_object);
        test_object* ptr_orig_raw = ptr_orig.get();
        test_ptr ptr(new test_object);
        test_object* ptr_raw = ptr.get();
        ptr.swap(ptr_orig);
        REQUIRE(instances == 2);
        REQUIRE(ptr_orig.get() == ptr_raw);
        REQUIRE(ptr.get() == ptr_orig_raw);
    }

    REQUIRE(instances == 0);
}

TEST_CASE("swap two instances with deleter", "[utility]") {
    {
        test_ptr_with_deleter ptr_orig(new test_object, test_deleter{42});
        test_object* ptr_orig_raw = ptr_orig.get();
        test_ptr_with_deleter ptr(new test_object, test_deleter{43});
        test_object* ptr_raw = ptr.get();
        ptr.swap(ptr_orig);
        REQUIRE(instances == 2);
        REQUIRE(instances_deleter == 2);
        REQUIRE(ptr_orig.get() == ptr_raw);
        REQUIRE(ptr.get() == ptr_orig_raw);
        REQUIRE(ptr.has_deleter() == true);
        REQUIRE(ptr_orig.has_deleter() == true);
        REQUIRE(ptr.get_deleter().state_ == 42);
        REQUIRE(ptr_orig.get_deleter().state_ == 43);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_deleter == 0);
}

TEST_CASE("make observable", "[make_observable_unique]") {
    {
        test_ptr ptr = oup::make_observable_unique<test_object>();
        REQUIRE(instances == 1);
        REQUIRE(ptr.get() != nullptr);
        REQUIRE(ptr.has_deleter() == false);
    }

    REQUIRE(instances == 0);
}
