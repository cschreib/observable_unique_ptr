#include "tests_common.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("owner default constructor", "[owner_construction]") {
    {
        test_ptr ptr{};
        REQUIRE(instances == 0);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.has_deleter() == false);
    }

    REQUIRE(instances == 0);
}

TEST_CASE("owner default constructor with deleter", "[owner_construction]") {
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

TEST_CASE("owner nullptr constructor", "[owner_construction]") {
    {
        test_ptr ptr{nullptr};
        REQUIRE(instances == 0);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.has_deleter() == false);
    }

    REQUIRE(instances == 0);
}

TEST_CASE("owner nullptr constructor with deleter", "[owner_construction]") {
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

TEST_CASE("owner move constructor", "[owner_construction]") {
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

TEST_CASE("owner move constructor with deleter", "[owner_construction]") {
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

TEST_CASE("owner acquiring constructor", "[owner_construction]") {
    {
        test_ptr ptr{new test_object};
        REQUIRE(instances == 1);
        REQUIRE(ptr.get() != nullptr);
        REQUIRE(ptr.has_deleter() == false);
    }

    REQUIRE(instances == 0);
}

TEST_CASE("owner acquiring constructor with deleter", "[owner_construction]") {
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

TEST_CASE("owner implicit conversion constructor", "[owner_construction]") {
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

TEST_CASE("owner implicit conversion constructor with deleter", "[owner_construction]") {
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

TEST_CASE("owner explicit conversion constructor", "[owner_construction]") {
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

TEST_CASE("owner explicit conversion constructor with deleter", "[owner_construction]") {
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

TEST_CASE("owner move assignment operator", "[owner_assignment]") {
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

TEST_CASE("owner move assignment operator with deleter", "[owner_assignment]") {
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

TEST_CASE("owner comparison valid ptr vs nullptr", "[owner_comparison]") {
    test_ptr ptr(new test_object);
    REQUIRE(ptr != nullptr);
    REQUIRE(!(ptr == nullptr));
    REQUIRE(nullptr != ptr);
    REQUIRE(!(nullptr == ptr));
}

TEST_CASE("owner comparison valid ptr vs nullptr with deleter", "[owner_comparison]") {
    test_ptr_with_deleter ptr(new test_object, test_deleter{42});
    REQUIRE(ptr != nullptr);
    REQUIRE(!(ptr == nullptr));
    REQUIRE(nullptr != ptr);
    REQUIRE(!(nullptr == ptr));
}

TEST_CASE("owner comparison invalid ptr vs nullptr", "[owner_comparison]") {
    test_ptr ptr;
    REQUIRE(ptr == nullptr);
    REQUIRE(!(ptr != nullptr));
    REQUIRE(nullptr == ptr);
    REQUIRE(!(nullptr != ptr));
}

TEST_CASE("owner comparison invalid ptr vs nullptr with deleter", "[owner_comparison]") {
    test_ptr_with_deleter ptr;
    REQUIRE(ptr == nullptr);
    REQUIRE(!(ptr != nullptr));
    REQUIRE(nullptr == ptr);
    REQUIRE(!(nullptr != ptr));
}

TEST_CASE("owner comparison invalid ptr vs nullptr with deleter explicit", "[owner_comparison]") {
    test_ptr_with_deleter ptr(nullptr, test_deleter{42});
    REQUIRE(ptr == nullptr);
    REQUIRE(!(ptr != nullptr));
    REQUIRE(nullptr == ptr);
    REQUIRE(!(nullptr != ptr));
}

TEST_CASE("owner comparison invalid ptr vs invalid ptr", "[owner_comparison]") {
    test_ptr ptr1;
    test_ptr ptr2;
    REQUIRE(ptr1 == ptr2);
    REQUIRE(!(ptr1 != ptr2));
}

TEST_CASE("owner comparison invalid ptr vs invalid ptr with deleter", "[owner_comparison]") {
    test_ptr_with_deleter ptr1;
    test_ptr_with_deleter ptr2;
    REQUIRE(ptr1 == ptr2);
    REQUIRE(!(ptr1 != ptr2));
}

TEST_CASE("owner comparison invalid ptr vs invalid ptr with deleter explicit", "[owner_comparison]") {
    test_ptr_with_deleter ptr1;
    test_ptr_with_deleter ptr2(nullptr, test_deleter{42});
    REQUIRE(ptr1 == ptr2);
    REQUIRE(ptr2 == ptr1);
    REQUIRE(!(ptr2 != ptr1));
    REQUIRE(!(ptr2 != ptr1));
}

TEST_CASE("owner comparison invalid ptr vs invalid ptr with both deleter explicit", "[owner_comparison]") {
    test_ptr_with_deleter ptr1(nullptr, test_deleter{43});
    test_ptr_with_deleter ptr2(nullptr, test_deleter{42});
    REQUIRE(ptr1 == ptr2);
    REQUIRE(ptr2 == ptr1);
    REQUIRE(!(ptr2 != ptr1));
    REQUIRE(!(ptr2 != ptr1));
}

TEST_CASE("owner comparison invalid ptr vs valid ptr", "[owner_comparison]") {
    test_ptr ptr1;
    test_ptr ptr2(new test_object);
    REQUIRE(ptr1 != ptr2);
    REQUIRE(!(ptr1 == ptr2));
    REQUIRE(ptr2 != ptr1);
    REQUIRE(!(ptr2 == ptr1));
}

TEST_CASE("owner comparison invalid ptr vs valid ptr with deleter", "[owner_comparison]") {
    test_ptr_with_deleter ptr1;
    test_ptr_with_deleter ptr2(new test_object, test_deleter{42});
    REQUIRE(ptr1 != ptr2);
    REQUIRE(!(ptr1 == ptr2));
    REQUIRE(ptr2 != ptr1);
    REQUIRE(!(ptr2 == ptr1));
}

TEST_CASE("owner comparison invalid ptr vs valid ptr with deleter explicit", "[owner_comparison]") {
    test_ptr_with_deleter ptr1(nullptr, test_deleter{43});
    test_ptr_with_deleter ptr2(new test_object, test_deleter{42});
    REQUIRE(ptr1 != ptr2);
    REQUIRE(!(ptr1 == ptr2));
    REQUIRE(ptr2 != ptr1);
    REQUIRE(!(ptr2 == ptr1));
}

TEST_CASE("owner comparison valid ptr vs valid ptr", "[owner_comparison]") {
    test_ptr ptr1(new test_object);
    test_ptr ptr2(new test_object);
    REQUIRE(ptr1 != ptr2);
    REQUIRE(!(ptr1 == ptr2));
    REQUIRE(ptr2 != ptr1);
    REQUIRE(!(ptr2 == ptr1));
}

TEST_CASE("owner comparison valid ptr vs valid ptr with deleter", "[owner_comparison]") {
    test_ptr_with_deleter ptr1(new test_object, test_deleter{43});
    test_ptr_with_deleter ptr2(new test_object, test_deleter{42});
    REQUIRE(ptr1 != ptr2);
    REQUIRE(!(ptr1 == ptr2));
    REQUIRE(ptr2 != ptr1);
    REQUIRE(!(ptr2 == ptr1));
}

TEST_CASE("owner reset to null", "[owner_utility]") {
    {
        test_ptr ptr(new test_object);
        ptr.reset();
        REQUIRE(instances == 0);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.has_deleter() == false);
    }

    REQUIRE(instances == 0);
}

TEST_CASE("owner reset to null with deleter", "[owner_utility]") {
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

TEST_CASE("owner reset to new", "[owner_utility]") {
    {
        test_ptr ptr(new test_object);
        ptr.reset(new test_object);
        REQUIRE(instances == 1);
        REQUIRE(ptr.get() != nullptr);
        REQUIRE(ptr.has_deleter() == false);
    }

    REQUIRE(instances == 0);
}

TEST_CASE("owner reset to new with deleter", "[owner_utility]") {
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

TEST_CASE("owner reset to new with new deleter", "[owner_utility]") {
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

TEST_CASE("owner swap no instance", "[owner_utility]") {
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

TEST_CASE("owner swap no instance with deleter", "[owner_utility]") {
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

TEST_CASE("owner swap one instance", "[owner_utility]") {
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

TEST_CASE("owner swap one instance with deleter", "[owner_utility]") {
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

TEST_CASE("owner swap two instances", "[owner_utility]") {
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

TEST_CASE("owner swap two instances with deleter", "[owner_utility]") {
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

TEST_CASE("make observable throw in constructor", "[make_observable_unique]") {
    REQUIRE_THROWS_AS(
        oup::make_observable_unique<test_object_thrower>(),
        throw_constructor);

    REQUIRE(instances_thrower == 0);
}

TEST_CASE("observer default constructor", "[observer_construction]") {
    {
        test_optr ptr{};
        REQUIRE(instances == 0);
        REQUIRE(ptr.lock() == nullptr);
        REQUIRE(ptr.expired() == true);
    }

    REQUIRE(instances == 0);
}

TEST_CASE("observer nullptr constructor", "[observer_construction]") {
    {
        test_optr ptr{nullptr};
        REQUIRE(instances == 0);
        REQUIRE(ptr.lock() == nullptr);
        REQUIRE(ptr.expired() == true);
    }

    REQUIRE(instances == 0);
}

TEST_CASE("observer move constructor", "[observer_construction]") {
    {
        test_ptr ptr_owner{new test_object};
        test_optr ptr_orig{ptr_owner};
        {
            test_optr ptr(std::move(ptr_orig));
            REQUIRE(instances == 1);
            REQUIRE(ptr.lock() != nullptr);
            REQUIRE(ptr.expired() == false);
        }

        REQUIRE(instances == 1);
    }

    REQUIRE(instances == 0);
}

TEST_CASE("observer acquiring constructor", "[observer_construction]") {
    {
        test_ptr ptr_owner{new test_object};
        test_optr ptr{ptr_owner};
        REQUIRE(instances == 1);
        REQUIRE(ptr.lock() != nullptr);
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
}

TEST_CASE("observer implicit copy conversion constructor", "[observer_construction]") {
    {
        test_ptr_derived ptr_owner{new test_object_derived};
        test_optr_derived ptr_orig{ptr_owner};
        {
            test_optr ptr(ptr_orig);
            REQUIRE(instances == 1);
            REQUIRE(instances_derived == 1);
            REQUIRE(ptr.lock() != nullptr);
            REQUIRE(ptr.expired() == false);
        }

        REQUIRE(instances == 1);
        REQUIRE(instances_derived == 1);
        REQUIRE(ptr_orig.lock() != nullptr);
        REQUIRE(ptr_orig.expired() == false);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_derived == 0);
}

TEST_CASE("observer implicit move conversion constructor", "[observer_construction]") {
    {
        test_ptr_derived ptr_owner{new test_object_derived};
        test_optr_derived ptr_orig{ptr_owner};
        {
            test_optr ptr(std::move(ptr_orig));
            REQUIRE(instances == 1);
            REQUIRE(instances_derived == 1);
            REQUIRE(ptr.lock() != nullptr);
            REQUIRE(ptr.expired() == false);
        }

        REQUIRE(instances == 1);
        REQUIRE(instances_derived == 1);
        REQUIRE(ptr_orig.lock() == nullptr);
        REQUIRE(ptr_orig.expired() == true);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_derived == 0);
}

TEST_CASE("observer expiring", "[observer_utility]") {
    test_optr ptr;

    {
        test_ptr ptr_owner{new test_object};
        ptr = ptr_owner;
        REQUIRE(instances == 1);
        REQUIRE(ptr.lock() != nullptr);
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
    REQUIRE(ptr.lock() == nullptr);
    REQUIRE(ptr.expired() == true);
}

TEST_CASE("observer reset to null", "[observer_utility]") {
    {
        test_ptr ptr_owner(new test_object);
        test_optr ptr(ptr_owner);
        ptr.reset();
        REQUIRE(instances == 1);
        REQUIRE(ptr.lock() == nullptr);
        REQUIRE(ptr.expired() == true);
    }

    REQUIRE(instances == 0);
}


TEST_CASE("observer swap no instance", "[observer_utility]") {
    {
        test_optr ptr_orig;
        test_optr ptr;
        ptr.swap(ptr_orig);
        REQUIRE(instances == 0);
        REQUIRE(ptr_orig.lock() == nullptr);
        REQUIRE(ptr.lock() == nullptr);
        REQUIRE(ptr_orig.expired() == true);
        REQUIRE(ptr.expired() == true);
    }

    REQUIRE(instances == 0);
}

TEST_CASE("observer swap one instance", "[observer_utility]") {
    {
        test_ptr ptr_owner(new test_object);
        test_optr ptr_orig(ptr_owner);
        test_optr ptr;
        ptr.swap(ptr_orig);
        REQUIRE(instances == 1);
        REQUIRE(ptr_orig.lock() == nullptr);
        REQUIRE(ptr.lock() != nullptr);
        REQUIRE(ptr_orig.expired() == true);
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
}

TEST_CASE("observer swap two same instance", "[observer_utility]") {
    {
        test_ptr ptr_owner(new test_object);
        test_optr ptr_orig(ptr_owner);
        test_optr ptr(ptr_owner);
        ptr.swap(ptr_orig);
        REQUIRE(instances == 1);
        REQUIRE(ptr_orig.lock() == ptr_owner.get());
        REQUIRE(ptr.lock() == ptr_owner.get());
        REQUIRE(ptr_orig.expired() == false);
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
}

TEST_CASE("observer swap two different instances", "[observer_utility]") {
    {
        test_ptr ptr_owner1(new test_object);
        test_ptr ptr_owner2(new test_object);
        test_optr ptr_orig(ptr_owner1);
        test_optr ptr(ptr_owner2);
        ptr.swap(ptr_orig);
        REQUIRE(instances == 2);
        REQUIRE(ptr_orig.lock() == ptr_owner2.get());
        REQUIRE(ptr.lock() == ptr_owner1.get());
        REQUIRE(ptr_orig.expired() == false);
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
}

TEST_CASE("observer comparison valid ptr vs nullptr", "[observer_comparison]") {
    test_ptr ptr_owner(new test_object);
    test_optr ptr(ptr_owner);
    REQUIRE(ptr != nullptr);
    REQUIRE(!(ptr == nullptr));
    REQUIRE(nullptr != ptr);
    REQUIRE(!(nullptr == ptr));
}

TEST_CASE("observer comparison invalid ptr vs nullptr", "[observer_comparison]") {
    test_optr ptr;
    REQUIRE(ptr == nullptr);
    REQUIRE(!(ptr != nullptr));
    REQUIRE(nullptr == ptr);
    REQUIRE(!(nullptr != ptr));
}

TEST_CASE("observer comparison invalid ptr vs invalid ptr", "[observer_comparison]") {
    test_optr ptr1;
    test_optr ptr2;
    REQUIRE(ptr1 == ptr2);
    REQUIRE(!(ptr1 != ptr2));
}

TEST_CASE("observer comparison invalid ptr vs valid ptr", "[observer_comparison]") {
    test_ptr ptr_owner(new test_object);
    test_optr ptr1;
    test_optr ptr2(ptr_owner);
    REQUIRE(ptr1 != ptr2);
    REQUIRE(!(ptr1 == ptr2));
    REQUIRE(ptr2 != ptr1);
    REQUIRE(!(ptr2 == ptr1));
}

TEST_CASE("observer comparison valid ptr vs valid ptr same owner", "[observer_comparison]") {
    test_ptr ptr_owner(new test_object);
    test_optr ptr1(ptr_owner);
    test_optr ptr2(ptr_owner);
    REQUIRE(ptr1 == ptr2);
    REQUIRE(!(ptr1 != ptr2));
    REQUIRE(ptr2 == ptr1);
    REQUIRE(!(ptr2 != ptr1));
}

TEST_CASE("observer comparison valid ptr vs valid ptr different owner", "[observer_comparison]") {
    test_ptr ptr_owner1(new test_object);
    test_ptr ptr_owner2(new test_object);
    test_optr ptr1(ptr_owner1);
    test_optr ptr2(ptr_owner2);
    REQUIRE(ptr1 != ptr2);
    REQUIRE(!(ptr1 == ptr2));
    REQUIRE(ptr2 != ptr1);
    REQUIRE(!(ptr2 == ptr1));
}
