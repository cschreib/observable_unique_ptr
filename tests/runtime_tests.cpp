#include "tests_common.hpp"

#include <catch2/catch_test_macros.hpp>
#include <cstdlib>

// Allocation tracker, to catch memory leaks and double delete
constexpr std::size_t max_allocations = 20'000;
void* allocations[max_allocations];
void* allocations_array[max_allocations];
std::size_t num_allocations = 0u;
std::size_t double_delete = 0u;
bool memory_tracking = false;

#if defined(OUP_PLATFORM_OSX)
// Getting weird errors on MacOS when overriding operator new and delete,
// so disable the memory leak checking for this platform.
#   define CHECK_MEMORY_LEAKS 0
#else
#   define CHECK_MEMORY_LEAKS 1
#endif

#if defined(CHECK_MEMORY_LEAKS) && CHECK_MEMORY_LEAKS
void* allocate(std::size_t size, bool array) {
    if (memory_tracking && num_allocations == max_allocations) {
        throw std::bad_alloc();
    }

    void* p = std::malloc(size);
    if (!p) {
        throw std::bad_alloc();
    }

    if (memory_tracking) {
        if (array) {
            allocations_array[num_allocations] = p;
        } else {
            allocations[num_allocations] = p;
        }

        ++num_allocations;
    }

    return p;
}

void deallocate(void* p, bool array) {
    if (memory_tracking) {
        bool found = false;
        void** allocations_type = array ? allocations_array : allocations;
        for (std::size_t i = 0; i < num_allocations; ++i) {
            if (allocations_type[i] == p) {
                std::swap(allocations_type[i], allocations_type[num_allocations-1]);
                --num_allocations;
                found = true;
                break;
            }
        }

        if (!found) {
            ++double_delete;
        }
    }

    std::free(p);
}

void* operator new(std::size_t size) {
    return allocate(size, false);
}

void* operator new[](size_t size) {
    return allocate(size, true);
}

void operator delete(void* p) noexcept {
    deallocate(p, false);
}

void operator delete[](void* p) noexcept {
    deallocate(p, true);
}
#endif

struct memory_tracker {
    std::size_t initial_allocations;
    std::size_t initial_double_delete;

    memory_tracker() noexcept :
        initial_allocations(num_allocations), initial_double_delete(double_delete) {
        memory_tracking = true;
    }

    ~memory_tracker() noexcept {
        memory_tracking = false;
    }

    std::size_t leaks() const { return num_allocations - initial_allocations; }
    std::size_t double_del() const { return double_delete - initial_double_delete; }
};

TEST_CASE("owner size", "[owner_size]") {
    REQUIRE(sizeof(test_ptr) == 2*sizeof(void*));
}

TEST_CASE("owner size sealed", "[owner_size]") {
    REQUIRE(sizeof(test_sptr) == 2*sizeof(void*));
}

