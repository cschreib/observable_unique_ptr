#ifndef OBSERVABLE_UNIQUE_PTR_INCLUDED
#define OBSERVABLE_UNIQUE_PTR_INCLUDED

#include <memory>
#include <cstddef>

namespace oup {

template<typename T>
class observer_ptr;

/// Unique-ownership smart pointer, can be observed by observer_ptr
/** This smart pointer mimics the interface of std::unique_ptr, in that
*   it is movable but not copiable. The smart pointer holds exclusive
*   (unique) ownership of the pointed object.
*
*   The main difference with std::unique_ptr is that it allows creating
*   observer_ptr instances to observe the lifetime of the pointed object,
*   as one would do with std::shared_ptr and std::weak_ptr. The price to pay,
*   compared to a standard std::unique_ptr, is the additional heap allocation
*   of the reference-counting control block, which make_observable_unique()
*   will optimise as a single heap allocation with the pointed object (as
*   std::make_shared() does for std::shared_ptr).
*
*   Other notable points (either limitations imposed by the current
*   implementation, or features not implemented simply because of lack of
*   motivation):
*    - because of the unique ownership, observer_ptr cannot extend
*      the lifetime of the pointed object, hence observable_unique_ptr provides
*      less thread-safety compared to std::shared_ptr.
*    - observable_unique_ptr does not support arrays.
*    - observable_unique_ptr does not allow custom allocators.
*    - observable_unique_ptr does not have a release() function to let go of
*      the ownership.
*    - observable_unique_ptr allows moving from other observable_unique_ptr
*      only if the deleter type is exactly the same, while std::unique_ptr
*      allows moving from a convertible deleter.
*    - observable_unique_ptr may not own a deleter instance; if in doubt, check
*      has_deleter() before calling get_deleter(), or use try_get_deleter().
*    - a moved-from observable_unique_ptr will not own a deleter instance.
*/
template<typename T, typename Deleter = std::default_delete<T>>
class observable_unique_ptr {
private:
    // Friendship is required for conversions.
    template<typename U, typename D>
    friend class observable_unique_ptr;

    struct control_block {
        std::size_t refcount = 0u;
        bool placement_allocated = false;
    };

    control_block* block = nullptr;
    T* pointer = nullptr;
    Deleter deleter;

    control_block* allocate_block_(T* value) {
        return new control_block{std::size_t{1u}};
    }

    void pop_ref_() noexcept {
        deleter(pointer);

        --block->refcount;
        if (block->refcount == 0u) {
            if (block->placement_allocated) {
                block->~control_block();
                delete[] static_cast<std::byte*>(block);
            } else {
                delete block;
            }
        }
    }

    /// Private constructor using pre-allocated control block.
    /** \param ctrl The control block pointer
    *   \param value The pointer to own
    *   \note This is used by make_observable_unique().
    */
    observable_unique_ptr(control_block* ctrl, T* value) : block(ctrl), pointer(value) {}

    /// Private constructor using pre-allocated control block.
    /** \param ctrl The control block pointer
    *   \param value The pointer to own
    *   \note This is used by make_observable_unique().
    */
    observable_unique_ptr(control_block* ctrl, T* value, Deleter del) :
        block(ctrl), pointer(value), deleter(del) {}

    // Friendship is required for access to std::shared_ptr base
    // template<typename U>
    // friend class observer_ptr;

public:
    /// Type of the pointed object
    using element_type = T;

    /// Type of the matching observer pointer
    using observer_type = observer_ptr<T>;

    /// Pointer type
    using pointer = element_type*;

    /// Deleter type
    using deleter_type = Deleter;

    /// Default constructor (null pointer).
    observable_unique_ptr() noexcept = default;

    /// Construct a null pointer.
    observable_unique_ptr(std::nullptr_t) noexcept {}

    /// Construct a null pointer with custom deleter.
    observable_unique_ptr(std::nullptr_t, Deleter deleter) noexcept :
        deleter(std::move(deleter)) {}

    /// Explicit ownership capture of a raw pointer.
    /** \param value The raw pointer to take ownership of
    *   \note Do *not* manually delete this raw pointer after the
    *         observable_unique_ptr is created. If possible, prefer
    *         using make_observable_unique() instead of this constructor.
    */
    explicit observable_unique_ptr(T* value) :
        observable_unique_ptr(allocate_block_(value), value) {}

