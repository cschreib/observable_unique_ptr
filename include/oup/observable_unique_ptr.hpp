#ifndef OBSERVABLE_UNIQUE_PTR_INCLUDED
#define OBSERVABLE_UNIQUE_PTR_INCLUDED

#include <memory>
#include <cstddef>
#include <utility>

namespace oup {

template<typename T>
class observer_ptr;

namespace details {
struct control_block {
    std::size_t refcount = 1u;
    bool placement_allocated = false;
    bool expired = false;
};
}

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
    using control_block = details::control_block;

    control_block* block = nullptr;
    T* pointer = nullptr;

    #if __cplusplus == 202002L
    [[no_unique_address]] Deleter deleter;
    #else
    Deleter deleter;
    #endif

    control_block* allocate_block_() {
        return new control_block;
    }

    static void pop_ref_(control_block* block) noexcept {
        --block->refcount;
        if (block->refcount == 0u) {
            if constexpr (std::is_same_v<Deleter, std::default_delete<T>>) {
                if (block->placement_allocated) {
                    block->~control_block();
                    delete[] reinterpret_cast<std::byte*>(block);
                } else {
                    delete block;
                }
            } else {
                delete block;
            }
        }
    }

    static void delete_and_pop_ref_(control_block* block, T* pointer, Deleter& deleter) noexcept {
        if constexpr (std::is_same_v<Deleter, std::default_delete<T>>) {
            if (block->placement_allocated) {
                pointer->~T();
            } else {
                deleter(pointer);
            }
        } else {
            deleter(pointer);
        }

        block->expired = true;

        pop_ref_(block);
    }

    void pop_ref_() noexcept {
        pop_ref_(block);
    }

