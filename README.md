# observable_unique_ptr<T, Deleter>

![Build Status](https://github.com/cschreib/observable_unique_ptr/actions/workflows/cmake.yml/badge.svg) ![Build Status](https://github.com/cschreib/observable_unique_ptr/actions/workflows/doc.yml/badge.svg)

## Introduction

This is a small header-only library, providing a unique-ownership smart pointer `observable_unique_ptr` that can be observed with non-owning pointers `observer_ptr`. It is a mixture of `std::unique_ptr` and `std::shared_ptr`: it borrows the unique-ownership semantic of `std::unique_ptr` (movable, non-copiable), but allows creating `observer_ptr` to monitor the lifetime of the pointed object (like `std::weak_ptr` for `std::shared_ptr`).

This is useful for cases where the shared-ownership of `std::shared_ptr` is not desirable, e.g., when lifetime must be carefully controlled and not be allowed to extend, yet non-owning/weak/observer references to the object may exist after the object has been deleted.

Note: Because of the unique ownership model, observer pointers cannot extend the lifetime of the pointed object, hence `observable_unique_ptr`/`observer_ptr` provides less thread-safety compared to `std::shared_ptr`/`std::weak_ptr`. This is also true of `std::unique_ptr`, and is a fundamental limitation of unique ownership. If this is an issue, you will need either to add your own explicit locking logic, or use `std::shared_ptr`/`std::weak_ptr`.


## Usage

This is a header-only library requiring a C++17-compliant compiler. You have multiple ways to set it up:
 - just include this repository as a submodule in your own git repository and use CMake `add_subdirectory`,
 - use CMake `FetchContent`,
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
        // Unique pointer that owns the object
        auto owner_ptr = oup::make_observable_unique<std::string>("hello");

        // Make the observer pointer point to the object
        obs_ptr = owner_ptr;

        // Observer pointer is valid
        assert(!obs_ptr.expired());

        // It can be used like a regular raw pointer
        assert(obs_ptr != nullptr);
        std::cout << *obs_ptr << std::endl;

        // The unique pointer cannot be copied
        auto tmp_copied = owner_ptr; // error!

        // ... but it can be moved
        auto tmp_moved = std::move(owner_ptr); // OK
    }

    // The unique pointer has gone out of scope, the object is deleted,
    // the observer pointer is now null.
    assert(obs_ptr.expired());
    assert(obs_ptr == nullptr);

    return 0;
}
```


## Limitations

The follownig limitations are features that were not implemented simply because of lack of motivation.

 - `observable_unique_ptr` does not support pointers to arrays, but `std::unique_ptr` and `std::shared_ptr` both do.
 - `observable_unique_ptr` does not support custom allocators, but `std::shared_ptr` does.


## Notes

An alternative implementation of an "observable unique pointer" can be found [here](https://www.codeproject.com/articles/1011134/smart-observers-to-use-with-unique-ptr). It does not compile out of the box with gcc unfortunately, but it does contain more features (like creating an observer pointer from a raw `this`). Have a look to check if this better suits your needs.
