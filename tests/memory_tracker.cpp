#include "memory_tracker.hpp"

#include <algorithm>
#include <cstdio>
#include <stdexcept>

volatile void*       allocations[max_allocations];
volatile void*       allocations_array[max_allocations];
volatile std::size_t allocations_bytes[max_allocations];
volatile std::size_t num_allocations               = 0u;
volatile std::size_t size_allocations              = 0u;
volatile std::size_t double_delete                 = 0u;
volatile bool        memory_tracking               = false;
volatile bool        force_next_allocation_failure = false;

constexpr bool debug_alloc    = false;
constexpr bool scramble_alloc = true;

void scramble(void* ptr, std::size_t size) {
    // Create a static random-like array, and use it to populate the allocated buffer.
    // clang-format off
    constexpr unsigned char random_bytes[] = {
        177, 250,   8, 188, 247,  14, 164, 181, 116, 101,  13, 153,   2,  90,  92,  36,
        240, 213,  38, 155, 223,  27, 100, 155, 182,  77, 106, 213, 219,  61,  36,  53,
        244, 206, 197,  28,  39,  55, 228, 147, 217,  13, 146,   1, 216, 252,  59, 109,
        143, 100, 101,  55, 204, 185,  40,  55, 197, 207, 187, 222,  13,  23, 177, 172,
        165,  67, 252, 163,  89,  51,  19,  15, 107,  92, 103, 129,  65,  89, 189,  21,
        206, 236, 130, 211, 148, 223, 104,  14,  54,  88,  54, 148, 227, 127, 234,   3,
        125,  86, 184, 161, 109,  32, 124, 150,  54, 194,  56, 128,   7, 144, 139,  30,
        226, 145,   4, 199,  11,  50, 241,  86,  72, 143, 215, 199,   0,   7, 124, 161
    };
    // clang-format on

    unsigned char* bytes_ptr = static_cast<unsigned char*>(ptr);
    while (size > sizeof(random_bytes)) {
        std::copy(random_bytes, random_bytes + sizeof(random_bytes), bytes_ptr);
        size -= sizeof(random_bytes);
    }

    std::copy(random_bytes, random_bytes + size, bytes_ptr);
}

void* allocate(std::size_t size, bool array, std::align_val_t align) {
    if (memory_tracking && num_allocations == max_allocations) {
        if constexpr (debug_alloc) {
            std::printf("alloc   %zu failed\n", size);
        }
        throw std::bad_alloc();
    }

    if (force_next_allocation_failure) {
        if constexpr (debug_alloc) {
            std::printf("alloc   %zu failed\n", size);
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
        if constexpr (debug_alloc) {
            std::printf("alloc   %zu failed\n", size);
        }
        throw std::bad_alloc();
    }

    if constexpr (debug_alloc) {
        std::printf("alloc   %zu -> %p\n", size, p);
    }

    if constexpr (scramble_alloc) {
        scramble(p, size);
    }

    if (memory_tracking) {
        if (array) {
            allocations_array[num_allocations] = p;
        } else {
            allocations[num_allocations] = p;
        }

        allocations_bytes[num_allocations] = size;

        num_allocations  = num_allocations + 1u;
        size_allocations = size_allocations + size;
    }

    return p;
}

void deallocate(void* p, bool array, std::align_val_t align [[maybe_unused]]) {
    if (p == nullptr) {
        return;
    }

    if constexpr (debug_alloc) {
        std::printf("dealloc %p\n", p);
    }

    if (memory_tracking) {
        bool            found            = false;
        volatile void** allocations_type = array ? allocations_array : allocations;
        for (std::size_t i = 0; i < num_allocations; ++i) {
            if (allocations_type[i] == p) {
                std::swap(allocations_type[i], allocations_type[num_allocations - 1]);
                std::swap(allocations_bytes[i], allocations_bytes[num_allocations - 1]);
                num_allocations  = num_allocations - 1u;
                size_allocations = size_allocations - allocations_bytes[num_allocations - 1];
                found            = true;
                break;
            }
        }

        if (!found) {
            double_delete = double_delete + 1u;
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

void* operator new[](std::size_t size) {
    return allocate(size, true, std::align_val_t{0});
}

void* operator new(std::size_t size, std::align_val_t al) {
    return allocate(size, false, al);
}

void* operator new[](std::size_t size, std::align_val_t al) {
    return allocate(size, true, al);
}

void operator delete(void* p) noexcept {
    deallocate(p, false, std::align_val_t{0});
}

void operator delete[](void* p) noexcept {
    deallocate(p, true, std::align_val_t{0});
}

void operator delete(void* p, std::size_t) noexcept {
    deallocate(p, false, std::align_val_t{0});
}

void operator delete[](void* p, std::size_t) noexcept {
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

std::size_t memory_tracker::allocated() const volatile {
    return ::num_allocations - initial_allocations;
}

std::size_t memory_tracker::double_delete() const volatile {
    return ::double_delete - initial_double_delete;
}

fail_next_allocation::fail_next_allocation() noexcept {
    force_next_allocation_failure = true;
}

fail_next_allocation::~fail_next_allocation() noexcept {
    force_next_allocation_failure = false;
}
