#include "memory_tracker.hpp"

#include <cstdio>

void*       allocations[max_allocations];
void*       allocations_array[max_allocations];
std::size_t allocations_bytes[max_allocations];
std::size_t num_allocations               = 0u;
std::size_t size_allocations              = 0u;
std::size_t double_delete                 = 0u;
bool        memory_tracking               = false;
bool        force_next_allocation_failure = false;

constexpr bool debug_alloc = false;

void* allocate(std::size_t size, bool array, std::align_val_t align) {
    if (memory_tracking && num_allocations == max_allocations) {
        if (debug_alloc) {
            printf("alloc   %ld failed\n", size);
        }
        throw std::bad_alloc();
    }

    if (force_next_allocation_failure) {
        if (debug_alloc) {
            printf("alloc   %ld failed\n", size);
        }
        force_next_allocation_failure = false;
        throw std::bad_alloc();
    }

    void* p = nullptr;
    if (align == std::align_val_t{0}) {
        p = std::malloc(size);
    } else {
#if defined(OUP_COMPILER_MSVC)
        p = _aligned_malloc(size, static_cast<std::size_t>(align));
#elif defined(OUP_COMPILER_EMSCRIPTEN)
        p = aligned_alloc(static_cast<std::size_t>(align), size);
#else
        p = std::aligned_alloc(static_cast<std::size_t>(align), size);
#endif
    }

    if (!p) {
        if (debug_alloc) {
            printf("alloc   %ld failed\n", size);
        }
        throw std::bad_alloc();
    }

    if (debug_alloc) {
        printf("alloc   %ld -> %p\n", size, p);
    }

    if (memory_tracking) {
        if (array) {
            allocations_array[num_allocations] = p;
        } else {
            allocations[num_allocations] = p;
        }

        allocations_bytes[num_allocations] = size;

        ++num_allocations;
        size_allocations += size;
    }

    return p;
}

void deallocate(void* p, bool array, std::align_val_t align [[maybe_unused]]) {
    if (p == nullptr) {
        return;
    }

    if (debug_alloc) {
        printf("dealloc %p\n", p);
    }

    if (memory_tracking) {
        bool   found            = false;
        void** allocations_type = array ? allocations_array : allocations;
        for (std::size_t i = 0; i < num_allocations; ++i) {
            if (allocations_type[i] == p) {
                std::swap(allocations_type[i], allocations_type[num_allocations - 1]);
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

    if (align == std::align_val_t{0}) {
        std::free(p);
    } else {
#if defined(OUP_COMPILER_MSVC)
        _aligned_free(p);
#else
        std::free(p);
#endif
    }
}

void* operator new(std::size_t size) {
    return allocate(size, false, std::align_val_t{0});
}

void* operator new[](size_t size) {
    return allocate(size, true, std::align_val_t{0});
}

void* operator new(std::size_t size, std::align_val_t al) {
    return allocate(size, false, al);
}

void* operator new[](size_t size, std::align_val_t al) {
    return allocate(size, true, al);
}

void operator delete(void* p) noexcept {
    deallocate(p, false, std::align_val_t{0});
}

void operator delete[](void* p) noexcept {
    deallocate(p, true, std::align_val_t{0});
}

void operator delete(void* p, std::align_val_t al) noexcept {
    deallocate(p, false, al);
}

void operator delete[](void* p, std::align_val_t al) noexcept {
    deallocate(p, true, al);
}

memory_tracker::memory_tracker() noexcept :
    initial_allocations(::num_allocations), initial_double_delete(::double_delete) {
    ::memory_tracking = true;
}

memory_tracker::~memory_tracker() noexcept {
    ::memory_tracking = false;
}

std::size_t memory_tracker::allocated() const {
    return ::num_allocations - initial_allocations;
}

std::size_t memory_tracker::double_delete() const {
    return ::double_delete - initial_double_delete;
}
