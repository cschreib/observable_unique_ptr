#include <cstdlib>
#include <new>

// Allocation tracker, to catch memory leaks and double delete
constexpr std::size_t       max_allocations = 20'000;
extern volatile void*       allocations[max_allocations];
extern volatile void*       allocations_array[max_allocations];
extern volatile std::size_t allocations_bytes[max_allocations];
extern volatile std::size_t num_allocations;
extern volatile std::size_t size_allocations;
extern volatile std::size_t double_delete;
extern volatile bool        memory_tracking;
extern volatile bool        force_next_allocation_failure;

void* operator new(std::size_t size);

void* operator new[](std::size_t size);

void* operator new(std::size_t size, std::align_val_t al);

void* operator new[](std::size_t size, std::align_val_t al);

void operator delete(void* p) noexcept;

void operator delete[](void* p) noexcept;

void operator delete(void* p, std::size_t size) noexcept;

void operator delete[](void* p, std::size_t size) noexcept;

void operator delete(void* p, std::align_val_t al) noexcept;

void operator delete[](void* p, std::align_val_t al) noexcept;

struct memory_tracker {
    std::size_t initial_allocations;
    std::size_t initial_double_delete;

    memory_tracker() noexcept;
    ~memory_tracker() noexcept;

    std::size_t allocated() const volatile;
    std::size_t double_delete() const volatile;
};

struct fail_next_allocation {
    fail_next_allocation() noexcept;
    ~fail_next_allocation() noexcept;
};