    /// Explicit ownership capture of a raw pointer, with customer deleter.
    /** \param value The raw pointer to take ownership of
    *   \param deleter The deleter object to use
    *   \note Do *not* manually delete this raw pointer after the
    *         observable_unique_ptr is created. If possible, prefer
    *         using make_observable_unique() instead of this constructor.
    */
    explicit observable_unique_ptr(T* value, Deleter del) :
        observable_unique_ptr(allocate_block_(value), value, std::move(del)) {}

    /// Transfer ownership by implicit casting
    /** \param value The pointer to take ownership from
    *   \note After this observable_unique_ptr is created, the source
    *         pointer is set to null and looses ownership.
    */
    template<typename U/*, typename enable = std::enable_if_t<std::is_convertible_v<U, T>>*/>
    observable_unique_ptr(observable_unique_ptr<U,Deleter>&& value) noexcept :
        observable_unique_ptr(value.block, value.pointer, std::move(value.deleter)) {
        value.block = nullptr;
        value.pointer = nullptr;
    }

    /// Transfer ownership by explicit casting
    /** \param manager The smart pointer to take ownership from
    *   \param value The casted pointer value to take ownership of
    *   \note After this observable_unique_ptr is created, the source
    *         pointer is set to null and looses ownership.
    */
    template<typename U>
    observable_unique_ptr(observable_unique_ptr<U,Deleter>&& manager, T* value) noexcept :
        observable_unique_ptr(manager.block, value, std::move(manager.deleter)) {
        manager.block = nullptr;
        manager.pointer = nullptr;
    }

    /// Transfer ownership by implicit casting
    /** \param value The pointer to take ownership from
    *   \note After this observable_unique_ptr is created, the source
    *         pointer is set to null and looses ownership.
    */
    template<typename U/*, typename enable = std::enable_if_t<std::is_convertible_v<U, T>>*/>
    observable_unique_ptr& operator=(observable_unique_ptr<U,Deleter>&& value) noexcept {
        if (pointer) {
            pop_ref_();
        }

        block = value.block;
        value.block = nullptr;
        pointer = value.pointer;
        value.pointer = nullptr;
        deleter = std::move(value.deleter);

        return *this;
    }

    // Non-copyable
    observable_unique_ptr(const observable_unique_ptr&) = delete;
    observable_unique_ptr& operator=(const observable_unique_ptr&) = delete;

    /// Returns the deleter object which would be used for destruction of the managed object.
    /** \return The deleter
    *   \note Using the return value of this function if has_deleter() returns 'false' will cause
    *         undefined behavior.
    */
    Deleter& get_deleter() noexcept {
        return deleter;
    }

    /// Returns the deleter object which would be used for destruction of the managed object.
    /** \return The deleter
    *   \note Using the return value of this function if has_deleter() returns 'false' will cause
    *         undefined behavior.
    */
    const Deleter& get_deleter() const noexcept {
        return deleter;
    }

    /// Swap the content of this pointer with that of another pointer.
    /** \param other The other pointer to swap with
    */
    void swap(observable_unique_ptr& other) noexcept {
        using std::swap;
        swap(block, other.block);
        swap(pointer, other.pointer);
        swap(deleter, other.deleter);
    }

    /// Replaces the managed object with a null pointer.
    /** \param ptr A nullptr_t instance
    */
    void reset(std::nullptr_t ptr = nullptr) noexcept {
        if (pointer) {
            pop_ref_();
            block = nullptr;
            pointer = nullptr;
        }
    }

    /// Replaces the managed object.
    /** \param ptr A nullptr_t instance
    */
    void reset(T* ptr) noexcept {
        // TODO: copy old ptr, assign new, then delete old
        // (to follow std::unique_ptr specs)
        if (pointer) {
            pop_ref_();
        }

        block = ptr != nullptr ? allocate_block_() : nullptr;
        pointer = ptr;
    }

    /// Replaces the managed object (with custom deleter).
    /** \param ptr A pointer to the new object to own
    *   \param deleter The new custom deleter instance to use
    *   \note After this call, any previously owner object will be deleted
    */
    void reset(T* ptr, Deleter deleter) noexcept {
        operator=(observable_unique_ptr{ptr, std::move(deleter)});
    }

    /// Get a non-owning raw pointer to the pointed object, or nullptr if deleted.
    /** \return 'nullptr' if expired() is 'true', or the pointed object otherwise
    *   \note Contrary to std::weak_ptr::lock(), this does not extend the lifetime
    *         of the pointed object. Therefore, when calling this function, you must
    *         make sure that the owning observable_unique_ptr will not be reset until
    *         you are done using the raw pointer.
    */
    T* get() const noexcept {
        return pointer;
    }

