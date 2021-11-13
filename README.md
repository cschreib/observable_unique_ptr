# observable_unique_ptr, observable_sealed_ptr, observer_ptr

![Build Status](https://github.com/cschreib/observable_unique_ptr/actions/workflows/cmake.yml/badge.svg) ![Docs Build Status](https://github.com/cschreib/observable_unique_ptr/actions/workflows/doc.yml/badge.svg) [![codecov](https://codecov.io/gh/cschreib/observable_unique_ptr/branch/main/graph/badge.svg?token=8C11D2U94D)](https://codecov.io/gh/cschreib/observable_unique_ptr)

Built and tested on:
 - Linux (GCC/clang)
 - Windows (MSVC 32/64)
 - MacOS (clang)
 - WebAssembly (Emscripten)

**Table of content:**
<!-- MarkdownTOC autolink="true" -->

- [Introduction](#introduction)
- [Usage](#usage)
- [Limitations](#limitations)
- [Comparison spreadsheet](#comparison-spreadsheet)
- [Speed benchmarks](#speed-benchmarks)
- [Alternative implementation](#alternative-implementation)

<!-- /MarkdownTOC -->


## Introduction

This is a small header-only library, providing the unique-ownership smart pointers `observable_unique_ptr` and `observable_sealed_ptr` that can be observed with non-owning pointers `observer_ptr`. This is a mixture of `std::unique_ptr` and `std::shared_ptr`: it borrows the unique-ownership semantic of `std::unique_ptr` (movable, non-copiable), but allows creating `observer_ptr` to monitor the lifetime of the pointed object (like `std::weak_ptr` for `std::shared_ptr`).

The only difference between `observable_unique_ptr` and `observable_sealed_ptr` is that the former can release ownership, while the latter cannot. Disallowing release of ownership enables allocation optimizations. Therefore, the recommendation is to use `observable_sealed_ptr` unless release of ownership is required.

These pointers are useful for cases where the shared-ownership of `std::shared_ptr` is not desirable, e.g., when lifetime must be carefully controlled and not be allowed to extend, yet non-owning/weak/observer references to the object may exist after the object has been deleted.

Note: Because of the unique ownership model, observer pointers cannot extend the lifetime of the pointed object, hence this library provides less safety compared to `std::shared_ptr`/`std::weak_ptr`. This is also true of `std::unique_ptr`, and is a fundamental limitation of unique ownership. If this is an issue, simply use `std::shared_ptr`/`std::weak_ptr`.


## Usage

This is a header-only library requiring a C++17-compliant compiler. You have multiple ways to set it up:
 - just include this repository as a submodule in your own git repository and use CMake `add_subdirectory` (or use CMake `FetchContent`), then link with `target_link_libraries(<your-target> PUBLIC oup::oup)`.
 - download the header and include it in your own sources.

From there, include the single header `<oup/observable_unique_ptr.hpp>`, and directly use the smart pointer in your own code:

```c++
#include <oup/observable_unique_ptr.hpp>

#include <string>
#include <iostream>
#include <cassert>

int main() {
    // Non-owning pointer that will outlive the object
    oup::observer_ptr<std::string> obs_ptr;

    {
        // Sealed (unique) pointer that owns the object
        auto owner_ptr = oup::make_observable_sealed<std::string>("hello");

        // A sealed pointer cannot be copied but it can be moved
        // auto tmp_copied = owner_ptr; // error!
        // auto tmp_moved = std::move(owner_ptr); // OK

        // Make the observer pointer point to the object
        obs_ptr = owner_ptr;

        // The observer pointer is now valid
        assert(!obs_ptr.expired());

        // It can be used like a regular raw pointer
        assert(obs_ptr != nullptr);
        std::cout << *obs_ptr << std::endl;

        // An observer pointer can be copied and moved
        // auto tmp_copied = obs_ptr; // OK
        // auto tmp_moved = std::move(obs_ptr); // OK
    }

    // The sealed pointer has gone out of scope, the object is deleted,
    // the observer pointer is now null.
    assert(obs_ptr.expired());
    assert(obs_ptr == nullptr);

    return 0;
}
```

As with `std::shared_ptr`/`std::weak_ptr`, if you need to obtain an observer pointer to an object when you only have `this` (i.e., from a member function), you can inherit from `oup::enable_observer_from_this<T>` to gain access to the `observer_from_this()` member function. This function will return a valid observer pointer as long as the object is owned by a unique or sealed pointer, and will return `nullptr` in all other cases. Contrary to `std::enable_shared_from_this<T>`, this feature naturally supports multiple inheritance.


## Limitations

The following limitations are features that were not implemented simply because of lack of motivation.

 - this library does not support pointers to arrays, but `std::unique_ptr` and `std::shared_ptr` both do.
 - this library does not support custom allocators, but `std::shared_ptr` does.


## Comparison spreadsheet

In this comparison spreadsheet, the raw pointer `T*` is assumed to never be owning, and used only to observe an existing object (which may or may not have been deleted). The stack and heap sizes were measured with gcc 9.3.0 and libstdc++.

Labels:
 - raw: `T*`
 - unique: `std::unique_ptr<T>`
 - weak: `std::weak_ptr<T>`
 - shared: `std::shared_ptr<T>`
 - observer: `oup::observer_ptr<T>`
 - obs_unique: `oup::observable_unique_ptr<T>`
 - obs_sealed: `oup::observable_sealed_ptr<T>`

| Pointer                  | raw  | weak   | observer | unique | shared | obs_unique | obs_sealed |
|--------------------------|------|--------|----------|--------|--------|------------|------------|
| Owning                   | no   | no     | no       | yes    | yes    | yes        | yes        |
| Releasable               | N/A  | N/A    | N/A      | yes    | no     | yes        | no         |
| Observable deletion      | no   | yes    | yes      | yes    | yes    | yes        | yes        |
| Thread-safe deletion     | no   | yes    | no(1)    | yes(2) | yes    | yes(2)     | yes(2)     |
| Atomic                   | yes  | no(3)  | no       | no     | no(3)  | no         | no         |
| Support arrays           | yes  | yes    | no       | yes    | yes    | no         | no         |
| Support custom allocator | N/A  | yes    | no       | yes    | yes    | no         | no         |
| Support custom deleter   | N/A  | N/A    | N/A      | yes    | yes(4) | yes        | no         |
| Number of heap alloc.    | 0    | 0      | 0        | 1      | 1/2(5) | 2          | 1          |
| Size in bytes (64 bit)   |      |        |          |        |        |            |            |
|  - Stack (per instance)  | 8    | 16     | 16       | 8      | 16     | 16         | 16         |
|  - Heap (shared)         | 0    | 0      | 0        | 0      | 24     | 8          | 8          |
|  - Total                 | 8    | 16     | 16       | 8      | 40     | 24         | 24         |
| Size in bytes (32 bit)   |      |        |          |        |        |            |            |
|  - Stack (per instance)  | 4    | 8      | 8        | 4      | 8      | 8          | 8          |
|  - Heap (shared)         | 0    | 0      | 0        | 0      | 16     | 8          | 8          |
|  - Total                 | 4    | 8      | 8        | 4      | 24     | 16         | 16         |

Notes:

 - (1) If `expired()` returns true, the pointer is guaranteed to remain `nullptr` forever, with no race condition. If `expired()` returns false, the pointer could still expire on the next instant, which can lead to race conditions.
 - (2) By construction, only one thread can own the pointer, therefore deletion is thread-safe.
 - (3) Yes if using `std::atomic<std::shared_ptr<T>>` and `std::atomic<std::weak_ptr<T>>`.
 - (4) Not if using `std::make_shared()`.
 - (5) 2 by default, or 1 if using `std::make_shared()`.


## Speed benchmarks

Labels are the same as in the comparison spreadsheet. The speed benchmarks were compiled with all optimizations turned on (except LTO). Speed is measured relative to `std::unique_ptr<T>` used as owner pointer, and `T*` used as observer pointer, which should be the fastest possible implementation (but obviously the one with least safety).

You can run the benchmarks yourself, they are located in `tests/speed_benchmark.cpp`. The benchmark executable runs tests for three object types: `int`, `float`, `std::string`, and `std::array<int,65'536>`, to simulate objects of various allocation cost. The timings below are the worst-case values measured across all object types, which should be most relevant to highlight the overhead from the pointer itself (and erases flukes from the benchmarking framework). In real life scenarios, the actual measured overhead will be substantially lower, as actual business logic is likely to dominate the time budget.

Detail of the benchmarks:
 - Create owner empty: default-construct an owner pointer (to nullptr).
 - Create owner: construct an owner pointer by taking ownership of an existing object.
 - Create owner factory: construct an owner pointer using `std::make_*` or `oup::make_*` factory functions.
 - Dereference owner: get a reference to the underlying owned object from an owner pointer.
 - Create observer empty: default-construct an observer pointer (to nullptr).
 - Create observer: construct an observer pointer from an owner pointer.
 - Create observer copy: construct a new observer pointer from another observer pointer.
 - Dereference observer: get a reference to the underlying object from an observer pointer.

*Compiler: gcc 9.3.0, std: libstdc++, OS: linux 5.1.0, CPU: Ryzen 5 2600:*

| Pointer                  | raw/unique | weak/shared | observer/obs_unique | observer/obs_sealed |
|--------------------------|------------|-------------|---------------------|---------------------|
| Create owner empty       |     1      |     1.1     |         1.1         |         1.1         |
| Create owner             |     1      |     2.2     |         1.9         |         N/A         |
| Create owner factory     |     1      |     1.3     |         1.8         |         1.3         |
| Dereference owner        |     1      |     1       |         1           |         1           |
| Create observer empty    |     1      |     1.2     |         1.2         |         1.3         |
| Create observer          |     1      |     1.5     |         1.6         |         1.6         |
| Create observer copy     |     1      |     1.7     |         1.7         |         1.7         |
| Dereference observer     |     1      |     4.8     |         1.2         |         1.3         |

*Compiler: MSVC 16.11.3, std: MS-STL, OS: Windows 10.0.19043, CPU: i7-7800x:*

| Pointer                  | raw/unique | weak/shared | observer/obs_unique | observer/obs_sealed |
|--------------------------|------------|-------------|---------------------|---------------------|
| Create owner empty       |     1      |     1.1     |         1.1         |         1.1         |
| Create owner             |     1      |     2.2     |         2.0         |         N/A         |
| Create owner factory     |     1      |     1.3     |         2.0         |         1.4         |
| Dereference owner        |     1      |     0.8     |         1.8         |         1.5         |
| Create observer empty    |     1      |     1.1     |         1.2         |         1.2         |
| Create observer          |     1      |     5.6     |         1.5         |         1.3         |
| Create observer copy     |     1      |     6.2     |         1.4         |         1.3         |
| Dereference observer     |     1      |     11      |         1.5         |         1.1         |

*Compiler: Emscripten 2.0.16, std: libc++, OS: Node.js 14.15.5 + linux kernel 5.1.0, CPU: Ryzen 5 2600:*

| Pointer                  | raw/unique | weak/shared | observer/obs_unique | observer/obs_sealed |
|--------------------------|------------|-------------|---------------------|---------------------|
| Create owner empty       |     1      |     20      |         1.2         |         1           |
| Create owner             |     1      |     1.6     |         1.6         |         N/A         |
| Create owner factory     |     1      |     1.1     |         1.6         |         1           |
| Dereference owner        |     1      |     1       |         1           |         1           |
| Create observer empty    |     1      |     35      |         1.8         |         1.7         |
| Create observer          |     1      |     36      |         2.4         |         2.5         |
| Create observer copy     |     1      |     41      |         2.3         |         2.3         |
| Dereference observer     |     1      |     114     |         1           |         1           |


## Alternative implementation

An alternative implementation of an "observable unique pointer" can be found [here](https://www.codeproject.com/articles/1011134/smart-observers-to-use-with-unique-ptr). It does not compile out of the box with gcc unfortunately and lacks certain features (their `make_observable` always performs two allocations). Have a look to check if this better suits your needs.
