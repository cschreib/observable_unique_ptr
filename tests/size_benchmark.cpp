#include <iostream>
#include <memory>
#include <oup/observable_unique_ptr.hpp>

// Allocation tracker, to catch memory leaks and double delete
constexpr std::size_t max_allocations = 20'000;
void*                 allocations[max_allocations];
std::size_t           allocations_bytes[max_allocations];
std::size_t           num_allocations  = 0u;
std::size_t           size_allocations = 0u;
std::size_t           double_delete    = 0u;
bool                  memory_tracking  = false;

// NB: getting weird errors on MacOS when doing this
#if !defined(__APPLE__)
void* operator new(size_t size) {
    if (memory_tracking && num_allocations == max_allocations) {
        throw std::bad_alloc();
    }

    void* p = std::malloc(size);
    if (!p) {
        throw std::bad_alloc();
    }

    if (memory_tracking) {
        allocations[num_allocations]       = p;
        allocations_bytes[num_allocations] = size;
        ++num_allocations;
        size_allocations += size;
    }

    return p;
}

void operator delete(void* p) noexcept {
    if (memory_tracking) {
        bool found = false;
        for (std::size_t i = 0; i < num_allocations; ++i) {
            if (allocations[i] == p) {
                std::swap(allocations[i], allocations[num_allocations - 1]);
                std::swap(allocations_bytes[i], allocations_bytes[num_allocations - 1]);
                --num_allocations;
                size_allocations -= allocations_bytes[num_allocations - 1];
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

    std::size_t leaks() const {
        return num_allocations - initial_allocations;
    }
    std::size_t double_del() const {
        return double_delete - initial_double_delete;
    }
};

int main() {
    memory_tracking        = true;
    std::size_t init_alloc = 0u;

    std::size_t unique_size = 0u;
    init_alloc              = size_allocations;
    {
        std::unique_ptr<int> ptr(new int);
        unique_size = size_allocations - sizeof(int) - init_alloc;
        std::cout << "unique_ptr size: " << sizeof(ptr) << ", " << unique_size << std::endl;
    }

    init_alloc = size_allocations;
    {
        std::unique_ptr<int> ptr(new int);
        int*                 wptr = ptr.get();
        std::cout << "raw pointer size: " << sizeof(wptr) << ", "
                  << size_allocations - sizeof(int) - init_alloc - unique_size << std::endl;
    }

    std::size_t shared_size = 0u;
    init_alloc              = size_allocations;
    {
        std::shared_ptr<int> ptr(new int);
        shared_size = size_allocations - sizeof(int) - init_alloc;
        std::cout << "shared_ptr size: " << sizeof(ptr) << ", " << shared_size << std::endl;
    }

    init_alloc = size_allocations;
    {
        std::shared_ptr<int> ptr(new int);
        std::weak_ptr<int>   wptr(ptr);
        std::cout << "weak_ptr size: " << sizeof(wptr) << ", "
                  << size_allocations - sizeof(int) - init_alloc - shared_size << std::endl;
    }

    std::size_t observable_size = 0u;
    init_alloc                  = size_allocations;
    {
        oup::observable_unique_ptr<int> ptr(new int);
        observable_size = size_allocations - sizeof(int) - init_alloc;
        std::cout << "observable_unique_ptr size: " << sizeof(ptr) << ", " << observable_size
                  << std::endl;
    }

    init_alloc = size_allocations;
    {
        oup::observable_unique_ptr<int> ptr(new int);
        oup::observer_ptr<int>          wptr(ptr);
        std::cout << "observer_ptr size: " << sizeof(wptr) << ", "
                  << size_allocations - sizeof(int) - init_alloc - observable_size << std::endl;
    }
}