    /// Get a reference to the pointed object (undefined behavior if deleted).
    /** \return A reference to the pointed object
    *   \note Using this function if expired() is 'true' will leave to undefined behavior.
    */
    T& operator*() const noexcept {
        return *pointer;
    }

    /// Get a non-owning raw pointer to the pointed object, or nullptr if deleted.
    /** \return 'nullptr' if expired() is 'true', or the pointed object otherwise
    *   \note Contrary to std::weak_ptr::lock(), this does not extend the lifetime
    *         of the pointed object. Therefore, when calling this function, you must
    *         make sure that the owning observable_unique_ptr will not be reset until
    *         you are done using the raw pointer.
    */
    T* operator->() const noexcept {
        return pointer;
    }

    /// Check if this pointer points to a valid object.
    /** \return 'true' if the pointed object is valid, 'false' otherwise
    */
    explicit operator bool() noexcept {
        return pointer != nullptr;
    }

    template<typename U, typename ... Args>
    friend observable_unique_ptr<U> make_observable_unique(Args&& ... args);
};

/// Create a new observable_unique_ptr with a newly constructed object.
/** \param args Arguments to construt the new object
*   \return The new observable_unique_ptr
*/
template<typename T, typename ... Args>
observable_unique_ptr<T> make_observable_unique(Args&& ... args) {
    using block_type = typename observable_unique_ptr<T>::control_block;

    // Allocate memory
    constexpr std::size_t block_size = sizeof(block_type);
    constexpr std::size_t object_size = sizeof(T);
    std::byte* buffer = new std::byte[block_size + object_size];

    try {
        // Construct control block and object
        block_type* block = new (buffer) control_block{1u};
        T* ptr = new (buffer + block_size) T(std::forward<Args>(args)...);

        // Make owner pointer
        return observable_unique_ptr<T>(block, ptr, deleter);
    } catch (...) {
        // Exception thrown durimg object construction,
        // clean up memory and let exception propagate
        delete[] buffer;
        throw;
    }
}

template<typename T, typename Deleter>
bool operator== (const observable_unique_ptr<T,Deleter>& value, std::nullptr_t) noexcept {
    return value.get() == nullptr;
}

template<typename T, typename Deleter>
bool operator== (std::nullptr_t, const observable_unique_ptr<T,Deleter>& value) noexcept {
    return value.get() == nullptr;
}

template<typename T, typename Deleter>
bool operator!= (const observable_unique_ptr<T,Deleter>& value, std::nullptr_t) noexcept {
    return value.get() != nullptr;
}

template<typename T, typename Deleter>
bool operator!= (std::nullptr_t, const observable_unique_ptr<T,Deleter>& value) noexcept {
    return value.get() != nullptr;
}

template<typename T, typename U, typename Deleter>
bool operator== (const observable_unique_ptr<T,Deleter>& first,
    const observable_unique_ptr<U,Deleter>& second) noexcept {
    return first.get() == second.get();
}

template<typename T, typename U, typename Deleter>
bool operator!= (const observable_unique_ptr<T,Deleter>& first,
    const observable_unique_ptr<U,Deleter>& second) noexcept {
    return first.get() != second.get();
}

/// Non-owning smart pointer that observes an observable_unique_ptr.
/** \see observable_unique_ptr
*/
template<typename T>
class observer_ptr : private std::weak_ptr<T> {
private:
    // Friendship is required for conversions.
    template<typename U>
    friend class observer_ptr;

public:
    /// Type of the pointed object
    using typename std::weak_ptr<T>::element_type;

    /// Set this pointer to null.
    using std::weak_ptr<T>::reset;

    /// Check if this pointer points to a valid object.
    /** \return 'true' if the pointed object is valid, 'false' otherwise
    */
    using std::weak_ptr<T>::expired;

    /// Default constructor (null pointer).
    observer_ptr() = default;

    /// Default constructor (null pointer).
    observer_ptr(std::nullptr_t) {}

    /// Create a weak pointer from an owning pointer.
    template<typename U>
    observer_ptr(const observable_unique_ptr<U>& owner) noexcept : std::weak_ptr<T>(owner) {}