    void delete_and_pop_ref_() noexcept {
        delete_and_pop_ref_(block, pointer, deleter);
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

    // Friendship is required for conversions.
    template<typename U>
    friend class observer_ptr;

    // Friendship is required for conversions.
    template<typename U, typename D>
    friend class observable_unique_ptr;

public:
    /// Type of the pointed object
    using element_type = T;

    /// Type of the matching observer pointer
    using observer_type = observer_ptr<T>;

    /// Pointer type
    using pointr = element_type*;

    /// Deleter type
    using deleter_type = Deleter;

    /// Default constructor (null pointer).
    observable_unique_ptr() noexcept = default;

    /// Construct a null pointer.
    observable_unique_ptr(std::nullptr_t) noexcept {}

    /// Construct a null pointer with custom deleter.
    observable_unique_ptr(std::nullptr_t, Deleter deleter) noexcept :
        deleter(std::move(deleter)) {}

    /// Destructor, releases owned object if any
    ~observable_unique_ptr() noexcept {
        if (pointer) {
            delete_and_pop_ref_();
            block = nullptr;
            pointer = nullptr;
        }
    }

    /// Explicit ownership capture of a raw pointer.
    /** \param value The raw pointer to take ownership of
    *   \note Do *not* manually delete this raw pointer after the
    *         observable_unique_ptr is created. If possible, prefer
    *         using make_observable_unique() instead of this constructor.
    */
    explicit observable_unique_ptr(T* value) :
        observable_unique_ptr(allocate_block_(), value) {}

    /// Explicit ownership capture of a raw pointer, with customer deleter.
    /** \param value The raw pointer to take ownership of
    *   \param deleter The deleter object to use
    *   \note Do *not* manually delete this raw pointer after the
    *         observable_unique_ptr is created. If possible, prefer
    *         using make_observable_unique() instead of this constructor.
    */
    explicit observable_unique_ptr(T* value, Deleter del) :
        observable_unique_ptr(allocate_block_(), value, std::move(del)) {}

    /// Transfer ownership by implicit casting
    /** \param value The pointer to take ownership from
    *   \note After this observable_unique_ptr is created, the source
    *         pointer is set to null and looses ownership.
    */
    observable_unique_ptr(observable_unique_ptr&& value) noexcept :
        observable_unique_ptr(value.block, value.pointer, std::move(value.deleter)) {
        value.block = nullptr;
        value.pointer = nullptr;
    }

    /// Transfer ownership by implicit casting
    /** \param value The pointer to take ownership from
    *   \note After this observable_unique_ptr is created, the source
    *         pointer is set to null and looses ownership.
    */
    template<typename U, typename D/*, typename enable = std::enable_if_t<std::is_convertible_v<U*, T*>>*/>
    observable_unique_ptr(observable_unique_ptr<U,D>&& value) noexcept :
        observable_unique_ptr(value.block, value.pointer) {
        value.block = nullptr;
        value.pointer = nullptr;
    }

    /// Transfer ownership by explicit casting
    /** \param manager The smart pointer to take ownership from
    *   \param value The casted pointer value to take ownership of
    *   \note After this observable_unique_ptr is created, the source
    *         pointer is set to null and looses ownership.
    */
    template<typename U, typename D/*, typename enable = std::enable_if_t<std::is_convertible_v<U*, T*>>*/>
    observable_unique_ptr(observable_unique_ptr<U,D>&& manager, T* value) noexcept :
        observable_unique_ptr(manager.block, value) {
        manager.block = nullptr;
        manager.pointer = nullptr;
    }

    /// Transfer ownership by implicit casting
    /** \param value The pointer to take ownership from
    *   \note After this observable_unique_ptr is created, the source
    *         pointer is set to null and looses ownership.
    */
    template<typename U, typename D/*, typename enable = std::enable_if_t<std::is_convertible_v<U*, T*>>*/>
    observable_unique_ptr& operator=(observable_unique_ptr<U,D>&& value) noexcept {
        if (pointer) {
            delete_and_pop_ref_();
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
            delete_and_pop_ref_();
            block = nullptr;
            pointer = nullptr;
        }
    }

    /// Replaces the managed object.
    /** \param ptr A nullptr_t instance
    */
    void reset(T* ptr) noexcept {
        // Copy old pointer
        T* old_ptr = pointer;
        control_block* old_block = block;

        // Assign the new one
        block = ptr != nullptr ? allocate_block_() : nullptr;
        pointer = ptr;

        // Delete the old pointer
        // (this follows std::unique_ptr specs)
        if (old_ptr) {
            delete_and_pop_ref_(old_block, old_ptr, deleter);
        }
    }

    /// Releases ownership of the managed object.
    /** \return A pointer to the un-managed object
    */
    T* release() noexcept {
        if (pointer) {
            pop_ref_();
        }

        return pointer;
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
        block_type* block = new (buffer) details::control_block{1u, true, false};
        T* ptr = new (buffer + block_size) T(std::forward<Args>(args)...);

        // Make owner pointer
        return observable_unique_ptr<T>(block, ptr);
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
class observer_ptr {
private:
    // Friendship is required for conversions.
    template<typename U>
    friend class observer_ptr;

    using control_block = details::control_block;

    control_block* block = nullptr;
    T* pointer = nullptr;

    void pop_ref_() noexcept {
        --block->refcount;
        if (block->refcount == 0u) {
            if (block->placement_allocated) {
                block->~control_block();
                delete[] reinterpret_cast<std::byte*>(block);
            } else {
                delete block;
            }
        }
    }

public:
    /// Type of the pointed object
    using element_type = T;

    /// Default constructor (null pointer).
    observer_ptr() = default;

    /// Default constructor (null pointer).
    observer_ptr(std::nullptr_t) noexcept {}

    /// Destructor
    ~observer_ptr() noexcept {
        if (pointer) {
            pop_ref_();
            block = nullptr;
            pointer = nullptr;
        }
    }

    /// Create a weak pointer from an owning pointer.
    template<typename U>
    observer_ptr(const observable_unique_ptr<U>& owner) noexcept :
        block(owner.block), pointer(owner.pointer) {
        if (block) {
            ++block->refcount;
        }
    }

    /// Copy an existing observer_ptr instance
    /** \param value The existing weak pointer to copy
    */
    template<typename U>
    observer_ptr(const observer_ptr<U>& value) noexcept :
        block(value.block), pointer(value.pointer) {
        if (block) {
            ++block->refcount;
        }
    }

    /// Move from an existing observer_ptr instance
    /** \param value The existing weak pointer to move from
    *   \note After this observable_unique_ptr is created, the source
    *         pointer is set to null.
    */
    template<typename U>
    observer_ptr(observer_ptr<U>&& value) noexcept : block(value.block), pointer(value.pointer) {
        value.block = nullptr;
        value.pointer = nullptr;
    }

    /// Point to another owning pointer.
    template<typename U>
    observer_ptr& operator=(const observable_unique_ptr<U>& owner) noexcept {
        if (pointer) {
            pop_ref_();
        }

        block = owner.block;
        pointer = owner.pointer;
        if (block) {
            ++block->refcount;
        }

        return *this;
    }

    /// Copy an existing observer_ptr instance
    /** \param value The existing weak pointer to copy
    */
    template<typename U>
    observer_ptr& operator=(const observer_ptr<U>& value) noexcept {
        if (pointer) {
            pop_ref_();
        }

        block = value.block;
        pointer = value.pointer;
        if (block) {
            ++block->refcount;
        }

        return *this;
    }

    /// Move from an existing observer_ptr instance
    /** \param value The existing weak pointer to move from
    *   \note After the assignment is complete, the source
    *         pointer is set to null and looses ownership.
    */
    template<typename U>
    observer_ptr& operator=(observer_ptr<U>&& value) noexcept {
        if (pointer) {
            pop_ref_();
        }

        block = value.block;
        value.block = nullptr;
        pointer = value.pointer;
        value.pointer = nullptr;

        return *this;
    }

    /// Set this pointer to null.
    void reset() noexcept {
        if (pointer) {
            pop_ref_();
            block = nullptr;
            pointer = nullptr;
        }
    }

    /// Get a non-owning raw pointer to the pointed object, or nullptr if deleted.
    /** \return 'nullptr' if expired() is 'true', or the pointed object otherwise
    *   \note This does not extend the lifetime of the pointed object. Therefore, when
    *         calling this function, you must make sure that the owning observable_unique_ptr
    *         will not be reset until you are done using the raw pointer.
    */
    T* get() const noexcept {
        return expired() ? nullptr : pointer;
    }

    /// Get a non-owning raw pointer to the pointed object, possibly dangling.
    /** \return The pointed object, which may be a dangling pointer if the object has been deleted
    *   \note This does not extend the lifetime of the pointed object. Therefore, when
    *         calling this function, you must make sure that the owning observable_unique_ptr
    *         will not be reset until you are done using the raw pointer. In addition,
    *         this function will not check if the pointer has expired (i.e., if the object
    *         has been deleted), so the returned pointer may be dangling.
    *         Only use this function if you know the object cannot have been deleted.
    */
    T* raw_get() const noexcept {
        return pointer;
    }

    /// Get a reference to the pointed object (undefined behavior if deleted).
    /** \return A reference to the pointed object
    *   \note Using this function if expired() is 'true' will leave to undefined behavior.
    */
    T& operator*() const noexcept {
        return *get();
    }

    /// Get a non-owning raw pointer to the pointed object, or nullptr if deleted.
    /** \return 'nullptr' if expired() is 'true', or the pointed object otherwise
    *   \note Contrary to std::weak_ptr::lock(), this does not extend the lifetime
    *         of the pointed object. Therefore, when calling this function, you must
    *         make sure that the owning observable_unique_ptr will not be reset until
    *         you are done using the raw pointer.
    */
    T* operator->() const noexcept {
        return get();
    }

    /// Check if this pointer points to a valid object.
    /** \return 'true' if the pointed object is valid, 'false' otherwise
    */
    bool expired() const noexcept {
        return block == nullptr || block->expired;
    }

    /// Check if this pointer points to a valid object.
    /** \return 'true' if the pointed object is valid, 'false' otherwise
    */
    explicit operator bool() noexcept {
        return block != nullptr && !block->expired;
    }

    /// Swap the content of this pointer with that of another pointer.
    /** \param other The other pointer to swap with
    */
    void swap(observer_ptr& other) noexcept {
        using std::swap;
        swap(block, other.block);
        swap(pointer, other.pointer);
    }
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
