#include "memory_tracker.hpp"

#include <iostream>
#include <memory>
#include <oup/observable_unique_ptr.hpp>

int main() {
    memory_tracking        = true;
    std::size_t init_alloc = 0u;

    using test_type = int;

    std::size_t unique_size = 0u;
    init_alloc              = size_allocations;
    {
        std::unique_ptr<test_type> ptr(new test_type);
        unique_size = size_allocations - sizeof(test_type) - init_alloc;
        std::cout << "unique_ptr size: " << sizeof(ptr) << ", " << unique_size << std::endl;
    }

    init_alloc = size_allocations;
    {
        std::unique_ptr<test_type> ptr(new test_type);
        test_type*                 wptr = ptr.get();
        std::cout << "raw pointer size: " << sizeof(wptr) << ", "
                  << size_allocations - sizeof(test_type) - init_alloc - unique_size << std::endl;
    }

    std::size_t shared_size = 0u;
    init_alloc              = size_allocations;
    {
        std::shared_ptr<test_type> ptr(new test_type);
        shared_size = size_allocations - sizeof(test_type) - init_alloc;
        std::cout << "shared_ptr size: " << sizeof(ptr) << ", " << shared_size << std::endl;
    }

    init_alloc = size_allocations;
    {
        std::shared_ptr<test_type> ptr(new test_type);
        std::weak_ptr<test_type>   wptr(ptr);
        std::cout << "weak_ptr size: " << sizeof(wptr) << ", "
                  << size_allocations - sizeof(test_type) - init_alloc - shared_size << std::endl;
    }

    init_alloc = size_allocations;
    {
        std::shared_ptr<test_type> ptr = std::make_shared<test_type>();
        shared_size                    = size_allocations - sizeof(test_type) - init_alloc;
        std::cout << "shared_ptr size (make_shared): " << sizeof(ptr) << ", " << shared_size
                  << std::endl;
    }

    init_alloc = size_allocations;
    {
        std::shared_ptr<test_type> ptr = std::make_shared<test_type>();
        std::weak_ptr<test_type>   wptr(ptr);
        std::cout << "weak_ptr size (make_shared): " << sizeof(wptr) << ", "
                  << size_allocations - sizeof(test_type) - init_alloc - shared_size << std::endl;
    }

    std::size_t observable_size = 0u;
    init_alloc                  = size_allocations;
    {
        oup::observable_unique_ptr<test_type> ptr(new test_type);
        observable_size = size_allocations - sizeof(test_type) - init_alloc;
        std::cout << "observable_unique_ptr size: " << sizeof(ptr) << ", " << observable_size
                  << std::endl;
    }

    init_alloc = size_allocations;
    {
        oup::observable_unique_ptr<test_type> ptr(new test_type);
        oup::observer_ptr<test_type>          wptr(ptr);
        std::cout << "observer_ptr size (unique): " << sizeof(wptr) << ", "
                  << size_allocations - sizeof(test_type) - init_alloc - observable_size
                  << std::endl;
    }

    init_alloc = size_allocations;
    {
        oup::observable_sealed_ptr<test_type> ptr = oup::make_observable_sealed<test_type>();
        observable_size = size_allocations - sizeof(test_type) - init_alloc;
        std::cout << "observable_sealed_ptr size: " << sizeof(ptr) << ", " << observable_size
                  << std::endl;
    }

    init_alloc = size_allocations;
    {
        oup::observable_sealed_ptr<test_type> ptr = oup::make_observable_sealed<test_type>();
        oup::observer_ptr<test_type>          wptr(ptr);
        std::cout << "observer_ptr size (sealed): " << sizeof(wptr) << ", "
                  << size_allocations - sizeof(test_type) - init_alloc - observable_size
                  << std::endl;
    }
}