    /// Copy an existing observer_ptr instance
    /** \param value The existing weak pointer to copy
    */
    template<typename U>
    observer_ptr(const observer_ptr<U>& value) noexcept :
        std::weak_ptr<T>(static_cast<const std::weak_ptr<U>&>(value)) {}

    /// Move from an existing observer_ptr instance
    /** \param value The existing weak pointer to move from
    *   \note After this observable_unique_ptr is created, the source
    *         pointer is set to null.
    */
    template<typename U>
    observer_ptr(observer_ptr<U>&& value) noexcept :
        std::weak_ptr<T>(std::move(static_cast<std::weak_ptr<U>&>(value))) {}

    /// Point to another owning pointer.
    observer_ptr& operator=(const observable_unique_ptr<T>& owner) noexcept {
        std::weak_ptr<T>::operator=(owner);
        return *this;
    }

    /// Copy an existing observer_ptr instance
    /** \param value The existing weak pointer to copy
    */
    template<typename U>
    observer_ptr& operator=(const observer_ptr<U>& value) noexcept {
        std::weak_ptr<T>::operator=(static_cast<std::weak_ptr<U>&>(value));
        return *this;
    }

    /// Move from an existing observer_ptr instance
    /** \param value The existing weak pointer to move from
    *   \note After the assignment is complete, the source
    *         pointer is set to null and looses ownership.
    */
    template<typename U>
    observer_ptr& operator=(observer_ptr<U>&& value) noexcept {
        std::weak_ptr<T>::operator=(std::move(static_cast<std::weak_ptr<U>&>(value)));
        return *this;
    }

    /// Get a non-owning raw pointer to the pointed object, or nullptr if deleted.
    /** \return 'nullptr' if expired() is 'true', or the pointed object otherwise
    *   \note Contrary to std::weak_ptr::lock(), this does not extend the lifetime
    *         of the pointed object. Therefore, when calling this function, you must
    *         make sure that the owning observable_unique_ptr will not be reset until
    *         you are done using the raw pointer.
    */
    T* get() const noexcept {
        return std::weak_ptr<T>::lock().get();
    }

    /// Get a reference to the pointed object (undefined behavior if deleted).
    /** \return A reference to the pointed object
    *   \note Using this function if expired() is 'true' will leave to undefined behavior.
    */
    T& operator*() const noexcept {
        return *std::weak_ptr<T>::lock().get();
    }

    /// Get a non-owning raw pointer to the pointed object, or nullptr if deleted.
    /** \return 'nullptr' if expired() is 'true', or the pointed object otherwise
    *   \note Contrary to std::weak_ptr::lock(), this does not extend the lifetime
    *         of the pointed object. Therefore, when calling this function, you must
    *         make sure that the owning observable_unique_ptr will not be reset until
    *         you are done using the raw pointer.
    */
    T* operator->() const noexcept {
        return std::weak_ptr<T>::lock().get();
    }

    /// Check if this pointer points to a valid object.
    /** \return 'true' if the pointed object is valid, 'false' otherwise
    */
    explicit operator bool() noexcept {
        return !expired();
    }

    /// Swap the content of this pointer with that of another pointer.
    /** \param other The other pointer to swap with
    */
    void swap(observer_ptr& other) noexcept {
        std::weak_ptr<T>::swap(other);
    }

    // Copiable
    observer_ptr(const observer_ptr&) noexcept = default;
    observer_ptr& operator=(const observer_ptr&) noexcept = default;
    // Movable
    observer_ptr(observer_ptr&&) noexcept = default;
    observer_ptr& operator=(observer_ptr&&) noexcept = default;
};


template<typename T>
bool operator== (const observer_ptr<T>& value, std::nullptr_t) noexcept {
    return value.expired();
}

template<typename T>
bool operator== (std::nullptr_t, const observer_ptr<T>& value) noexcept {
    return value.expired();
}

template<typename T>
bool operator!= (const observer_ptr<T>& value, std::nullptr_t) noexcept {
    return !value.expired();
}

template<typename T>
bool operator!= (std::nullptr_t, const observer_ptr<T>& value) noexcept {
    return !value.expired();
}

template<typename T, typename U>
bool operator== (const observer_ptr<T>& first, const observer_ptr<U>& second) noexcept {
    return first.get() == second.get();
}

template<typename T, typename U>
bool operator!= (const observer_ptr<T>& first, const observer_ptr<U>& second) noexcept {
    return first.get() != second.get();
}

}

#endif