TEST_CASE("owner default constructor", "[owner_construction]") {
    memory_tracker mem_track;

    {
        test_ptr ptr;
        REQUIRE(instances == 0);
        REQUIRE(ptr.get() == nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner default constructor sealed", "[owner_construction]") {
    memory_tracker mem_track;

    {
        test_sptr ptr;
        REQUIRE(instances == 0);
        REQUIRE(ptr.get() == nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner default constructor with deleter", "[owner_construction]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr;
        REQUIRE(instances == 0);
        REQUIRE(instances_deleter == 1);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.get_deleter().state_ == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_deleter == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner nullptr constructor", "[owner_construction]") {
    memory_tracker mem_track;

    {
        test_ptr ptr{nullptr};
        REQUIRE(instances == 0);
        REQUIRE(ptr.get() == nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner nullptr constructor sealed", "[owner_construction]") {
    memory_tracker mem_track;

    {
        test_sptr ptr{nullptr};
        REQUIRE(instances == 0);
        REQUIRE(ptr.get() == nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner nullptr constructor with deleter", "[owner_construction]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr{nullptr, test_deleter{42}};
        REQUIRE(instances == 0);
        REQUIRE(instances_deleter == 1);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.get_deleter().state_ == 42);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_deleter == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner move constructor", "[owner_construction]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_orig(new test_object);
        {
            test_ptr ptr(std::move(ptr_orig));
            REQUIRE(instances == 1);
            REQUIRE(ptr.get() != nullptr);
        }

        REQUIRE(instances == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner move constructor sealed", "[owner_construction]") {
    memory_tracker mem_track;

    {
        test_sptr ptr_orig = oup::make_observable_sealed<test_object>();
        {
            test_sptr ptr(std::move(ptr_orig));
            REQUIRE(instances == 1);
            REQUIRE(ptr.get() != nullptr);
        }

        REQUIRE(instances == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner move constructor with deleter", "[owner_construction]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr_orig(new test_object, test_deleter{42});
        {
            test_ptr_with_deleter ptr(std::move(ptr_orig));
            REQUIRE(instances == 1);
            REQUIRE(instances_deleter == 2);
            REQUIRE(ptr.get() != nullptr);
            REQUIRE(ptr.get_deleter().state_ == 42);
        }

        REQUIRE(instances == 0);
        REQUIRE(instances_deleter == 1);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_deleter == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner acquiring constructor", "[owner_construction]") {
    memory_tracker mem_track;

    {
        test_ptr ptr{new test_object};
        REQUIRE(instances == 1);
        REQUIRE(ptr.get() != nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner acquiring constructor with deleter", "[owner_construction]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr{new test_object, test_deleter{42}};
        REQUIRE(instances == 1);
        REQUIRE(instances_deleter == 1);
        REQUIRE(ptr.get() != nullptr);
        REQUIRE(ptr.get_deleter().state_ == 42);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_deleter == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner acquiring constructor null", "[owner_construction]") {
    memory_tracker mem_track;

    {
        test_ptr ptr{static_cast<test_object*>(nullptr)};
        REQUIRE(instances == 0);
        REQUIRE(ptr.get() == nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner acquiring constructor null with deleter", "[owner_construction]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr{static_cast<test_object*>(nullptr), test_deleter{42}};
        REQUIRE(instances == 0);
        REQUIRE(instances_deleter == 1);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.get_deleter().state_ == 42);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_deleter == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner implicit conversion constructor", "[owner_construction]") {
    memory_tracker mem_track;

    {
        test_ptr_derived ptr_orig{new test_object_derived};
        {
            test_ptr ptr(std::move(ptr_orig));
            REQUIRE(instances == 1);
            REQUIRE(instances_derived == 1);
            REQUIRE(ptr.get() != nullptr);
        }

        REQUIRE(instances == 0);
        REQUIRE(instances_derived == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_derived == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner implicit conversion constructor sealed", "[owner_construction]") {
    memory_tracker mem_track;

    {
        test_sptr_derived ptr_orig = oup::make_observable_sealed<test_object_derived>();
        {
            test_sptr ptr(std::move(ptr_orig));
            REQUIRE(instances == 1);
            REQUIRE(instances_derived == 1);
            REQUIRE(ptr.get() != nullptr);
        }

        REQUIRE(instances == 0);
        REQUIRE(instances_derived == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_derived == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner implicit conversion constructor with deleter", "[owner_construction]") {
    memory_tracker mem_track;

    {
        test_ptr_derived_with_deleter ptr_orig{new test_object_derived, test_deleter{42}};
        {
            test_ptr_with_deleter ptr(std::move(ptr_orig));
            REQUIRE(instances == 1);
            REQUIRE(instances_derived == 1);
            REQUIRE(instances_deleter == 2);
            REQUIRE(ptr.get() != nullptr);
            REQUIRE(ptr.get_deleter().state_ == 42);
        }

        REQUIRE(instances == 0);
        REQUIRE(instances_derived == 0);
        REQUIRE(instances_deleter == 1);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_derived == 0);
    REQUIRE(instances_deleter == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner explicit conversion constructor", "[owner_construction]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_orig{new test_object_derived};
        {
            test_ptr_derived ptr(std::move(ptr_orig),
                dynamic_cast<test_object_derived*>(ptr_orig.get()));
            REQUIRE(instances == 1);
            REQUIRE(instances_derived == 1);
            REQUIRE(ptr.get() != nullptr);
        }

        REQUIRE(instances == 0);
        REQUIRE(instances_derived == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_derived == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner explicit conversion constructor sealed", "[owner_construction]") {
    memory_tracker mem_track;

    {
        test_sptr ptr_orig = oup::make_observable_sealed<test_object_derived>();
        {
            test_sptr_derived ptr(std::move(ptr_orig),
                dynamic_cast<test_object_derived*>(ptr_orig.get()));
            REQUIRE(instances == 1);
            REQUIRE(instances_derived == 1);
            REQUIRE(ptr.get() != nullptr);
        }

        REQUIRE(instances == 0);
        REQUIRE(instances_derived == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_derived == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner explicit conversion constructor with default deleter", "[owner_construction]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr_orig{new test_object_derived, test_deleter{42}};
        {
            test_ptr_derived_with_deleter ptr(std::move(ptr_orig),
                dynamic_cast<test_object_derived*>(ptr_orig.get()));
            REQUIRE(instances == 1);
            REQUIRE(instances_derived == 1);
            REQUIRE(instances_deleter == 2);
            REQUIRE(ptr.get() != nullptr);
            REQUIRE(ptr.get_deleter().state_ == 0);
        }

        REQUIRE(instances == 0);
        REQUIRE(instances_derived == 0);
        REQUIRE(instances_deleter == 1);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_derived == 0);
    REQUIRE(instances_deleter == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner explicit conversion constructor with custom deleter", "[owner_construction]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr_orig{new test_object_derived, test_deleter{42}};
        {
            test_ptr_derived_with_deleter ptr(std::move(ptr_orig),
                dynamic_cast<test_object_derived*>(ptr_orig.get()),
                test_deleter{43});
            REQUIRE(instances == 1);
            REQUIRE(instances_derived == 1);
            REQUIRE(instances_deleter == 2);
            REQUIRE(ptr.get() != nullptr);
            REQUIRE(ptr.get_deleter().state_ == 43);
        }

        REQUIRE(instances == 0);
        REQUIRE(instances_derived == 0);
        REQUIRE(instances_deleter == 1);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_derived == 0);
    REQUIRE(instances_deleter == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner move assignment operator valid to empty", "[owner_assignment]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_orig(new test_object);
        {
            test_ptr ptr;
            ptr = std::move(ptr_orig);
            REQUIRE(instances == 1);
            REQUIRE(ptr.get() != nullptr);
        }

        REQUIRE(instances == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner move assignment operator valid to empty sealed", "[owner_assignment]") {
    memory_tracker mem_track;

    {
        test_sptr ptr_orig = oup::make_observable_sealed<test_object>();
        {
            test_sptr ptr;
            ptr = std::move(ptr_orig);
            REQUIRE(instances == 1);
            REQUIRE(ptr.get() != nullptr);
        }

        REQUIRE(instances == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner move assignment operator valid to empty with deleter", "[owner_assignment]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr_orig(new test_object, test_deleter{42});
        {
            test_ptr_with_deleter ptr;
            ptr = std::move(ptr_orig);
            REQUIRE(instances == 1);
            REQUIRE(instances_deleter == 2);
            REQUIRE(ptr.get() != nullptr);
            REQUIRE(ptr.get_deleter().state_ == 42);
        }

        REQUIRE(instances == 0);
        REQUIRE(instances_deleter == 1);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_deleter == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner move assignment operator empty to valid", "[owner_assignment]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_orig;
        {
            test_ptr ptr(new test_object);
            ptr = std::move(ptr_orig);
            REQUIRE(instances == 0);
            REQUIRE(ptr.get() == nullptr);
        }

        REQUIRE(instances == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner move assignment operator empty to valid sealed", "[owner_assignment]") {
    memory_tracker mem_track;

    {
        test_sptr ptr_orig;
        {
            test_sptr ptr = oup::make_observable_sealed<test_object>();
            ptr = std::move(ptr_orig);
            REQUIRE(instances == 0);
            REQUIRE(ptr.get() == nullptr);
        }

        REQUIRE(instances == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner move assignment operator empty to valid with deleter", "[owner_assignment]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr_orig;
        {
            test_ptr_with_deleter ptr(new test_object, test_deleter{42});
            ptr = std::move(ptr_orig);
            REQUIRE(instances == 0);
            REQUIRE(instances_deleter == 2);
            REQUIRE(ptr.get() == nullptr);
            REQUIRE(ptr.get_deleter().state_ == 0);
        }

        REQUIRE(instances == 0);
        REQUIRE(instances_deleter == 1);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_deleter == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner move assignment operator empty to empty", "[owner_assignment]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_orig;
        {
            test_ptr ptr;
            ptr = std::move(ptr_orig);
            REQUIRE(instances == 0);
            REQUIRE(ptr.get() == nullptr);
        }

        REQUIRE(instances == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner move assignment operator empty to empty sealed", "[owner_assignment]") {
    memory_tracker mem_track;

    {
        test_sptr ptr_orig;
        {
            test_sptr ptr;
            ptr = std::move(ptr_orig);
            REQUIRE(instances == 0);
            REQUIRE(ptr.get() == nullptr);
        }

        REQUIRE(instances == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner move assignment operator empty to empty with deleter", "[owner_assignment]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr_orig;
        {
            test_ptr_with_deleter ptr;
            ptr = std::move(ptr_orig);
            REQUIRE(instances == 0);
            REQUIRE(instances_deleter == 2);
            REQUIRE(ptr.get() == nullptr);
            REQUIRE(ptr.get_deleter().state_ == 0);
        }

        REQUIRE(instances == 0);
        REQUIRE(instances_deleter == 1);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_deleter == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner move assignment operator valid to valid", "[owner_assignment]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_orig(new test_object);
        {
            test_ptr ptr(new test_object);
            ptr = std::move(ptr_orig);
            REQUIRE(instances == 1);
            REQUIRE(ptr.get() != nullptr);
        }

        REQUIRE(instances == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner move assignment operator valid to valid sealed", "[owner_assignment]") {
    memory_tracker mem_track;

    {
        test_sptr ptr_orig = oup::make_observable_sealed<test_object>();
        {
            test_sptr ptr = oup::make_observable_sealed<test_object>();
            ptr = std::move(ptr_orig);
            REQUIRE(instances == 1);
            REQUIRE(ptr.get() != nullptr);
        }

        REQUIRE(instances == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner move assignment operator valid to valid with deleter", "[owner_assignment]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr_orig(new test_object, test_deleter{42});
        {
            test_ptr_with_deleter ptr(new test_object, test_deleter{43});
            ptr = std::move(ptr_orig);
            REQUIRE(instances == 1);
            REQUIRE(instances_deleter == 2);
            REQUIRE(ptr.get() != nullptr);
            REQUIRE(ptr.get_deleter().state_ == 42);
        }

        REQUIRE(instances == 0);
        REQUIRE(instances_deleter == 1);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_deleter == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner comparison valid ptr vs nullptr", "[owner_comparison]") {
    memory_tracker mem_track;

    {
        test_ptr ptr(new test_object);
        REQUIRE(ptr != nullptr);
        REQUIRE(!(ptr == nullptr));
        REQUIRE(nullptr != ptr);
        REQUIRE(!(nullptr == ptr));
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner comparison valid ptr vs nullptr sealed", "[owner_comparison]") {
    memory_tracker mem_track;

    {
        test_sptr ptr = oup::make_observable_sealed<test_object>();
        REQUIRE(ptr != nullptr);
        REQUIRE(!(ptr == nullptr));
        REQUIRE(nullptr != ptr);
        REQUIRE(!(nullptr == ptr));
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner comparison valid ptr vs nullptr with deleter", "[owner_comparison]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr(new test_object, test_deleter{42});
        REQUIRE(ptr != nullptr);
        REQUIRE(!(ptr == nullptr));
        REQUIRE(nullptr != ptr);
        REQUIRE(!(nullptr == ptr));
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner comparison invalid ptr vs nullptr", "[owner_comparison]") {
    memory_tracker mem_track;

    {
        test_ptr ptr;
        REQUIRE(ptr == nullptr);
        REQUIRE(!(ptr != nullptr));
        REQUIRE(nullptr == ptr);
        REQUIRE(!(nullptr != ptr));
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner comparison invalid ptr vs nullptr sealed", "[owner_comparison]") {
    memory_tracker mem_track;

    {
        test_sptr ptr;
        REQUIRE(ptr == nullptr);
        REQUIRE(!(ptr != nullptr));
        REQUIRE(nullptr == ptr);
        REQUIRE(!(nullptr != ptr));
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner comparison invalid ptr vs nullptr with deleter", "[owner_comparison]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr;
        REQUIRE(ptr == nullptr);
        REQUIRE(!(ptr != nullptr));
        REQUIRE(nullptr == ptr);
        REQUIRE(!(nullptr != ptr));
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner comparison invalid ptr vs nullptr with deleter explicit", "[owner_comparison]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr(nullptr, test_deleter{42});
        REQUIRE(ptr == nullptr);
        REQUIRE(!(ptr != nullptr));
        REQUIRE(nullptr == ptr);
        REQUIRE(!(nullptr != ptr));
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner comparison invalid ptr vs invalid ptr", "[owner_comparison]") {
    memory_tracker mem_track;

    {
        test_ptr ptr1;
        test_ptr ptr2;
        REQUIRE(ptr1 == ptr2);
        REQUIRE(!(ptr1 != ptr2));
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner comparison invalid ptr vs invalid ptr sealed", "[owner_comparison]") {
    memory_tracker mem_track;

    {
        test_sptr ptr1;
        test_sptr ptr2;
        REQUIRE(ptr1 == ptr2);
        REQUIRE(!(ptr1 != ptr2));
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner comparison invalid ptr vs invalid ptr with deleter", "[owner_comparison]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr1;
        test_ptr_with_deleter ptr2;
        REQUIRE(ptr1 == ptr2);
        REQUIRE(!(ptr1 != ptr2));
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner comparison invalid ptr vs invalid ptr with deleter explicit", "[owner_comparison]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr1;
        test_ptr_with_deleter ptr2(nullptr, test_deleter{42});
        REQUIRE(ptr1 == ptr2);
        REQUIRE(ptr2 == ptr1);
        REQUIRE(!(ptr2 != ptr1));
        REQUIRE(!(ptr2 != ptr1));
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner comparison invalid ptr vs invalid ptr with both deleter explicit", "[owner_comparison]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr1(nullptr, test_deleter{43});
        test_ptr_with_deleter ptr2(nullptr, test_deleter{42});
        REQUIRE(ptr1 == ptr2);
        REQUIRE(ptr2 == ptr1);
        REQUIRE(!(ptr2 != ptr1));
        REQUIRE(!(ptr2 != ptr1));
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner comparison invalid ptr vs valid ptr", "[owner_comparison]") {
    memory_tracker mem_track;

    {
        test_ptr ptr1;
        test_ptr ptr2(new test_object);
        REQUIRE(ptr1 != ptr2);
        REQUIRE(!(ptr1 == ptr2));
        REQUIRE(ptr2 != ptr1);
        REQUIRE(!(ptr2 == ptr1));
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner comparison invalid ptr vs valid ptr sealed", "[owner_comparison]") {
    memory_tracker mem_track;

    {
        test_sptr ptr1;
        test_sptr ptr2 = oup::make_observable_sealed<test_object>();
        REQUIRE(ptr1 != ptr2);
        REQUIRE(!(ptr1 == ptr2));
        REQUIRE(ptr2 != ptr1);
        REQUIRE(!(ptr2 == ptr1));
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner comparison invalid ptr vs valid ptr with deleter", "[owner_comparison]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr1;
        test_ptr_with_deleter ptr2(new test_object, test_deleter{42});
        REQUIRE(ptr1 != ptr2);
        REQUIRE(!(ptr1 == ptr2));
        REQUIRE(ptr2 != ptr1);
        REQUIRE(!(ptr2 == ptr1));
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner comparison invalid ptr vs valid ptr with deleter explicit", "[owner_comparison]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr1(nullptr, test_deleter{43});
        test_ptr_with_deleter ptr2(new test_object, test_deleter{42});
        REQUIRE(ptr1 != ptr2);
        REQUIRE(!(ptr1 == ptr2));
        REQUIRE(ptr2 != ptr1);
        REQUIRE(!(ptr2 == ptr1));
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner comparison valid ptr vs valid ptr", "[owner_comparison]") {
    memory_tracker mem_track;

    {
        test_ptr ptr1(new test_object);
        test_ptr ptr2(new test_object);
        REQUIRE(ptr1 != ptr2);
        REQUIRE(!(ptr1 == ptr2));
        REQUIRE(ptr2 != ptr1);
        REQUIRE(!(ptr2 == ptr1));
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner comparison valid ptr vs valid ptr sealed", "[owner_comparison]") {
    memory_tracker mem_track;

    {
        test_sptr ptr1 = oup::make_observable_sealed<test_object>();
        test_sptr ptr2 = oup::make_observable_sealed<test_object>();
        REQUIRE(ptr1 != ptr2);
        REQUIRE(!(ptr1 == ptr2));
        REQUIRE(ptr2 != ptr1);
        REQUIRE(!(ptr2 == ptr1));
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner comparison valid ptr vs valid ptr with deleter", "[owner_comparison]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr1(new test_object, test_deleter{43});
        test_ptr_with_deleter ptr2(new test_object, test_deleter{42});
        REQUIRE(ptr1 != ptr2);
        REQUIRE(!(ptr1 == ptr2));
        REQUIRE(ptr2 != ptr1);
        REQUIRE(!(ptr2 == ptr1));
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner reset to null", "[owner_utility]") {
    memory_tracker mem_track;

    {
        test_ptr ptr(new test_object);
        ptr.reset();
        REQUIRE(instances == 0);
        REQUIRE(ptr.get() == nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner reset to null sealed", "[owner_utility]") {
    memory_tracker mem_track;

    {
        test_sptr ptr = oup::make_observable_sealed<test_object>();
        ptr.reset();
        REQUIRE(instances == 0);
        REQUIRE(ptr.get() == nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner reset to null with deleter", "[owner_utility]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr(new test_object, test_deleter{42});
        ptr.reset();
        REQUIRE(instances == 0);
        REQUIRE(instances_deleter == 1);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.get_deleter().state_ == 42);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_deleter == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner reset to new", "[owner_utility]") {
    memory_tracker mem_track;

    {
        test_ptr ptr(new test_object);
        ptr.reset(new test_object);
        REQUIRE(instances == 1);
        REQUIRE(ptr.get() != nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner reset to new with deleter", "[owner_utility]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr(new test_object, test_deleter{42});
        ptr.reset(new test_object);
        REQUIRE(instances == 1);
        REQUIRE(instances_deleter == 1);
        REQUIRE(ptr.get() != nullptr);
        REQUIRE(ptr.get_deleter().state_ == 42);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_deleter == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner swap no instance", "[owner_utility]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_orig;
        test_ptr ptr;
        ptr.swap(ptr_orig);
        REQUIRE(instances == 0);
        REQUIRE(ptr_orig.get() == nullptr);
        REQUIRE(ptr.get() == nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner swap no instance sealed", "[owner_utility]") {
    memory_tracker mem_track;

    {
        test_sptr ptr_orig;
        test_sptr ptr;
        ptr.swap(ptr_orig);
        REQUIRE(instances == 0);
        REQUIRE(ptr_orig.get() == nullptr);
        REQUIRE(ptr.get() == nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner swap no instance with deleter", "[owner_utility]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr_orig(nullptr, test_deleter{42});
        test_ptr_with_deleter ptr(nullptr, test_deleter{43});
        ptr.swap(ptr_orig);
        REQUIRE(instances == 0);
        REQUIRE(instances_deleter == 2);
        REQUIRE(ptr_orig.get() == nullptr);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.get_deleter().state_ == 42);
        REQUIRE(ptr_orig.get_deleter().state_ == 43);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_deleter == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner swap one instance", "[owner_utility]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_orig(new test_object);
        test_ptr ptr;
        ptr.swap(ptr_orig);
        REQUIRE(instances == 1);
        REQUIRE(ptr_orig.get() == nullptr);
        REQUIRE(ptr.get() != nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner swap one instance sealed", "[owner_utility]") {
    memory_tracker mem_track;

    {
        test_sptr ptr_orig = oup::make_observable_sealed<test_object>();
        test_sptr ptr;
        ptr.swap(ptr_orig);
        REQUIRE(instances == 1);
        REQUIRE(ptr_orig.get() == nullptr);
        REQUIRE(ptr.get() != nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner swap one instance with deleter", "[owner_utility]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr_orig(new test_object, test_deleter{42});
        test_ptr_with_deleter ptr(nullptr, test_deleter{43});
        ptr.swap(ptr_orig);
        REQUIRE(instances == 1);
        REQUIRE(instances_deleter == 2);
        REQUIRE(ptr_orig.get() == nullptr);
        REQUIRE(ptr.get() != nullptr);
        REQUIRE(ptr.get_deleter().state_ == 42);
        REQUIRE(ptr_orig.get_deleter().state_ == 43);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_deleter == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner swap two instances", "[owner_utility]") {
    memory_tracker mem_track;

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
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner swap two instances sealed", "[owner_utility]") {
    memory_tracker mem_track;

    {
        test_sptr ptr_orig = oup::make_observable_sealed<test_object>();
        test_object* ptr_orig_raw = ptr_orig.get();
        test_sptr ptr = oup::make_observable_sealed<test_object>();
        test_object* ptr_raw = ptr.get();
        ptr.swap(ptr_orig);
        REQUIRE(instances == 2);
        REQUIRE(ptr_orig.get() == ptr_raw);
        REQUIRE(ptr.get() == ptr_orig_raw);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner swap two instances with deleter", "[owner_utility]") {
    memory_tracker mem_track;

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
        REQUIRE(ptr.get_deleter().state_ == 42);
        REQUIRE(ptr_orig.get_deleter().state_ == 43);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_deleter == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner dereference", "[owner_utility]") {
    memory_tracker mem_track;

    {
        test_ptr ptr(new test_object);
        REQUIRE(ptr->state_ == 1337);
        REQUIRE((*ptr).state_ == 1337);
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner dereference sealed", "[owner_utility]") {
    memory_tracker mem_track;

    {
        test_sptr ptr = oup::make_observable_sealed<test_object>();
        REQUIRE(ptr->state_ == 1337);
        REQUIRE((*ptr).state_ == 1337);
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner operator bool valid", "[owner_utility]") {
    memory_tracker mem_track;

    {
        test_ptr ptr(new test_object);
        if (ptr) {} else FAIL("if (ptr) should have been true");
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner operator bool valid sealed", "[owner_utility]") {
    memory_tracker mem_track;

    {
        test_sptr ptr = oup::make_observable_sealed<test_object>();
        if (ptr) {} else FAIL("if (ptr) should have been true");
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner operator bool invalid", "[owner_utility]") {
    memory_tracker mem_track;

    {
        test_ptr ptr;
        if (ptr) FAIL("if (ptr) should not have been true");
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner operator bool invalid sealed", "[owner_utility]") {
    memory_tracker mem_track;

    {
        test_sptr ptr;
        if (ptr) FAIL("if (ptr) should not have been true");
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner release valid", "[owner_utility]") {
    memory_tracker mem_track;

    {
        test_ptr ptr(new test_object);
        test_object* ptr_raw = ptr.release();
        REQUIRE(ptr_raw != nullptr);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(instances == 1);
        delete ptr_raw;
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner release valid with observer", "[owner_utility]") {
    memory_tracker mem_track;

    {
        test_optr optr;
        {
            test_ptr ptr(new test_object);
            optr = ptr;
            test_object* ptr_raw = ptr.release();
            REQUIRE(ptr_raw != nullptr);
            REQUIRE(ptr.get() == nullptr);
            REQUIRE(instances == 1);
            delete ptr_raw;
        }

        REQUIRE(optr.expired());
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner release valid from make_observable_unique", "[owner_utility]") {
    memory_tracker mem_track;

    {
        test_ptr ptr = oup::make_observable_unique<test_object>();
        test_object* ptr_raw = ptr.release();
        REQUIRE(ptr_raw != nullptr);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(instances == 1);
        delete ptr_raw;
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner release valid from make_observable_unique with observer", "[owner_utility]") {
    memory_tracker mem_track;

    {
        test_optr optr;
        {
            test_ptr ptr = oup::make_observable_unique<test_object>();
            optr = ptr;
            test_object* ptr_raw = ptr.release();
            REQUIRE(ptr_raw != nullptr);
            REQUIRE(ptr.get() == nullptr);
            REQUIRE(instances == 1);
            delete ptr_raw;
        }

        REQUIRE(optr.expired());
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner release valid with deleter", "[owner_utility]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr(new test_object, test_deleter{42});
        test_object* ptr_raw = ptr.release();
        REQUIRE(ptr_raw != nullptr);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(instances == 1);
        REQUIRE(instances_deleter == 1);
        REQUIRE(ptr.get_deleter().state_ == 42);
        delete ptr_raw;
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_deleter == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner release valid with deleter with observer", "[owner_utility]") {
    memory_tracker mem_track;

    {
        test_optr optr;
        {
            test_ptr_with_deleter ptr(new test_object, test_deleter{42});
            optr = ptr;
            test_object* ptr_raw = ptr.release();
            REQUIRE(ptr_raw != nullptr);
            REQUIRE(ptr.get() == nullptr);
            REQUIRE(instances == 1);
            REQUIRE(instances_deleter == 1);
            REQUIRE(ptr.get_deleter().state_ == 42);
            delete ptr_raw;
        }

        REQUIRE(optr.expired());
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_deleter == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner release invalid", "[owner_utility]") {
    memory_tracker mem_track;

    {
        test_ptr ptr;
        REQUIRE(ptr.release() == nullptr);
        REQUIRE(instances == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("owner release invalid with deleter", "[owner_utility]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr;
        REQUIRE(ptr.release() == nullptr);
        REQUIRE(instances == 0);
        REQUIRE(instances_deleter == 1);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_deleter == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("make observable unique", "[make_observable_unique]") {
    memory_tracker mem_track;

    {
        test_ptr ptr = oup::make_observable_unique<test_object>();
        REQUIRE(instances == 1);
        REQUIRE(ptr.get() != nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("make observable unique throw in constructor", "[make_observable_unique]") {
    memory_tracker mem_track;

    REQUIRE_THROWS_AS(
        oup::make_observable_unique<test_object_thrower>(),
        throw_constructor);

    REQUIRE(instances_thrower == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("make observable sealed", "[make_observable_sealed]") {
    memory_tracker mem_track;

    {
        test_sptr ptr = oup::make_observable_sealed<test_object>();
        REQUIRE(instances == 1);
        REQUIRE(ptr.get() != nullptr);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("make observable sealed throw in constructor", "[make_observable_sealed]") {
    memory_tracker mem_track;

    REQUIRE_THROWS_AS(
        oup::make_observable_sealed<test_object_thrower>(),
        throw_constructor);

    REQUIRE(instances_thrower == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer size", "[observer_size]") {
    REQUIRE(sizeof(test_optr) == 2*sizeof(void*));
}

TEST_CASE("observer default constructor", "[observer_construction]") {
    memory_tracker mem_track;

    {
        test_optr ptr{};
        REQUIRE(instances == 0);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.expired() == true);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer nullptr constructor", "[observer_construction]") {
    memory_tracker mem_track;

    {
        test_optr ptr{nullptr};
        REQUIRE(instances == 0);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.expired() == true);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer copy constructor", "[observer_construction]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_owner{new test_object};
        test_optr ptr_orig{ptr_owner};
        {
            test_optr ptr(ptr_orig);
            REQUIRE(instances == 1);
            REQUIRE(ptr.get() != nullptr);
            REQUIRE(ptr.expired() == false);
            REQUIRE(ptr_orig.get() != nullptr);
            REQUIRE(ptr_orig.expired() == false);
        }

        REQUIRE(instances == 1);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer move constructor", "[observer_construction]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_owner{new test_object};
        test_optr ptr_orig{ptr_owner};
        {
            test_optr ptr(std::move(ptr_orig));
            REQUIRE(instances == 1);
            REQUIRE(ptr.get() != nullptr);
            REQUIRE(ptr.expired() == false);
            REQUIRE(ptr_orig.get() == nullptr);
            REQUIRE(ptr_orig.expired() == true);
        }

        REQUIRE(instances == 1);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer acquiring constructor", "[observer_construction]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_owner{new test_object};
        test_optr ptr{ptr_owner};
        REQUIRE(instances == 1);
        REQUIRE(ptr.get() != nullptr);
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer acquiring constructor sealed", "[observer_construction]") {
    memory_tracker mem_track;

    {
        test_sptr ptr_owner = oup::make_observable_sealed<test_object>();
        test_optr ptr{ptr_owner};
        REQUIRE(instances == 1);
        REQUIRE(ptr.get() != nullptr);
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer acquiring constructor derived", "[observer_construction]") {
    memory_tracker mem_track;

    {
        test_ptr_derived ptr_owner{new test_object_derived};
        test_optr ptr{ptr_owner};
        REQUIRE(instances == 1);
        REQUIRE(ptr.get() != nullptr);
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer acquiring constructor derived sealed", "[observer_construction]") {
    memory_tracker mem_track;

    {
        test_sptr_derived ptr_owner = oup::make_observable_sealed<test_object_derived>();
        test_optr ptr{ptr_owner};
        REQUIRE(instances == 1);
        REQUIRE(ptr.get() != nullptr);
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer acquiring constructor with deleter", "[observer_construction]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr_owner{new test_object, test_deleter{42}};
        test_optr ptr{ptr_owner};
        REQUIRE(instances == 1);
        REQUIRE(ptr.get() != nullptr);
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer implicit copy conversion constructor", "[observer_construction]") {
    memory_tracker mem_track;

    {
        test_ptr_derived ptr_owner{new test_object_derived};
        test_optr_derived ptr_orig{ptr_owner};
        {
            test_optr ptr(ptr_orig);
            REQUIRE(instances == 1);
            REQUIRE(instances_derived == 1);
            REQUIRE(ptr.get() != nullptr);
            REQUIRE(ptr.expired() == false);
        }

        REQUIRE(instances == 1);
        REQUIRE(instances_derived == 1);
        REQUIRE(ptr_orig.get() != nullptr);
        REQUIRE(ptr_orig.expired() == false);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_derived == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer implicit move conversion constructor", "[observer_construction]") {
    memory_tracker mem_track;

    {
        test_ptr_derived ptr_owner{new test_object_derived};
        test_optr_derived ptr_orig{ptr_owner};
        {
            test_optr ptr(std::move(ptr_orig));
            REQUIRE(instances == 1);
            REQUIRE(instances_derived == 1);
            REQUIRE(ptr.get() != nullptr);
            REQUIRE(ptr.expired() == false);
        }

        REQUIRE(instances == 1);
        REQUIRE(instances_derived == 1);
        REQUIRE(ptr_orig.get() == nullptr);
        REQUIRE(ptr_orig.expired() == true);
    }

    REQUIRE(instances == 0);
    REQUIRE(instances_derived == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer expiring", "[observer_utility]") {
    memory_tracker mem_track;

    {
        test_optr ptr;

        {
            test_ptr ptr_owner{new test_object};
            ptr = ptr_owner;
            REQUIRE(instances == 1);
            REQUIRE(ptr.get() != nullptr);
            REQUIRE(ptr.expired() == false);
        }

        REQUIRE(instances == 0);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.expired() == true);
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer expiring sealed", "[observer_utility]") {
    memory_tracker mem_track;

    {
        test_optr ptr;

        {
            test_sptr ptr_owner = oup::make_observable_sealed<test_object>();
            ptr = ptr_owner;
            REQUIRE(instances == 1);
            REQUIRE(ptr.get() != nullptr);
            REQUIRE(ptr.expired() == false);
        }

        REQUIRE(instances == 0);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.expired() == true);
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer expiring reset", "[observer_utility]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_owner{new test_object};
        test_optr ptr = ptr_owner;
        REQUIRE(!ptr.expired());
        ptr_owner.reset();
        REQUIRE(ptr.expired());
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer expiring reset sealed", "[observer_utility]") {
    memory_tracker mem_track;

    {
        test_sptr ptr_owner = oup::make_observable_sealed<test_object>();
        test_optr ptr = ptr_owner;
        REQUIRE(!ptr.expired());
        ptr_owner.reset();
        REQUIRE(ptr.expired());
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer not expiring when owner moved", "[observer_utility]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_owner{new test_object};
        test_optr ptr = ptr_owner;
        REQUIRE(!ptr.expired());
        test_ptr ptr_owner_new = std::move(ptr_owner);
        REQUIRE(!ptr.expired());
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer not expiring when owner moved sealed", "[observer_utility]") {
    memory_tracker mem_track;

    {
        test_sptr ptr_owner = oup::make_observable_sealed<test_object>();
        test_optr ptr = ptr_owner;
        REQUIRE(!ptr.expired());
        test_sptr ptr_owner_new = std::move(ptr_owner);
        REQUIRE(!ptr.expired());
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer reset to null", "[observer_utility]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_owner(new test_object);
        test_optr ptr(ptr_owner);
        ptr.reset();
        REQUIRE(instances == 1);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.expired() == true);
    }

    REQUIRE(instances == 0);
}

TEST_CASE("observer swap no instance", "[observer_utility]") {
    memory_tracker mem_track;

    {
        test_optr ptr_orig;
        test_optr ptr;
        ptr.swap(ptr_orig);
        REQUIRE(instances == 0);
        REQUIRE(ptr_orig.get() == nullptr);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr_orig.expired() == true);
        REQUIRE(ptr.expired() == true);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer swap one instance", "[observer_utility]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_owner(new test_object);
        test_optr ptr_orig(ptr_owner);
        test_optr ptr;
        ptr.swap(ptr_orig);
        REQUIRE(instances == 1);
        REQUIRE(ptr_orig.get() == nullptr);
        REQUIRE(ptr.get() != nullptr);
        REQUIRE(ptr_orig.expired() == true);
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer swap two same instance", "[observer_utility]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_owner(new test_object);
        test_optr ptr_orig(ptr_owner);
        test_optr ptr(ptr_owner);
        ptr.swap(ptr_orig);
        REQUIRE(instances == 1);
        REQUIRE(ptr_orig.get() == ptr_owner.get());
        REQUIRE(ptr.get() == ptr_owner.get());
        REQUIRE(ptr_orig.expired() == false);
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer swap two different instances", "[observer_utility]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_owner1(new test_object);
        test_ptr ptr_owner2(new test_object);
        test_optr ptr_orig(ptr_owner1);
        test_optr ptr(ptr_owner2);
        ptr.swap(ptr_orig);
        REQUIRE(instances == 2);
        REQUIRE(ptr_orig.get() == ptr_owner2.get());
        REQUIRE(ptr.get() == ptr_owner1.get());
        REQUIRE(ptr_orig.expired() == false);
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer dereference", "[observer_utility]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_owner(new test_object);
        test_optr ptr(ptr_owner);
        REQUIRE(ptr->state_ == 1337);
        REQUIRE((*ptr).state_ == 1337);
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer operator bool valid", "[observer_utility]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_owner(new test_object);
        test_optr ptr(ptr_owner);
        if (ptr) {} else FAIL("if (ptr) should have been true");
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer operator bool invalid", "[observer_utility]") {
    memory_tracker mem_track;

    {
        test_optr ptr;
        if (ptr) FAIL("if (ptr) should not have been true");
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer copy assignment operator from valid", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_owner{new test_object};
        test_optr ptr_orig{ptr_owner};
        {
            test_optr ptr;
            ptr = ptr_orig;
            REQUIRE(instances == 1);
            REQUIRE(ptr.get() != nullptr);
            REQUIRE(ptr.expired() == false);
            REQUIRE(ptr_orig.get() != nullptr);
            REQUIRE(ptr_orig.expired() == false);
        }

        REQUIRE(instances == 1);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer copy assignment operator from empty", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_optr ptr_orig;
        {
            test_optr ptr;
            ptr = ptr_orig;
            REQUIRE(instances == 0);
            REQUIRE(ptr.get() == nullptr);
            REQUIRE(ptr.expired() == true);
            REQUIRE(ptr_orig.get() == nullptr);
            REQUIRE(ptr_orig.expired() == true);
        }

        REQUIRE(instances == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer move assignment operator from valid", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_owner{new test_object};
        test_optr ptr_orig{ptr_owner};
        {
            test_optr ptr;
            ptr = std::move(ptr_orig);
            REQUIRE(instances == 1);
            REQUIRE(ptr.get() != nullptr);
            REQUIRE(ptr.expired() == false);
            REQUIRE(ptr_orig.get() == nullptr);
            REQUIRE(ptr_orig.expired() == true);
        }

        REQUIRE(instances == 1);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer move assignment operator from empty", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_optr ptr_orig;
        {
            test_optr ptr;
            ptr = std::move(ptr_orig);
            REQUIRE(instances == 0);
            REQUIRE(ptr.get() == nullptr);
            REQUIRE(ptr.expired() == true);
            REQUIRE(ptr_orig.get() == nullptr);
            REQUIRE(ptr_orig.expired() == true);
        }

        REQUIRE(instances == 0);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer acquiring assignment operator from valid", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_owner{new test_object};
        test_optr ptr;
        ptr = ptr_owner;

        REQUIRE(instances == 1);
        REQUIRE(ptr.get() != nullptr);
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer acquiring assignment operator from valid sealed", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_sptr ptr_owner = oup::make_observable_sealed<test_object>();
        test_optr ptr;
        ptr = ptr_owner;

        REQUIRE(instances == 1);
        REQUIRE(ptr.get() != nullptr);
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer acquiring assignment operator from valid with deleter", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr_owner{new test_object, test_deleter{42}};
        test_optr ptr;
        ptr = ptr_owner;

        REQUIRE(instances == 1);
        REQUIRE(ptr.get() != nullptr);
        REQUIRE(ptr.expired() == false);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer acquiring assignment operator from empty", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_owner;
        test_optr ptr;
        ptr = ptr_owner;

        REQUIRE(instances == 0);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.expired() == true);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer acquiring assignment operator from empty sealed", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_sptr ptr_owner;
        test_optr ptr;
        ptr = ptr_owner;

        REQUIRE(instances == 0);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.expired() == true);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer acquiring assignment operator from empty with deleter", "[observer_assignment]") {
    memory_tracker mem_track;

    {
        test_ptr_with_deleter ptr_owner{nullptr, test_deleter{42}};
        test_optr ptr;
        ptr = ptr_owner;

        REQUIRE(instances == 0);
        REQUIRE(ptr.get() == nullptr);
        REQUIRE(ptr.expired() == true);
    }

    REQUIRE(instances == 0);
    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer comparison valid ptr vs nullptr", "[observer_comparison]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_owner(new test_object);
        test_optr ptr(ptr_owner);
        REQUIRE(ptr != nullptr);
        REQUIRE(!(ptr == nullptr));
        REQUIRE(nullptr != ptr);
        REQUIRE(!(nullptr == ptr));
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer comparison invalid ptr vs nullptr", "[observer_comparison]") {
    memory_tracker mem_track;

    {
        test_optr ptr;
        REQUIRE(ptr == nullptr);
        REQUIRE(!(ptr != nullptr));
        REQUIRE(nullptr == ptr);
        REQUIRE(!(nullptr != ptr));
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer comparison invalid ptr vs invalid ptr", "[observer_comparison]") {
    memory_tracker mem_track;

    {
        test_optr ptr1;
        test_optr ptr2;
        REQUIRE(ptr1 == ptr2);
        REQUIRE(!(ptr1 != ptr2));
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer comparison invalid ptr vs valid ptr", "[observer_comparison]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_owner(new test_object);
        test_optr ptr1;
        test_optr ptr2(ptr_owner);
        REQUIRE(ptr1 != ptr2);
        REQUIRE(!(ptr1 == ptr2));
        REQUIRE(ptr2 != ptr1);
        REQUIRE(!(ptr2 == ptr1));
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer comparison valid ptr vs valid ptr same owner", "[observer_comparison]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_owner(new test_object);
        test_optr ptr1(ptr_owner);
        test_optr ptr2(ptr_owner);
        REQUIRE(ptr1 == ptr2);
        REQUIRE(!(ptr1 != ptr2));
        REQUIRE(ptr2 == ptr1);
        REQUIRE(!(ptr2 != ptr1));
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer comparison valid ptr vs valid ptr different owner", "[observer_comparison]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_owner1(new test_object);
        test_ptr ptr_owner2(new test_object);
        test_optr ptr1(ptr_owner1);
        test_optr ptr2(ptr_owner2);
        REQUIRE(ptr1 != ptr2);
        REQUIRE(!(ptr1 == ptr2));
        REQUIRE(ptr2 != ptr1);
        REQUIRE(!(ptr2 == ptr1));
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer comparison valid ptr vs valid ptr same owner derived", "[observer_comparison]") {
    memory_tracker mem_track;

    {
        test_ptr_derived ptr_owner(new test_object_derived);
        test_optr ptr1(ptr_owner);
        test_optr_derived ptr2(ptr_owner);
        REQUIRE(ptr1 == ptr2);
        REQUIRE(!(ptr1 != ptr2));
        REQUIRE(ptr2 == ptr1);
        REQUIRE(!(ptr2 != ptr1));
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("observer comparison valid ptr vs valid ptr different owner derived", "[observer_comparison]") {
    memory_tracker mem_track;

    {
        test_ptr ptr_owner1(new test_object);
        test_ptr_derived ptr_owner2(new test_object_derived);
        test_optr ptr1(ptr_owner1);
        test_optr_derived ptr2(ptr_owner2);
        REQUIRE(ptr1 != ptr2);
        REQUIRE(!(ptr1 == ptr2));
        REQUIRE(ptr2 != ptr1);
        REQUIRE(!(ptr2 == ptr1));
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

struct observer_owner {
    oup::observer_ptr<observer_owner> obs;
};

TEST_CASE("object owning observer pointer to itself", "[system_tests]") {
    memory_tracker mem_track;

    {
        auto ptr = oup::make_observable_sealed<observer_owner>();
        ptr->obs = ptr;
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("object owning observer pointer to other", "[system_tests]") {
    memory_tracker mem_track;

    {
        auto ptr1 = oup::make_observable_sealed<observer_owner>();
        auto ptr2 = oup::make_observable_sealed<observer_owner>();
        ptr1->obs = ptr2;
        ptr2->obs = ptr1;
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("object owning observer pointer chain", "[system_tests]") {
    memory_tracker mem_track;

    {
        auto ptr1 = oup::make_observable_sealed<observer_owner>();
        auto ptr2 = oup::make_observable_sealed<observer_owner>();
        auto ptr3 = oup::make_observable_sealed<observer_owner>();
        ptr1->obs = ptr2;
        ptr2->obs = ptr3;
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}

TEST_CASE("object owning observer pointer chain reversed", "[system_tests]") {
    memory_tracker mem_track;

    {
        auto ptr1 = oup::make_observable_sealed<observer_owner>();
        auto ptr2 = oup::make_observable_sealed<observer_owner>();
        auto ptr3 = oup::make_observable_sealed<observer_owner>();
        ptr3->obs = ptr2;
        ptr2->obs = ptr1;
    }

    REQUIRE(mem_track.leaks() == 0u);
    REQUIRE(mem_track.double_del() == 0u);
}
