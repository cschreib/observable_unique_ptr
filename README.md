# observable_unique_ptr<T, Deleter>

## Introduction

This is a small header-only library, providing a unique-ownership smart pointer that can be observed with weak pointers. It is a mixture of `std::unique_ptr` and `std::shared_ptr`: it borrows the unique-ownership semantic of `std::unique_ptr` (movable, non-copiable), but allows creating `std::weak_ptr` instances to monitor the lifetime of the pointed object.

This is useful for cases where the shared-ownership of `std::shared_ptr` is not desirable, e.g., when lifetime must be carefully controlled and not be allowed to extend, yet non-owning "weak" references to the object may exist after the object has been deleted.


## Usage

This is a header-only library. You have multiple ways to set it up:
 - just include this repository as a submodule in your own git repository and use CMake `add_subdirectory`,
 - use CMake `FetchContent`,
 - download the header and include it in your own sources.

From there, include the single header `<oup/observable_unique_ptr.hpp>`, and directly use the smart pointer in your own code:

```c++
#include <oup/observable_unique_ptr.hpp>

int main() {
    // Weak pointer that will outlive the object
    oup::weak_ptr<std::string> wptr;

    {
        // Unique pointer that owns the object
        auto ptr = oup::make_observable_unique<std::string>("hello");

        // Make the weak pointer observe the object
        wptr = ptr;

        // Weak pointer is valid
        assert(!wptr.expired());

        // It can be locked to get a (non-owning!) pointer to the object
        std::string* s = wptr.lock();
        assert(s != nullptr);
        std::cout << *s << std::endl;

        // The unique pointer cannot be copied
        oup::observable_unique_ptr<std::string> tmp_copied = ptr; // error!

        // ... but it can be moved
        oup::observable_unique_ptr<std::string> tmp_moved = std::move(ptr); // OK
    }

    // The unique pointer has gone out of scope, the object is deleted,
    // the weak pointer is now null.
    assert(wptr.expired());
    assert(wptr.lock() == nullptr);

    return 0;
}
```

## Limitation

 - Because of the unique ownership, weak pointers locking cannot extend the lifetime of the pointed object, hence `observable_unique_ptr` provides less thread-safety compared to `std::shared_ptr`.
 - `observable_unique_ptr` does not support pointers to arrays, but `std::unique_ptr` and `std::shared_ptr` both do.
 - `observable_unique_ptr` does not support custom allocators, but `std::shared_ptr` does.
 - `observable_unique_ptr` does not have a `release()` function to let go of the ownership, which `std::unique_ptr` has.
 - `observable_unique_ptr` allows moving from other `observable_unique_ptr` only if the deleter type is exactly the same, while `std::unique_ptr` allows moving from a convertible deleter.
 - `observable_unique_ptr` may or may not own a deleter instance; if in doubt, check `has_deleter()` before calling `get_deleter()`, or use `try_get_deleter()`.
 - a moved-from `observable_unique_ptr` will not own a deleter instance.
